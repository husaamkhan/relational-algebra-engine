#include "common.h"
#include "parser.h"

void parser_init(Parser *parser, Arena *arena, size_t count)
{
	*parser = (Parser){
		.arena = arena,
		.token_count = count,
		.cur_pos = 0
	};
}

bool peek(Parser *parser, Token *t)
{
	if (parser->cur_pos >= parser->token_count) return false;

	*t = *((Token *)parser->arena->base + parser->cur_pos + sizeof(Token));
	return true;
}

bool advance(Parser *parser)
{
	Token *t;
	if (!peek(parser, t)) return false;

	parser->cur_pos += sizeof(Token);

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
