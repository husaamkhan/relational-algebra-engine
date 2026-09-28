#include <string.h>
#include "unity.h"
#include "lexer.h"
#include "parser.h"

typedef struct ExpectedNode
{
	Category category;
	const char *lexemes[8];
	size_t token_count;
	const struct ExpectedNode *left;
	const struct ExpectedNode *right;
} ExpectedNode;

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

	if (expected == NULL)
	{
		TEST_ASSERT_NULL(token->lexeme_start);
		TEST_ASSERT_EQUAL_size_t(0, token->lexeme_length);
		return;
	}

	TEST_ASSERT_EQUAL_size_t(strlen(expected), token->lexeme_length);
	TEST_ASSERT_EQUAL_INT(
		0,
		strncmp(token->lexeme_start, expected, token->lexeme_length)
	);
}

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

void test_parse_project_single_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source("project[A](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,   .lexemes = {"R"},       .token_count = 1 };
	ExpectedNode attr   = { .category = IDENT,   .lexemes = {"A"},       .token_count = 1 };
	ExpectedNode root   = { .category = PROJECT, .lexemes = {"project"}, .token_count = 1,
	                        .left = &attr, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_project_multiple_attributes(void)
{
	bool has_error = false;
	Tree tree = parse_source("project[A,B,C](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,   .lexemes = {"R"},             .token_count = 1 };
	ExpectedNode attrs  = { .category = IDENT,   .lexemes = {"A", "B", "C"},  .token_count = 3 };
	ExpectedNode root   = { .category = PROJECT, .lexemes = {"project"},       .token_count = 1,
	                        .left = &attrs, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_project_keyword_as_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source("project[union](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,   .lexemes = {"R"},       .token_count = 1 };
	ExpectedNode attr   = { .category = IDENT,   .lexemes = {"union"},   .token_count = 1 };
	ExpectedNode root   = { .category = PROJECT, .lexemes = {"project"}, .token_count = 1,
	                        .left = &attr, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_rename(void)
{
	bool has_error = false;
	Tree tree = parse_source("rename[B](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r      = { .category = IDENT,  .lexemes = {"R"},      .token_count = 1 };
	ExpectedNode name   = { .category = IDENT,  .lexemes = {"B"},      .token_count = 1 };
	ExpectedNode root   = { .category = RENAME, .lexemes = {"rename"}, .token_count = 1,
	                        .left = &name, .right = &r };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_project_nested_select(void)
{
	bool has_error = false;
	Tree tree = parse_source("project[A,B](select[Age>30](R))", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r       = { .category = IDENT,        .lexemes = {"R"},       .token_count = 1 };
	ExpectedNode age     = { .category = IDENT,        .lexemes = {"Age"},     .token_count = 1 };
	ExpectedNode num30   = { .category = NUMBER,       .lexemes = {"30"},      .token_count = 1 };
	ExpectedNode cmp     = { .category = GREATER_THAN, .lexemes = {">"},       .token_count = 1,
	                         .left = &age, .right = &num30 };
	ExpectedNode select  = { .category = SELECT,       .lexemes = {"select"},  .token_count = 1,
	                         .left = &cmp, .right = &r };
	ExpectedNode attrs   = { .category = IDENT,        .lexemes = {"A", "B"},  .token_count = 2 };
	ExpectedNode root    = { .category = PROJECT,      .lexemes = {"project"}, .token_count = 1,
	                         .left = &attrs, .right = &select };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_rename_nested_project(void)
{
	bool has_error = false;
	Tree tree = parse_source("rename[Employees](project[A](R))", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r       = { .category = IDENT,   .lexemes = {"R"},         .token_count = 1 };
	ExpectedNode attr    = { .category = IDENT,   .lexemes = {"A"},         .token_count = 1 };
	ExpectedNode project = { .category = PROJECT, .lexemes = {"project"},   .token_count = 1,
	                         .left = &attr, .right = &r };
	ExpectedNode name    = { .category = IDENT,   .lexemes = {"Employees"}, .token_count = 1 };
	ExpectedNode root    = { .category = RENAME,  .lexemes = {"rename"},    .token_count = 1,
	                         .left = &name, .right = &project };

	assert_tree_node(tree.head->root, &root);
}

void test_parse_project_missing_bracket(void)
{
	bool has_error = false;
	parse_source("project(A)", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_project_empty_attribute_list(void)
{
	bool has_error = false;
	parse_source("project[](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_project_missing_attribute_after_comma(void)
{
	bool has_error = false;
	parse_source("project[A,](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_project_missing_closing_bracket(void)
{
	bool has_error = false;
	parse_source("project[A(R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_project_missing_parenthesis(void)
{
	bool has_error = false;
	parse_source("project[A]R", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_project_missing_closing_parenthesis(void)
{
	bool has_error = false;
	parse_source("project[A](R", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_rename_missing_bracket(void)
{
	bool has_error = false;
	parse_source("rename(R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_rename_empty_identifier(void)
{
	bool has_error = false;
	parse_source("rename[](R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_rename_missing_closing_bracket(void)
{
	bool has_error = false;
	parse_source("rename[A(R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_rename_missing_parenthesis(void)
{
	bool has_error = false;
	parse_source("rename[A]R", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_rename_missing_closing_parenthesis(void)
{
	bool has_error = false;
	parse_source("rename[A](R", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_union(void)
{
	bool has_error = false;
	Tree tree = parse_source("R union S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode root = {
		.category = UNION,
		.lexemes = {"union"},
		.token_count = 1,
		.left = &r,
		.right = &s
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_intersect(void)
{
	bool has_error = false;
	Tree tree = parse_source("R intersect S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode root = {
		.category = INTERSECT,
		.lexemes = {"intersect"},
		.token_count = 1,
		.left = &r,
		.right = &s
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_minus(void)
{
	bool has_error = false;
	Tree tree = parse_source("R minus S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode root = {
		.category = MINUS,
		.lexemes = {"minus"},
		.token_count = 1,
		.left = &r,
		.right = &s
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_times(void)
{
	bool has_error = false;
	Tree tree = parse_source("R times S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode root = {
		.category = TIMES,
		.lexemes = {"times"},
		.token_count = 1,
		.left = &r,
		.right = &s
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_join(void)
{
	bool has_error = false;
	Tree tree = parse_source("R join[A=B] S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode relations = {
		.category = JOIN_RELATIONS,
		.token_count = 1,
		.lexemes = {"JOIN_RELATIONS"},
		.left = &r,
		.right = &s
	};

	ExpectedNode a = {
		.category = IDENT,
		.lexemes = {"A"},
		.token_count = 1
	};

	ExpectedNode b = {
		.category = IDENT,
		.lexemes = {"B"},
		.token_count = 1
	};

	ExpectedNode condition = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &a,
		.right = &b
	};

	ExpectedNode root = {
		.category = JOIN,
		.lexemes = {"join"},
		.token_count = 1,
		.left = &relations,
		.right = &condition
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_join_complex_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source(
		"R join[A=B and C>10] S",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode relations = {
		.category = JOIN_RELATIONS,
		.lexemes = {"JOIN_RELATIONS"},
		.token_count = 1,
		.left = &r,
		.right = &s
	};

	ExpectedNode a = {
		.category = IDENT,
		.lexemes = {"A"},
		.token_count = 1
	};

	ExpectedNode b = {
		.category = IDENT,
		.lexemes = {"B"},
		.token_count = 1
	};

	ExpectedNode c = {
		.category = IDENT,
		.lexemes = {"C"},
		.token_count = 1
	};

	ExpectedNode ten = {
		.category = NUMBER,
		.lexemes = {"10"},
		.token_count = 1
	};

	ExpectedNode equal = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &a,
		.right = &b
	};

	ExpectedNode greater = {
		.category = GREATER_THAN,
		.lexemes = {">"},
		.token_count = 1,
		.left = &c,
		.right = &ten
	};

	ExpectedNode and = {
		.category = AND,
		.lexemes = {"and"},
		.token_count = 1,
		.left = &equal,
		.right = &greater
	};

	ExpectedNode root = {
		.category = JOIN,
		.lexemes = {"join"},
		.token_count = 1,
		.left = &relations,
		.right = &and
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_join_qualified_attributes(void)
{
	bool has_error = false;
	Tree tree = parse_source(
		"R join[R.id=S.id] S",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode relations = {
		.category = JOIN_RELATIONS,
		.lexemes = {"JOIN_RELATIONS"},
		.token_count = 1,
		.left = &r,
		.right = &s
	};

	ExpectedNode left_attr = {
		.category = IDENT,
		.lexemes = {"R", "id"},
		.token_count = 2
	};

	ExpectedNode right_attr = {
		.category = IDENT,
		.lexemes = {"S", "id"},
		.token_count = 2
	};

	ExpectedNode condition = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &left_attr,
		.right = &right_attr
	};

	ExpectedNode root = {
		.category = JOIN,
		.lexemes = {"join"},
		.token_count = 1,
		.left = &relations,
		.right = &condition
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_join_missing_bracket(void)
{
	bool has_error = false;
	parse_source("R join A=B] S", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_empty_condition(void)
{
	bool has_error = false;
	parse_source("R join[] S", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_missing_closing_bracket(void)
{
	bool has_error = false;
	parse_source("R join[A=B S", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_missing_right_expression(void)
{
	bool has_error = false;
	parse_source("R join[A=B]", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_dangling_condition_operator(void)
{
	bool has_error = false;
	parse_source("R join[A=] S", &has_error);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_binary_left_associativity(void)
{
	bool has_error = false;
	Tree tree = parse_source("R union S union T", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = { .category = IDENT, .lexemes = {"R"}, .token_count = 1 };
	ExpectedNode s = { .category = IDENT, .lexemes = {"S"}, .token_count = 1 };
	ExpectedNode t = { .category = IDENT, .lexemes = {"T"}, .token_count = 1 };

	ExpectedNode union_rs = {
		.category = UNION, .lexemes = {"union"}, .token_count = 1,
		.left = &r, .right = &s
	};

	ExpectedNode root = {
		.category = UNION, .lexemes = {"union"}, .token_count = 1,
		.left = &union_rs, .right = &t
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_binary_precedence(void)
{
	bool has_error = false;
	Tree tree = parse_source("R union S times T", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = { .category = IDENT, .lexemes = {"R"}, .token_count = 1 };
	ExpectedNode s = { .category = IDENT, .lexemes = {"S"}, .token_count = 1 };
	ExpectedNode t = { .category = IDENT, .lexemes = {"T"}, .token_count = 1 };

	ExpectedNode times = {
		.category = TIMES, .lexemes = {"times"}, .token_count = 1,
		.left = &s, .right = &t
	};

	ExpectedNode root = {
		.category = UNION, .lexemes = {"union"}, .token_count = 1,
		.left = &r, .right = &times
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_unary_binary_precedence(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[A=1](R) union S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode a = { .category = IDENT, .lexemes = {"A"}, .token_count = 1 };
	ExpectedNode one = { .category = NUMBER, .lexemes = {"1"}, .token_count = 1 };

	ExpectedNode condition = {
		.category = EQUAL, .lexemes = {"="}, .token_count = 1,
		.left = &a, .right = &one
	};

	ExpectedNode r = { .category = IDENT, .lexemes = {"R"}, .token_count = 1 };

	ExpectedNode select = {
		.category = SELECT, .lexemes = {"select"}, .token_count = 1,
		.left = &condition, .right = &r
	};

	ExpectedNode s = { .category = IDENT, .lexemes = {"S"}, .token_count = 1 };

	ExpectedNode root = {
		.category = UNION, .lexemes = {"union"}, .token_count = 1,
		.left = &select, .right = &s
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_parentheses_override_precedence(void)
{
	bool has_error = false;
	Tree tree = parse_source("(R union S) times T", &has_error);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = { .category = IDENT, .lexemes = {"R"}, .token_count = 1 };
	ExpectedNode s = { .category = IDENT, .lexemes = {"S"}, .token_count = 1 };
	ExpectedNode t = { .category = IDENT, .lexemes = {"T"}, .token_count = 1 };

	ExpectedNode union_rs = {
		.category = UNION, .lexemes = {"union"}, .token_count = 1,
		.left = &r, .right = &s
	};

	ExpectedNode root = {
		.category = TIMES, .lexemes = {"times"}, .token_count = 1,
		.left = &union_rs, .right = &t
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_relation_definition(void)
{
	bool has_error = false;

	Tree tree = parse_source(
		"Employees (EID, Name, Age, DID) = {\n"
		"E1, John, 32, D1\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode name = {
		.category = IDENT,
		.lexemes = {"Employees"},
		.token_count = 1
	};

	ExpectedNode attributes = {
		.category = ATTRIBUTE_LIST,
		.lexemes = {"EID", "Name", "Age", "DID"},
		.token_count = 4
	};

	ExpectedNode definition = {
		.category = RELATION_DEFINITION,
		.lexemes = {NULL},
		.token_count = 1,
		.left = &name,
		.right = &attributes
	};

	ExpectedNode tuple = {
		.category = TUPLE,
		.lexemes = {"E1", "John", "32", "D1"},
		.token_count = 4
	};

	ExpectedNode root = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &definition,
		.right = &tuple
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_relation_definition_multiple_tuples(void)
{
	bool has_error = false;

	Tree tree = parse_source(
		"Employees (EID, Name, Age, DID) = {\n"
		"E1, John, 32, D1\n"
		"E2, Alice, 28, D2\n"
		"E3, Bob, 29, D1\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode name = {
		.category = IDENT,
		.lexemes = {"Employees"},
		.token_count = 1
	};

	ExpectedNode attributes = {
		.category = ATTRIBUTE_LIST,
		.lexemes = {"EID", "Name", "Age", "DID"},
		.token_count = 4
	};

	ExpectedNode definition = {
		.category = RELATION_DEFINITION,
		.lexemes = {NULL},
		.token_count = 1,
		.left = &name,
		.right = &attributes
	};

	ExpectedNode tuple3 = {
		.category = TUPLE,
		.lexemes = {"E3", "Bob", "29", "D1"},
		.token_count = 4
	};

	ExpectedNode tuple2 = {
		.category = TUPLE,
		.lexemes = {"E2", "Alice", "28", "D2"},
		.token_count = 4,
		.left = &tuple3
	};

	ExpectedNode tuple1 = {
		.category = TUPLE,
		.lexemes = {"E1", "John", "32", "D1"},
		.token_count = 4,
		.left = &tuple2
	};

	ExpectedNode root = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &definition,
		.right = &tuple1
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_relation_definition_single_attribute(void)
{
	bool has_error = false;

	Tree tree = parse_source(
		"R (A) = {\n"
		"1\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode name = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode attributes = {
		.category = ATTRIBUTE_LIST,
		.lexemes = {"A"},
		.token_count = 1
	};

	ExpectedNode definition = {
		.category = RELATION_DEFINITION,
		.lexemes = {NULL},
		.token_count = 1,
		.left = &name,
		.right = &attributes
	};

	ExpectedNode tuple = {
		.category = TUPLE,
		.lexemes = {"1"},
		.token_count = 1
	};

	ExpectedNode root = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &definition,
		.right = &tuple
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_relation_definition_tuple_values(void)
{
	bool has_error = false;

	Tree tree = parse_source(
		"R (A, B, C, D) = {\n"
		"123, 'hello', 45.67, foo\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode name = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode attributes = {
		.category = ATTRIBUTE_LIST,
		.lexemes = {"A", "B", "C", "D"},
		.token_count = 4
	};

	ExpectedNode definition = {
		.category = RELATION_DEFINITION,
		.lexemes = {NULL},
		.token_count = 1,
		.left = &name,
		.right = &attributes
	};

	ExpectedNode tuple = {
		.category = TUPLE,
		.lexemes = {"123", "'hello'", "45.67", "foo"},
		.token_count = 4
	};

	ExpectedNode root = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &definition,
		.right = &tuple
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_relation_definition_keyword_attribute(void)
{
	bool has_error = false;

	Tree tree = parse_source(
		"R (select, union, Age) = {\n"
		"1, 2, 3\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode name = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode attributes = {
		.category = ATTRIBUTE_LIST,
		.lexemes = {"select", "union", "Age"},
		.token_count = 3
	};

	ExpectedNode definition = {
		.category = RELATION_DEFINITION,
		.lexemes = {NULL},
		.token_count = 1,
		.left = &name,
		.right = &attributes
	};

	ExpectedNode tuple = {
		.category = TUPLE,
		.lexemes = {"1", "2", "3"},
		.token_count = 3
	};

	ExpectedNode root = {
		.category = EQUAL,
		.lexemes = {"="},
		.token_count = 1,
		.left = &definition,
		.right = &tuple
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_relation_definition_keyword_relation_name(void)
{
	bool has_error = false;

	parse_source(
		"select (A) = {\n"
		"1\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_open_paren(void)
{
	bool has_error = false;
	parse_source("R A) = { 1 }\n", &has_error);
	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_attribute(void)
{
	bool has_error = false;
	parse_source("R () = { 1 }\n", &has_error);
	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_comma(void)
{
	bool has_error = false;
	parse_source("R (A B) = { 1, 2 }\n", &has_error);
	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_close_paren(void)
{
	bool has_error = false;
	parse_source("R (A, B = { 1, 2 }\n", &has_error);
	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_equals(void)
{
	bool has_error = false;
	parse_source("R (A, B) { 1, 2 }\n", &has_error);
	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_open_brace(void)
{
	bool has_error = false;
	parse_source("R (A, B) = 1, 2\n", &has_error);
	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_close_brace(void)
{
	bool has_error = false;
	parse_source(
		"R (A, B) = {\n"
		"1, 2\n",
		&has_error
	);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_invalid_attribute(void)
{
	bool has_error = false;
	parse_source(
		"R (A, 123) = {\n"
		"1, 2\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_empty_attribute_list(void)
{
	bool has_error = false;
	parse_source(
		"R () = {\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_value(void)
{
	bool has_error = false;
	parse_source(
		"R (A, B) = {\n"
		"1,\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_too_few_values(void)
{
	bool has_error = false;
	parse_source(
		"R (A, B, C) = {\n"
		"1, 2\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_too_many_values(void)
{
	bool has_error = false;
	parse_source(
		"R (A, B) = {\n"
		"1, 2, 3\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_identifier_expression_not_relation_definition(void)
{
	bool has_error = false;

	Tree tree = parse_source(
		"R union S\n",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	ExpectedNode r = {
		.category = IDENT,
		.lexemes = {"R"},
		.token_count = 1
	};

	ExpectedNode s = {
		.category = IDENT,
		.lexemes = {"S"},
		.token_count = 1
	};

	ExpectedNode root = {
		.category = UNION,
		.lexemes = {"union"},
		.token_count = 1,
		.left = &r,
		.right = &s
	};

	assert_tree_node(tree.head->root, &root);
}

void test_parse_relation_definition_dispatch(void)
{
	bool has_error = false;

	Tree tree = parse_source(
		"R (A) = {\n"
		"1\n"
		"}\n",
		&has_error
	);

	TEST_ASSERT_FALSE(has_error);

	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(
		EQUAL,
		tree.head->root->token_arr[0]->category
	);
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

    RUN_TEST(test_parse_project_single_attribute);
    RUN_TEST(test_parse_project_multiple_attributes);
    RUN_TEST(test_parse_project_keyword_as_attribute);
    RUN_TEST(test_parse_project_nested_select);
    RUN_TEST(test_parse_project_missing_bracket);
    RUN_TEST(test_parse_project_empty_attribute_list);
    RUN_TEST(test_parse_project_missing_attribute_after_comma);
    RUN_TEST(test_parse_project_missing_closing_bracket);
    RUN_TEST(test_parse_project_missing_parenthesis);
    RUN_TEST(test_parse_project_missing_closing_parenthesis);

    RUN_TEST(test_parse_rename);
    RUN_TEST(test_parse_rename_nested_project);
    RUN_TEST(test_parse_rename_missing_bracket);
    RUN_TEST(test_parse_rename_empty_identifier);
    RUN_TEST(test_parse_rename_missing_closing_bracket);
    RUN_TEST(test_parse_rename_missing_parenthesis);
    RUN_TEST(test_parse_rename_missing_closing_parenthesis);

    RUN_TEST(test_parse_union);
    RUN_TEST(test_parse_intersect);
    RUN_TEST(test_parse_minus);
    RUN_TEST(test_parse_times);

    RUN_TEST(test_parse_join);
    RUN_TEST(test_parse_join_complex_condition);
    RUN_TEST(test_parse_join_qualified_attributes);
    RUN_TEST(test_parse_join_missing_bracket);
    RUN_TEST(test_parse_join_empty_condition);
    RUN_TEST(test_parse_join_missing_closing_bracket);
    RUN_TEST(test_parse_join_missing_right_expression);
    RUN_TEST(test_parse_join_dangling_condition_operator);

    RUN_TEST(test_parse_binary_left_associativity);
    RUN_TEST(test_parse_binary_precedence);
    RUN_TEST(test_parse_unary_binary_precedence);
    RUN_TEST(test_parse_parentheses_override_precedence);

    RUN_TEST(test_parse_relation_definition);
    RUN_TEST(test_parse_relation_definition_multiple_tuples);
    RUN_TEST(test_parse_relation_definition_single_attribute);
    RUN_TEST(test_parse_relation_definition_tuple_values);
    RUN_TEST(test_parse_relation_definition_keyword_attribute);
    RUN_TEST(test_parse_relation_definition_keyword_relation_name);
    RUN_TEST(test_parse_relation_definition_missing_open_paren);
    RUN_TEST(test_parse_relation_definition_missing_attribute);
    RUN_TEST(test_parse_relation_definition_missing_comma);
    RUN_TEST(test_parse_relation_definition_missing_close_paren);
    RUN_TEST(test_parse_relation_definition_missing_equals);
    RUN_TEST(test_parse_relation_definition_missing_open_brace);
    RUN_TEST(test_parse_relation_definition_missing_close_brace);
    RUN_TEST(test_parse_relation_definition_invalid_attribute);
    RUN_TEST(test_parse_relation_definition_empty_attribute_list);
    RUN_TEST(test_parse_relation_definition_missing_value);
    RUN_TEST(test_parse_relation_definition_too_few_values);
    RUN_TEST(test_parse_relation_definition_too_many_values);
    RUN_TEST(test_parse_identifier_expression_not_relation_definition);
    RUN_TEST(test_parse_relation_definition_dispatch);

    return UNITY_END();
}
