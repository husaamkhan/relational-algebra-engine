#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "common.h"

#define LEX_ERR(fmt, ...) LOG_ERR("[LEXICAL] " fmt, ##__VA_ARGS__)

/*
 * Initializes a lexer with the given input buffer.
 *
 * Input:
 *   lexer		- Lexer to initialize.
 *   file_contents 	- Pointer to the input buffer.
 *   file_size     	- Size of the input buffer in bytes.
 *
 * Output:
 *   The lexer's internal state is initialized.
 *
 * Returns:
 *   None.
 */
void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size);

/*
 * Tokenizes the input buffer into a sequence of tokens.
 *
 * Input:
 *   lexer     - Initialized lexer with input buffer.
 *   arena     - Arena to allocate tokens from.
 *   count_out - Pointer to store the number of tokens produced.
 *
 * Output:
 *   Tokens are allocated in the arena. count_out is set to the token count.
 *   lexer->has_error is set to true if any lexical errors occurred.
 *
 * Returns:
 *   None.
 */
void lex(Lexer *lexer, Arena *arena, size_t *count_out);

typedef struct
{
	const char *file_contents;
	size_t file_size;
	FilePosition pos;

	char cur_char;
	int cur_pos;

	int lexeme_start;
	bool has_error;
} Lexer;

void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size);
void lex(Lexer *lexer, Arena *arena, size_t *count_out);
