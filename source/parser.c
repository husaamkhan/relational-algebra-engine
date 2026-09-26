#include "common.h"
#include "parser.h"
#include <string.h>

static Token *peek(Parser *parser)
{
	if (parser->cur_pos >= parser->token_count)
	{
		return NULL;
	}
	return parser->cur_token;
}

static Token *advance(Parser *parser)
{
	if (parser->cur_pos >= parser->token_count)
	{
		return NULL;
	}
	Token *token = parser->cur_token;
	parser->cur_token++;
	parser->cur_pos++;
	return token;
}

void parser_init(Parser *parser, Arena *token_arena, size_t token_count, Arena *node_arena)
{
	parser->token_arena  = token_arena;
	parser->token_count  = token_count;
	parser->cur_pos      = 0;
	parser->node_arena   = node_arena;
	parser->cur_token    = (Token *)token_arena->base;
}

void parse(Parser *parser, Tree *tree)
{
	tree->root = NULL;

	while (peek(parser) != NULL)
	{
		Token *token = peek(parser);

		if (token->category != WORD && token->category != LPAREN)
		{
			PARSE_ERR("Expected identifier or '(' at %d:%d (got '%.*s')",
			          token->pos.row, token->pos.col,
			          (int)token->lexeme_length, token->lexeme_start);
			advance(parser);
			continue;
		}

		if (token->category == LPAREN)
		{
			/* TODO: handle "(" query_expression ")" */
			advance(parser);
			continue;
		}

		/*
		 * token->category == WORD: for now, treat every WORD as a simple
		 * relation identifier. Later, this needs to match the lexeme
		 * against the keyword list to decide whether it's actually a
		 * keyword (select/project/rename/union/...) rather than an
		 * identifier.
		 */
		Token *identifier = advance(parser);
		TreeNode *node = tree_node_create(parser->node_arena, 1, NULL, NULL, NULL);
		node->token_arr[0] = identifier;

		tree->root = node;
	}
}
