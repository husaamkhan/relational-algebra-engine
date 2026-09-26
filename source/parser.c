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

static bool check(Parser *parser, Category category)
{
	Token *token = peek(parser);
	return token != NULL && token->category == category;
}

void parser_init(Parser *parser, Arena *token_arena, size_t token_count, Arena *node_arena)
{
	parser->token_arena  = token_arena;
	parser->token_count  = token_count;
	parser->cur_pos      = 0;
	parser->node_arena   = node_arena;
	parser->cur_token    = (Token *)token_arena->base;
	parser->has_error    = false;
}

static TreeNode *parse_statement(Parser *parser)
{
	Token *token = peek(parser);

	if (token->category != WORD && token->category != LPAREN)
	{
		SYNTAX_ERR("Expected identifier or '(' at %d:%d (got '%.*s')",
		           token->pos.row, token->pos.col,
		           (int)token->lexeme_length, token->lexeme_start);
		parser->has_error = true;
		return NULL;
	}

	if (token->category == LPAREN)
	{
		/* TODO: handle "(" query_expression ")" */
		advance(parser);
		return NULL;
	}

	if (token->lexeme_length == 6 && strncmp(token->lexeme_start, "select", 6) == 0)
	{
		token->category = SELECT;
	}
	else if (token->lexeme_length == 7 && strncmp(token->lexeme_start, "project", 7) == 0)
	{
		token->category = PROJECT;
	}
	else if (token->lexeme_length == 6 && strncmp(token->lexeme_start, "rename", 6) == 0)
	{
		token->category = RENAME;
	}

	if (token->category == SELECT || token->category == PROJECT || token->category == RENAME)
	{
		SYNTAX_ERR("Expected '[' after '%.*s' at %d:%d",
		           (int)token->lexeme_length, token->lexeme_start,
		           token->pos.row, token->pos.col);
		parser->has_error = true;

		TreeNode *node = tree_node_create(parser->node_arena, 1, NULL, NULL, NULL);
		node->token_arr[0] = advance(parser);
		return node;
	}

	token->category = IDENT;
	Token *identifier = advance(parser);
	TreeNode *node = tree_node_create(parser->node_arena, 1, NULL, NULL, NULL);
	node->token_arr[0] = identifier;
	return node;
}

static void print_tree_node(const TreeNode *node, const char *prefix, bool is_last)
{
	if (node == NULL)
	{
		return;
	}

	printf("%s%s", prefix, is_last ? "└── " : "├── ");

	for (size_t i = 0; i < node->token_count; i++)
	{
		Token *token = node->token_arr[i];
		printf("%.*s", (int)token->lexeme_length, token->lexeme_start);
		if (i + 1 < node->token_count)
		{
			printf(" ");
		}
	}
	printf("\n");

	char child_prefix[256];
	snprintf(child_prefix, sizeof(child_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

	bool has_left  = node->left_child != NULL;
	bool has_right = node->right_child != NULL;

	if (has_left)
	{
		print_tree_node(node->left_child, child_prefix, !has_right);
	}
	if (has_right)
	{
		print_tree_node(node->right_child, child_prefix, true);
	}
}

void print_tree(const Tree *tree)
{
	if (tree == NULL || tree->head == NULL)
	{
		printf("PROGRAM\n(empty)\n");
		return;
	}

	printf("PROGRAM\n");

	for (StatementNode *stmt = tree->head; stmt != NULL; stmt = stmt->next)
	{
		bool is_last = (stmt->next == NULL);

		if (stmt->root == NULL)
		{
			printf("%s(error)\n", is_last ? "└── " : "├── ");
			continue;
		}

		print_tree_node(stmt->root, "", is_last);
	}
}

void parse(Parser *parser, Tree *tree)
{
	tree->head = NULL;
	tree->tail = NULL;

	while (peek(parser) != NULL)
	{
		while (check(parser, NEWLINE))
		{
			advance(parser);
		}

		if (peek(parser) == NULL)
		{
			break;
		}

		bool error_before = parser->has_error;
		TreeNode *root = parse_statement(parser);

		Token *next = peek(parser);
		if (next != NULL && next->category != NEWLINE)
		{
			SYNTAX_ERR("Expected end of statement at %d:%d (got '%.*s')",
			           next->pos.row, next->pos.col,
			           (int)next->lexeme_length, next->lexeme_start);
			parser->has_error = true;
		}

		StatementNode *stmt = arena_push(parser->node_arena, sizeof(StatementNode), _Alignof(StatementNode));
		stmt->root = root;
		stmt->next = NULL;

		if (tree->tail == NULL)
		{
			tree->head = stmt;
		}
		else
		{
			tree->tail->next = stmt;
		}
		tree->tail = stmt;

		bool this_statement_failed = parser->has_error && !error_before;
		if (this_statement_failed)
		{
			while (peek(parser) != NULL && !check(parser, NEWLINE))
			{
				advance(parser);
			}
		}

		if (check(parser, NEWLINE))
		{
			advance(parser);
		}
	}
}
