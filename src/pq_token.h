/**
 * @file pq_token.h
 * @author Brandon Foster
 * @brief poqer-lang token header.
 * the pq_tok struct is used to hold a poqer token.
 * at this moment, the requirements of a poqer token follows from a prolog token.
 * 
 * @version 0.002
 * @date 10-11-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_TOKEN_H
#define _PQ_TOKEN_H
#include "pq_globals.h"
#include <stdlib.h>

typedef enum pq_tag
{
    PQ_NAME_TOK,   //Ex: myatom123, my_atom123, my_Atom123
    PQ_INT_TOK,    //Ex: Dec 16, Oct 020, Hex 0x10
    PQ_FLT_TOK,    //Ex: 3.14, 1.23e-2
    PQ_VAR_TOK,    //Ex: _, MyVar123, _MyVar123

    PQ_LPAR_TOK,   //(
    PQ_RPAR_TOK,   //)
    PQ_LLIST_TOK,  //[
    PQ_RLIST_TOK,  //]
    PQ_LCURLY_TOK, //{
    PQ_RCURLY_TOK, //}
    
    PQ_HT_SEP_TOK, //|
    PQ_COMMA_TOK,  //,
    PQ_END_TOK     //.
} pq_tag;

typedef union pq_val
{
    PQstr s;
    PQflt f;
    PQint i;
} pq_val;

typedef struct pq_tok
{
    pq_tag tag; //Token Type
    pq_val val; //Token Value
    int8_t pri; //Token Priority

    int8_t _dealloc_str;
} pq_tok;

static inline pq_tok* pq_new_str_token(const pq_tag tag, const PQstr val, const int8_t pri)
{
    pq_tok* tok = malloc(sizeof(pq_tok));
    if(NULL == tok) return NULL;

    tok->tag = tag;
    tok->val.s = val;
    tok->pri = pri;
    tok->_dealloc_str = 1;
    return tok;
}

static inline pq_tok* pq_new_str_literal_token(const pq_tag tag, const PQstr val, const int8_t pri)
{
    pq_tok* tok = malloc(sizeof(pq_tok));
    if(NULL == tok) return NULL;

    tok->tag = tag;
    tok->val.s = val;
    tok->pri = pri;
    tok->_dealloc_str = 0;
    return tok;
}

static inline pq_tok* pq_new_flt_token(const pq_tag tag, const PQflt val, const int8_t pri)
{
    pq_tok* tok = malloc(sizeof(pq_tok));
    if(NULL == tok) return NULL;

    tok->tag = tag;
    tok->val.f = val;
    tok->pri = pri;
    tok->_dealloc_str = 0;
    return tok;
}

static inline pq_tok* pq_new_int_token(const pq_tag tag, const PQint val, const int8_t pri)
{
    pq_tok* tok = malloc(sizeof(pq_tok));
    if(NULL == tok) return NULL;

    tok->tag = tag;
    tok->val.i = val;
    tok->pri = pri;
    tok->_dealloc_str = 0;
    return tok;
}

static inline void pq_del_token(pq_tok* tok)
{
    if(tok != NULL && tok->_dealloc_str)
    {
        free((char*)tok->val.s);
    }
    free(tok);
}

#endif