#include <string.h>
#include "unity.h"
#include "lexer.h"
#include "parser.h"

static Tree parse_source(const char *src, bool *has_error_out)
{
	Lexer lexer;
	Arena token_arena = arena_create(4096);
	Arena tree_node_arena = arena_create(4096);

	lexer_init(&lexer, src, strlen(src));
	size_t token_count = 0;
	lex(&lexer, &token_arena, &token_count);

	TEST_ASSERT_FALSE_MESSAGE(lexer.has_error, "lexer should not report errors");

	Parser parser;
	parser_init(&parser, &token_arena, token_count, &tree_node_arena);

	Tree tree = (Tree){ .head = NULL, .tail = NULL };
	parse(&parser, &tree);

	printf("--- parse tree for: %s ---\n", src);
	print_tree(&tree);

	if (has_error_out != NULL)
	{
		*has_error_out = parser.has_error;
	}

	return tree;
}

static void assert_token_lexeme(Token *token, const char *expected)
{
	TEST_ASSERT_NOT_NULL(token);
	TEST_ASSERT_EQUAL_size_t(strlen(expected), token->lexeme_length);
	TEST_ASSERT_EQUAL_INT(0, strncmp(token->lexeme_start, expected, token->lexeme_length));
}

typedef struct ExpectedNode
{
	Category category;
	const char *lexemes[2];
	size_t token_count;
	const struct ExpectedNode *left;
	const struct ExpectedNode *right;
} ExpectedNode;

static void assert_tree_node(const TreeNode *actual, const ExpectedNode *expected)
{
	if (expected == NULL)
	{
		TEST_ASSERT_NULL(actual);
		return;
	}

	TEST_ASSERT_NOT_NULL(actual);
	TEST_ASSERT_EQUAL_size_t(expected->token_count, actual->token_count);

	for (size_t i = 0; i < expected->token_count; i++)
	{
		assert_token_lexeme(actual->token_arr[i], expected->lexemes[i]);
	}

	TEST_ASSERT_EQUAL_INT(expected->category, actual->token_arr[0]->category);

	assert_tree_node(actual->left_child, expected->left);
	assert_tree_node(actual->right_child, expected->right);
}

void setUp(void)
{
	printf("\n=== %s ===\n", Unity.CurrentTestName);
}

void tearDown(void)
{
}

void test_parse_simple_relation(void)
{
	bool has_error = false;
	Tree tree = parse_source("selection", &has_error);

	TEST_ASSERT_FALSE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_size_t(1, tree.head->root->token_count);
	assert_token_lexeme(tree.head->root->token_arr[0], "selection");
	TEST_ASSERT_EQUAL_INT(IDENT, tree.head->root->token_arr[0]->category);
	TEST_ASSERT_NULL(tree.head->root->left_child);
	TEST_ASSERT_NULL(tree.head->root->right_child);
	TEST_ASSERT_NULL(tree.head->next);
}

void test_parse_bare_select_is_error(void)
{
	bool has_error = false;
	Tree tree = parse_source("select", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_size_t(1, tree.head->root->token_count);
	assert_token_lexeme(tree.head->root->token_arr[0], "select");
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_bare_project_is_error(void)
{
	bool has_error = false;
	Tree tree = parse_source("project", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_size_t(1, tree.head->root->token_count);
	assert_token_lexeme(tree.head->root->token_arr[0], "project");
	TEST_ASSERT_EQUAL_INT(PROJECT, tree.head->root->token_arr[0]->category);
}

void test_parse_bare_rename_is_error(void)
{
	bool has_error = false;
	Tree tree = parse_source("rename", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_size_t(1, tree.head->root->token_count);
	assert_token_lexeme(tree.head->root->token_arr[0], "rename");
	TEST_ASSERT_EQUAL_INT(RENAME, tree.head->root->token_arr[0]->category);
}

void test_parse_select_simple_comparison(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>30](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,         .lexemes = {"R"},   .token_count = 1 };
	ExpectedNode age    = { .category = IDENT,         .lexemes = {"Age"}, .token_count = 1 };
	ExpectedNode num30  = { .category = NUMBER,        .lexemes = {"30"},  .token_count = 1 };
	ExpectedNode cond   = { .category = GREATER_THAN,  .lexemes = {">"},   .token_count = 1, .left = &age, .right = &num30 };
	ExpectedNode root   = { .category = SELECT,        .lexemes = {"select"}, .token_count = 1, .left = &cond, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_and_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>30 and DID='D1'](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r       = { .category = IDENT,        .lexemes = {"R"},      .token_count = 1 };
	ExpectedNode age     = { .category = IDENT,        .lexemes = {"Age"},    .token_count = 1 };
	ExpectedNode num30   = { .category = NUMBER,       .lexemes = {"30"},     .token_count = 1 };
	ExpectedNode left_cmp = { .category = GREATER_THAN, .lexemes = {">"},     .token_count = 1, .left = &age, .right = &num30 };
	ExpectedNode did     = { .category = IDENT,        .lexemes = {"DID"},    .token_count = 1 };
	ExpectedNode d1      = { .category = STRING,       .lexemes = {"'D1'"},   .token_count = 1 };
	ExpectedNode right_cmp = { .category = EQUAL,      .lexemes = {"="},      .token_count = 1, .left = &did, .right = &d1 };
	ExpectedNode and_node = { .category = AND,         .lexemes = {"and"},    .token_count = 1, .left = &left_cmp, .right = &right_cmp };
	ExpectedNode root    = { .category = SELECT,       .lexemes = {"select"}, .token_count = 1, .left = &and_node, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_or_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>30 or Age<10](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r       = { .category = IDENT,        .lexemes = {"R"},      .token_count = 1 };
	ExpectedNode age1    = { .category = IDENT,        .lexemes = {"Age"},    .token_count = 1 };
	ExpectedNode num30   = { .category = NUMBER,       .lexemes = {"30"},     .token_count = 1 };
	ExpectedNode left_cmp = { .category = GREATER_THAN, .lexemes = {">"},     .token_count = 1, .left = &age1, .right = &num30 };
	ExpectedNode age2    = { .category = IDENT,        .lexemes = {"Age"},    .token_count = 1 };
	ExpectedNode num10   = { .category = NUMBER,       .lexemes = {"10"},     .token_count = 1 };
	ExpectedNode right_cmp = { .category = LESS_THAN,  .lexemes = {"<"},      .token_count = 1, .left = &age2, .right = &num10 };
	ExpectedNode or_node = { .category = OR,           .lexemes = {"or"},     .token_count = 1, .left = &left_cmp, .right = &right_cmp };
	ExpectedNode root    = { .category = SELECT,       .lexemes = {"select"}, .token_count = 1, .left = &or_node, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_not_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[not Age>30](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,        .lexemes = {"R"},      .token_count = 1 };
	ExpectedNode age    = { .category = IDENT,        .lexemes = {"Age"},    .token_count = 1 };
	ExpectedNode num30  = { .category = NUMBER,       .lexemes = {"30"},     .token_count = 1 };
	ExpectedNode cmp    = { .category = GREATER_THAN, .lexemes = {">"},      .token_count = 1, .left = &age, .right = &num30 };
	ExpectedNode not_node = { .category = NOT,        .lexemes = {"not"},    .token_count = 1, .left = &cmp, .right = NULL };
	ExpectedNode root   = { .category = SELECT,       .lexemes = {"select"}, .token_count = 1, .left = &not_node, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_not_parenthesized_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[not (Age>30 and DID='D1')](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r        = { .category = IDENT,        .lexemes = {"R"},      .token_count = 1 };
	ExpectedNode age      = { .category = IDENT,        .lexemes = {"Age"},    .token_count = 1 };
	ExpectedNode num30    = { .category = NUMBER,       .lexemes = {"30"},     .token_count = 1 };
	ExpectedNode left_cmp = { .category = GREATER_THAN, .lexemes = {">"},      .token_count = 1, .left = &age, .right = &num30 };
	ExpectedNode did      = { .category = IDENT,        .lexemes = {"DID"},    .token_count = 1 };
	ExpectedNode d1       = { .category = STRING,       .lexemes = {"'D1'"},   .token_count = 1 };
	ExpectedNode right_cmp = { .category = EQUAL,       .lexemes = {"="},      .token_count = 1, .left = &did, .right = &d1 };
	ExpectedNode and_node = { .category = AND,          .lexemes = {"and"},    .token_count = 1, .left = &left_cmp, .right = &right_cmp };
	ExpectedNode not_node = { .category = NOT,          .lexemes = {"not"},    .token_count = 1, .left = &and_node, .right = NULL };
	ExpectedNode root     = { .category = SELECT,       .lexemes = {"select"}, .token_count = 1, .left = &not_node, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_attribute_vs_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age=Salary](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,  .lexemes = {"R"},      .token_count = 1 };
	ExpectedNode age    = { .category = IDENT,  .lexemes = {"Age"},    .token_count = 1 };
	ExpectedNode salary = { .category = IDENT,  .lexemes = {"Salary"}, .token_count = 1 };
	ExpectedNode cmp    = { .category = EQUAL,  .lexemes = {"="},      .token_count = 1, .left = &age, .right = &salary };
	ExpectedNode root   = { .category = SELECT, .lexemes = {"select"}, .token_count = 1, .left = &cmp, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_qualified_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Emp.Age>30](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r          = { .category = IDENT,        .lexemes = {"R"},        .token_count = 1 };
	ExpectedNode emp_age    = { .category = IDENT,        .lexemes = {"Emp", "Age"}, .token_count = 2 };
	ExpectedNode num30      = { .category = NUMBER,       .lexemes = {"30"},       .token_count = 1 };
	ExpectedNode cmp        = { .category = GREATER_THAN, .lexemes = {">"},        .token_count = 1, .left = &emp_age, .right = &num30 };
	ExpectedNode root       = { .category = SELECT,       .lexemes = {"select"},   .token_count = 1, .left = &cmp, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_keyword_as_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[union=3](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,  .lexemes = {"R"},      .token_count = 1 };
	ExpectedNode uni    = { .category = IDENT,  .lexemes = {"union"},  .token_count = 1 };
	ExpectedNode num3   = { .category = NUMBER, .lexemes = {"3"},      .token_count = 1 };
	ExpectedNode cmp    = { .category = EQUAL,  .lexemes = {"="},      .token_count = 1, .left = &uni, .right = &num3 };
	ExpectedNode root   = { .category = SELECT, .lexemes = {"select"}, .token_count = 1, .left = &cmp, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_select_missing_bracket(void)
{
	bool has_error = false;
	Tree tree = parse_source("select(R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_missing_open_paren(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>30]R", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_missing_close_paren(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>30](R", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_empty_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_condition_missing_operand(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_condition_missing_operator(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age 30](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_condition_dangling_and(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>30 and](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_condition_unclosed_paren(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[(Age>30](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_condition_dangling_not(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[not](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);
}

void test_parse_select_error_then_next_statement_recovers(void)
{
	bool has_error = false;
	Tree tree = parse_source("select(R)\nT", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(SELECT, tree.head->root->token_arr[0]->category);

	TEST_ASSERT_NOT_NULL(tree.head->next);
	TEST_ASSERT_NOT_NULL(tree.head->next->root);
	assert_token_lexeme(tree.head->next->root->token_arr[0], "T");
	TEST_ASSERT_EQUAL_INT(IDENT, tree.head->next->root->token_arr[0]->category);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_parse_simple_relation);
	RUN_TEST(test_parse_bare_select_is_error);
	RUN_TEST(test_parse_bare_project_is_error);
	RUN_TEST(test_parse_bare_rename_is_error);
	RUN_TEST(test_parse_select_simple_comparison);
	RUN_TEST(test_parse_select_and_condition);
	RUN_TEST(test_parse_select_or_condition);
	RUN_TEST(test_parse_select_not_condition);
	RUN_TEST(test_parse_select_not_parenthesized_condition);
	RUN_TEST(test_parse_select_attribute_vs_attribute);
	RUN_TEST(test_parse_select_qualified_attribute);
	RUN_TEST(test_parse_select_keyword_as_attribute);
	RUN_TEST(test_parse_select_missing_bracket);
	RUN_TEST(test_parse_select_missing_open_paren);
	RUN_TEST(test_parse_select_missing_close_paren);
	RUN_TEST(test_parse_select_empty_condition);
	RUN_TEST(test_parse_select_condition_missing_operand);
	RUN_TEST(test_parse_select_condition_missing_operator);
	RUN_TEST(test_parse_select_condition_dangling_and);
	RUN_TEST(test_parse_select_condition_unclosed_paren);
	RUN_TEST(test_parse_select_condition_dangling_not);
	RUN_TEST(test_parse_select_error_then_next_statement_recovers);
	return UNITY_END();
}
