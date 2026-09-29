#include "lexer.h"
#include "unity.h"
#include <string.h>

static const char *CATEGORY_NAMES[] = {
	[LPAREN] = "LPAREN",
	[RPAREN] = "RPAREN",
	[LBRACKET] = "LBRACKET",
	[RBRACKET] = "RBRACKET",
	[LBRACE] = "LBRACE",
	[RBRACE] = "RBRACE",
	[COMMA] = "COMMA",
	[DOT] = "DOT",
	[EQUAL] = "EQUAL",
	[NOT_EQUAL] = "NOT_EQUAL",
	[LESS_THAN] = "LESS_THAN",
	[LESS_THAN_OR_EQUAL] = "LESS_THAN_OR_EQUAL",
	[GREATER_THAN] = "GREATER_THAN",
	[GREATER_THAN_OR_EQUAL] = "GREATER_THAN_OR_EQUAL",
	[NUMBER] = "NUMBER",
	[STRING] = "STRING",
	[WORD] = "WORD",
	[NEWLINE] = "NEWLINE",
	[COMMENT] = "COMMENT"
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
	printf("Token stream (%zu token%s):\n",
			count, count == 1 ? "" : "s");

	for (size_t i = 0; i < count; i++)
	{
		printf("  [%zu] %-22s lexeme='%.*s' pos=%d:%d\n",
				i,
				CATEGORY_NAMES[tokens[i].category],
				(int)tokens[i].lexeme_length,
				tokens[i].lexeme_start,
				tokens[i].pos.row,
				tokens[i].pos.col);
	}
}

static void run_test(
		const char *src,
		size_t input_length,
		size_t expected_token_count,
		ExpectedToken *expected_tokens,
		bool expect_error)
{
	Lexer lexer;
	lexer_init(&lexer, src, input_length);

	Arena arena = arena_create(4096);
	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;
	print_tokens(tokens, token_count);

	TEST_ASSERT_EQUAL_size_t(expected_token_count, token_count);

	for (size_t i = 0; i < expected_token_count; i++)
	{
		TEST_ASSERT_EQUAL_INT(
				expected_tokens[i].category,
				tokens[i].category
				);

		TEST_ASSERT_EQUAL_size_t(
				expected_tokens[i].lexeme_length,
				tokens[i].lexeme_length
				);

		TEST_ASSERT_EQUAL_INT(
				0,
				strncmp(
					expected_tokens[i].lexeme,
					tokens[i].lexeme_start,
					expected_tokens[i].lexeme_length
					)
				);

		TEST_ASSERT_EQUAL_INT(
				expected_tokens[i].pos.row,
				tokens[i].pos.row
				);

		TEST_ASSERT_EQUAL_INT(
				expected_tokens[i].pos.col,
				tokens[i].pos.col
				);
	}

	TEST_ASSERT(expect_error == lexer.has_error);

	arena_destroy(&arena);
}

void setUp(void) { printf("\n=== %s ===\n", Unity.CurrentTestName); }
void tearDown(void) {}


/* 1. select[x1=3](R) */
void test_spec_lexer_1(void)
{
	const char *src = "select[x1=3](R)";

	ExpectedToken expected[] = {
		{WORD,     "select", 6, {1, 1}},
		{LBRACKET, "[",      1, {1, 7}},
		{WORD,     "x1",     2, {1, 8}},
		{EQUAL,    "=",      1, {1, 10}},
		{NUMBER,   "3",      1, {1, 11}},
		{RBRACKET, "]",      1, {1, 12}},
		{LPAREN,   "(",      1, {1, 13}},
		{WORD,     "R",      1, {1, 14}},
		{RPAREN,   ")",      1, {1, 15}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 2. select[ x1 = 3 ](R) */
void test_spec_lexer_2(void)
{
	const char *src = "select[ x1 = 3 ](R)";

	ExpectedToken expected[] = {
		{WORD,     "select", 6, {1, 1}},
		{LBRACKET, "[",      1, {1, 7}},
		{WORD,     "x1",     2, {1, 9}},
		{EQUAL,    "=",      1, {1, 12}},
		{NUMBER,   "3",      1, {1, 14}},
		{RBRACKET, "]",      1, {1, 16}},
		{LPAREN,   "(",      1, {1, 17}},
		{WORD,     "R",      1, {1, 18}},
		{RPAREN,   ")",      1, {1, 19}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 3. select[Age>=30](R) */
void test_spec_lexer_3(void)
{
	const char *src = "select[Age>=30](R)";

	ExpectedToken expected[] = {
		{WORD,                  "select", 6, {1, 1}},
		{LBRACKET,              "[",      1, {1, 7}},
		{WORD,                  "Age",    3, {1, 8}},
		{GREATER_THAN_OR_EQUAL, ">=",     2, {1, 11}},
		{NUMBER,                "30",     2, {1, 13}},
		{RBRACKET,              "]",      1, {1, 15}},
		{LPAREN,                "(",      1, {1, 16}},
		{WORD,                  "R",      1, {1, 17}},
		{RPAREN,                ")",      1, {1, 18}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 4. select[Age>-30](R) */
void test_spec_lexer_4(void)
{
	const char *src = "select[Age>-30](R)";

	ExpectedToken expected[] = {
		{WORD,         "select", 6, {1, 1}},
		{LBRACKET,     "[",      1, {1, 7}},
		{WORD,         "Age",    3, {1, 8}},
		{GREATER_THAN, ">",      1, {1, 11}},
		{NUMBER,       "-30",    3, {1, 12}},
		{RBRACKET,     "]",      1, {1, 15}},
		{LPAREN,       "(",      1, {1, 16}},
		{WORD,         "R",      1, {1, 17}},
		{RPAREN,       ")",      1, {1, 18}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 5. select[Name='Bob)'](R) */
void test_spec_lexer_5(void)
{
	const char *src = "select[Name='Bob)'](R)";

	ExpectedToken expected[] = {
		{WORD,     "select", 6, {1, 1}},
		{LBRACKET, "[",      1, {1, 7}},
		{WORD,     "Name",   4, {1, 8}},
		{EQUAL,    "=",      1, {1, 12}},
		{STRING,   "'Bob)'", 6, {1, 13}},
		{RBRACKET, "]",      1, {1, 19}},
		{LPAREN,   "(",      1, {1, 20}},
		{WORD,     "R",      1, {1, 21}},
		{RPAREN,   ")",      1, {1, 22}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 6. select[Name='a,b'](R) */
void test_spec_lexer_6(void)
{
	const char *src = "select[Name='a,b'](R)";

	ExpectedToken expected[] = {
		{WORD,     "select", 6, {1, 1}},
		{LBRACKET, "[",      1, {1, 7}},
		{WORD,     "Name",   4, {1, 8}},
		{EQUAL,    "=",      1, {1, 12}},
		{STRING,   "'a,b'",  5, {1, 13}},
		{RBRACKET, "]",      1, {1, 18}},
		{LPAREN,   "(",      1, {1, 19}},
		{WORD,     "R",      1, {1, 20}},
		{RPAREN,   ")",      1, {1, 21}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 7. select[Name='O''Brien'](R) */
void test_spec_lexer_7(void)
{
	const char *src = "select[Name='O''Brien'](R)";

	ExpectedToken expected[] = {
		{WORD,     "select",     6, {1, 1}},
		{LBRACKET, "[",          1, {1, 7}},
		{WORD,     "Name",       4, {1, 8}},
		{EQUAL,    "=",           1, {1, 12}},
		{STRING,   "'O''Brien'", 10, {1, 13}},
		{RBRACKET, "]",           1, {1, 23}},
		{LPAREN,   "(",           1, {1, 24}},
		{WORD,     "R",           1, {1, 25}},
		{RPAREN,   ")",           1, {1, 26}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 8. select[union=3](R) */
void test_spec_lexer_8(void)
{
	const char *src = "select[union=3](R)";

	ExpectedToken expected[] = {
		{WORD,     "select", 6, {1, 1}},
		{LBRACKET, "[",      1, {1, 7}},
		{WORD,     "union",  5, {1, 8}},
		{EQUAL,    "=",      1, {1, 13}},
		{NUMBER,   "3",      1, {1, 14}},
		{RBRACKET, "]",      1, {1, 15}},
		{LPAREN,   "(",      1, {1, 16}},
		{WORD,     "R",      1, {1, 17}},
		{RPAREN,   ")",      1, {1, 18}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, false);
}


/* 9. select[Name='Bob](R) */
void test_spec_lexer_9(void)
{
	const char *src = "select[Name='Bob](R)";

	ExpectedToken expected[] = {
		{WORD,     "select", 6, {1, 1}},
		{LBRACKET, "[",      1, {1, 7}},
		{WORD,     "Name",   4, {1, 8}},
		{EQUAL,    "=",      1, {1, 12}}
	};

	run_test(src, strlen(src),
			sizeof(expected) / sizeof(expected[0]),
			expected, true);
}


int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_spec_lexer_1);
	RUN_TEST(test_spec_lexer_2);
	RUN_TEST(test_spec_lexer_3);
	RUN_TEST(test_spec_lexer_4);
	RUN_TEST(test_spec_lexer_5);
	RUN_TEST(test_spec_lexer_6);
	RUN_TEST(test_spec_lexer_7);
	RUN_TEST(test_spec_lexer_8);
	RUN_TEST(test_spec_lexer_9);

	return UNITY_END();
}
