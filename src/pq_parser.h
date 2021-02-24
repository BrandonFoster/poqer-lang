/**
 * @file pq_parser.h
 * @author Brandon Foster
 * @brief poqer-lang parser header.
 * the pq_parser struct is used to build a syntax tree of the poqer program.
 * create/destroy the parser with the pq_new_* and pq_del_* functions.
 * set the buffer with pq_parser_set_buffer function.
 * build the syntax tree with pq_parser_parse function.
 * When added, auxiliary functions for more functionality will be documented below.
 * 
 * @version 0.001
 * @date 11-5-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_PARSER_H
#define _PQ_PARSER_H
#include "pq_globals.h"
#include "pq_scanner.h"
#include "pq_syntax_tree.h"

/**
 * @brief The structure of a poqer-lang parser.
 */
typedef struct pq_parser
{   //these variables should only be read externally, not modified.

    //the poqer scanner
    pq_scanner* scanner;
    pq_tok* curr_tok;
    char* err;
} pq_parser;

/**
 * @brief Safe allocation for a pq_parser struct, initializes the parser, then returns the pointer.
 * 
 * @return A pointer to the allocated pq_parser struct.
 */
static inline pq_parser* pq_new_parser()
{
    pq_parser* parser = (pq_parser*)malloc(sizeof(pq_parser));
    if(!parser) return NULL;

    parser->scanner = pq_new_scanner();
    if(!parser->scanner)
    {
        free(parser);
        return NULL;
    }
    parser->curr_tok = NULL;
    parser->err = NULL;
    return parser;
}

/**
 * @brief Safe deallocation of a pq_parser struct.
 * 
 * @param parser The parser that will be deallocated.
 */
static inline void pq_del_parser(pq_parser* parser)
{
    if(!parser) return;

    if(parser->scanner) pq_del_scanner(parser->scanner);
    free(parser);
}

/**
 * @brief Sets the buffer of the parser.
 * The previous buffer is deallocated from the parser.
 * 
 * @param parser The parser that will be modified.
 * @param buffer A valid utf8 null-terminated c-string to be used (it will be deallocated upon replacement).
 */
static inline void pq_parser_set_buffer(pq_parser* parser, const char* buffer)
{
    pq_scanner_set_buffer(parser->scanner, buffer);
}

pq_syntax_tree* pq_parser_parse(pq_parser* parser);

#endif