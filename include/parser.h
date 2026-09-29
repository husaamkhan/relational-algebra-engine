#pragma once

#include <stdbool.h>
#include "common.h"

#define SYNTAX_ERR(fmt, ...) LOG_ERR("[SYNTAX] " fmt, ##__VA_ARGS__)

typedef struct
{
	Arena *arena;
	size_t token_count;

	Token *cur_token;
	size_t cur_pos;

	bool has_error;
	bool statement_has_error;
} Parser;

void parser_init(Parser *parser, Arena *arena, size_t token_count);

/*
 * Parses the token stream into an AST.
 *
 * Input:
 *   parser - Initialized parser with token stream.
 *   tree   - Tree whose root will be set to the parsed AST root.
 *
 * Output:
 *   tree->head/tree->tail are populated with one StatementNode per
 *   parsed statement. parser->has_error is set to true if any parse
 *   errors occurred. Errors are reported via LOG_ERR.
 *
 * Returns:
 *   None.
 */
void parse(Parser *parser, Tree *tree);
void print_tree(const Tree *tree);
