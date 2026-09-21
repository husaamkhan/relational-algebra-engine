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
	[EQUAL]                 = "EQUAL",
	[NOT_EQUAL]             = "NOT_EQUAL",
	[LESS_THAN]             = "LESS_THAN",
	[LESS_THAN_OR_EQUAL]    = "LESS_THAN_OR_EQUAL",
	[GREATER_THAN]          = "GREATER_THAN",
	[GREATER_THAN_OR_EQUAL] = "GREATER_THAN_OR_EQUAL",
	[NUMBER]                = "NUMBER",
	[STRING]                = "STRING",
	[IDENT]                 = "IDENT",
	[SELECT]                = "SELECT",
	[PROJECT]               = "PROJECT",
	[RENAME]                = "RENAME",
	[UNION]                 = "UNION",
	[INTERSECT]             = "INTERSECT",
	[MINUS]                 = "MINUS",
	[TIMES]                 = "TIMES",
	[JOIN]                  = "JOIN",
	[AND]                   = "AND",
	[OR]                    = "OR",
	[NOT]                   = "NOT",
	[NEWLINE]               = "NEWLINE",
};

typedef struct
{
    Category category;
    const char *lexeme;
    size_t lexeme_length;
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

static void run_test(const char *src, size_t input_length, size_t expected_token_count, ExpectedToken *expected_tokens)
{
	Lexer lexer;
	lexer_init(&lexer, src, input_length);

	Arena arena = arena_create(256);
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
	}
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 2
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 2
			}
		});
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
				.lexeme_length = 1
			}
		});
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
				.lexeme_length = 2
			}
		});
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
	return UNITY_END();
}
