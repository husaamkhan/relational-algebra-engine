# Grammar

*The following document outlines the grammar rules that any queries to the DB must make in EBNF form.*

---

## Operator Precedence and Associativity

Below is a table listing operator precedence from lowest to highest.

| Level | Operator(s)                  | Type    | Associativity  |
|-------|------------------------------|---------|----------------|
| 1     | `union`, `intersect`, `minus`| binary  | left           |
| 2     | `times`, `join[c]`           | binary  | left           |
| 3     | `select[c]`, `project[a]`, `rename[n]` | unary | —  |
| 4     | relation name, `( expr )`    | atom    | —              |

**Note:**
- Higher level indicates higher precedence. Operators with higher precedence will bind more tightly to their operands than other operators
- Left-associativity means `A <operator1> B <operator> C` is `(A <operator> B) <operator> C`
- The unary operators (`select`, `project`, `rename`) bind more tightly than any binary operator, so `select[c](R) union S` is
  `(select[c](R)) union S`, never `select[c](R union S)`
- Parentheses override precedence

---

## An ambiguity demonstration

**Naive Grammar**
```
Expr ::= Expr "union" Expr
       | Expr "minus" Expr
       | "(" Expr ")"
       | IDENT
```
Below are 2 parse trees for `A union B Minus C` given the above naive grammar

TODO: ensure that this is the correct form for these parse trees

**Tree 1** — `union` applied first: `A union B Minus C` or `(A union B) minus C`

```
        minus
       /     \
    union     C
   /     \
  A       B
```

**Tree 2** — `minus` applied first: `A union (B minus C)`

```
      union
     /     \
    A      minus
           /   \
          B     C
```

### Concrete Instance Showing Different Results

Let the three relations be (each with a single integer attribute `X`):

```
A = { 1, 2, 3 }
B = { 2, 3 }
C = { 1 }
```

**Tree 1: `(A union B) minus C`**

```
A union B = { 1, 2, 3 }   (union removes duplicates)
{ 1, 2, 3 } minus { 1 } = { 2, 3 }
```
Result: `{ 2, 3 }`

**Tree 2: `A union (B minus C)`**

```
B minus C = { 2, 3 }      (1 is not in B, so nothing is removed)
A union { 2, 3 } = { 1, 2, 3 }
```
Result: `{ 1, 2, 3 }`

The two trees produce different results, confirming the grammar is ambiguous.

### 2.4 The Stratified Grammar That Removes the Ambiguity

The stratified grammar (see Section 5 for the full version) encodes precedence
and left-associativity by splitting `Expr` into one non-terminal per
precedence level:

```
Expr        ::= AddExpr
AddExpr     ::= AddExpr ("union" | "intersect" | "minus") MulExpr
              | MulExpr
MulExpr     ::= MulExpr ("times" | "join" "[" Condition "]") UnaryExpr
              | UnaryExpr
UnaryExpr   ::= "select"  "[" Condition    "]" "(" Expr ")"
              | "project" "[" AttrList     "]" "(" Expr ")"
              | "rename"  "[" Identifier   "]" "(" Expr ")"
              | Atom
Atom        ::= IDENT | "(" Expr ")"
```

**Which tree does it force?**

`A union B minus C` is parsed as `(A union B) minus C` — **Tree 1**.

Both `union` and `minus` live at the same precedence level (`AddExpr`).
Because the rule is left-recursive (`AddExpr ::= AddExpr op MulExpr`), the
parser always reduces the leftmost sub-expression first, producing a
left-leaning tree. The input is therefore read as
`(A union B) minus C`, not `A union (B minus C)`.

> **Left recursion note:** The stratified grammar above uses left recursion
> (`AddExpr ::= AddExpr ...`, `MulExpr ::= MulExpr ...`). This is intentional
> for expressing left-associativity, but it cannot be used directly in a
> recursive-descent (top-down) parser without modification. See
> [Section 10](#10-left-recursion-notes) for how this is resolved.

---

## 3. Top-Level Structure

A program is a sequence of one or more statements separated by optional
whitespace. Each statement is either a relation definition or a query
expression.

```ebnf
program            = statement , { statement } ;

statement          = relation-definition
                   | query-expression ;
```

A comment begins with `//` and extends to the end of the line. Comments may
appear before a relation definition or stand alone; they are ignored by the
parser.

```ebnf
comment            = "//" , { any-char-except-newline } , newline ;
```

---

## 4. Relation Definitions

```ebnf
relation-definition = [ comment ] ,
                      identifier , "(" , attribute-name-list , ")" ,
                      "=" , "{" , { tuple-row } , "}" ;

attribute-name-list = identifier , { "," , identifier } ;

tuple-row           = value , { "," , value } ;
```

Example:

```
// employees and their departments
Employees (EID, Name, Age, DID) = {
  E1, John, 32, D1
  E2, Alice, 28, D2
  E3, Bob, 29, D1
}
```

Each value in a `tuple-row` is an unquoted identifier (for string-like data),
a number, or a quoted string. Rows are separated by newlines; no trailing
comma is required after the last value on a row.

---

## 5. Query Expressions — Stratified Grammar

This grammar encodes the precedence table from Section 1. Each level is a
separate non-terminal.

```ebnf
query-expression    = additive-expr ;

(* Level 1 — union / intersect / minus — left-associative *)
additive-expr       = additive-expr , additive-op , multiplicative-expr
                    | multiplicative-expr ;

additive-op         = "union" | "intersect" | "minus" ;

(* Level 2 — times / join — left-associative *)
multiplicative-expr = multiplicative-expr , multiplicative-op , unary-expr
                    | unary-expr ;

multiplicative-op   = "times"
                    | "join" , "[" , condition , "]" ;

(* Level 3 — unary prefix operators *)
unary-expr          = "select"  , "[" , condition          , "]" , "(" , query-expression , ")"
                    | "project" , "[" , attribute-name-list , "]" , "(" , query-expression , ")"
                    | "rename"  , "[" , identifier          , "]" , "(" , query-expression , ")"
                    | atom-expr ;

(* Level 4 — atoms *)
atom-expr           = identifier
                    | "(" , query-expression , ")" ;
```

> **Left recursion note:** `additive-expr` and `multiplicative-expr` are
> left-recursive. See [Section 10](#10-left-recursion-notes).

---

## 6. Conditions

Used inside `select[...]`, `join[...]`, and parenthesised sub-conditions.
Precedence: `not` (highest) > `and` > `or` (lowest).

```ebnf
condition           = or-expr ;

or-expr             = or-expr , "or" , and-expr
                    | and-expr ;

and-expr            = and-expr , "and" , not-expr
                    | not-expr ;

not-expr            = "not" , not-expr
                    | "(" , condition , ")"
                    | comparison ;

comparison          = operand , comparison-op , operand ;

comparison-op       = "=" | "!=" | "<" | "<=" | ">" | ">=" ;

operand             = qualified-attribute
                    | number
                    | string ;

qualified-attribute = identifier , [ "." , identifier ] ;
```

A `qualified-attribute` with no dot is a plain attribute name (e.g. `Age`).
With a dot it is `relation-name.attribute-name` (e.g. `Emp.DID`).

> **Left recursion note:** `or-expr` and `and-expr` are left-recursive.
> See [Section 10](#10-left-recursion-notes).

---

## 7. Values and Literals

```ebnf
value               = number | string | identifier ;

number              = [ "-" ] , digit , { digit } , [ "." , digit , { digit } ] ;

string              = '"' , { string-char } , '"' ;

string-char         = any-char-except-double-quote-or-newline
                    | '\"' ;
```

---

## 8. Identifiers and Base Rules

```ebnf
identifier          = letter , { letter | digit | "_" } ;

letter              = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I"
                    | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R"
                    | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
                    | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i"
                    | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r"
                    | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" ;

digit               = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

newline             = ? newline character (U+000A) ? ;

any-char-except-newline
                    = ? any Unicode character except U+000A ? ;

any-char-except-double-quote-or-newline
                    = ? any Unicode character except U+0022 and U+000A ? ;
```

Identifiers are case-sensitive. `EID` and `eid` are distinct.

Reserved words that may not be used as relation or attribute names:
`select`, `project`, `rename`, `union`, `intersect`, `minus`, `times`,
`join`, `and`, `or`, `not`.

---

## 9. Semantic Constraints

These rules cannot be expressed in a context-free grammar and are enforced
during evaluation.

| Operator | Output schema and behaviour |
|---|---|
| `select[c]` | Same schema as the input. The condition may compare two attributes, not only an attribute against a constant. All attribute references in `c` must exist in the input schema. |
| `project[a₁,…,aₙ]` | Attributes `a₁…aₙ` in that order. All listed attributes must exist in the input schema. Duplicate tuples are removed from the result. |
| `rename[N]` | Same attributes under the new relation name `N`. Required to make self-joins expressible. |
| `times` | All attributes of both inputs, each qualified by their source relation name. If two qualified names would collide, that is an error. |
| `join[c]` | Equivalent to `select[c](L times R)`. Theta join — not a natural join. No implicit attribute matching occurs. |
| `union` | Schema of the left input. Both inputs must be union-compatible. |
| `intersect` | Schema of the left input. Both inputs must be union-compatible. |
| `minus` | Schema of the left input. Both inputs must be union-compatible. |

**Union compatibility** requires: the same number of attributes, the same
attribute names in the same order, and compatible types position by position.
Anything else is an error.

**Type error rule:** Comparing a number to a string in a condition is an
error, not a silent false.

---

## 10. Left Recursion Notes

Several rules in this grammar are intentionally left-recursive to express
left-associativity cleanly. Left recursion cannot be used directly in a
recursive-descent (top-down) parser; it must be eliminated or handled by an
iterative loop.

The affected rules are:

| Rule | Why left-recursive |
|---|---|
| `additive-expr` | `additive-expr ::= additive-expr op multiplicative-expr` |
| `multiplicative-expr` | `multiplicative-expr ::= multiplicative-expr op unary-expr` |
| `or-expr` | `or-expr ::= or-expr "or" and-expr` |
| `and-expr` | `and-expr ::= and-expr "and" not-expr` |

**Resolution to be determined.** *(This section will be updated once the
resolution approach has been decided.)*

---

## Sources
TODO - convert all to proper reference format
https://www.youtube.com/watch?v=IO5ie7GbJGI - Left recursion
https://www.youtube.com/watch?v=iddRD8tJi44 - Recursive descent parsers
Engineering a compiler third edition - Scanners, Parsers
