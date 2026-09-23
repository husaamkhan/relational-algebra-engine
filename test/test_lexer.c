#include <string.h>
#include "unity.h"
#include "lexer.h"

static const char *CATEGORY_NAMES[] = {
	[LPAREN]                = "LPAREN",
	[RPAREN]                = "RPAREN",
	[LBRACKET]              = "LBRACKET",
	[RBRACKET]              = "RBRACKET",
	[LBRACE]                = "LBRACE",
	[RBRACE]                = "RBRACE",
	[COMMA]                 = "COMMA",
	[DOT]			= "DOT",
	[EQUAL]                 = "EQUAL",
	[NOT_EQUAL]             = "NOT_EQUAL",
	[LESS_THAN]             = "LESS_THAN",
	[LESS_THAN_OR_EQUAL]    = "LESS_THAN_OR_EQUAL",
	[GREATER_THAN]          = "GREATER_THAN",
	[GREATER_THAN_OR_EQUAL] = "GREATER_THAN_OR_EQUAL",
	[NUMBER]                = "NUMBER",
	[STRING]                = "STRING",
	[WORD]                  = "WORD",
	[NEWLINE]               = "NEWLINE",
};

typedef struct
{
    Category category;
    const char *lexeme;
    size_t lexeme_length;
    FilePosition pos;
} ExpectedToken;

static void print_tokens(Token *tokens, size_t count)
{
	printf("Token stream (%zu token%s):\n", count, count == 1 ? "" : "s");
	for (size_t i = 0; i < count; i++)
	{
		printf("  [%zu] %-22s lexeme='%.*s' pos=%d:%d\n",
				i,
				CATEGORY_NAMES[tokens[i].category],
				(int)tokens[i].lexeme_length, tokens[i].lexeme_start,
				tokens[i].pos.row, tokens[i].pos.col);
	}
}

static void run_test(const char *src, size_t input_length, size_t expected_token_count, ExpectedToken *expected_tokens, bool expect_error)
{
	Lexer lexer;
	lexer_init(&lexer, src, input_length);

	Arena arena = arena_create(4096);
	size_t count = 0;
	lex(&lexer, &arena, &count);

	Token *tokens = (Token *)arena.base;
	print_tokens(tokens, count);

	TEST_ASSERT_EQUAL_size_t(expected_token_count, count);

	for (int i = 0; i < expected_token_count; i++)
	{
		TEST_ASSERT_EQUAL_INT(expected_tokens[i].category, tokens[i].category);
		TEST_ASSERT_EQUAL_size_t(expected_tokens[i].lexeme_length, tokens[i].lexeme_length);
		TEST_ASSERT_EQUAL_INT(0, strncmp(expected_tokens[i].lexeme, tokens[i].lexeme_start, expected_tokens[i].lexeme_length));
		TEST_ASSERT_EQUAL_INT(expected_tokens[i].pos.row, tokens[i].pos.row);
		TEST_ASSERT_EQUAL_INT(expected_tokens[i].pos.col, tokens[i].pos.col);
	}

	TEST_ASSERT(expect_error == lexer.has_error);
}

void setUp(void)
{
	printf("\n=== %s ===\n", Unity.CurrentTestName);
}

void tearDown(void) {}

void test_lparen(void) {
	run_test(
		"(",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = LPAREN,
				.lexeme = "(",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_rparen(void)
{
	run_test(
		")",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = RPAREN,
				.lexeme = ")",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_lbracket(void)
{
	run_test(
		"[",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = LBRACKET,
				.lexeme = "[",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}
void test_rbracket(void)
{
	run_test(
		"]",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = RBRACKET,
				.lexeme = "]",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_lbrace(void)
{
	run_test(
		"{",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = LBRACE,
				.lexeme = "{",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_rbrace(void)
{
	run_test(
		"}",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = RBRACE,
				.lexeme = "}",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_comma(void)
{
	run_test(
		",",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = COMMA,
				.lexeme = ",",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_equal(void)
{
	run_test(
		"=",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = EQUAL,
				.lexeme = "=",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_not_equal(void)
{
	run_test(
		"!=",
		2,
		1,
		(ExpectedToken[]){
			{
				.category = NOT_EQUAL,
				.lexeme = "!=",
				.lexeme_length = 2,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}


void test_less_than(void)
{
	run_test(
		"<",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = LESS_THAN,
				.lexeme = "<",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_less_than_or_equal(void)
{
	run_test(
		"<=",
		2,
		1,
		(ExpectedToken[]){
			{
				.category = LESS_THAN_OR_EQUAL,
				.lexeme = "<=",
				.lexeme_length = 2,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_greater_than(void)
{
	run_test(
		">",
		1,
		1,
		(ExpectedToken[]){
			{
				.category = GREATER_THAN,
				.lexeme = ">",
				.lexeme_length = 1,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_greater_than_or_equal(void)
{
	run_test(
		">=",
		2,
		1,
		(ExpectedToken[]){
			{
				.category = GREATER_THAN_OR_EQUAL,
				.lexeme = ">=",
				.lexeme_length = 2,
				.pos = { .row = 1, .col = 1 }
			}
		},
		false);
}

void test_spaces_are_ignored(void)
{
	run_test(
		" ( ) ",
		5,
		2,
		(ExpectedToken[]){
			{ .category = LPAREN, .lexeme = "(", .lexeme_length = 1, .pos = { .row = 1, .col = 2 } },
			{ .category = RPAREN, .lexeme = ")", .lexeme_length = 1, .pos = { .row = 1, .col = 4 } },
		},
		false);
}

void test_newline_is_emitted_and_updates_position(void)
{
	run_test(
		"\n(",
		2,
		2,
		(ExpectedToken[]){
			{ .category = NEWLINE, .lexeme = "\n", .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
			{ .category = LPAREN,  .lexeme = "(",  .lexeme_length = 1, .pos = { .row = 2, .col = 1 } },
		},
		false);
}

void test_two_character_comparison_operators_consume_equals(void)
{
	run_test(
		"<= >= !=",
		8,
		3,
		(ExpectedToken[]){
			{ .category = LESS_THAN_OR_EQUAL,    .lexeme = "<=", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
			{ .category = GREATER_THAN_OR_EQUAL, .lexeme = ">=", .lexeme_length = 2, .pos = { .row = 1, .col = 4 } },
			{ .category = NOT_EQUAL,             .lexeme = "!=", .lexeme_length = 2, .pos = { .row = 1, .col = 7 } },
		},
		false);
}
static void test_word_single_letter(void)
{
    const char *src = "R";
    ExpectedToken expected[] = {
        {WORD, "R", 1, {1, 1}},
    };

    run_test(src, strlen(src), 1, expected, false);
}

static void test_word_multi_letter(void)
{
    const char *src = "foo";
    ExpectedToken expected[] = {
        {WORD, "foo", 3, {1, 1}},
    };

    run_test(src, strlen(src), 1, expected, false);
}

static void test_word_with_digit(void)
{
    const char *src = "R1";
    ExpectedToken expected[] = {
        {WORD, "R1", 2, {1, 1}},
    };

    run_test(src, strlen(src), 1, expected, false);
}

static void test_word_with_underscore(void)
{
    const char *src = "foo_bar";
    ExpectedToken expected[] = {
        {WORD, "foo_bar", 7, {1, 1}},
    };

    run_test(src, strlen(src), 1, expected, false);
}

static void test_word_with_at(void)
{
	const char *src = "user@host";
	ExpectedToken expected[] = {
		{WORD, "user@host", 9, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_at_cannot_start_word(void)
{
	const char *src = "@host";
	ExpectedToken expected[] = {
		{WORD, "host", 4, {1, 2}},
	};
	run_test(src, strlen(src), 1, expected, true);
}

static void test_word_bare_string(void)
{
    const char *src = "hello-world";
    ExpectedToken expected[] = {
        {WORD, "hello-world", 11, {1, 1}},
    };

    run_test(src, strlen(src), 1, expected, false);
}

static void test_words_separated_by_spaces(void)
{
    const char *src = "foo bar baz";
    ExpectedToken expected[] = {
        {WORD, "foo", 3, {1, 1}},
        {WORD, "bar", 3, {1, 5}},
        {WORD, "baz", 3, {1, 9}},
    };

    run_test(src, strlen(src), 3, expected, false);
}

static void test_words_separated_by_comma(void)
{
    const char *src = "foo,bar";
    ExpectedToken expected[] = {
        {WORD, "foo", 3, {1, 1}},
        {COMMA, ",", 1, {1, 4}},
        {WORD, "bar", 3, {1, 5}},
    };

    run_test(src, strlen(src), 3, expected, false);
}

static void test_words_separated_by_parentheses(void)
{
    const char *src = "foo(bar)";
    ExpectedToken expected[] = {
        {WORD, "foo", 3, {1, 1}},
        {LPAREN, "(", 1, {1, 4}},
        {WORD, "bar", 3, {1, 5}},
        {RPAREN, ")", 1, {1, 8}},
    };

    run_test(src, strlen(src), 4, expected, false);
}

static void test_word_stops_at_dot(void)
{
	const char *src = "Emp.DID";
	ExpectedToken expected[] = {
		{WORD, "Emp", 3, {1, 1}},
		{DOT, ".", 1, {1, 4}},
		{WORD, "DID", 3, {1, 5}},
	};
	run_test(src, strlen(src), 3, expected, false);
}

static void test_word_stops_at_single_quote(void)
{
	const char *src = "hello'world'";
	ExpectedToken expected[] = {
		{WORD, "hello", 5, {1, 1}},
		{STRING, "'world'", 7, {1, 6}},
	};
	run_test(src, strlen(src), 2, expected, false);
}

static void test_word_stops_at_double_quote(void)
{
	const char *src = "hello\"world";
	ExpectedToken expected[] = {
		{WORD, "hello", 5, {1, 1}},
		{WORD, "world", 5, {1, 7}},
	};
	run_test(src, strlen(src), 2, expected, true);
}

static void test_word_stops_at_exclamation(void)
{
	const char *src = "hello!world";
	ExpectedToken expected[] = {
		{WORD, "hello", 5, {1, 1}},
		{WORD, "world", 5, {1, 7}},
	};
	run_test(src, strlen(src), 2, expected, true);
}

/* ===================== Numbers ===================== */

void test_number_single_digit(void)
{
	run_test(
		"0",
		1,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "0", .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_multi_digit(void)
{
	run_test(
		"12345",
		5,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "12345", .lexeme_length = 5, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_negative_single_digit(void)
{
	run_test(
		"-5",
		2,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "-5", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_negative_multi_digit(void)
{
	run_test(
		"-42",
		3,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "-42", .lexeme_length = 3, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_decimal(void)
{
	run_test(
		"3.14",
		4,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "3.14", .lexeme_length = 4, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_decimal_leading_zero(void)
{
	run_test(
		"0.5",
		3,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "0.5", .lexeme_length = 3, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_decimal_trailing_zero(void)
{
	run_test(
		"1.0",
		3,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "1.0", .lexeme_length = 3, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_negative_decimal(void)
{
	run_test(
		"-3.14",
		5,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "-3.14", .lexeme_length = 5, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_number_decimal_all_digits(void)
{
	run_test(
		"99.99",
		5,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "99.99", .lexeme_length = 5, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_numbers_separated_by_comma(void)
{
	run_test(
		"1,2,3",
		5,
		5,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "1", .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
			{ .category = COMMA,  .lexeme = ",", .lexeme_length = 1, .pos = { .row = 1, .col = 2 } },
			{ .category = NUMBER, .lexeme = "2", .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
			{ .category = COMMA,  .lexeme = ",", .lexeme_length = 1, .pos = { .row = 1, .col = 4 } },
			{ .category = NUMBER, .lexeme = "3", .lexeme_length = 1, .pos = { .row = 1, .col = 5 } },
		},
		false);
}

void test_number_followed_by_word(void)
{
	run_test(
		"42R",
		3,
		2,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "42", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
			{ .category = WORD,   .lexeme = "R",  .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
		},
		false);
}

void test_number_in_comparison(void)
{
	run_test(
		"x < 5",
		5,
		3,
		(ExpectedToken[]){
			{ .category = WORD,      .lexeme = "x",   .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
			{ .category = LESS_THAN,  .lexeme = "<",   .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
			{ .category = NUMBER,     .lexeme = "5",   .lexeme_length = 1, .pos = { .row = 1, .col = 5 } },
		},
		false);
}

void test_number_negative_followed_by_space_and_word(void)
{
	run_test(
		"-5 x",
		4,
		2,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "-5", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
			{ .category = WORD,  .lexeme = "x",  .lexeme_length = 1, .pos = { .row = 1, .col = 4 } },
		},
		false);
}

static void test_number_leading_dot(void)
{
	const char *src = ".5";
	ExpectedToken expected[] = {
		{DOT, ".", 1, {1, 1}},
		{NUMBER, "5", 1, {1, 2}},
	};
	run_test(src, strlen(src), 2, expected, false);
}

void test_number_error_trailing_dot(void)
{
	run_test(
		"5.",
		2,
		0,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "", .lexeme_length = 0, .pos = { .row = 1, .col = 1 } },
		},
		true);
}

void test_number_error_bare_minus(void)
{
	run_test(
		"-",
		1,
		0,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "", .lexeme_length = 0, .pos = { .row = 1, .col = 1 } },
		},
		true);
}

void test_number_error_bare_minus_followed_by_non_digit(void)
{
	run_test(
		"-x",
		2,
		0,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "", .lexeme_length = 0, .pos = { .row = 1, .col = 1 } },
		},
		true);
}

void test_number_error_dot_without_digits_after(void)
{
	run_test(
		"5.x",
		3,
		1,
		(ExpectedToken[]){
			{ .category = WORD, .lexeme = "x", .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
		},
		true);
}

/* ===================== Quoted Strings ===================== */

static void test_string_basic(void)
{
	const char *src = "'Bob'";
	ExpectedToken expected[] = {
		{STRING, "'Bob'", 5, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_string_with_comma(void)
{
	const char *src = "'a,b'";
	ExpectedToken expected[] = {
		{STRING, "'a,b'", 5, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_string_with_open_parenthesis(void)
{
	const char *src = "'(Bob'";
	ExpectedToken expected[] = {
		{STRING, "'(Bob'", 6, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_string_with_parentheses(void)
{
	const char *src = "'Bob)'";
	ExpectedToken expected[] = {
		{STRING, "'Bob)'", 6, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_string_with_space(void)
{
	const char *src = "'hello world'";
	ExpectedToken expected[] = {
		{STRING, "'hello world'", 13, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_string_with_quote(void)
{
	const char *src = "'O''Brien'";
	ExpectedToken expected[] = {
		{STRING, "'O''Brien'", 10, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_string_unterminated(void)
{
	const char *src = "'Bob";
	ExpectedToken expected[] = {
	};
	run_test(src, strlen(src), 0, expected, true);
}

static void test_double_quote_is_invalid(void)
{
	const char *src = "\"Bob\"";
	ExpectedToken expected[] = {
		{WORD, "Bob", 3, {1, 2}},
	};
	run_test(src, strlen(src), 1, expected, true);
}

static void test_string_newline(void)
{
	const char *src = "'hello\nworld'";
	ExpectedToken expected[] = {
		{NEWLINE, "\n", 1, {1, 7}},
		{WORD, "world", 5, {2, 1}},
	};
	run_test(src, strlen(src), 2, expected, true);
}
static void test_string_empty(void)
{
	const char *src = "''";
	ExpectedToken expected[] = {
		{STRING, "''", 2, {1, 1}},
	};
	run_test(src, strlen(src), 1, expected, false);
}

static void test_select_statement(void)
{
	const char *src = "select[x=3](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "x", 1, {1, 8}},
		{EQUAL, "=", 1, {1, 9}},
		{NUMBER, "3", 1, {1, 10}},
		{RBRACKET, "]", 1, {1, 11}},
		{LPAREN, "(", 1, {1, 12}},
		{WORD, "R", 1, {1, 13}},
		{RPAREN, ")", 1, {1, 14}},
	};
	run_test(src, strlen(src), 9, expected, false);
}

static void test_select_statement_with_string(void)
{
	const char *src = "select[Name='Bob'](Emp)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Name", 4, {1, 8}},
		{EQUAL, "=", 1, {1, 12}},
		{STRING, "'Bob'", 5, {1, 13}},
		{RBRACKET, "]", 1, {1, 18}},
		{LPAREN, "(", 1, {1, 19}},
		{WORD, "Emp", 3, {1, 20}},
		{RPAREN, ")", 1, {1, 23}},
	};
	run_test(src, strlen(src), 9, expected, false);
}

static void test_select_with_qualified_attribute(void)
{
	const char *src = "select[Emp.DID>=30](Emp)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Emp", 3, {1, 8}},
		{DOT, ".", 1, {1, 11}},
		{WORD, "DID", 3, {1, 12}},
		{GREATER_THAN_OR_EQUAL, ">=", 2, {1, 15}},
		{NUMBER, "30", 2, {1, 17}},
		{RBRACKET, "]", 1, {1, 19}},
		{LPAREN, "(", 1, {1, 20}},
		{WORD, "Emp", 3, {1, 21}},
		{RPAREN, ")", 1, {1, 24}},
	};
	run_test(src, strlen(src), 11, expected, false);
}

static void test_required_select_no_whitespace(void)
{
	const char *src = "select[x1=3](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "x1", 2, {1, 8}},
		{EQUAL, "=", 1, {1, 10}},
		{NUMBER, "3", 1, {1, 11}},
		{RBRACKET, "]", 1, {1, 12}},
		{LPAREN, "(", 1, {1, 13}},
		{WORD, "R", 1, {1, 14}},
		{RPAREN, ")", 1, {1, 15}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_select_with_whitespace(void)
{
	const char *src = "select[ x1 = 3 ](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "x1", 2, {1, 9}},
		{EQUAL, "=", 1, {1, 12}},
		{NUMBER, "3", 1, {1, 14}},
		{RBRACKET, "]", 1, {1, 16}},
		{LPAREN, "(", 1, {1, 17}},
		{WORD, "R", 1, {1, 18}},
		{RPAREN, ")", 1, {1, 19}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_greater_than_or_equal(void)
{
	const char *src = "select[Age>=30](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Age", 3, {1, 8}},
		{GREATER_THAN_OR_EQUAL, ">=", 2, {1, 11}},
		{NUMBER, "30", 2, {1, 13}},
		{RBRACKET, "]", 1, {1, 15}},
		{LPAREN, "(", 1, {1, 16}},
		{WORD, "R", 1, {1, 17}},
		{RPAREN, ")", 1, {1, 18}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_greater_than_negative_number(void)
{
	const char *src = "select[Age>-30](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Age", 3, {1, 8}},
		{GREATER_THAN, ">", 1, {1, 11}},
		{NUMBER, "-30", 3, {1, 12}},
		{RBRACKET, "]", 1, {1, 15}},
		{LPAREN, "(", 1, {1, 16}},
		{WORD, "R", 1, {1, 17}},
		{RPAREN, ")", 1, {1, 18}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_parenthesis_inside_string(void)
{
	const char *src = "select[Name='Bob)'](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Name", 4, {1, 8}},
		{EQUAL, "=", 1, {1, 12}},
		{STRING, "'Bob)'", 6, {1, 13}},
		{RBRACKET, "]", 1, {1, 19}},
		{LPAREN, "(", 1, {1, 20}},
		{WORD, "R", 1, {1, 21}},
		{RPAREN, ")", 1, {1, 22}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_comma_inside_string(void)
{
	const char *src = "select[Name='a,b'](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Name", 4, {1, 8}},
		{EQUAL, "=", 1, {1, 12}},
		{STRING, "'a,b'", 5, {1, 13}},
		{RBRACKET, "]", 1, {1, 18}},
		{LPAREN, "(", 1, {1, 19}},
		{WORD, "R", 1, {1, 20}},
		{RPAREN, ")", 1, {1, 21}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_doubled_quote(void)
{
	const char *src = "select[Name='O''Brien'](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Name", 4, {1, 8}},
		{EQUAL, "=", 1, {1, 12}},
		{STRING, "'O''Brien'", 10, {1, 13}},
		{RBRACKET, "]", 1, {1, 23}},
		{LPAREN, "(", 1, {1, 24}},
		{WORD, "R", 1, {1, 25}},
		{RPAREN, ")", 1, {1, 26}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_keyword_as_attribute(void)
{
	const char *src = "select[union=3](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "union", 5, {1, 8}},
		{EQUAL, "=", 1, {1, 13}},
		{NUMBER, "3", 1, {1, 14}},
		{RBRACKET, "]", 1, {1, 15}},
		{LPAREN, "(", 1, {1, 16}},
		{WORD, "R", 1, {1, 17}},
		{RPAREN, ")", 1, {1, 18}},
	};

	run_test(src, strlen(src), 9, expected, false);
}

static void test_required_unterminated_string(void)
{
	const char *src = "select[Name='Bob](R)";
	ExpectedToken expected[] = {
		{WORD, "select", 6, {1, 1}},
		{LBRACKET, "[", 1, {1, 7}},
		{WORD, "Name", 4, {1, 8}},
		{EQUAL, "=", 1, {1, 12}},
	};

	run_test(src, strlen(src), 4, expected, true);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_lparen);
	RUN_TEST(test_rparen);
	RUN_TEST(test_lbracket);
	RUN_TEST(test_rbracket);
	RUN_TEST(test_lbrace);
	RUN_TEST(test_rbrace);
	RUN_TEST(test_comma);
	RUN_TEST(test_equal);
	RUN_TEST(test_not_equal);
	RUN_TEST(test_less_than);
	RUN_TEST(test_less_than_or_equal);
	RUN_TEST(test_greater_than);
	RUN_TEST(test_greater_than_or_equal);
	RUN_TEST(test_spaces_are_ignored);
	RUN_TEST(test_newline_is_emitted_and_updates_position);
	RUN_TEST(test_two_character_comparison_operators_consume_equals);
	RUN_TEST(test_word_single_letter);
	RUN_TEST(test_word_multi_letter);
	RUN_TEST(test_word_with_digit);
	RUN_TEST(test_word_with_underscore);
	RUN_TEST(test_words_separated_by_spaces);
	RUN_TEST(test_word_bare_string);
	RUN_TEST(test_words_separated_by_comma);
	RUN_TEST(test_words_separated_by_parentheses);
	RUN_TEST(test_word_stops_at_dot);
	RUN_TEST(test_word_stops_at_single_quote);
	RUN_TEST(test_word_stops_at_double_quote);
	RUN_TEST(test_word_stops_at_exclamation);
	RUN_TEST(test_number_single_digit);
	RUN_TEST(test_number_multi_digit);
	RUN_TEST(test_number_negative_single_digit);
	RUN_TEST(test_number_negative_multi_digit);
	RUN_TEST(test_number_decimal);
	RUN_TEST(test_number_decimal_leading_zero);
	RUN_TEST(test_number_decimal_trailing_zero);
	RUN_TEST(test_number_negative_decimal);
	RUN_TEST(test_number_decimal_all_digits);
	RUN_TEST(test_numbers_separated_by_comma);
	RUN_TEST(test_number_followed_by_word);
	RUN_TEST(test_number_in_comparison);
	RUN_TEST(test_number_negative_followed_by_space_and_word);
	RUN_TEST(test_number_leading_dot);
	RUN_TEST(test_number_error_trailing_dot);
	RUN_TEST(test_number_error_bare_minus);
	RUN_TEST(test_number_error_bare_minus_followed_by_non_digit);
	RUN_TEST(test_number_error_dot_without_digits_after);
	RUN_TEST(test_string_basic);
	RUN_TEST(test_string_with_comma);
	RUN_TEST(test_string_with_parentheses);
	RUN_TEST(test_string_with_open_parenthesis);
	RUN_TEST(test_string_with_space);
	RUN_TEST(test_string_with_quote);
	RUN_TEST(test_string_empty);
	RUN_TEST(test_string_unterminated);
	RUN_TEST(test_double_quote_is_invalid);
	RUN_TEST(test_string_newline);
	RUN_TEST(test_select_statement);
	RUN_TEST(test_select_statement_with_string);
	RUN_TEST(test_select_with_qualified_attribute);
	RUN_TEST(test_required_select_no_whitespace);
	RUN_TEST(test_required_select_with_whitespace);
	RUN_TEST(test_required_greater_than_or_equal);
	RUN_TEST(test_required_greater_than_negative_number);
	RUN_TEST(test_required_parenthesis_inside_string);
	RUN_TEST(test_required_comma_inside_string);
	RUN_TEST(test_required_doubled_quote);
	RUN_TEST(test_required_keyword_as_attribute);
	RUN_TEST(test_required_unterminated_string);
	return UNITY_END();
}
