#include "common.h"
#include "parser.h"
#include <string.h>

void parser_init( Parser *parser, Arena *token_arena, size_t token_count)
{
	*parser = (Parser){
		.token_arena = token_arena,
		.token_count = token_count,
		.cur_pos = 0,
	};
}

bool peek(Parser *parser, Token *token)
{
	if (parser->cur_pos >= parser->token_count)
		return false;

	*token = *(Token *)(parser->token_arena->base +
		parser->cur_pos * sizeof(Token));

	return true;
}

bool advance(Parser *parser)
{
	if (parser->cur_pos >= parser->token_count)
		return false;

	parser->cur_token = (Token *)(parser->token_arena->base +
		parser->cur_pos * sizeof(Token));

	parser->cur_pos++;

	return true;
}



void parse(Parser *parser, Tree *tree)
{
}
