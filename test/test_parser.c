#include <string.h>
#include "unity.h"
#include "lexer.h"
#include "parser.h"

/* ==================================================
 * TEST HELPERS
 * ================================================== */

static Tree parse_source(const char *src)
{
	Lexer lexer;
	Arena token_arena = arena_create(4096);
	Arena tree_node_arena = arena_create(4096);

	lexer_init(&lexer, src, strlen(src));
	size_t token_count = 0;
	lex(&lexer, &token_arena, &token_count);

	TEST_ASSERT_FALSE_MESSAGE(lexer.has_error, "lexer should not report errors");

	Parser parser;
	parser_init(&parser, &token_arena, &tree_node_arena, token_count);

	Tree tree = (Tree){ .root = NULL };
	parse(&parser, &tree);

	return tree;
}

static void assert_token_lexeme(Token *token, const char *expected)
{
	TEST_ASSERT_NOT_NULL(token);
	TEST_ASSERT_EQUAL_size_t(strlen(expected), token->lexeme_length);
	TEST_ASSERT_EQUAL_INT(0, strncmp(token->lexeme_start, expected, token->lexeme_length));
}

void setUp(void)
{
	printf("\n=== %s ===\n", Unity.CurrentTestName);
}

void tearDown(void)
{
}

/* ==================================================
 * TEST CASES
 * ================================================== */

/*
 * Grammar rule: atom_expr = identifier | "(" query_expression ")"
 * Input "R" should parse as a single-identifier atom.
 */
void test_parse_simple_relation(void)
{
	Tree tree = parse_source("R");

	TEST_ASSERT_NOT_NULL(tree.root);
	TEST_ASSERT_EQUAL_size_t(1, tree.root->token_count);
	assert_token_lexeme(tree.root->token_arr[0], "R");
	TEST_ASSERT_NULL(tree.root->left_child);
	TEST_ASSERT_NULL(tree.root->right_child);
}

/* ==================================================
 * MAIN
 * ================================================== */

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_parse_simple_relation);
	return UNITY_END();
}
