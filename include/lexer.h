#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "common.h"

#define LEX_ERR(fmt, ...) LOG_ERR("[LEXICAL] " fmt, ##__VA_ARGS__)

typedef struct
{
	const char *file_contents;
	size_t file_size;
	FilePosition pos;

	char cur_char;
	int cur_pos;
	int last_accepting_pos; // TODO: is this still going to be used?

	int lexeme_start;
	bool has_error;
} Lexer;

void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size);
void lex(Lexer *lexer, Arena *arena, size_t *count_out);
