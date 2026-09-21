#include "lexer.h"

void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size)
{
	lexer->file_contents      = file_contents;
	lexer->file_size          = file_size;
	lexer->cur_pos            = 0;
	lexer->last_accepting_pos = 0;
	lexer->lexeme_start       = 0;
	lexer->has_error          = false;
	lexer->pos.row            = 1;
	lexer->pos.col            = 1;
}

bool peek(Lexer *lexer)
{
	if (lexer->cur_pos >= (int)lexer->file_size)
	{
		return false;
	}

	lexer->cur_char = lexer->file_contents[lexer->cur_pos];

	if (lexer->cur_char == '\n')
	{
		lexer->pos.row++;
		lexer->pos.col = 1;
	}
	else
	{
		lexer->pos.col++;
	}

	lexer->cur_pos++;
	return true;
}

void lex(Lexer *lexer, Arena *arena, size_t *count_out)
{
	*count_out = 0;

	while (lexer->cur_pos < (int)lexer->file_size)
	{
		// TODO: Missing EOF handling. If a peek ahead was done in the previous loop,
		// or char c = cur_pos++ lead to an EOF, it needs to be handled here
		lexer->lexeme_start  = lexer->cur_pos;
		char c               = lexer->file_contents[lexer->cur_pos++];

		Token *token         = token_new(arena);
		token->lexeme_start  = lexer->file_contents + lexer->lexeme_start;
		token->lexeme_length = 1;
		token->pos           = lexer->pos;

		lexer->pos.col++;

		switch (c)
		{
			// TODO: add cases for ' ', \n
			case '(': token->category = LPAREN;		break;
			case ')': token->category = RPAREN;    		break;
			case '[': token->category = LBRACKET;  		break;
			case ']': token->category = RBRACKET;  		break;
			case '{': token->category = LBRACE;    		break;
			case '}': token->category = RBRACE;    		break;
			case ',': token->category = COMMA;		break;
			case '=': token->category = EQUAL;		break;
			case '!':
			{
				if (!peek(lexer))
				{
					// TODO: arena->used -= sizeof(Token) is technically incorrect since we are aligning upwards
					// for tokens
					arena->used -= sizeof(Token);
					lexer->has_error = true;
					LEX_ERR("End of file unexpectedly reached at %d:%d", lexer->pos.row, lexer->pos.col);
					continue;
				}
				else if (lexer->cur_char == '=')
				{
					token->category      = NOT_EQUAL;
					token->lexeme_length = 2;
				}
				else
				{
					arena->used -= sizeof(Token);
					lexer->has_error = true;
					LEX_ERR("Unexpected character '%c' at %d:%d", c, token->pos.row, token->pos.col);
					continue;
				}
				break;
			}
			case '<':
			{
				peek(lexer); // don't care if EOF reached. in this case just emit LESS_THAN token
				if (lexer->cur_char == '=')
				{
					token->category = LESS_THAN_OR_EQUAL;
					token->lexeme_length = 2;
				}

				else
				{
					token->category = LESS_THAN;
				}

				break;
			}
			case '>':
			{
				peek(lexer); // don't care if EOF reached. in this case just emit LESS_THAN token
				if (lexer->cur_char == '=')
				{
					token->category = GREATER_THAN_OR_EQUAL;
					token->lexeme_length = 2;
				}

				else
				{
					token->category = GREATER_THAN;
				}

				break;
			}
			default:
			  {
				  // TODO: We skip to the next ' ', but i'm pretty sure the grammar does not enforce tokens to be
				  // separated by spaces. so this might need to be thought out a bit more.
				  // Skip the word with an unexpected character to the next word
				  while (lexer->cur_pos < (int)lexer->file_size)
				  {
					  char next = lexer->file_contents[lexer->cur_pos];
					  if (next == ' ' || next == '\n')
						  break;
					  lexer->cur_pos++;
					  lexer->pos.col++;
				  }

				  arena->used -= sizeof(Token);
				  lexer->has_error = true;

				  LEX_ERR("Unexpected character '%c' at %d:%d", c, token->pos.row, token->pos.col);
				  continue;
			  }
		}

		(*count_out)++;
	}
}
