#include "common.h"
#include "parser.h"
#include <string.h>

static Token *peek(Parser *parser)
{
	if (parser->cur_pos >= parser->token_count)
	{
		return NULL;
	}
	return &parser->tokens[parser->cur_pos];
}

static Token *advance(Parser *parser)
{
	if (parser->cur_pos >= parser->token_count)
	{
		return NULL;
	}
	return &parser->tokens[parser->cur_pos++];
}

void parser_init(Parser *parser, Arena *token_arena, Arena *tree_node_arena, size_t token_count)
{
	parser->token_arena      = token_arena;
	parser->tree_node_arena  = tree_node_arena;
	parser->token_count      = token_count;
	parser->cur_pos          = 0;
	parser->tokens           = (Token *)token_arena->base;
	parser->has_error        = false;
}

void parse(Parser *parser, Tree *tree)
{
}
