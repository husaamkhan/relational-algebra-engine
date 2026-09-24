#include "common.h"
#include "parser.h"

void parser_init( Parser *parser, Arena *token_arena, Arena *tree_arena, size_t token_count)
{
	*parser = (Parser){
		.token_arena = token_arena,
		.tree_arena = tree_arena,
		.token_count = token_count,
		.cur_pos = 0,
		.cur_token = NULL
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
// void parse(Parser *parser, Arena *arena, size_t token_count, Tree *tree)
void parse(Parser *parser, Arena *arena, size_t token_count)
{
	for (size_t i; i <= token_count; i++)
	{
		advance(parser);

		// Expect query-expression
		if (parser->cur_token->category == LPAREN)
		{
			LOG("asdf");
		}
	}
}
