#pragma once

#include <stdbool.h>
#include "common.h"
#include <stdbool.h>

#define PARSE_ERR(fmt, ...) LOG_ERR("[PARSE] " fmt, ##__VA_ARGS__)

typedef struct
{
	Arena *token_arena;

	size_t token_count;
	size_t cur_pos;
} Parser;

void parser_init(Parser *parser, Arena *token_arena, size_t token_count);

/*
 * Parses the token stream into an AST.
 *
 * Input:
 *   parser - Initialized parser with token stream.
 *   tree   - Tree whose root will be set to the parsed AST root.
 *
 * Output:
 *   tree->root is set to the root TreeNode of the parsed AST.
 *   parser->has_error is set to true if any parse errors occurred.
 *   Errors are reported via LOG_ERR.
 *
 * Returns:
 *   None.
 */
void parse(Parser *parser, Tree *tree);
