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

static void assert_single_token(size_t token_count, Token token, Category expected_category)
{
    TEST_ASSERT_EQUAL_size_t(1, token_count);
    TEST_ASSERT_EQUAL_INT(expected_category, token.category);
    TEST_ASSERT_EQUAL_size_t(1, token.lexeme_length);
}

static void run_single_token_test(const char *src, Category expected_category)
{
    Lexer lexer;
    lexer_init(&lexer, src, 1);

    Arena arena = arena_create(256);
    size_t count = 0;
    lex(&lexer, &arena, &count);

    print_tokens((Token *)arena.base, count);
   
    Token token = ((Token *)arena.base)[0];
    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(expected_category, token.category);
    TEST_ASSERT_EQUAL_size_t(1, token.lexeme_length);
}

void setUp(void)
{
    printf("\n=== %s ===\n", Unity.CurrentTestName);
}

void tearDown(void) {}

void test_lparen(void) 		{ run_single_token_test("(", LPAREN); }
void test_rparen(void) 		{ run_single_token_test(")", RPAREN); }
void test_lbracket(void) 	{ run_single_token_test("[", LBRACKET); }
void test_rbracket(void) 	{ run_single_token_test("]", RBRACKET); }
void test_lbrace(void) 		{ run_single_token_test("{", LBRACE); }
void test_rbrace(void)		{ run_single_token_test("}", RBRACE); }
void test_comma(void)		{ run_single_token_test(",", COMMA); }
void test_equal(void)		{ run_single_token_test("=", EQUAL); }

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
    return UNITY_END();
}
