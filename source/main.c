#include "common.h"
#include "lexer.h"
#include "parser.h"

int main()
{
	Lexer lexer;
	Arena token_arena = arena_create(1024);
	size_t token_count = 0;

	lex(&lexer, &token_arena, &token_count);

	if (lexer.has_error)
	{
		LOG_ERR("Errors occured during lexing");
		return -1;
	}

	Tree tree = (Tree){ .root = NULL };
	Arena tree_node_arena = arena_create(1024);

	Parser parser;
	parser_init(Parser *parser, Arena *token_arena, Arena *tree_node_arena, size_t token_count);

	parse(&parser, &tree);

	return 0;
}
