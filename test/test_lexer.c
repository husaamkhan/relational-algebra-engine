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

/* ===================== Identifiers ===================== */

void test_ident_single_letter(void)
{
	run_test(
		"R",
		1,
		1,
		(ExpectedToken[]){
			{ .category = IDENT, .lexeme = "R", .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_ident_multi_letter(void)
{
	run_test(
		"foo",
		3,
		1,
		(ExpectedToken[]){
			{ .category = IDENT, .lexeme = "foo", .lexeme_length = 3, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_ident_with_digit(void)
{
	run_test(
		"R1",
		2,
		1,
		(ExpectedToken[]){
			{ .category = IDENT, .lexeme = "R1", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_ident_with_underscore(void)
{
	run_test(
		"foo_bar",
		7,
		1,
		(ExpectedToken[]){
			{ .category = IDENT, .lexeme = "foo_bar", .lexeme_length = 7, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_ident_uppercase_keyword_spelling_is_ident(void)
{
	/* Keywords are lowercase only — "SELECT" is an identifier. */
	run_test(
		"SELECT",
		6,
		1,
		(ExpectedToken[]){
			{ .category = IDENT, .lexeme = "SELECT", .lexeme_length = 6, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_ident_keyword_prefix_with_trailing_chars_is_ident(void)
{
	/* "select2" begins with a keyword but is not an exact match — must be IDENT. */
	run_test(
		"select2",
		7,
		1,
		(ExpectedToken[]){
			{ .category = IDENT, .lexeme = "select2", .lexeme_length = 7, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_multiple_idents_separated_by_spaces(void)
{
	run_test(
		"A B C",
		5,
		3,
		(ExpectedToken[]){
			{ .category = IDENT, .lexeme = "A", .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
			{ .category = IDENT, .lexeme = "B", .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
			{ .category = IDENT, .lexeme = "C", .lexeme_length = 1, .pos = { .row = 1, .col = 5 } },
		},
		false);
}

void test_ident_adjacent_to_punctuation(void)
{
	/* No spaces — ident must stop at the first non-identifier character. */
	run_test(
		"R(A)",
		4,
		4,
		(ExpectedToken[]){
			{ .category = IDENT,  .lexeme = "R", .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
			{ .category = LPAREN, .lexeme = "(", .lexeme_length = 1, .pos = { .row = 1, .col = 2 } },
			{ .category = IDENT,  .lexeme = "A", .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
			{ .category = RPAREN, .lexeme = ")", .lexeme_length = 1, .pos = { .row = 1, .col = 4 } },
		},
		false);
}

/* ===================== Keywords ===================== */

void test_keyword_select(void)
{
	run_test(
		"select",
		6,
		1,
		(ExpectedToken[]){
			{ .category = SELECT, .lexeme = "select", .lexeme_length = 6, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_project(void)
{
	run_test(
		"project",
		7,
		1,
		(ExpectedToken[]){
			{ .category = PROJECT, .lexeme = "project", .lexeme_length = 7, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_rename(void)
{
	run_test(
		"rename",
		6,
		1,
		(ExpectedToken[]){
			{ .category = RENAME, .lexeme = "rename", .lexeme_length = 6, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_union(void)
{
	run_test(
		"union",
		5,
		1,
		(ExpectedToken[]){
			{ .category = UNION, .lexeme = "union", .lexeme_length = 5, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_intersect(void)
{
	run_test(
		"intersect",
		9,
		1,
		(ExpectedToken[]){
			{ .category = INTERSECT, .lexeme = "intersect", .lexeme_length = 9, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_minus(void)
{
	run_test(
		"minus",
		5,
		1,
		(ExpectedToken[]){
			{ .category = MINUS, .lexeme = "minus", .lexeme_length = 5, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_times(void)
{
	run_test(
		"times",
		5,
		1,
		(ExpectedToken[]){
			{ .category = TIMES, .lexeme = "times", .lexeme_length = 5, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_join(void)
{
	run_test(
		"join",
		4,
		1,
		(ExpectedToken[]){
			{ .category = JOIN, .lexeme = "join", .lexeme_length = 4, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_and(void)
{
	run_test(
		"and",
		3,
		1,
		(ExpectedToken[]){
			{ .category = AND, .lexeme = "and", .lexeme_length = 3, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_or(void)
{
	run_test(
		"or",
		2,
		1,
		(ExpectedToken[]){
			{ .category = OR, .lexeme = "or", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keyword_not(void)
{
	run_test(
		"not",
		3,
		1,
		(ExpectedToken[]){
			{ .category = NOT, .lexeme = "not", .lexeme_length = 3, .pos = { .row = 1, .col = 1 } },
		},
		false);
}

void test_keywords_in_sequence(void)
{
	run_test(
		"union intersect minus",
		21,
		3,
		(ExpectedToken[]){
			{ .category = UNION,     .lexeme = "union",     .lexeme_length = 5, .pos = { .row = 1, .col = 1  } },
			{ .category = INTERSECT, .lexeme = "intersect", .lexeme_length = 9, .pos = { .row = 1, .col = 7  } },
			{ .category = MINUS,     .lexeme = "minus",     .lexeme_length = 5, .pos = { .row = 1, .col = 17 } },
		},
		false);
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

void test_number_followed_by_ident(void)
{
	run_test(
		"42R",
		3,
		2,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "42", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
			{ .category = IDENT,  .lexeme = "R",  .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
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
			{ .category = IDENT,      .lexeme = "x",   .lexeme_length = 1, .pos = { .row = 1, .col = 1 } },
			{ .category = LESS_THAN,  .lexeme = "<",   .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
			{ .category = NUMBER,     .lexeme = "5",   .lexeme_length = 1, .pos = { .row = 1, .col = 5 } },
		},
		false);
}

void test_number_negative_followed_by_space_and_ident(void)
{
	run_test(
		"-5 x",
		4,
		2,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "-5", .lexeme_length = 2, .pos = { .row = 1, .col = 1 } },
			{ .category = IDENT,  .lexeme = "x",  .lexeme_length = 1, .pos = { .row = 1, .col = 4 } },
		},
		false);
}

void test_number_error_leading_dot(void)
{
	run_test(
		".5",
		2,
		1,
		(ExpectedToken[]){
			{ .category = NUMBER, .lexeme = "5", .lexeme_length = 1, .pos = { .row = 1, .col = 2 } },
		},
		true);
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
			{ .category = IDENT, .lexeme = "x", .lexeme_length = 1, .pos = { .row = 1, .col = 3 } },
		},
		true);
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
	RUN_TEST(test_ident_single_letter);
	RUN_TEST(test_ident_multi_letter);
	RUN_TEST(test_ident_with_digit);
	RUN_TEST(test_ident_with_underscore);
	RUN_TEST(test_ident_uppercase_keyword_spelling_is_ident);
	RUN_TEST(test_ident_keyword_prefix_with_trailing_chars_is_ident);
	RUN_TEST(test_multiple_idents_separated_by_spaces);
	RUN_TEST(test_ident_adjacent_to_punctuation);
	RUN_TEST(test_keyword_select);
	RUN_TEST(test_keyword_project);
	RUN_TEST(test_keyword_rename);
	RUN_TEST(test_keyword_union);
	RUN_TEST(test_keyword_intersect);
	RUN_TEST(test_keyword_minus);
	RUN_TEST(test_keyword_times);
	RUN_TEST(test_keyword_join);
	RUN_TEST(test_keyword_and);
	RUN_TEST(test_keyword_or);
	RUN_TEST(test_keyword_not);
	RUN_TEST(test_keywords_in_sequence);
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
	RUN_TEST(test_number_followed_by_ident);
	RUN_TEST(test_number_in_comparison);
	RUN_TEST(test_number_negative_followed_by_space_and_ident);
	RUN_TEST(test_number_error_leading_dot);
	RUN_TEST(test_number_error_trailing_dot);
	RUN_TEST(test_number_error_bare_minus);
	RUN_TEST(test_number_error_bare_minus_followed_by_non_digit);
	RUN_TEST(test_number_error_dot_without_digits_after);
	return UNITY_END();
}
