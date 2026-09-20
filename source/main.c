#include "common.h"
#include "lexer.h"
#include "parser.h"

int main()
{
	LOG("Hello world!");

	Lexer lexer;
	lex(&lexer);

	Parser parser;
	parse(&parser);
}
