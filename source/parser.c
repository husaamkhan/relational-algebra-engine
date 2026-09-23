#include "common.h"
#include "parser.h"

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
	/*
	 * program is a statement, optionally followed by more statements
	 * statement can be relation definition of query expression
	 * parser starts by determining which statement it is and figure out what to do from there
	 *
	 * we can use a state machine to switch between different parser states, which lets the parser
	 * switch between different expressinos that it is expecting/are valid given the current statement
	 * being parsed
	 * */	

	for (size_t i; i <= token_count; i++)
	{
		if (token->category != WORD)
		{
			printf("print an error here");
		}

		i
	}
}
