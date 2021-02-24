/**
 * @file pq_scanner.h
 * @author Brandon Foster
 * @brief poqer-lang scanner header.
 * the pq_scanner struct is used to convert utf8 strings into poqer tokens.
 * create/destroy the scanner with the pq_new_* and pq_del_* functions.
 * set the buffer with pq_scanner_set_buffer function.
 * read tokens with pq_scanner_next_token function.
 * When added, auxiliary functions for more functionality will be documented below.
 * 
 * @version 0.003
 * @date 10-12-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_SCANNER_H
#define _PQ_SCANNER_H
#include "pq_globals.h"
#include "pq_unicode.h"
#include "pq_string.h"
#include "pq_token.h"
#include <stdlib.h>

/**
 * @brief The structure of a poqer-lang scanner.
 */
typedef struct pq_scanner
{   //these variables should only be read externally, not modified.

    //general use.
    const char* buffer; //the utf8 string that is being read.
    size_t buffer_sz; //the size of the buffer, excluding the null character.
    size_t ln; //current line position in the buffer.
    size_t col; //current column position in the buffer (counted in terms of unicode characters).
    size_t beg; //the lexeme's beginning position in the buffer.
    size_t end; //the lexeme's ending position in the buffer.
    
    //used to represent a single utf8 character.
    uint32_t cp; //the codepoint of the current unicode char.
    int8_t cp_bytes; //the number of bytes in the current unicode char.
    
    //quoted atom helpers.
    pq_string *quoted_atom_name; //the name of the quoted atom that will be used as the lexeme of the atom token.
    pq_string *quoted_atom_escape; //the numerical value of the hex/oct escape sequence.
    int8_t quoted_atom_append_mode; //whether the quote is reading a hex/oct escape sequence. 0 for none, 1 for oct, 2 for hex.
} pq_scanner;

/**
 * @brief Safe allocation for a pq_scanner struct, initializes the scanner, then returns the pointer.
 * 
 * @return A pointer to the allocated pq_scanner struct.
 */
static inline pq_scanner* pq_new_scanner()
{
    pq_scanner* scanner = (pq_scanner*)malloc(sizeof(pq_scanner));
    if(!scanner) return NULL;

    scanner->quoted_atom_name = pq_new_string();
    if(!scanner->quoted_atom_name)
    {
        free(scanner);
        return NULL;
    }
    scanner->quoted_atom_escape = pq_new_string();
    if(!scanner->quoted_atom_escape)
    {
        free(scanner->quoted_atom_name);
        free(scanner);
        return NULL;
    }
    scanner->buffer = NULL;
    scanner->buffer_sz = 0;
    scanner->ln = 1;
    scanner->col = 1;
    scanner->beg = 0;
    scanner->end = 0;
    scanner->cp = 0;
    scanner->cp_bytes = 0;
    return scanner;
}

/**
 * @brief Safe deallocation of a pq_scanner struct.
 * 
 * @param scanner The scanner that will be deallocated.
 */
static inline void pq_del_scanner(pq_scanner* scanner)
{
    if(!scanner) return;

    if(scanner->buffer)
        free((void*)scanner->buffer);
    if(scanner->quoted_atom_escape)
        pq_del_string(scanner->quoted_atom_escape);
    if(scanner->quoted_atom_name)
        pq_del_string(scanner->quoted_atom_name);
    free(scanner);
}

/**
 * @brief Sets the buffer of the scanner.
 * The previous buffer is deallocated and the scanner's state resets to the start of the new buffer.
 * 
 * @param scanner The scanner that will be modified.
 * @param buffer A valid utf8 null-terminated c-string to be used (it will be deallocated upon replacement).
 */
static inline void pq_scanner_set_buffer(pq_scanner* scanner, const char* buffer)
{
    if(scanner->buffer) free((void*)scanner->buffer);
    scanner->buffer = buffer;
    scanner->buffer_sz = strlen(scanner->buffer);
    scanner->beg = scanner->end = 0;
    scanner->ln = scanner->col = 1;
    scanner->cp_bytes = pq_utf8_to_cp(&scanner->cp, scanner->buffer);
}

/**
 * @brief Reads the next token in the scanner.
 * The scanner's lexing position will change when a token is found or layout characters and comments are being skipped.
 * So the preferred method to obtain all readable tokens in the buffer is to call this in a loop.
 * If a token is found, it's pointer is returned.
 * If a token is not found or a lexer error occurs, NULL is returned.
 * If an invalid utf8 c-string is used as the buffer, the behavior is undefined.
 * If there is not enough memory for required operations, the behavior is undefined.
 * 
 * @param scanner The scanner that will be used.
 * @param err The error message is stored here if any. 
 * @return The pointer to the next token found else NULL.
 */
pq_tok* pq_scanner_next_token(pq_scanner* scanner, char** err);

#endif