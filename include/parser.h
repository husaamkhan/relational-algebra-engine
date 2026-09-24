#pragma once

#include "common.h"
#include <stdbool.h>

typedef struct
{
	Arena *arena;
	size_t token_count;

	int cur_pos;
	Token *cur_token;
} Parser;

/*
 * Parses the token stream into an AST.
 *
 * Input:
 *   parser - Initialized parser with token stream.
 *
 * Output:
 *   The parser's internal state is updated with the parse result.
 *   Errors are reported via LOG_ERR.
 *
 * Returns:
 *   None.
 */
//void parse(Parser *parser, Arena *arena, size_t token_count, Tree *tree);
void parse(Parser *parser, Arena *arena, size_t token_count);
