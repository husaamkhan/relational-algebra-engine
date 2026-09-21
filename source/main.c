#include "common.h"
#include "lexer.h"
#include "parser.h"

int main()
{
	LOG("Hello world!");

	Lexer lexer;
	Arena arena = arena_create(1024);
	size_t count = 0;
	lex(&lexer, &arena, &count);

	Parser parser;
	parse(&parser);
}
