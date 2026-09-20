#pragma once

#include <stdlib.h>
#include "common.h"

typedef struct
{
	int row;
	int col;
} FilePosition;

typedef struct
{
	const char *file_contents;
	size_t file_size;
	FilePosition pos;
	
	char cur_char;
	int cur_pos;
	int last_accepting_pos;

	int lexeme_start;
	// Category category;
} Lexer;

void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size);
// void lex(Lexer *lexer, Arena *arena, size_t *count_out);
void lex(Lexer *lexer);
