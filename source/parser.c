#include "common.h"
#include "parser.h"
#include <string.h>

static TreeNode *parse_condition(Parser *parser);
static TreeNode *parse_additive_expression(Parser *parser);
static TreeNode *parse_join(Parser *parser, TreeNode *left);

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

static TreeNode *parse_rename(Parser *parser)
{
	Token *rename_token = advance(parser);
	rename_token->category = RENAME;

	TreeNode *node = tree_node_create(
		parser->node_arena,
		1,
		NULL,
		NULL,
		NULL
	);

	node->token_arr[0] = rename_token;

	if (!check(parser, LBRACKET))
	{
		Token *token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete rename expression at end of input: expected '[' identifier ']' and relation expression");
		else SYNTAX_ERR("Expected '[' after 'rename' at %d:%d", token->pos.row, token->pos.col);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* '[' */

	Token *name = peek(parser);

	if (name == NULL)
	{
		SYNTAX_ERR("Incomplete rename expression at end of input: expected identifier and closing ']'");
		parser->has_error = true;
		return node;
	}

	if (name->category != WORD)
	{
		SYNTAX_ERR("Expected identifier at %d:%d (got '%.*s')",
		           name->pos.row,
		           name->pos.col,
		           (int)name->lexeme_length,
		           name->lexeme_start);
		parser->has_error = true;
		return node;
	}

	name->category = IDENT;

	TreeNode *new_name = tree_node_create(
		parser->node_arena,
		1,
		NULL,
		NULL,
		NULL
	);

	new_name->token_arr[0] = advance(parser);

	if (!check(parser, RBRACKET))
	{
		Token *token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete rename expression at end of input: expected ']' and relation expression");
		else SYNTAX_ERR("Expected ']' after rename identifier at %d:%d (got '%.*s')", token->pos.row, token->pos.col, (int)token->lexeme_length, token->lexeme_start);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* ']' */

	if (!check(parser, LPAREN))
	{
		Token *token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete rename expression at end of input: expected '(' and relation expression");
		else SYNTAX_ERR("Expected '(' after rename identifier at %d:%d", token->pos.row, token->pos.col);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* '(' */

	TreeNode *expression = parse_additive_expression(parser);

	if (expression == NULL)
	{
		return node;
	}

	if (!check(parser, RPAREN))
	{
		Token *token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete rename expression at end of input: expected ')'");
		else SYNTAX_ERR("Expected ')' at %d:%d (got '%.*s')", token->pos.row, token->pos.col, (int)token->lexeme_length, token->lexeme_start);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* ')' */

	node->left_child = new_name;
	node->right_child = expression;

	new_name->parent = node;
	expression->parent = node;

	return node;
}

static TreeNode *parse_project(Parser *parser)
{
	Token *project_token = advance(parser);
	project_token->category = PROJECT;

	TreeNode *node = tree_node_create(
		parser->node_arena,
		1,
		NULL,
		NULL,
		NULL
	);

	node->token_arr[0] = project_token;

	if (!check(parser, LBRACKET))
	{
		Token *token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete project expression at end of input: expected '[' attribute list ']' and relation expression");
		else SYNTAX_ERR("Expected '[' after 'project' at %d:%d", token->pos.row, token->pos.col);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* '[' */

	Token *token = peek(parser);

	if (token == NULL)
	{
		SYNTAX_ERR("Incomplete project expression at end of input: expected attribute name and closing ']'");
		parser->has_error = true;
		return node;
	}

	if (token->category != WORD)
	{
		SYNTAX_ERR("Expected attribute name at %d:%d (got '%.*s')",
		           token->pos.row,
		           token->pos.col,
		           (int)token->lexeme_length,
		           token->lexeme_start);
		parser->has_error = true;
		return node;
	}

	/*
	 * Count the attributes first so that the attribute node can
	 * contain exactly one token pointer for each attribute.
	 */
	size_t attribute_count = 1;
	size_t lookahead = parser->cur_pos + 1;

	while (lookahead < parser->token_count)
	{
		token = (Token *)parser->token_arena->base + lookahead;

		if (token->category != COMMA)
		{
			break;
		}

		lookahead++;

		if (lookahead >= parser->token_count)
		{
			SYNTAX_ERR("Incomplete project expression at end of input: expected attribute name and closing ']'");
			parser->has_error = true;
			return node;
		}

		token = (Token *)parser->token_arena->base + lookahead;

		if (token->category != WORD)
		{
			SYNTAX_ERR("Expected attribute name after ',' at %d:%d (got '%.*s')",
			           token->pos.row,
			           token->pos.col,
			           (int)token->lexeme_length,
			           token->lexeme_start);
			parser->has_error = true;
			return node;
		}

		attribute_count++;
		lookahead++;
	}

	TreeNode *attributes = tree_node_create(
		parser->node_arena,
		attribute_count,
		NULL,
		NULL,
		NULL
	);

	for (size_t i = 0; i < attribute_count; i++)
	{
		token = peek(parser);

		token->category = IDENT;
		attributes->token_arr[i] = advance(parser);

		if (i + 1 < attribute_count)
		{
			advance(parser); /* ',' */
		}
	}

	if (!check(parser, RBRACKET))
	{
		token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete project expression at end of input: expected ']'");
		else SYNTAX_ERR("Expected ']' after project attributes at %d:%d (got '%.*s')", token->pos.row, token->pos.col, (int)token->lexeme_length, token->lexeme_start);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* ']' */

	if (!check(parser, LPAREN))
	{
		token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete project expression at end of input: expected '(' and relation expression");
		else SYNTAX_ERR("Expected '(' after project attributes at %d:%d", token->pos.row, token->pos.col);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* '(' */

	TreeNode *expression = parse_additive_expression(parser);

	if (expression == NULL)
	{
		return node;
	}

	if (!check(parser, RPAREN))
	{
		token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete project expression at end of input: expected ')'");
		else SYNTAX_ERR("Expected ')' at %d:%d (got '%.*s')", token->pos.row, token->pos.col, (int)token->lexeme_length, token->lexeme_start);

		parser->has_error = true;
		return node;
	}

	advance(parser); /* ')' */

	node->left_child = attributes;
	node->right_child = expression;

	attributes->parent = node;
	expression->parent = node;

	return node;
}

static TreeNode *parse_select(Parser *parser)
{
	Token *select_token = advance(parser);
	select_token->category = SELECT;

	TreeNode *node = tree_node_create(
		parser->node_arena,
		1,
		NULL,
		NULL,
		NULL
	);

	node->token_arr[0] = select_token;

	if (!check(parser, LBRACKET))
	{
		Token *token = peek(parser);

		if (token == NULL) SYNTAX_ERR("Incomplete select expression at end of input: expected '[' condition ']' and relation expression");
		else SYNTAX_ERR("Expected '[' after 'select' at %d:%d", token->pos.row, token->pos.col);

		parser->has_error = true;
		return node;
	}

	advance(parser);

	TreeNode *condition = parse_condition(parser);

	if (condition == NULL)
	{
		return node;
	}

	if (!check(parser, RBRACKET))
	{
		Token *token = peek(parser);

		if (token == NULL)
		{
			SYNTAX_ERR("Incomplete select expression at end of input: expected ']' and relation expression");
		}
		else
		{
			SYNTAX_ERR("Expected ']' at %d:%d (got '%.*s')",
			           token->pos.row,
			           token->pos.col,
			           (int)token->lexeme_length,
			           token->lexeme_start);
		}

		parser->has_error = true;
		return node;
	}

	advance(parser);

	if (!check(parser, LPAREN))
	{
		Token *token = peek(parser);

		if (token == NULL)
		{
			SYNTAX_ERR("Incomplete select expression at end of input: expected '(' and relation expression");
		}
		else
		{
			SYNTAX_ERR("Expected '(' after select condition at %d:%d",
			           token->pos.row,
			           token->pos.col);
		}

		parser->has_error = true;
		return node;
	}

	advance(parser);

	TreeNode *expression = parse_additive_expression(parser);

	if (expression == NULL)
	{
		return node;
	}

	if (!check(parser, RPAREN))
	{
		Token *token = peek(parser);

		if (token == NULL)
		{
			SYNTAX_ERR("Expected ')' at end of input");
		}
		else
		{
			SYNTAX_ERR("Expected ')' at %d:%d (got '%.*s')",
			           token->pos.row,
			           token->pos.col,
			           (int)token->lexeme_length,
			           token->lexeme_start);
		}

		parser->has_error = true;
		return node;
	}

	advance(parser);

	node->left_child = condition;
	node->right_child = expression;

	condition->parent = node;
	expression->parent = node;

	return node;
}

static TreeNode *parse_atom_expression(Parser *parser)
{
	Token *token = peek(parser);

	if (token == NULL)
	{
		SYNTAX_ERR("Expected expression at end of input");
		parser->has_error = true;
		return NULL;
	}

	if (token->category == LPAREN)
	{
		advance(parser);

		TreeNode *node = parse_additive_expression(parser);

		if (!check(parser, RPAREN))
		{
			token = peek(parser);

			if (token == NULL)
			{
				SYNTAX_ERR("Expected ')' at end of input");
			}
			else
			{
				SYNTAX_ERR("Expected ')' at %d:%d (got '%.*s')",
				           token->pos.row,
				           token->pos.col,
				           (int)token->lexeme_length,
				           token->lexeme_start);
			}

			parser->has_error = true;
			return node;
		}

		advance(parser);
		return node;
	}

	if (token->category == WORD)
	{
		token->category = IDENT;
		advance(parser);

		TreeNode *node = tree_node_create(
			parser->node_arena,
			1,
			NULL,
			NULL,
			NULL
		);

		node->token_arr[0] = token;

		return node;
	}

	SYNTAX_ERR("Expected expression at %d:%d (got '%.*s')",
	           token->pos.row,
	           token->pos.col,
	           (int)token->lexeme_length,
	           token->lexeme_start);
	parser->has_error = true;
	return NULL;
}

static TreeNode *parse_unary_expression(Parser *parser)
{
	Token *token = peek(parser);

	if (token != NULL &&
	    token->category == WORD &&
	    token->lexeme_length == 6 &&
	    strncmp(token->lexeme_start, "select", 6) == 0)
	{
		return parse_select(parser);
	}

	if (token != NULL &&
	    token->category == WORD &&
	    token->lexeme_length == 7 &&
	    strncmp(token->lexeme_start, "project", 7) == 0)
	{
		return parse_project(parser);
	}

	if (token != NULL &&
	    token->category == WORD &&
	    token->lexeme_length == 6 &&
	    strncmp(token->lexeme_start, "rename", 6) == 0)
	{
		return parse_rename(parser);
	}

	return parse_atom_expression(parser);
}

static TreeNode *parse_join(Parser *parser, TreeNode *left)
{
	Token *join_token = advance(parser);
	join_token->category = JOIN;

	if (!check(parser, LBRACKET))
	{
		Token *token = peek(parser);

		if (token == NULL)
		{
			SYNTAX_ERR("Incomplete join expression at end of input: expected '[' condition ']' and right relation expression");
		}
		else
		{
			SYNTAX_ERR("Expected '[' after 'join' at %d:%d",
			           token->pos.row,
			           token->pos.col);
		}

		parser->has_error = true;
		return NULL;
	}

	advance(parser); /* '[' */

	TreeNode *condition = parse_condition(parser);

	if (condition == NULL)
	{
		return NULL;
	}

	if (!check(parser, RBRACKET))
	{
		Token *token = peek(parser);

		if (token == NULL)
		{
			SYNTAX_ERR("Incomplete join expression at end of input: expected ']' and right relation expression");
		}
		else
		{
			SYNTAX_ERR("Expected ']' after join condition at %d:%d (got '%.*s')",
			           token->pos.row,
			           token->pos.col,
			           (int)token->lexeme_length,
			           token->lexeme_start);
		}

		parser->has_error = true;
		return NULL;
	}

	advance(parser); /* ']' */

	TreeNode *right = parse_unary_expression(parser);

	if (right == NULL)
	{
		return NULL;
	}

	Token *relations_token = token_new(parser->node_arena);
	*relations_token = (Token){
		.category = JOIN_RELATIONS,
		.lexeme_start = "JOIN_RELATIONS",
		.lexeme_length = 14,
		.pos = {0, 0}
	};

	TreeNode *relations = tree_node_create(
		parser->node_arena,
		1,
		NULL,
		left,
		right
	);

	relations->token_arr[0] = relations_token;

	TreeNode *node = tree_node_create(
		parser->node_arena,
		1,
		NULL,
		relations,
		condition
	);

	node->token_arr[0] = join_token;

	left->parent = relations;
	right->parent = relations;

	relations->parent = node;
	condition->parent = node;

	return node;
}

static TreeNode *parse_multiplicative_expression(Parser *parser)
{
	TreeNode *left = parse_unary_expression(parser);

	if (left == NULL)
	{
		return NULL;
	}

	while (check(parser, WORD))
	{
		if (parser->cur_token->lexeme_length == 5 &&
		    strncmp(parser->cur_token->lexeme_start, "times", 5) == 0)
		{
			Token *operator_token = advance(parser);
			operator_token->category = TIMES;

			TreeNode *right = parse_unary_expression(parser);

			if (right == NULL)
			{
				return NULL;
			}

			TreeNode *node = tree_node_create(
				parser->node_arena,
				1,
				NULL,
				left,
				right
			);

			node->token_arr[0] = operator_token;

			left->parent = node;
			right->parent = node;

			left = node;
		}
		else if (parser->cur_token->lexeme_length == 4 &&
			 strncmp(parser->cur_token->lexeme_start, "join", 4) == 0)
		{
			left = parse_join(parser, left);

			if (left == NULL)
			{
				return NULL;
			}
		}
		else
		{
			break;
		}
	}
	return left;
}

static TreeNode *parse_additive_expression(Parser *parser)
{
	TreeNode *left = parse_multiplicative_expression(parser);

	if (left == NULL)
	{
		return NULL;
	}

	while (check(parser, WORD) &&
	       ((parser->cur_token->lexeme_length == 5 &&
	         strncmp(parser->cur_token->lexeme_start, "union", 5) == 0) ||
	        (parser->cur_token->lexeme_length == 9 &&
	         strncmp(parser->cur_token->lexeme_start, "intersect", 9) == 0) ||
	        (parser->cur_token->lexeme_length == 5 &&
	         strncmp(parser->cur_token->lexeme_start, "minus", 5) == 0)))
	{
		Token *operator_token = advance(parser);

		if (operator_token->lexeme_length == 5 &&
		    strncmp(operator_token->lexeme_start, "union", 5) == 0)
		{
			operator_token->category = UNION;
		}
		else if (operator_token->lexeme_length == 9 &&
		         strncmp(operator_token->lexeme_start, "intersect", 9) == 0)
		{
			operator_token->category = INTERSECT;
		}
		else
		{
			operator_token->category = MINUS;
		}

		TreeNode *right = parse_multiplicative_expression(parser);

		if (right == NULL)
		{
			return NULL;
		}

		TreeNode *node = tree_node_create(
			parser->node_arena,
			1,
			NULL,
			left,
			right
		);

		node->token_arr[0] = operator_token;

		left->parent = node;
		right->parent = node;

		left = node;
	}

	return left;
}

static TreeNode *parse_operand(Parser *parser)
{
	Token *token = peek(parser);

	if (token == NULL)
	{
		SYNTAX_ERR("Expected operand at end of input");
		parser->has_error = true;
		return NULL;
	}

	if (token->category == NUMBER || token->category == STRING)
	{
		TreeNode *node = tree_node_create(
			parser->node_arena,
			1,
			NULL,
			NULL,
			NULL
		);

		node->token_arr[0] = advance(parser);

		return node;
	}

	if (token->category == WORD)
	{
		token->category = IDENT;
		Token *first = advance(parser);

		if (check(parser, DOT))
		{
			advance(parser);

			Token *second = peek(parser);

			if (second == NULL)
			{
				SYNTAX_ERR("Expected identifier after '.' at end of input");
				parser->has_error = true;
				return NULL;
			}

			if (second->category != WORD)
			{
				SYNTAX_ERR("Expected identifier after '.' at %d:%d (got '%.*s')",
				           second->pos.row,
				           second->pos.col,
				           (int)second->lexeme_length,
				           second->lexeme_start);
				parser->has_error = true;
				return NULL;
			}

			second->category = IDENT;
			advance(parser);

			TreeNode *node = tree_node_create(
				parser->node_arena,
				2,
				NULL,
				NULL,
				NULL
			);

			node->token_arr[0] = first;
			node->token_arr[1] = second;

			return node;
		}

		TreeNode *node = tree_node_create(
			parser->node_arena,
			1,
			NULL,
			NULL,
			NULL
		);

		node->token_arr[0] = first;

		return node;
	}

	SYNTAX_ERR("Expected operand at %d:%d (got '%.*s')",
	           token->pos.row,
	           token->pos.col,
	           (int)token->lexeme_length,
	           token->lexeme_start);
	parser->has_error = true;
	return NULL;
}

static TreeNode *parse_comparison(Parser *parser)
{
	TreeNode *left = parse_operand(parser);

	if (left == NULL)
	{
		return NULL;
	}

	Token *operator_token = peek(parser);

	if (operator_token == NULL ||
	    (operator_token->category != EQUAL &&
	     operator_token->category != NOT_EQUAL &&
	     operator_token->category != LESS_THAN &&
	     operator_token->category != LESS_THAN_OR_EQUAL &&
	     operator_token->category != GREATER_THAN &&
	     operator_token->category != GREATER_THAN_OR_EQUAL))
	{
		if (operator_token == NULL)
		{
			SYNTAX_ERR("Expected comparison operator at end of input");
		}
		else
		{
			SYNTAX_ERR("Expected comparison operator at %d:%d (got '%.*s')",
			           operator_token->pos.row,
			           operator_token->pos.col,
			           (int)operator_token->lexeme_length,
			           operator_token->lexeme_start);
		}

		parser->has_error = true;
		return left;
	}

	advance(parser);

	TreeNode *right = parse_operand(parser);

	if (right == NULL)
	{
		return NULL;
	}

	TreeNode *node = tree_node_create(
		parser->node_arena,
		1,
		NULL,
		left,
		right
	);

	node->token_arr[0] = operator_token;

	left->parent = node;
	right->parent = node;

	return node;
}

static TreeNode *parse_not_expression(Parser *parser)
{
	if (check(parser, WORD) &&
	    parser->cur_token->lexeme_length == 3 &&
	    strncmp(parser->cur_token->lexeme_start, "not", 3) == 0)
	{
		Token *operator_token = advance(parser);
		operator_token->category = NOT;

		TreeNode *child = parse_not_expression(parser);

		if (child == NULL)
		{
			return NULL;
		}

		TreeNode *node = tree_node_create(
			parser->node_arena,
			1,
			NULL,
			child,
			NULL
		);

		node->token_arr[0] = operator_token;
		child->parent = node;

		return node;
	}

	if (check(parser, LPAREN))
	{
		advance(parser);

		TreeNode *node = parse_condition(parser);

		if (!check(parser, RPAREN))
		{
			Token *token = peek(parser);

			if (token == NULL)
			{
				SYNTAX_ERR("Expected ')' at end of input");
			}
			else
			{
				SYNTAX_ERR("Expected ')' at %d:%d (got '%.*s')",
				           token->pos.row,
				           token->pos.col,
				           (int)token->lexeme_length,
				           token->lexeme_start);
			}

			parser->has_error = true;
			return node;
		}

		advance(parser);
		return node;
	}

	return parse_comparison(parser);
}

static TreeNode *parse_and_expression(Parser *parser)
{
	TreeNode *left = parse_not_expression(parser);

	while (check(parser, WORD) &&
	       parser->cur_token->lexeme_length == 3 &&
	       strncmp(parser->cur_token->lexeme_start, "and", 3) == 0)
	{
		Token *operator_token = advance(parser);
		operator_token->category = AND;

		TreeNode *right = parse_not_expression(parser);

		if (right == NULL)
		{
			return NULL;
		}

		TreeNode *node = tree_node_create(
			parser->node_arena,
			1,
			NULL,
			left,
			right
		);

		node->token_arr[0] = operator_token;

		left->parent = node;
		right->parent = node;

		left = node;
	}

	return left;
}

static TreeNode *parse_or_expression(Parser *parser)
{
	TreeNode *left = parse_and_expression(parser);

	while (check(parser, WORD) &&
	       parser->cur_token->lexeme_length == 2 &&
	       strncmp(parser->cur_token->lexeme_start, "or", 2) == 0)
	{
		Token *operator_token = advance(parser);
		operator_token->category = OR;

		TreeNode *right = parse_and_expression(parser);

		if (right == NULL)
		{
			return NULL;
		}

		TreeNode *node = tree_node_create(
			parser->node_arena,
			1,
			NULL,
			left,
			right
		);

		node->token_arr[0] = operator_token;

		left->parent = node;
		right->parent = node;

		left = node;
	}

	return left;
}

static TreeNode *parse_condition(Parser *parser)
{
	return parse_or_expression(parser);
}

static TreeNode *parse_statement(Parser *parser)
{
	Token *token = peek(parser);

	if (token == NULL)
	{
		return NULL;
	}

	if (token->category != WORD && token->category != LPAREN)
	{
		SYNTAX_ERR("Expected identifier or '(' at %d:%d (got '%.*s')",
		           token->pos.row,
		           token->pos.col,
		           (int)token->lexeme_length,
		           token->lexeme_start);
		parser->has_error = true;
		return NULL;
	}

	return parse_additive_expression(parser);
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
