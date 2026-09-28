#include <stdio.h>
#include <string.h>
#include "common.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv)
{
	FILE *in = stdin;

	if (argc > 2)
	{
		LOG_ERR("Usage: %s [query-file]", argv[0]);
		return -1;
	}

	if (argc == 2)
	{
		in = fopen(argv[1], "rb");

		if (in == NULL)
		{
			LOG_ERR("Could not open file '%s'", argv[1]);
			return -1;
		}
	}

	size_t file_size = 0;
	char *file_contents = read_file(in, &file_size);

	Lexer lexer;
	lexer_init(&lexer, file_contents, file_size);

	Arena token_arena = arena_create(1024 * 1024);
	size_t token_count = 0;

	lex(&lexer, &token_arena, &token_count);
	free(file_contents);

	if (lexer.has_error)
	{
		LOG_ERR("Errors occurred during lexing");
		arena_destroy(&token_arena);
		return -1;
	}

	Arena tree_node_arena = arena_create(1024 * 1024);

	Parser parser;
	parser_init(&parser, &token_arena, token_count, &tree_node_arena);

	Tree tree = (Tree){ .head = NULL, .tail = NULL };
	parse(&parser, &tree);

	print_tree(&tree);

	int exit_code = parser.has_error ? -1 : 0;

	arena_destroy(&tree_node_arena);
	arena_destroy(&token_arena);

	return exit_code;
}
