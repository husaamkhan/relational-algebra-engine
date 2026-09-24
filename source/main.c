#include "common.h"
#include "lexer.h"
#include "parser.h"

int main()
{
	Lexer lexer;
	Arena arena = arena_create(1024);
	size_t count = 0;
	lex(&lexer, &arena, &count);

	if (lexer.has_error)
	{
		LOG_ERR("Errors occured during lexing");
		return -1;
	}

	Parser parser;
	Tree tree = (Tree){ .root = NULL };
	parse(&parser, &arena, count);

	return 0;
}
