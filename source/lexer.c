#include <string.h>
#include "lexer.h"
#include <ctype.h>

/*
 * Initializes a lexer with the given input buffer.
 *
 * Input:
 *   lexer         - Lexer to initialize.
 *   file_contents - Pointer to the input buffer.
 *   file_size     - Size of the input buffer in bytes.
 *
 * Output:
 *   The lexer's internal state is initialized.
 *
 * Returns:
 *   None.
 */
void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size)
{
	lexer->file_contents      = file_contents;
	lexer->file_size          = file_size;
	lexer->cur_pos            = 0;
	lexer->lexeme_start       = 0;
	lexer->has_error          = false;
	lexer->pos.row            = 1;
	lexer->pos.col            = 1;
}

/*
 * Peeks at the next character without consuming it.
 *
 * Input:
 *   lexer - Lexer to peek from.
 *   c     - Output pointer to store the peeked character.
 *
 * Output:
 *   *c is set to the next character if available.
 *
 * Returns:
 *   true if a character was peeked, false if at EOF.
 */
bool peek(const Lexer *lexer, char *c)
{
	if (lexer->cur_pos >= (int)lexer->file_size)
	{
		return false;
	}

	*c = lexer->file_contents[lexer->cur_pos];
	return true;
}

/*
 * Advances the lexer by one character, updating position tracking.
 *
 * Input:
 *   lexer - Lexer to advance.
 *
 * Output:
 *   lexer->cur_char is set to the consumed character.
 *   lexer->cur_pos is incremented.
 *   lexer->pos.row/col are updated for newlines.
 *
 * Returns:
 *   true if a character was consumed, false if at EOF.
 */
bool advance(Lexer *lexer)
{
	char next_char;
	if (!peek(lexer, &next_char))
	{
		return false;
	}

	lexer->cur_char = next_char;
	lexer->cur_pos++;
	if (lexer->cur_char == '\n')
	{
		lexer->pos.row++;
		lexer->pos.col = 1;
	}
	else
	{
		lexer->pos.col++;
	}
	
	return true;
}

/*
 * Tokenizes the input buffer into a sequence of tokens.
 *
 * Input:
 *   lexer     - Initialized lexer with input buffer.
 *   arena     - Arena to allocate tokens from.
 *   count_out - Pointer to store the number of tokens produced.
 *
 * Output:
 *   Tokens are allocated in the arena. count_out is set to the token count.
 *   lexer->has_error is set to true if any lexical errors occurred.
 *
 * Returns:
 *   None.
 */
void lex(Lexer *lexer, Arena *arena, size_t *count_out)
{
	*count_out = 0;

	while (lexer->cur_pos < (int)lexer->file_size)
	{
		lexer->lexeme_start  = lexer->cur_pos;
		Token *token         = token_new(arena);
		token->lexeme_start  = lexer->file_contents + lexer->lexeme_start;
		token->lexeme_length = 1;
		token->pos           = lexer->pos;

		advance(lexer);

		switch (lexer->cur_char)
		{
			case ' ':
			{
				arena_pop(arena);
				continue;
			}
			case '\n': token->category = NEWLINE; break;
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
				char next_char;
				if (!peek(lexer, &next_char))
				{
					arena_pop(arena);
					lexer->has_error = true;
					LEX_ERR("End of file unexpectedly reached at %d:%d", lexer->pos.row, lexer->pos.col);
					continue;
				}
				else if (next_char == '=')
				{
					token->category      = NOT_EQUAL;
					token->lexeme_length = 2;
					advance(lexer);
				}
				else
				{
					arena_pop(arena);
					lexer->has_error = true;
					LEX_ERR("Unexpected character '%c' at %d:%d", lexer->cur_char, token->pos.row, token->pos.col);
					continue;
				}
				break;
			}
			case '<':
			{
				char next_char;
				if (peek(lexer, &next_char) && next_char == '=')
				{
					token->category = LESS_THAN_OR_EQUAL;
					token->lexeme_length = 2;
					advance(lexer);
				}
				else
				{
					token->category = LESS_THAN;
				}
				break;
			}
			case '>':
			{
				char next_char;
				if (peek(lexer, &next_char) && next_char == '=')
				{
					token->category = GREATER_THAN_OR_EQUAL;
					token->lexeme_length = 2;
					advance(lexer);
				}
				else
				{
					token->category = GREATER_THAN;
				}
				break;
			}
			default:
			{
			if ((lexer->cur_char >= 'a' && lexer->cur_char <= 'z') ||
			    (lexer->cur_char >= 'A' && lexer->cur_char <= 'Z'))
			{
				/* Consume the rest of the identifier: letters, digits, underscores. */
				char next_char;
				while (peek(lexer, &next_char) &&
				       ((next_char >= 'a' && next_char <= 'z') ||
				        (next_char >= 'A' && next_char <= 'Z') ||
				        (next_char >= '0' && next_char <= '9') ||
				        next_char == '_'))
				{
					advance(lexer);
				}

				token->lexeme_length = lexer->cur_pos - lexer->lexeme_start;

				/* Classify: check if the lexeme exactly matches a keyword. */
				const char *lex = token->lexeme_start;
				size_t      len = token->lexeme_length;

				if      (len == 6 && strncmp(lex, "select",    6) == 0) token->category = SELECT;
				else if (len == 7 && strncmp(lex, "project",   7) == 0) token->category = PROJECT;
				else if (len == 6 && strncmp(lex, "rename",    6) == 0) token->category = RENAME;
				else if (len == 5 && strncmp(lex, "union",     5) == 0) token->category = UNION;
				else if (len == 9 && strncmp(lex, "intersect", 9) == 0) token->category = INTERSECT;
				else if (len == 5 && strncmp(lex, "minus",     5) == 0) token->category = MINUS;
				else if (len == 5 && strncmp(lex, "times",     5) == 0) token->category = TIMES;
				else if (len == 4 && strncmp(lex, "join",      4) == 0) token->category = JOIN;
				else if (len == 3 && strncmp(lex, "and",       3) == 0) token->category = AND;
				else if (len == 2 && strncmp(lex, "or",        2) == 0) token->category = OR;
				else if (len == 3 && strncmp(lex, "not",       3) == 0) token->category = NOT;
				else                                                    token->category = IDENT;
			}
			
			else if (lexer->cur_char == '-' || isdigit(lexer->cur_char))
			{
				if (lexer->cur_char == '-')
				{
					advance(lexer);
				}

				if (!isdigit(lexer->cur_char))
				{
					arena_pop(arena);
					lexer->has_error = true;
					LEX_ERR("Unexpected character '%c' at %d:%d", lexer->cur_char, token->pos.row, token->pos.col);
					continue;
				}
				
				char next_char;
				while (peek(lexer, &next_char) && isdigit(next_char)) advance(lexer);

				if (peek(lexer, &next_char) && next_char == '.')
				{
					advance(lexer);

					if (!peek(lexer, &next_char) || !isdigit(next_char))
					{
						arena_pop(arena);
						lexer->has_error = true;
						LEX_ERR("Unexpected character '%c' at %d:%d", lexer->cur_char, token->pos.row, token->pos.col);
						continue;
					}

					while (peek(lexer, &next_char) && isdigit(next_char)) advance(lexer);
				}

				token->lexeme_length = (lexer->file_contents + lexer->cur_pos) - token->lexeme_start;
				token->category = NUMBER;
			}

			else
			{
				arena_pop(arena);
				lexer->has_error = true;
				LEX_ERR("Unexpected character '%c' at %d:%d", lexer->cur_char, token->pos.row, token->pos.col);
				continue;
			}
			}
		}

		(*count_out)++;
	}
}
