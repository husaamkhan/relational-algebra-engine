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

void lex(Lexer *lexer, Arena *arena, size_t *count_out)
{
	*count_out = 0;

	while (lexer->cur_pos < (int)lexer->file_size)
	{
		lexer->lexeme_start  = lexer->cur_pos;
		char c               = lexer->file_contents[lexer->cur_pos++];

		Token *token         = token_new(arena);
		token->lexeme_start  = lexer->file_contents + lexer->lexeme_start;
		token->lexeme_length = 1;
		token->pos           = lexer->pos;

		lexer->pos.col++;

		switch (c)
		{
			case '(': token->category = LPAREN;	break;
			case ')': token->category = RPAREN;    	break;
			case '[': token->category = LBRACKET;  	break;
			case ']': token->category = RBRACKET;  	break;
			case '{': token->category = LBRACE;    	break;
			case '}': token->category = RBRACE;    	break;
			case ',': token->category = COMMA;	break;
			case '=': token->category = EQUAL;	break;
			default:
				  {
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
