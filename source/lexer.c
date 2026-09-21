#include "lexer.h"

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
		// TODO: Missing EOF handling. If a peek ahead was done in the previous loop,
		// or char c = cur_pos++ lead to an EOF, it needs to be handled here
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
			arena_pop(arena);
			lexer->has_error = true;

			LEX_ERR("Unexpected character '%c' at %d:%d", lexer->cur_char, token->pos.row, token->pos.col);
			continue;
		}
		}

		(*count_out)++;
	}
}
