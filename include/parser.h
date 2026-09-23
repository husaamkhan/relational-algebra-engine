#pragma once

#include "common.h"

typedef struct
{
	/* TODO: add parser fields */
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
