/**
 * @file pq_scanner.c
 * @author Brandon Foster
 * @brief poqer-lang scanner implementation.
 * the internal implementation of the pq_scanner_* functions are documented below.
 * 
 * @version 0.006
 * @date 10-15-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#include "pq_scanner.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief The states for reading the regular expressions that builds a poqer token.
 */
typedef enum pq_scanner_state {
    PQ_SCANNER_STATE_BEGIN,
    PQ_SCANNER_STATE_POT_END,

    PQ_SCANNER_STATE_S_COMMENT,
    PQ_SCANNER_STATE_POT_M_COMMENT_OP,
    PQ_SCANNER_STATE_M_COMMENT,
    PQ_SCANNER_STATE_POT_M_COMMENT_CL,

    PQ_SCANNER_STATE_A_NAME,
    PQ_SCANNER_STATE_G_NAME,
    
    PQ_SCANNER_STATE_Q_NAME_OP,
    PQ_SCANNER_STATE_Q_NAME_ESC_SEQ,
    PQ_SCANNER_STATE_Q_NAME_OCT_ESC_SEQ,
    PQ_SCANNER_STATE_Q_NAME_POT_HEX_ESC_SEQ,
    PQ_SCANNER_STATE_Q_NAME_HEX_ESC_SEQ,
    PQ_SCANNER_STATE_POT_Q_NAME_CL,

    PQ_SCANNER_STATE_POT_RAD_INT,
    PQ_SCANNER_STATE_POT_BIN_INT,
    PQ_SCANNER_STATE_POT_OCT_INT,
    PQ_SCANNER_STATE_POT_DEC_INT,
    PQ_SCANNER_STATE_POT_HEX_INT,

    PQ_SCANNER_STATE_BIN_INT,
    PQ_SCANNER_STATE_OCT_INT,
    PQ_SCANNER_STATE_HEX_INT,

    PQ_SCANNER_STATE_POT_FLOAT_FRAC,
    PQ_SCANNER_STATE_FLOAT_FRAC,
    PQ_SCANNER_STATE_POT_FLOAT_EXP,
    PQ_SCANNER_STATE_POT_FLOAT_EXP_INT,
    PQ_SCANNER_STATE_FLOAT_EXP_INT,

    PQ_SCANNER_STATE_VAR
} pq_scanner_state;

/**
 * @brief Creates and stores a scanner error with the line and column of the scanner.
 * 
 * @param scanner The scanner that will be used.
 * @param err The error will be stored here.
 * @param str The null-terminated c-string error message.
 */
static inline void pq_scanner_make_error_from_cstr(pq_scanner* scanner, char** err, const char* str)
{
    //number of line and column digits to store.
    char ln_digits[21];
    char col_digits[21];

    //stores the digits.
#ifdef PQ_OS_LINUX
    sprintf(ln_digits, "%lu", scanner->ln);
    sprintf(col_digits, "%lu", scanner->col);
#elif PQ_OS_WINDOWS
    sprintf(ln_digits, "%llu", scanner->ln);
    sprintf(col_digits, "%llu", scanner->col);
#endif

    //allocates and uses string struct to build the error string.
    pq_string* err_string = (pq_string*)pq_new_string();
    pq_string_append_str(err_string, "[ln ");
    pq_string_append_str(err_string, ln_digits);
    pq_string_append_str(err_string, " col ");
    pq_string_append_str(err_string, col_digits);
    pq_string_append_str(err_string, "] error, ");
    pq_string_append_str(err_string, str);

    //stores the error string into the err reference then deallocates the string struct.
    *err = (char*)malloc(pq_string_get_size(err_string) + 1);
    strcpy(*err, pq_string_get_cstr(err_string));
    pq_del_string(err_string);
}

/**
 * @brief Moves the beginning of the lexeme after its current ending point.
 * If this cannot be done (the ending point is at the end of the buffer)
 * then the beginning is also moved to the end of the buffer. 
 * 
 * @param scanner The scanner that will be modified.
 */
static inline void pq_scanner_next_lexeme(pq_scanner* scanner)
{
    if(scanner->end < scanner->buffer_sz)
    {   //moves the beginning of the lexeme after its ending point if possible.
        scanner->beg = (scanner->end += scanner->cp_bytes);
        scanner->col++;
        scanner->cp_bytes = pq_utf8_to_cp(&scanner->cp, scanner->buffer + scanner->end);
    }
    else if(scanner->beg != scanner->end)
    {   //ensures that the beginning of the lexeme is at the ending point of the buffer.
        scanner->beg = scanner->end;
    }
}

/**
 * @brief Moves the beginning of the lexeme towards the end of the buffer.
 * This is used for skipping unnecessary values (e.g. whitespaces and comments).
 * 
 * @param scanner The scanner that will be modified.
 * @param i The amount of utf8 characters to skip.
 */
static inline void pq_scanner_skip(pq_scanner* scanner, size_t i)
{
    if(i > 0)
    {
        uint32_t cp;
        int8_t cp_bytes;
        for(int j = 0; j < i && scanner->beg <= scanner->buffer_sz - 1; ++j)
            scanner->beg += (cp_bytes = pq_utf8_to_cp(&cp, scanner->buffer + scanner->beg));
    }
}

/**
 * @brief Moves the ending point of the lexeme towards the beginning of the buffer.
 * This is used to allow the scanner to get the lexeme of the appropriate token upon token creation
 * and continue the next lexeme right after it.
 * 
 * @param scanner The scanner that will be modified.
 * @param i The amount of utf8 characters to rewind.
 */
static inline void pq_scanner_rewind(pq_scanner* scanner, size_t i)
{
    if(i > 0)
    {
        for(int j = i - 1; j >= 0 && scanner->end > 0; --j)
        {
            if(scanner->cp == '\n') scanner->ln--;
            while(!pq_is_utf8_1st_byte(*(scanner->buffer + --scanner->end)));
            scanner->col--;
        }
        scanner->cp_bytes = pq_utf8_to_cp(&scanner->cp, scanner->buffer + scanner->end);
    }
}

/**
 * @brief Moves the ending point of the lexeme towards the end of the buffer.
 * This is used to allow the scanner to read the next character after the current one is read successfully.
 * 
 * @param scanner The scanner that will be modified.
 * @param i The amount of utf8 characters to forward.
 */
static inline void pq_scanner_forward(pq_scanner* scanner, size_t i)
{
    if(i > 0)
    {
        for(int j = 0; j < i && scanner->end <= scanner->buffer_sz - 1; ++j)
        {
            if(scanner->cp == '\n') scanner->ln++;
            scanner->end += scanner->cp_bytes;
            scanner->cp_bytes = pq_utf8_to_cp(&scanner->cp, scanner->buffer + scanner->end);
            scanner->col++;
        }
    }
}

/**
 * @brief Creates and returns a utf8 null-terminated c-string containing the current lexeme.
 * Please deallocate the c-string after use.
 * 
 * @param scanner The scanner that will be used.
 * @return The created utf8 null-terminated c-string containing the current lexeme.
 */
static inline char* pq_scanner_get_lexeme(const pq_scanner* scanner)
{
    size_t lexeme_sz = scanner->end - scanner->beg + scanner->cp_bytes;
    char* lexeme = malloc(lexeme_sz + 1); //+1 for the null character.
    strncpy(lexeme, scanner->buffer + scanner->beg, lexeme_sz);
    lexeme[lexeme_sz] = '\0';
    return lexeme;
}

/**
 * @brief Sets the quote character that will be used for the quoted atom.
 * 
 * @param scanner The scanner that will be used.
 * @param cp The unicode codepoint that will be used as the open/closing quote for the quoted atom name.
 */
static inline void pq_scanner_quoted_atom_set_quote(pq_scanner* scanner, const uint32_t cp)
{
    char utf8ch[PQ_UTF8_LEN_MAX];
    pq_cp_to_utf8(utf8ch, cp);
    pq_string_clear(scanner->quoted_atom_escape);
    pq_string_clear(scanner->quoted_atom_name);
    pq_string_append_str(scanner->quoted_atom_name, utf8ch);
    scanner->quoted_atom_append_mode = 0;
    return;
}

/**
 * @brief Appends a unicode character to the quoted atom name or the escaped sequence depending on the append mode.
 * 
 * @param scanner The scanner that will be used.
 * @param cp The unicode codepoint that will be appended to the quoted atom.
 */
static inline void pq_scanner_quoted_atom_append(pq_scanner* scanner, const uint32_t cp)
{
    char utf8ch[PQ_UTF8_LEN_MAX];
    pq_cp_to_utf8(utf8ch, cp);
    switch(scanner->quoted_atom_append_mode)
    {
    case 0:
        pq_string_append_str(scanner->quoted_atom_name, utf8ch);
        break;
    case 1:
    case 2:
        if(pq_string_get_char(scanner->quoted_atom_escape, 0) == '0' && pq_string_get_char(scanner->quoted_atom_escape, 1) == '\0')
        {   //removes unnecessary leading 0s.
            pq_string_clear(scanner->quoted_atom_escape);
        }
        pq_string_append_str(scanner->quoted_atom_escape, utf8ch);
        break;
    }
    
    return;
}

/**
 * @brief Appends the escaped sequence if the append mode is currently oct or hex then resets the mode.
 * 
 * @param scanner The scanner that will be used.
 * @return int8_t Whether the utf8 character from the sequence was appended to the string. 1 for yes, 0 for no.
 */
static inline int8_t pq_scanner_quoted_atom_append_escape(pq_scanner* scanner)
{
    if(scanner->quoted_atom_append_mode == 1)
    {   //appends the octal escape sequence
        if(pq_string_get_size(scanner->quoted_atom_escape) > 7)
        {
            pq_string_clear(scanner->quoted_atom_escape);
            scanner->quoted_atom_append_mode = 0;
            return 0;
        }
        uint32_t cp = strtol(pq_string_get_cstr(scanner->quoted_atom_escape), NULL, 8);
        pq_string_clear(scanner->quoted_atom_escape);
        scanner->quoted_atom_append_mode = 0;
        if(cp <= 0x10FFFF)
        {
            pq_scanner_quoted_atom_append(scanner, cp);
            return 1;
        }
    }
    else if(scanner->quoted_atom_append_mode == 2)
    {   //appends the hexadecimal escape sequence
        if(pq_string_get_size(scanner->quoted_atom_escape) > 6)
        {
            pq_string_clear(scanner->quoted_atom_escape);
            scanner->quoted_atom_append_mode = 0;
            return 0;
        }
        uint32_t cp = strtol(pq_string_get_cstr(scanner->quoted_atom_escape), NULL, 16);
        pq_string_clear(scanner->quoted_atom_escape);
        scanner->quoted_atom_append_mode = 0;
        if(cp <= 0x10FFFF)
        {
            pq_scanner_quoted_atom_append(scanner, cp);
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Creates and returns a utf8 null-terminated c-string containing the current lexeme.
 * Please deallocate the c-string after use.
 * 
 * @param scanner The scanner that will be used.
 * @return The created utf8 null-terminated c-string containing the current lexeme.
 */
static inline char* pq_scanner_quoted_atom_get_lexeme(const pq_scanner* scanner)
{
    char* lexeme = malloc(pq_string_get_size(scanner->quoted_atom_name) + 1);
    return strcpy(lexeme, pq_string_get_cstr(scanner->quoted_atom_name));
}

pq_tok* pq_scanner_next_token(pq_scanner* scanner, char** err)
{
    *err = NULL;

    //holds the current state of the scanner to reset upon lexer error.
    size_t ln = scanner->ln;
    size_t col = scanner->col;
    size_t beg = scanner->beg;
    size_t end = scanner->end;

    //starts reading one utf8 character at a time via their codepoint.
    pq_scanner_state state = PQ_SCANNER_STATE_BEGIN;
    for(;;)
    {
        switch(state)
        {
        case PQ_SCANNER_STATE_BEGIN:
            //checks for the beginning of a token.
            switch(scanner->cp)
            {
            //halts the lexer.
            case '\0':
                return NULL;

            //starts skipping single-line comment.
            case '%':
                pq_scanner_skip(scanner, 1);
                state = PQ_SCANNER_STATE_S_COMMENT;
                break;

            //the start of a multi-line comment or a graphic atom.
            case '/':
                state = PQ_SCANNER_STATE_POT_M_COMMENT_OP;
                break;

            //generates some special tokens.
            case '(':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_LPAR_TOK, "(", 0);

            case ')':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_RPAR_TOK, ")", 0);

            case '[':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_LLIST_TOK, "[", 0);

            case ']':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_RLIST_TOK, "]", 0);

            case '{':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_LCURLY_TOK, "{", 0);

            case '}':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_RCURLY_TOK, "}", 0);

            case '|':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_HT_SEP_TOK, "|", 0);
            
            case ',':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_COMMA_TOK, ",", 0);

            //generates some single-token atoms.
            case ';':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_NAME_TOK, ";", 0);

            case '!':
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_NAME_TOK, "!", 0);

            //the start of a single quoted atom.
            case '\'':
            case '"':
            case '`':
                pq_scanner_quoted_atom_set_quote(scanner, scanner->cp);
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;

            //the start of a graphic atom or an end token.
            case '.':
                state = PQ_SCANNER_STATE_POT_END;
                break;

            //the start of a binary, octal, decimal, or hexadecimal integer or a floating-point number.
            case '0':
                state = PQ_SCANNER_STATE_POT_RAD_INT;
                break;

            default:
                if(pq_is_unicode_layout_char(scanner->cp))
                {   //skips layout characters (white spaces).
                    pq_scanner_skip(scanner, 1);
                }
                else if(pq_is_unicode_lower_char(scanner->cp))
                {   //the start of an alphanumeric atom.
                    state = PQ_SCANNER_STATE_A_NAME;
                }
                else if(pq_is_unicode_dec_char(scanner->cp))
                {   //the start of a decimal or a floating-point number.
                    //only for the numbers 1 to 9. 0 is handled above.
                    state = PQ_SCANNER_STATE_POT_DEC_INT;
                }
                else if(pq_is_unicode_graphic_token_char(scanner->cp))
                {   //the start of a graphic atom.
                    state = PQ_SCANNER_STATE_G_NAME;
                }
                else if(pq_is_unicode_alnum_char(scanner->cp))
                {   //the start of a variable.
                    state = PQ_SCANNER_STATE_VAR;
                }
                else
                {   //lexer error: unrecognized character read.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized character");
                    break;
                }
                break;
            }
            break;

        //period token states.
        case PQ_SCANNER_STATE_POT_END:
            //the start of a graphic atom or an end token (optionally followed by a single  line comment).
            if(pq_is_unicode_graphic_token_char(scanner->cp))
            {   //the start of a graphic atom.
                if('%' == scanner->cp)
                {   //an end token followed by a single line comment.
                    //generates the end token and rewinds the scanner to begin at the single line comment.
                    pq_scanner_rewind(scanner, 1);
                    pq_scanner_next_lexeme(scanner);
                    return pq_new_str_literal_token(PQ_END_TOK, ".", 0);
                }
                else
                {   //the start of a graphic atom.
                    state = PQ_SCANNER_STATE_G_NAME;
                }
            }
            else if(pq_is_unicode_layout_char(scanner->cp) || '\0' == scanner->cp)
            {   //generates an end token and rewinds the scanner to the current character.
                pq_scanner_rewind(scanner, 1);
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_END_TOK, ".", 0);
            }
            else
            {   //generates the graphic token '.' as a name token and rewinds the scanner to the current character.
                pq_scanner_rewind(scanner, 1);
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_literal_token(PQ_NAME_TOK, ".", 0);
            }
            
            break;

        //single comment states.
        case PQ_SCANNER_STATE_S_COMMENT:
            //single comment was previously read, skips following characters until a newline or eof is reached.
            if(pq_is_unicode_newline_char(scanner->cp) || '\0' == scanner->cp)
            {   //resets the states when a newline or eof is reached.
                state = PQ_SCANNER_STATE_BEGIN;
            }
            pq_scanner_skip(scanner, 1);
            break;

        //multi-line comment states
        case PQ_SCANNER_STATE_POT_M_COMMENT_OP:
            //forward slash was previously read, could be the start of a graphic atom or multi-line comment.
            if('*' == scanner->cp)
            {   //starts skipping the multi-line comment body.
                state = PQ_SCANNER_STATE_M_COMMENT;
                pq_scanner_skip(scanner, 2); //for skipping the previous '/' and current '*'.
            }
            else
            {   //starts reading a graphic atom.
                pq_scanner_rewind(scanner, 1); //required to properly process the current character into the graphic atom.
                state = PQ_SCANNER_STATE_G_NAME;
            }
            break;

        case PQ_SCANNER_STATE_M_COMMENT:
            //skips the characters in the multi-line comment body.
            if('*' == scanner->cp)
            {   //the start of the terminating multi-line comment sequence or another arbitrary * to skip.
                state = PQ_SCANNER_STATE_POT_M_COMMENT_CL;
            }
            else if('\0' == scanner->cp)
            {   //lexer error: did not terminate the multiline comment.
                pq_scanner_make_error_from_cstr(scanner, err, "expected end of multi-line comment");
                break;
            }
            pq_scanner_skip(scanner, 1);
            break;

        case PQ_SCANNER_STATE_POT_M_COMMENT_CL:
            //checks for the closing character of the multi-line comment or continues the skipping of its comment text.
            //another '*' will keep the lexer in this state.
            switch(scanner->cp)
            {
            //the closing character of the multi-line comment found, resets the lexer.
            case '/':
                state = PQ_SCANNER_STATE_BEGIN;
                break;
            
            //lexer error: did not terminate the comment text.
            case '\0':
                pq_scanner_make_error_from_cstr(scanner, err, "expected end of multi-line comment");
                break;

            //remains in the current state.
            case '*':
                break;

            //arbitrary character other than '*' was found, goes back to skipping the comment text.
            default:
                state = PQ_SCANNER_STATE_M_COMMENT;
                break;
            }
            pq_scanner_skip(scanner, 1);
            break;

        //atom states.
        case PQ_SCANNER_STATE_A_NAME:
            //reads an alphanumerical atom.
            if(!pq_is_unicode_alnum_char(scanner->cp))
            {   //creates the alphanumerical atom the moment an alphanum character is not found.
                //and rewinds the scanner back to the non-alphanum character.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_token(PQ_NAME_TOK, lexeme, 0);
            }
            break;

        case PQ_SCANNER_STATE_Q_NAME_OP:
            //reads the start of a quoted atom.
            switch(scanner->cp)
            {
            //checks for the end of the single quoted atom.
            case '\'':
            case '"':
            case '`':
                if(scanner->cp == pq_string_get_char(scanner->quoted_atom_name, 0))
                {   //either the closing quote of the quoted atom or two consecutive quotes that will be appended as one quote.
                    state = PQ_SCANNER_STATE_POT_Q_NAME_CL;
                }
                else
                {   //appends the non-closing quote to the quoted atom.
                    pq_scanner_quoted_atom_append(scanner, scanner->cp);
                }
                break;
            
            //lexer error: did not terminate the single quoted atom.
            case '\0':
                pq_scanner_make_error_from_cstr(scanner, err, "expected closing quotation");
                break;

            //appends single quoted escape sequences.
            case '\a':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            case '\v':
                pq_scanner_quoted_atom_append(scanner, scanner->cp);
                break;
            
            case '\\':
                //the start of a single quote escape sequence.
                state = PQ_SCANNER_STATE_Q_NAME_ESC_SEQ;
                break;

            default:
                if(pq_is_unicode_single_quoted_token_char(scanner->cp))
                {   //appends the character to the quoted atom.
                    pq_scanner_quoted_atom_append(scanner, scanner->cp);
                }
                else
                {   //lexer error: unrecognized single quoted character.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized quote character");
                }
                break;
            }
            break;

        case PQ_SCANNER_STATE_POT_Q_NAME_CL:
            //checks for either the end of the single quoted atom or appends a single quote.
            if(scanner->cp == pq_string_get_char(scanner->quoted_atom_name, 0))
            {   //double end quotes within a quoted atom are read as that quote in the atom's name.
                pq_scanner_quoted_atom_append(scanner, scanner->cp);
                state = PQ_SCANNER_STATE_Q_NAME_OP;
            }
            else
            {   //end of the quoted atom.
                pq_scanner_rewind(scanner, 1);
                pq_string_append_char(scanner->quoted_atom_name, pq_string_get_char(scanner->quoted_atom_name, 0));
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_token(PQ_NAME_TOK, pq_scanner_quoted_atom_get_lexeme(scanner), 0);
            }
            break;

        case PQ_SCANNER_STATE_Q_NAME_ESC_SEQ:
            //checks for single escape sequence, octal escape sequence, or hexadecimal escape sequence.
            switch(scanner->cp)
            {
            //appends control escape then goes back to reading the single quote body.
            case 'a':
                pq_scanner_quoted_atom_append(scanner, '\a');
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;
            case 'b':
                pq_scanner_quoted_atom_append(scanner, '\b');
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;
            case 'f':
                pq_scanner_quoted_atom_append(scanner, '\f');
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;
            case 'n':
                pq_scanner_quoted_atom_append(scanner, '\n');
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;
            case 'r':
                pq_scanner_quoted_atom_append(scanner, '\r');
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;
            case 't':
                pq_scanner_quoted_atom_append(scanner, '\t');
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;
            case 'v':
                pq_scanner_quoted_atom_append(scanner, '\v');
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;

            //appends meta escape then goes back to reading the single quote body.
            case '\\':
            case '\'':
            case '"':
            case '`':
                pq_scanner_quoted_atom_append(scanner, scanner->cp);
                state = PQ_SCANNER_STATE_Q_NAME_OP;
                break;
            
            //either the start of a hexadecimal escape sequence or an error.
            case 'x':
                state = PQ_SCANNER_STATE_Q_NAME_POT_HEX_ESC_SEQ;
                break;
            
            default:
                if(pq_is_unicode_oct_char(scanner->cp))
                {   //starts the octal escape sequence.
                    scanner->quoted_atom_append_mode = 1;
                    pq_scanner_quoted_atom_append(scanner, scanner->cp);
                    state = PQ_SCANNER_STATE_Q_NAME_OCT_ESC_SEQ;
                }
                else
                {   //lexer error: unrecognized escape sequence character.
                    pq_scanner_make_error_from_cstr(scanner, err, "illegal escape sequence character");
                }
                break;
            }
            break;

        case PQ_SCANNER_STATE_Q_NAME_OCT_ESC_SEQ:
            //reads the octal escape sequence.
            switch(scanner->cp)
            {
            //end of the octal sequence.
            case '\\':
                if(pq_scanner_quoted_atom_append_escape(scanner))
                {   //successfully appends the escaped octal sequence and goes back to reading the quoted atom.
                    state = PQ_SCANNER_STATE_Q_NAME_OP;
                }
                else
                {   //lexer error: unrecognized octal escape sequence.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized octal escape sequence character");
                }
                break;

            //not a part of the ISO standard, but uses a single quote or layout text char as the end as well.
            case ' ':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            case '\v':
                if(pq_scanner_quoted_atom_append_escape(scanner))
                {   //successfully appends the escaped octal sequence and goes back to reading the quoted atom.
                    pq_scanner_rewind(scanner, 1);
                    state = PQ_SCANNER_STATE_Q_NAME_OP;
                }
                else
                {   //lexer error: unrecognized octal escape sequence.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized octal escape sequence character");
                }
                break;

            default:
                //not a part of the ISO standard, but uses a closing quote as the end as well.
                if(scanner->cp == pq_string_get_char(scanner->quoted_atom_name, 0))
                {
                    if(pq_scanner_quoted_atom_append_escape(scanner))
                    {   //successfully appends the escaped octal sequence and goes back to reading the quoted atom.
                        pq_scanner_rewind(scanner, 1);
                        state = PQ_SCANNER_STATE_Q_NAME_OP;
                    }
                    else
                    {   //lexer error: unrecognized octal escape sequence.
                        pq_scanner_make_error_from_cstr(scanner, err, "unrecognized octal escape sequence character");
                    }
                }
                else if(pq_is_unicode_oct_char(scanner->cp))
                {   //continues reading the octal sequence.
                    pq_scanner_quoted_atom_append(scanner, scanner->cp);
                }
                else
                {   //lexer error: unrecognized octal escape sequence.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized octal escape sequence character");
                }
                break;
            }
            break;

        case PQ_SCANNER_STATE_Q_NAME_POT_HEX_ESC_SEQ:
            //reads the first hexadecimal escape sequence number or an error.
            if(pq_is_unicode_hex_char(scanner->cp))
            {   //starts the hexadecimal escape sequence.
                scanner->quoted_atom_append_mode = 2;
                pq_scanner_quoted_atom_append(scanner, scanner->cp);
                state = PQ_SCANNER_STATE_Q_NAME_HEX_ESC_SEQ;
            }
            else
            {   //lexer error: unrecognized hexadecimal escape sequence character.
                pq_scanner_make_error_from_cstr(scanner, err, "unrecognized hexadecimal escape sequence character");
            }
            break;

        case PQ_SCANNER_STATE_Q_NAME_HEX_ESC_SEQ:
            //reads the hexadecimal escape sequence.
            switch(scanner->cp)
            {
            //end of the hexadecimal sequence.
            case '\\':
                if(pq_scanner_quoted_atom_append_escape(scanner))
                {   //successfully appends the escaped hexadecimal sequence and goes back to reading the quoted atom.
                    state = PQ_SCANNER_STATE_Q_NAME_OP;
                }
                else
                {   //lexer error: unrecognized hexadecimal escape sequence.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized hexadecimal escape sequence character");
                }
                break;

            //not a part of the ISO standard, but uses a single quote or layout text char as the end as well.
            case ' ':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            case '\v':
                if(pq_scanner_quoted_atom_append_escape(scanner))
                {   //successfully appends the escaped hexadecimal sequence and goes back to reading the quoted atom.
                    pq_scanner_rewind(scanner, 1);
                    state = PQ_SCANNER_STATE_Q_NAME_OP;
                }
                else
                {   //lexer error: unrecognized hexadecimal escape sequence.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized hexadecimal escape sequence character");
                }
                break;

            default:
                //not a part of the ISO standard, but uses a closing quote as the end as well.
                if(scanner->cp == pq_string_get_char(scanner->quoted_atom_name, 0))
                {
                    if(pq_scanner_quoted_atom_append_escape(scanner))
                    {   //successfully appends the escaped hexadecimal sequence and goes back to reading the quoted atom.
                        pq_scanner_rewind(scanner, 1);
                        state = PQ_SCANNER_STATE_Q_NAME_OP;
                    }
                    else
                    {   //lexer error: unrecognized hexadecimal escape sequence.
                        pq_scanner_make_error_from_cstr(scanner, err, "unrecognized hexadecimal escape sequence character");
                    }
                }
                else if(pq_is_unicode_hex_char(scanner->cp))
                {   //continues reading the hexadecimal sequence.
                    pq_scanner_quoted_atom_append(scanner, scanner->cp);
                }
                else
                {   //lexer error: unrecognized hexadecimal escape sequence.
                    pq_scanner_make_error_from_cstr(scanner, err, "unrecognized hexadecimal escape sequence character");
                }
                break;
            }
            break;

        case PQ_SCANNER_STATE_G_NAME:
            //reads a graphic atom.
            if(!pq_is_unicode_graphic_token_char(scanner->cp))
            {   //creates the graphic atom the moment a graphic token character is not found.
                //and rewinds the scanner back to the non-graphic token character.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_token(PQ_NAME_TOK, lexeme, 0);
            }
            break;

        //numeric literals states
        case PQ_SCANNER_STATE_POT_RAD_INT:
            //the starting character of the lexeme is a 0.
            switch(scanner->cp)
            {
            //checks for a binary literal or just a 0 followed by an atom beginning with b.
            case 'b':
                state = PQ_SCANNER_STATE_POT_BIN_INT;
                break;
            
            //checks for an octal literal or just a 0 followed by an atom beginning with o.
            case 'o':
                state = PQ_SCANNER_STATE_POT_OCT_INT;
                break;
            
            //checks for an hexadecimal literal or just a 0 followed by an atom beginning with x.
            case 'x':
                state = PQ_SCANNER_STATE_POT_HEX_INT;
                break;

            default:
                if(pq_is_unicode_dec_char(scanner->cp))
                {   //checks for either a decimal integer or floating-point number.
                    state = PQ_SCANNER_STATE_POT_DEC_INT;
                }
                else if('.' == scanner->cp)
                {   //checks for the fractional part of a floating-point number.
                    state = PQ_SCANNER_STATE_POT_FLOAT_FRAC;
                }
                else
                {   //generates the 0 into a decimal and reset the states at the current character.
                    pq_scanner_rewind(scanner, 1);
                    pq_scanner_next_lexeme(scanner);
                    return pq_new_int_token(PQ_INT_TOK, 0, 0);
                }
                break;
            }
            break;

        case PQ_SCANNER_STATE_POT_BIN_INT:
            //checks for the remaining part for the binary int or generates a 0 integer.
            if(pq_is_unicode_bin_char(scanner->cp))
            {   //reads the remaining part of the binary int.
                state = PQ_SCANNER_STATE_BIN_INT;
            }
            else
            {   //generates the 0 into a decimal and reset the states at the previous 'b'.
                pq_scanner_rewind(scanner, 2);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, 0, 0);
            }
            break;

        case PQ_SCANNER_STATE_BIN_INT:
            //reads the remaining part of the binary int.
            if(!pq_is_unicode_bin_char(scanner->cp))
            {   //generates the binary integer.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQint val = strtoll(lexeme+2, NULL, 2);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, val, 0);
            }
            break;
        
        case PQ_SCANNER_STATE_POT_OCT_INT:
            //checks for the remaining part for the octal int or generates a 0 integer.
            if(pq_is_unicode_oct_char(scanner->cp))
            {   //reads the remaining part of the octal int.
                state = PQ_SCANNER_STATE_OCT_INT;
            }
            else
            {   //generates the 0 into a decimal and reset the states at the previous 'o'.
                pq_scanner_rewind(scanner, 2);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, 0, 0);
            }
            break;

        case PQ_SCANNER_STATE_OCT_INT:
            //reads the remaining part of the octal int.
            if(!pq_is_unicode_oct_char(scanner->cp))
            {   //generates the octal integer.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQint val = strtoll(lexeme+2, NULL, 8);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, val, 0);
            }
            break;

        case PQ_SCANNER_STATE_POT_DEC_INT:
            //checks for the remaining part for the decimal int or a floating-point number.
            if('.' == scanner->cp)
            {   //checks for the fractional part of a floating-point number.
                state = PQ_SCANNER_STATE_POT_FLOAT_FRAC;
            }
            else if(!pq_is_unicode_dec_char(scanner->cp))
            {   //generates the decimal integer.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQint val = strtoll(lexeme, NULL, 10);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, val, 0);
            }
            break;
        
        case PQ_SCANNER_STATE_POT_HEX_INT:
            //checks for the remaining part for the hex int or generates a 0 integer.
            if(pq_is_unicode_hex_char(scanner->cp))
            {   //reads the remaining part of the hex int.
                state = PQ_SCANNER_STATE_HEX_INT;
            }
            else
            {   //generates the 0 into a decimal and reset the states at the previous 'x'.
                pq_scanner_rewind(scanner, 2);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, 0, 0);
            }
            break;

        case PQ_SCANNER_STATE_HEX_INT:
            //reads the remaining part of the hex int.
            if(!pq_is_unicode_hex_char(scanner->cp))
            {   //generates the hex integer.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQint val = strtoll(lexeme+2, NULL, 16);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, val, 0);
            }
            break;

        //floating-point states
        case PQ_SCANNER_STATE_POT_FLOAT_FRAC:
            //checks if the decimal point is for a floating-point value or for a different token.
            if(pq_is_unicode_dec_char(scanner->cp))
            {   //if a decimal was found, starts reading the rest of the floating-point value.
                state = PQ_SCANNER_STATE_FLOAT_FRAC;
            }
            else
            {   //the decimal point is for a different token.
                //generates the decimal integer before it and rewinds the scanner back to the decimal point.
                pq_scanner_rewind(scanner, 2);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQint val = strtoll(lexeme, NULL, 10);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_int_token(PQ_INT_TOK, val, 0);
            }
            break;

        case PQ_SCANNER_STATE_FLOAT_FRAC:
            //checks for the start of a potential exponent, more digits in the fractional part,
            //or generates the floating-point obtain so far.
            if('e' == scanner->cp || 'E' == scanner->cp)
            {   //the start of a potential exponent.
                state = PQ_SCANNER_STATE_POT_FLOAT_EXP;
            }
            else if(!pq_is_unicode_dec_char(scanner->cp))
            {   //generates the floating-point obtain so far and rewinds the scanner back to the unknown character.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQflt val = strtod(lexeme, NULL);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_flt_token(PQ_FLT_TOK, val, 0);
            }
            break;

        case PQ_SCANNER_STATE_POT_FLOAT_EXP:
            //the start of the exponent part or the 'E'/'e' is being used for another token.
            if('+' == scanner->cp || '-' == scanner->cp)
            {   //checks for the potential integer part for the exponent.
                //if it's not found, the 'E'/'e' and '+'/'-' are for two different tokens.
                state = PQ_SCANNER_STATE_POT_FLOAT_EXP_INT;
            }
            else if(pq_is_unicode_dec_char(scanner->cp))
            {   //checks for the rest of the integer part for the exponent.
                state = PQ_SCANNER_STATE_FLOAT_EXP_INT;
            }
            else
            {   //the 'E'/'e' is being used for another token.
                //generates the floating-point value so far and rewinds the scanner back to the 'E'/'e'.
                pq_scanner_rewind(scanner, 2);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQflt val = strtod(lexeme, NULL);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_flt_token(PQ_FLT_TOK, val, 0);
            }
            break;
        
        case PQ_SCANNER_STATE_POT_FLOAT_EXP_INT:
            //the start of the integer part of the exponent or 'E'/'e' and '+'/'-' are for two different tokens.
            if(pq_is_unicode_dec_char(scanner->cp))
            {   //checks for the rest of the integer part for the exponent.
                state = PQ_SCANNER_STATE_FLOAT_EXP_INT;
            }
            else
            {   //the 'E'/'e' and '+'/'-' are for two different tokens.
                //generates the floating-point obtain so far and rewinds the scanner to the 'E'/'e'
                pq_scanner_rewind(scanner, 3);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQflt val = strtod(lexeme, NULL);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_flt_token(PQ_FLT_TOK, val, 0);
            }
            break;

        case PQ_SCANNER_STATE_FLOAT_EXP_INT:
            if(!pq_is_unicode_dec_char(scanner->cp))
            {   //generates the floating-point value
                //and rewinds the scanner back to the non-decimal character.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                PQflt val = strtod(lexeme, NULL);
                free(lexeme);
                pq_scanner_next_lexeme(scanner);
                return pq_new_flt_token(PQ_FLT_TOK, val, 0);
            }
            break;

        //variable states
        case PQ_SCANNER_STATE_VAR:
            if(!pq_is_unicode_alnum_char(scanner->cp))
            {   //creates the variable the moment a alphanum token character is not found.
                //and rewinds the scanner back to the non-alphanum token character.
                pq_scanner_rewind(scanner, 1);
                char* lexeme = pq_scanner_get_lexeme(scanner);
                pq_scanner_next_lexeme(scanner);
                return pq_new_str_token(PQ_VAR_TOK, lexeme, 0);
            }
            break;

        default:
            //undefined state was entered.
            pq_scanner_make_error_from_cstr(scanner, err, "unhandled scanner state");
            break;
        }

        if(*err != NULL) break;
        pq_scanner_forward(scanner, 1); //goes to the next utf8 char.
    }

    //resets the scanner back upon lexer error.
    scanner->ln = ln;
    scanner->col = col;
    scanner->beg = beg;
    scanner->end = end;

    return NULL;
}