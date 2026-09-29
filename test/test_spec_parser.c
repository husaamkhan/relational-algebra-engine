#include <string.h>
#include "unity.h"
#include "lexer.h"
#include "parser.h"

static Arena test_arena;

static Token *test_token(Arena *arena, Category category, const char *lexeme)
{
	Token *token = token_new(arena);

	*token = (Token){
		.category = category,
		.lexeme_start = lexeme,
		.lexeme_length = lexeme != NULL ? strlen(lexeme) : 0,
		.pos = {0, 0}
	};

	return token;
}

static TreeNode *test_node(
		Arena *arena,
		Category category,
		const char *const *lexemes,
		size_t token_count,
		TreeNode *left_child,
		TreeNode *right_child)
{
	TreeNode *node = tree_node_create(
			arena,
			token_count,
			NULL,
			left_child,
			right_child
			);

	for (size_t i = 0; i < token_count; i++)
		node->token_arr[i] = test_token(arena, category, lexemes[i]);

	if (left_child != NULL)
		left_child->parent = node;

	if (right_child != NULL)
		right_child->parent = node;

	return node;
}

static TreeNode *test_single_node(
		Arena *arena,
		Category category,
		const char *lexeme,
		TreeNode *left_child,
		TreeNode *right_child)
{
	return test_node(
			arena,
			category,
			&lexeme,
			1,
			left_child,
			right_child
			);
}

static Tree parse_source(const char *src, bool *has_error_out)
{
	Lexer lexer;

	lexer_init(&lexer, src, strlen(src));

	size_t token_count = 0;
	lex(&lexer, &test_arena, &token_count);

	TEST_ASSERT_FALSE_MESSAGE(
			lexer.has_error,
			"lexer should not report errors"
			);

	Parser parser;
	parser_init(&parser, &test_arena, token_count);

	Tree tree = (Tree){
		.head = NULL,
		.tail = NULL
	};

	parse(&parser, &tree);

	printf("--- parse tree for: %s ---\n", src);
	print_tree(&tree);

	if (has_error_out != NULL)
		*has_error_out = parser.has_error;

	return tree;
}

static void assert_token_lexeme(Token *token, const char *expected)
{
	TEST_ASSERT_NOT_NULL(token);

	if (expected == NULL)
	{
		TEST_ASSERT_NULL(token->lexeme_start);
		TEST_ASSERT_EQUAL_size_t(0, token->lexeme_length);
		return;
	}

	TEST_ASSERT_EQUAL_size_t(
			strlen(expected),
			token->lexeme_length
			);

	TEST_ASSERT_EQUAL_INT(
			0,
			strncmp(
				token->lexeme_start,
				expected,
				token->lexeme_length
			       )
			);
}

static void assert_tree_node(TreeNode *actual, TreeNode *expected)
{
	if (expected == NULL)
	{
		TEST_ASSERT_NULL(actual);
		return;
	}

	TEST_ASSERT_NOT_NULL(actual);

	TEST_ASSERT_EQUAL_size_t(
			expected->token_count,
			actual->token_count
			);

	for (size_t i = 0; i < expected->token_count; i++)
	{
		TEST_ASSERT_EQUAL_INT(
				expected->token_arr[i]->category,
				actual->token_arr[i]->category
				);

		assert_token_lexeme(
				actual->token_arr[i],
				expected->token_arr[i]->lexeme_start
				);
	}

	assert_tree_node(
			actual->left_child,
			expected->left_child
			);

	assert_tree_node(
			actual->right_child,
			expected->right_child
			);

	if (actual->left_child != NULL)
	{
		TEST_ASSERT_EQUAL_PTR(
				actual,
				actual->left_child->parent
				);
	}

	if (actual->right_child != NULL)
	{
		TEST_ASSERT_EQUAL_PTR(
				actual,
				actual->right_child->parent
				);
	}
}

void setUp(void)
{
	test_arena = arena_create(4096);
	printf("\n=== %s ===\n", Unity.CurrentTestName);
}

void tearDown(void)
{
	arena_destroy(&test_arena);
}


/*
 * Case 10:
 *
 * A union B minus C
 *
 * union, intersect, and minus have the same precedence and are
 * left-associative, so this groups as:
 *
 *     (A union B) minus C
 */
void test_spec_parser_10_binary_precedence(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"A union B minus C",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *a = test_single_node(
			&test_arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *b = test_single_node(
			&test_arena,
			IDENT,
			"B",
			NULL,
			NULL
			);

	TreeNode *c = test_single_node(
			&test_arena,
			IDENT,
			"C",
			NULL,
			NULL
			);

	TreeNode *union_node = test_single_node(
			&test_arena,
			UNION,
			"union",
			a,
			b
			);

	TreeNode *root = test_single_node(
			&test_arena,
			MINUS,
			"minus",
			union_node,
			c
			);

	assert_tree_node(tree.head->root, root);
}


/*
 * Case 11:
 *
 * A minus B minus C
 *
 * Left-associative:
 *
 *     (A minus B) minus C
 */
void test_spec_parser_11_minus_left_associativity(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"A minus B minus C",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *a = test_single_node(
			&test_arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *b = test_single_node(
			&test_arena,
			IDENT,
			"B",
			NULL,
			NULL
			);

	TreeNode *c = test_single_node(
			&test_arena,
			IDENT,
			"C",
			NULL,
			NULL
			);

	TreeNode *left_minus = test_single_node(
			&test_arena,
			MINUS,
			"minus",
			a,
			b
			);

	TreeNode *root = test_single_node(
			&test_arena,
			MINUS,
			"minus",
			left_minus,
			c
			);

	assert_tree_node(tree.head->root, root);
}


/*
 * Case 12:
 *
 * select[not (a=1 and b=2) or c>3](R)
 *
 * not binds tighter than and, which binds tighter than or:
 *
 *     (not (a=1 and b=2)) or (c>3)
 */
void test_spec_parser_12_not_and_or_precedence(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"select[not (a=1 and b=2) or c>3](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *a = test_single_node(
			&test_arena,
			IDENT,
			"a",
			NULL,
			NULL
			);

	TreeNode *one = test_single_node(
			&test_arena,
			NUMBER,
			"1",
			NULL,
			NULL
			);

	TreeNode *a_equal_one = test_single_node(
			&test_arena,
			EQUAL,
			"=",
			a,
			one
			);

	TreeNode *b = test_single_node(
			&test_arena,
			IDENT,
			"b",
			NULL,
			NULL
			);

	TreeNode *two = test_single_node(
			&test_arena,
			NUMBER,
			"2",
			NULL,
			NULL
			);

	TreeNode *b_equal_two = test_single_node(
			&test_arena,
			EQUAL,
			"=",
			b,
			two
			);

	TreeNode *and_node = test_single_node(
			&test_arena,
			AND,
			"and",
			a_equal_one,
			b_equal_two
			);

	TreeNode *not_node = test_single_node(
			&test_arena,
			NOT,
			"not",
			and_node,
			NULL
			);

	TreeNode *c = test_single_node(
			&test_arena,
			IDENT,
			"c",
			NULL,
			NULL
			);

	TreeNode *three = test_single_node(
			&test_arena,
			NUMBER,
			"3",
			NULL,
			NULL
			);

	TreeNode *c_greater_three = test_single_node(
			&test_arena,
			GREATER_THAN,
			">",
			c,
			three
			);

	TreeNode *or_node = test_single_node(
			&test_arena,
			OR,
			"or",
			not_node,
			c_greater_three
			);

	TreeNode *r = test_single_node(
			&test_arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&test_arena,
			SELECT,
			"select",
			or_node,
			r
			);

	assert_tree_node(tree.head->root, root);
}


/*
 * Case 13:
 *
 * select[a=1 and b=2 or c=3](R)
 *
 * and binds tighter than or:
 *
 *     (a=1 and b=2) or c=3
 */
void test_spec_parser_13_and_or_precedence(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"select[a=1 and b=2 or c=3](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *a = test_single_node(
			&test_arena,
			IDENT,
			"a",
			NULL,
			NULL
			);

	TreeNode *one = test_single_node(
			&test_arena,
			NUMBER,
			"1",
			NULL,
			NULL
			);

	TreeNode *a_equal_one = test_single_node(
			&test_arena,
			EQUAL,
			"=",
			a,
			one
			);

	TreeNode *b = test_single_node(
			&test_arena,
			IDENT,
			"b",
			NULL,
			NULL
			);

	TreeNode *two = test_single_node(
			&test_arena,
			NUMBER,
			"2",
			NULL,
			NULL
			);

	TreeNode *b_equal_two = test_single_node(
			&test_arena,
			EQUAL,
			"=",
			b,
			two
			);

	TreeNode *and_node = test_single_node(
			&test_arena,
			AND,
			"and",
			a_equal_one,
			b_equal_two
			);

	TreeNode *c = test_single_node(
			&test_arena,
			IDENT,
			"c",
			NULL,
			NULL
			);

	TreeNode *three = test_single_node(
			&test_arena,
			NUMBER,
			"3",
			NULL,
			NULL
			);

	TreeNode *c_equal_three = test_single_node(
			&test_arena,
			EQUAL,
			"=",
			c,
			three
			);

	TreeNode *or_node = test_single_node(
			&test_arena,
			OR,
			"or",
			and_node,
			c_equal_three
			);

	TreeNode *r = test_single_node(
			&test_arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&test_arena,
			SELECT,
			"select",
			or_node,
			r
			);

	assert_tree_node(tree.head->root, root);
}


/*
 * Case 14:
 *
 * project[Name](
 *     select[Age>30](
 *         select[DID='D1'](Employees)
 *     )
 * )
 */
void test_spec_parser_14_nested_expressions(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"project[Name](select[Age>30](select[DID='D1'](Employees)))",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *employees = test_single_node(
			&test_arena,
			IDENT,
			"Employees",
			NULL,
			NULL
			);

	TreeNode *did = test_single_node(
			&test_arena,
			IDENT,
			"DID",
			NULL,
			NULL
			);

	TreeNode *d1 = test_single_node(
			&test_arena,
			STRING,
			"'D1'",
			NULL,
			NULL
			);

	TreeNode *did_equal_d1 = test_single_node(
			&test_arena,
			EQUAL,
			"=",
			did,
			d1
			);

	TreeNode *inner_select = test_single_node(
			&test_arena,
			SELECT,
			"select",
			did_equal_d1,
			employees
			);

	TreeNode *age = test_single_node(
			&test_arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *thirty = test_single_node(
			&test_arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *age_greater_thirty = test_single_node(
			&test_arena,
			GREATER_THAN,
			">",
			age,
			thirty
			);

	TreeNode *outer_select = test_single_node(
			&test_arena,
			SELECT,
			"select",
			age_greater_thirty,
			inner_select
			);

	TreeNode *name = test_single_node(
			&test_arena,
			IDENT,
			"Name",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&test_arena,
			PROJECT,
			"project",
			name,
			outer_select
			);

	assert_tree_node(tree.head->root, root);
}


/*
 * Case 15:
 *
 * (A union B) minus (C intersect D)
 *
 * Explicit parentheses override the normal precedence.
 */
void test_spec_parser_15_parentheses_override_precedence(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"(A union B) minus (C intersect D)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *a = test_single_node(
			&test_arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *b = test_single_node(
			&test_arena,
			IDENT,
			"B",
			NULL,
			NULL
			);

	TreeNode *c = test_single_node(
			&test_arena,
			IDENT,
			"C",
			NULL,
			NULL
			);

	TreeNode *d = test_single_node(
			&test_arena,
			IDENT,
			"D",
			NULL,
			NULL
			);

	TreeNode *union_node = test_single_node(
			&test_arena,
			UNION,
			"union",
			a,
			b
			);

	TreeNode *intersect_node = test_single_node(
			&test_arena,
			INTERSECT,
			"intersect",
			c,
			d
			);

	TreeNode *root = test_single_node(
			&test_arena,
			MINUS,
			"minus",
			union_node,
			intersect_node
			);

	assert_tree_node(tree.head->root, root);
}


/*
 * Case 16:
 *
 * select[Age>30](R
 *
 * Missing closing parenthesis.
 */
void test_spec_parser_16_missing_closing_parenthesis(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"select[Age>30](R",
			&has_error
			);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(
			SELECT,
			tree.head->root->token_arr[0]->category
			);
}


/*
 * Case 17:
 *
 * project[](R)
 *
 * An empty projection attribute list is invalid.
 */
void test_spec_parser_17_empty_projection(void)
{
	bool has_error = false;

	Tree tree = parse_source(
			"project[](R)",
			&has_error
			);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(
			PROJECT,
			tree.head->root->token_arr[0]->category
			);
}


int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_spec_parser_10_binary_precedence);
	RUN_TEST(test_spec_parser_11_minus_left_associativity);
	RUN_TEST(test_spec_parser_12_not_and_or_precedence);
	RUN_TEST(test_spec_parser_13_and_or_precedence);
	RUN_TEST(test_spec_parser_14_nested_expressions);
	RUN_TEST(test_spec_parser_15_parentheses_override_precedence);
	RUN_TEST(test_spec_parser_16_missing_closing_parenthesis);
	RUN_TEST(test_spec_parser_17_empty_projection);

	return UNITY_END();
}
