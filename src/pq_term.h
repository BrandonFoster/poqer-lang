/**
 * @file pq_term.h
 * @author Brandon Foster
 * @brief poqer-lang term header.
 * the pq_term struct is used to represent the term construct in a poqer program.
 * create/destroy the term with the pq_new_* and pq_del_* functions.
 * 
 * @version 0.001
 * @date 11-6-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_TERM_H
#define _PQ_TERM_H
#include "pq_globals.h"
#include "pq_list.h"
#include <stdlib.h>
#include <inttypes.h>

const uint16_t PQ_TERM_NUMERIC_TYPE = 1 << 0;
const uint16_t PQ_TERM_INTEGER_TYPE = 1 << 1;
const uint16_t PQ_TERM_FLOAT_TYPE = 1 << 2;
const uint16_t PQ_TERM_ATOM_TYPE = 1 << 3;
const uint16_t PQ_TERM_OPERATOR_TYPE = 1 << 4;
const uint16_t PQ_TERM_VARIABLE_TYPE = 1 << 5;
const uint16_t PQ_TERM_FUNCTOR_TYPE = 1 << 6;
const uint16_t PQ_TERM_LIST_TYPE = 1 << 7;
const uint16_t PQ_TERM_EXPR_ARG_TYPE = 1 << 8;

typedef enum pq_op_specifier
{
    //infix
    PQ_OP_XFX,
    PQ_OP_XFY,
    PQ_OP_YFX,

    //prefix
    PQ_OP_FX,
    PQ_OP_FY,

    //postfix
    PQ_OP_XF,
    PQ_OP_YF
} pq_op_specifier;

typedef struct pq_operator_term {
    PQstr id;
    pq_op_specifier specifier;
} pq_operator_term;

typedef struct pq_functor_term {
    PQstr id;
    pq_list* args;
} pq_functor_term;

typedef union pq_term_data {
    pq_operator_term* op_data;
    pq_functor_term* fun_data;
    pq_list* list_items;
    PQstr var_id;
    PQstr atom_id;
    PQflt float_val;
    PQint int_val;
} pq_term_data;

typedef struct pq_term {
    pq_term_data data;
    pq_priority priority;
    uint16_t types;
} pq_term;

static inline pq_term* pq_new_term()
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    return term;
}

static inline pq_term* pq_new_integer_term(PQint val)
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    term->priority = 0;
    term->types = PQ_TERM_NUMERIC_TYPE | PQ_TERM_INTEGER_TYPE;
    term->data.int_val = val;
    return term;
}

static inline pq_term* pq_new_float_term(PQflt val)
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    term->priority = 0;
    term->types = PQ_TERM_NUMERIC_TYPE | PQ_TERM_FLOAT_TYPE;
    term->data.float_val = val;
    return term;
}

static inline pq_term* pq_new_variable_term(PQstr val)
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    term->priority = 0;
    term->types = PQ_TERM_VARIABLE_TYPE;
    term->data.var_id = val;
    return term;
}

static inline pq_term* pq_new_atom_term(PQstr val, const pq_priority priority)
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    term->priority = priority;
    term->types = PQ_TERM_ATOM_TYPE;
    term->data.atom_id = val;
    return term;
}

static inline pq_term* pq_new_operator_term(PQstr val, const pq_priority priority, const pq_op_specifier specifier)
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    term->priority = priority;
    term->types = PQ_TERM_ATOM_TYPE | PQ_TERM_OPERATOR_TYPE;
    term->data.op_data = (pq_operator_term*)malloc(sizeof(pq_operator_term));
    term->data.op_data->specifier = specifier;
    term->data.op_data->id = val;
    return term;
}

static inline pq_term* pq_new_functor_term(PQstr val, const pq_priority priority, pq_list* args)
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    term->priority = priority;
    term->types = PQ_TERM_ATOM_TYPE | PQ_TERM_FUNCTOR_TYPE;
    term->data.fun_data = (pq_functor_term*)malloc(sizeof(pq_functor_term));
    term->data.fun_data->args = args;
    term->data.fun_data->id = val;
    return term;
}

static inline pq_term* pq_new_list_term(pq_list* items)
{
    pq_term* term = (pq_term*)malloc(sizeof(pq_term));
    term->priority = 0;
    term->types = PQ_TERM_ATOM_TYPE | PQ_TERM_LIST_TYPE;
    term->data.list_items = items;
    return term;
}

#endif