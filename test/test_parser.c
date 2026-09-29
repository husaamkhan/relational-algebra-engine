#include <string.h>
#include "unity.h"
#include "lexer.h"
#include "parser.h"

static Arena arena;

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

static TreeNode *test_tuple(
		Arena *arena,
		const Category *categories,
		const char *const *lexemes,
		size_t value_count)
{
	TreeNode *node = tree_node_create(
			arena,
			value_count + 1,
			NULL,
			NULL,
			NULL
			);

	node->token_arr[0] = test_token(arena, TUPLE, NULL);

	for (size_t i = 0; i < value_count; i++)
	{
		node->token_arr[i + 1] = test_token(
				arena,
				categories[i],
				lexemes[i]
				);
	}

	return node;
}

static Tree parse_source(const char *src, bool *has_error_out)
{
	Lexer lexer;

	lexer_init(&lexer, src, strlen(src));

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	TEST_ASSERT_FALSE_MESSAGE(
			lexer.has_error,
			"lexer should not report errors"
			);

	Parser parser;
	parser_init(&parser, &arena, token_count);

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
	arena = arena_create(4096);
	printf("\n=== %s ===\n", Unity.CurrentTestName);
}

void tearDown(void)
{
	arena_destroy(&arena);
}

void test_parse_simple_relation(void)
{
	bool has_error = false;
	Tree tree = parse_source("selection", &has_error);

	TEST_ASSERT_FALSE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);

	const char *lexemes[] = {"selection"};

	TreeNode *expected = test_node(
			&arena,
			IDENT,
			lexemes,
			1,
			NULL,
			NULL
			);

	assert_tree_node(tree.head->root, expected);
	TEST_ASSERT_NULL(tree.head->next);
}

void test_parse_bare_select_is_error(void)
{
	bool has_error = false;
	Tree tree = parse_source("select", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);

	const char *lexemes[] = {"select"};

	TreeNode *expected = test_node(
			&arena,
			SELECT,
			lexemes,
			1,
			NULL,
			NULL
			);

	assert_tree_node(tree.head->root, expected);
}

void test_parse_bare_project_is_error(void)
{
	bool has_error = false;
	Tree tree = parse_source("project", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);

	const char *lexemes[] = {"project"};

	TreeNode *expected = test_node(
			&arena,
			PROJECT,
			lexemes,
			1,
			NULL,
			NULL
			);

	assert_tree_node(tree.head->root, expected);
}

void test_parse_bare_rename_is_error(void)
{
	bool has_error = false;
	Tree tree = parse_source("rename", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);

	const char *lexemes[] = {"rename"};

	TreeNode *expected = test_node(
			&arena,
			RENAME,
			lexemes,
			1,
			NULL,
			NULL
			);

	assert_tree_node(tree.head->root, expected);
}

void test_parse_select_simple_comparison(void)
{
	bool has_error = false;
	Tree tree = parse_source("select[Age>30](R)", &has_error);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *age = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *num30 = test_single_node(
			&arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *cond = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			age,
			num30
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			cond,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_and_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Age>30 and DID='D1'](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *age = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *num30 = test_single_node(
			&arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *left_cmp = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			age,
			num30
			);

	TreeNode *did = test_single_node(
			&arena,
			IDENT,
			"DID",
			NULL,
			NULL
			);

	TreeNode *d1 = test_single_node(
			&arena,
			STRING,
			"'D1'",
			NULL,
			NULL
			);

	TreeNode *right_cmp = test_single_node(
			&arena,
			EQUAL,
			"=",
			did,
			d1
			);

	TreeNode *and_node = test_single_node(
			&arena,
			AND,
			"and",
			left_cmp,
			right_cmp
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			and_node,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_or_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Age>30 or Age<10](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *age1 = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *num30 = test_single_node(
			&arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *left_cmp = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			age1,
			num30
			);

	TreeNode *age2 = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *num10 = test_single_node(
			&arena,
			NUMBER,
			"10",
			NULL,
			NULL
			);

	TreeNode *right_cmp = test_single_node(
			&arena,
			LESS_THAN,
			"<",
			age2,
			num10
			);

	TreeNode *or_node = test_single_node(
			&arena,
			OR,
			"or",
			left_cmp,
			right_cmp
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			or_node,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_not_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[not Age>30](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *age = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *num30 = test_single_node(
			&arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *cmp = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			age,
			num30
			);

	TreeNode *not_node = test_single_node(
			&arena,
			NOT,
			"not",
			cmp,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			not_node,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_not_parenthesized_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[not (Age>30 and DID='D1')](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *age = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *num30 = test_single_node(
			&arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *left_cmp = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			age,
			num30
			);

	TreeNode *did = test_single_node(
			&arena,
			IDENT,
			"DID",
			NULL,
			NULL
			);

	TreeNode *d1 = test_single_node(
			&arena,
			STRING,
			"'D1'",
			NULL,
			NULL
			);

	TreeNode *right_cmp = test_single_node(
			&arena,
			EQUAL,
			"=",
			did,
			d1
			);

	TreeNode *and_node = test_single_node(
			&arena,
			AND,
			"and",
			left_cmp,
			right_cmp
			);

	TreeNode *not_node = test_single_node(
			&arena,
			NOT,
			"not",
			and_node,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			not_node,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_attribute_vs_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Age=Salary](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *age = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *salary = test_single_node(
			&arena,
			IDENT,
			"Salary",
			NULL,
			NULL
			);

	TreeNode *cmp = test_single_node(
			&arena,
			EQUAL,
			"=",
			age,
			salary
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			cmp,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_qualified_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Emp.Age>30](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	const char *emp_age_lexemes[] = {"Emp", "Age"};

	TreeNode *emp_age = test_node(
			&arena,
			IDENT,
			emp_age_lexemes,
			2,
			NULL,
			NULL
			);

	TreeNode *num30 = test_single_node(
			&arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *cmp = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			emp_age,
			num30
			);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			cmp,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_keyword_as_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[union=3](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *uni = test_single_node(
			&arena,
			IDENT,
			"union",
			NULL,
			NULL
			);

	TreeNode *num3 = test_single_node(
			&arena,
			NUMBER,
			"3",
			NULL,
			NULL
			);

	TreeNode *cmp = test_single_node(
			&arena,
			EQUAL,
			"=",
			uni,
			num3
			);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			SELECT,
			"select",
			cmp,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_select_missing_bracket(void)
{
	bool has_error = false;
	Tree tree = parse_source("select(R)", &has_error);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(
			SELECT,
			tree.head->root->token_arr[0]->category
			);
}

void test_parse_select_missing_open_paren(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Age>30]R",
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

void test_parse_select_missing_close_paren(void)
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

void test_parse_select_empty_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[](R)",
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

void test_parse_select_condition_missing_operand(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Age>](R)",
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

void test_parse_select_condition_missing_operator(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Age 30](R)",
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

void test_parse_select_condition_dangling_and(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[Age>30 and](R)",
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

void test_parse_select_condition_unclosed_paren(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[(Age>30](R)",
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

void test_parse_select_condition_dangling_not(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[not](R)",
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

void test_parse_select_error_then_next_statement_recovers(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select(R)\nT",
			&has_error
			);

	TEST_ASSERT_TRUE(has_error);
	TEST_ASSERT_NOT_NULL(tree.head);
	TEST_ASSERT_NOT_NULL(tree.head->root);
	TEST_ASSERT_EQUAL_INT(
			SELECT,
			tree.head->root->token_arr[0]->category
			);

	TEST_ASSERT_NOT_NULL(tree.head->next);
	TEST_ASSERT_NOT_NULL(tree.head->next->root);

	assert_token_lexeme(
			tree.head->next->root->token_arr[0],
			"T"
			);

	TEST_ASSERT_EQUAL_INT(
			IDENT,
			tree.head->next->root->token_arr[0]->category
			);
}

void test_parse_project_single_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"project[A](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *attr = test_single_node(
			&arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			PROJECT,
			"project",
			attr,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_project_multiple_attributes(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"project[A,B,C](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	const char *attrs_lexemes[] = {"A", "B", "C"};

	TreeNode *attrs = test_node(
			&arena,
			IDENT,
			attrs_lexemes,
			3,
			NULL,
			NULL
			);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			PROJECT,
			"project",
			attrs,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_project_keyword_as_attribute(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"project[union](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *attr = test_single_node(
			&arena,
			IDENT,
			"union",
			NULL,
			NULL
			);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			PROJECT,
			"project",
			attr,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_rename(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"rename[B](R)",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *name = test_single_node(
			&arena,
			IDENT,
			"B",
			NULL,
			NULL
			);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			RENAME,
			"rename",
			name,
			r
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_project_nested_select(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"project[A,B](select[Age>30](R))",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *age = test_single_node(
			&arena,
			IDENT,
			"Age",
			NULL,
			NULL
			);

	TreeNode *num30 = test_single_node(
			&arena,
			NUMBER,
			"30",
			NULL,
			NULL
			);

	TreeNode *cmp = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			age,
			num30
			);

	TreeNode *select = test_single_node(
			&arena,
			SELECT,
			"select",
			cmp,
			r
			);

	const char *attrs_lexemes[] = {"A", "B"};

	TreeNode *attrs = test_node(
			&arena,
			IDENT,
			attrs_lexemes,
			2,
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			PROJECT,
			"project",
			attrs,
			select
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_rename_nested_project(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"rename[Employees](project[A](R))",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *attr = test_single_node(
			&arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *project = test_single_node(
			&arena,
			PROJECT,
			"project",
			attr,
			r
			);

	TreeNode *name = test_single_node(
			&arena,
			IDENT,
			"Employees",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			RENAME,
			"rename",
			name,
			project
			);

	assert_tree_node(tree.head->root, root);
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

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			UNION,
			"union",
			r,
			s
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_intersect(void)
{
	bool has_error = false;
	Tree tree = parse_source("R intersect S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			INTERSECT,
			"intersect",
			r,
			s
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_minus(void)
{
	bool has_error = false;
	Tree tree = parse_source("R minus S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			MINUS,
			"minus",
			r,
			s
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_times(void)
{
	bool has_error = false;
	Tree tree = parse_source("R times S", &has_error);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			TIMES,
			"times",
			r,
			s
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_join(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"R join[A=B] S",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *relations = test_single_node(
			&arena,
			JOIN_RELATIONS,
			"JOIN_RELATIONS",
			r,
			s
			);

	TreeNode *a = test_single_node(
			&arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *b = test_single_node(
			&arena,
			IDENT,
			"B",
			NULL,
			NULL
			);

	TreeNode *condition = test_single_node(
			&arena,
			EQUAL,
			"=",
			a,
			b
			);

	TreeNode *root = test_single_node(
			&arena,
			JOIN,
			"join",
			relations,
			condition
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_join_complex_condition(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"R join[A=B and C>10] S",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *relations = test_single_node(
			&arena,
			JOIN_RELATIONS,
			"JOIN_RELATIONS",
			r,
			s
			);

	TreeNode *a = test_single_node(
			&arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *b = test_single_node(
			&arena,
			IDENT,
			"B",
			NULL,
			NULL
			);

	TreeNode *c = test_single_node(
			&arena,
			IDENT,
			"C",
			NULL,
			NULL
			);

	TreeNode *ten = test_single_node(
			&arena,
			NUMBER,
			"10",
			NULL,
			NULL
			);

	TreeNode *equal = test_single_node(
			&arena,
			EQUAL,
			"=",
			a,
			b
			);

	TreeNode *greater = test_single_node(
			&arena,
			GREATER_THAN,
			">",
			c,
			ten
			);

	TreeNode *and_node = test_single_node(
			&arena,
			AND,
			"and",
			equal,
			greater
			);

	TreeNode *root = test_single_node(
			&arena,
			JOIN,
			"join",
			relations,
			and_node
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_join_qualified_attributes(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"R join[R.id=S.id] S",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *relations = test_single_node(
			&arena,
			JOIN_RELATIONS,
			"JOIN_RELATIONS",
			r,
			s
			);

	const char *left_attr_lexemes[] = {"R", "id"};

	TreeNode *left_attr = test_node(
			&arena,
			IDENT,
			left_attr_lexemes,
			2,
			NULL,
			NULL
			);

	const char *right_attr_lexemes[] = {"S", "id"};

	TreeNode *right_attr = test_node(
			&arena,
			IDENT,
			right_attr_lexemes,
			2,
			NULL,
			NULL
			);

	TreeNode *condition = test_single_node(
			&arena,
			EQUAL,
			"=",
			left_attr,
			right_attr
			);

	TreeNode *root = test_single_node(
			&arena,
			JOIN,
			"join",
			relations,
			condition
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_join_missing_bracket(void)
{
	bool has_error = false;
	parse_source(
			"R join A=B] S",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_empty_condition(void)
{
	bool has_error = false;
	parse_source(
			"R join[] S",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_missing_closing_bracket(void)
{
	bool has_error = false;
	parse_source(
			"R join[A=B S",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_missing_right_expression(void)
{
	bool has_error = false;
	parse_source(
			"R join[A=B]",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_join_dangling_condition_operator(void)
{
	bool has_error = false;
	parse_source(
			"R join[A=] S",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_binary_left_associativity(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"R union S union T",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *t = test_single_node(
			&arena,
			IDENT,
			"T",
			NULL,
			NULL
			);

	TreeNode *union_rs = test_single_node(
			&arena,
			UNION,
			"union",
			r,
			s
			);

	TreeNode *root = test_single_node(
			&arena,
			UNION,
			"union",
			union_rs,
			t
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_binary_precedence(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"R union S times T",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *t = test_single_node(
			&arena,
			IDENT,
			"T",
			NULL,
			NULL
			);

	TreeNode *times = test_single_node(
			&arena,
			TIMES,
			"times",
			s,
			t
			);

	TreeNode *root = test_single_node(
			&arena,
			UNION,
			"union",
			r,
			times
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_unary_binary_precedence(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"select[A=1](R) union S",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *a = test_single_node(
			&arena,
			IDENT,
			"A",
			NULL,
			NULL
			);

	TreeNode *one = test_single_node(
			&arena,
			NUMBER,
			"1",
			NULL,
			NULL
			);

	TreeNode *condition = test_single_node(
			&arena,
			EQUAL,
			"=",
			a,
			one
			);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *select = test_single_node(
			&arena,
			SELECT,
			"select",
			condition,
			r
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			UNION,
			"union",
			select,
			s
			);

	assert_tree_node(tree.head->root, root);
}

void test_parse_parentheses_override_precedence(void)
{
	bool has_error = false;
	Tree tree = parse_source(
			"(R union S) times T",
			&has_error
			);

	TEST_ASSERT_FALSE(has_error);

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *t = test_single_node(
			&arena,
			IDENT,
			"T",
			NULL,
			NULL
			);

	TreeNode *union_rs = test_single_node(
			&arena,
			UNION,
			"union",
			r,
			s
			);

	TreeNode *root = test_single_node(
			&arena,
			TIMES,
			"times",
			union_rs,
			t
			);

	assert_tree_node(tree.head->root, root);
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

	TreeNode *name = test_single_node(
			&arena,
			IDENT,
			"Employees",
			NULL,
			NULL
			);

	const char *attribute_lexemes[] = {
		"EID",
		"Name",
		"Age",
		"DID"
	};

	TreeNode *attributes = test_node(
			&arena,
			IDENT,
			attribute_lexemes,
			4,
			NULL,
			NULL
			);

	TreeNode *definition = test_single_node(
			&arena,
			RELATION_DEFINITION,
			NULL,
			name,
			attributes
			);

	const Category tuple_categories[] = {
		WORD,
		WORD,
		NUMBER,
		WORD
	};

	const char *tuple_values[] = {
		"E1",
		"John",
		"32",
		"D1"
	};

	TreeNode *tuple = test_tuple(
			&arena,
			tuple_categories,
			tuple_values,
			4
			);

	TreeNode *root = test_single_node(
			&arena,
			EQUAL,
			"=",
			definition,
			tuple
			);

	assert_tree_node(tree.head->root, root);
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

	TreeNode *name = test_single_node(
			&arena,
			IDENT,
			"Employees",
			NULL,
			NULL
			);

	const char *attribute_lexemes[] = {
		"EID",
		"Name",
		"Age",
		"DID"
	};

	TreeNode *attributes = test_node(
			&arena,
			IDENT,
			attribute_lexemes,
			4,
			NULL,
			NULL
			);

	TreeNode *definition = test_single_node(
			&arena,
			RELATION_DEFINITION,
			NULL,
			name,
			attributes
			);

	const Category tuple1_categories[] = {
		WORD,
		WORD,
		NUMBER,
		WORD
	};

	const char *tuple1_values[] = {
		"E1",
		"John",
		"32",
		"D1"
	};

	TreeNode *tuple1 = test_tuple(
			&arena,
			tuple1_categories,
			tuple1_values,
			4
			);


	const Category tuple2_categories[] = {
		WORD,
		WORD,
		NUMBER,
		WORD
	};

	const char *tuple2_values[] = {
		"E2",
		"Alice",
		"28",
		"D2"
	};

	TreeNode *tuple2 = test_tuple(
			&arena,
			tuple2_categories,
			tuple2_values,
			4
			);


	const Category tuple3_categories[] = {
		WORD,
		WORD,
		NUMBER,
		WORD
	};

	const char *tuple3_values[] = {
		"E3",
		"Bob",
		"29",
		"D1"
	};

	TreeNode *tuple3 = test_tuple(
			&arena,
			tuple3_categories,
			tuple3_values,
			4
			);


	tuple2->left_child = tuple3;
	tuple3->parent = tuple2;

	tuple1->left_child = tuple2;
	tuple2->parent = tuple1;


	TreeNode *root = test_single_node(
			&arena,
			EQUAL,
			"=",
			definition,
			tuple1
			);

	assert_tree_node(tree.head->root, root);
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

	TreeNode *name = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	const char *attribute_lexemes[] = {"A"};

	TreeNode *attributes = test_node(
			&arena,
			IDENT,
			attribute_lexemes,
			1,
			NULL,
			NULL
			);

	TreeNode *definition = test_single_node(
			&arena,
			RELATION_DEFINITION,
			NULL,
			name,
			attributes
			);

	const Category tuple_categories[] = {
		NUMBER
	};

	const char *tuple_values[] = {
		"1"
	};

	TreeNode *tuple = test_tuple(
			&arena,
			tuple_categories,
			tuple_values,
			1
			);

	TreeNode *root = test_single_node(
			&arena,
			EQUAL,
			"=",
			definition,
			tuple
			);

	assert_tree_node(tree.head->root, root);
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

	TreeNode *name = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	const char *attribute_lexemes[] = {
		"A",
		"B",
		"C",
		"D"
	};

	TreeNode *attributes = test_node(
			&arena,
			IDENT,
			attribute_lexemes,
			4,
			NULL,
			NULL
			);

	TreeNode *definition = test_single_node(
			&arena,
			RELATION_DEFINITION,
			NULL,
			name,
			attributes
			);

	const Category tuple_categories[] = {
		NUMBER,
		STRING,
		NUMBER,
		WORD
	};

	const char *tuple_values[] = {
		"123",
		"'hello'",
		"45.67",
		"foo"
	};

	TreeNode *tuple = test_tuple(
			&arena,
			tuple_categories,
			tuple_values,
			4
			);

	TreeNode *root = test_single_node(
			&arena,
			EQUAL,
			"=",
			definition,
			tuple
			);

	assert_tree_node(tree.head->root, root);
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

	TreeNode *name = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	const char *attribute_lexemes[] = {
		"select",
		"union",
		"Age"
	};

	TreeNode *attributes = test_node(
			&arena,
			IDENT,
			attribute_lexemes,
			3,
			NULL,
			NULL
			);

	TreeNode *definition = test_single_node(
			&arena,
			RELATION_DEFINITION,
			NULL,
			name,
			attributes
			);

	const Category tuple_categories[] = {
		NUMBER,
		NUMBER,
		NUMBER
	};

	const char *tuple_values[] = {
		"1",
		"2",
		"3"
	};

	TreeNode *tuple = test_tuple(
			&arena,
			tuple_categories,
			tuple_values,
			3
			);

	TreeNode *root = test_single_node(
			&arena,
			EQUAL,
			"=",
			definition,
			tuple
			);

	assert_tree_node(tree.head->root, root);
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

	parse_source(
			"R A) = { 1 }\n",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_attribute(void)
{
	bool has_error = false;

	parse_source(
			"R () = { 1 }\n",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_comma(void)
{
	bool has_error = false;

	parse_source(
			"R (A B) = { 1, 2 }\n",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_close_paren(void)
{
	bool has_error = false;

	parse_source(
			"R (A, B = { 1, 2 }\n",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_equals(void)
{
	bool has_error = false;

	parse_source(
			"R (A, B) { 1, 2 }\n",
			&has_error
		    );

	TEST_ASSERT_TRUE(has_error);
}

void test_parse_relation_definition_missing_open_brace(void)
{
	bool has_error = false;

	parse_source(
			"R (A, B) = 1, 2\n",
			&has_error
		    );

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

	TreeNode *r = test_single_node(
			&arena,
			IDENT,
			"R",
			NULL,
			NULL
			);

	TreeNode *s = test_single_node(
			&arena,
			IDENT,
			"S",
			NULL,
			NULL
			);

	TreeNode *root = test_single_node(
			&arena,
			UNION,
			"union",
			r,
			s
			);

	assert_tree_node(tree.head->root, root);
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
