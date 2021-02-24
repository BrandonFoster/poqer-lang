/**
 * @file pq_parser.c
 * @author Brandon Foster
 * @brief poqer-lang parser implementation.
 * the internal implementation of the pq_parser_* functions are documented below.
 * 
 * @version 0.001
 * @date 11-5-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#include "pq_parser.h"
#include "pq_term.h"
#include <string.h>

pq_syntax_tree_node* pq_parse_prolog_text(pq_parser* parser);
pq_syntax_tree_node* pq_parse_prolog_term(pq_parser* parser, pq_priority priority);
pq_syntax_tree_node* pq_parse_prolog_prefix_op_or_functor_or_atom(pq_parser* parser, pq_priority priority);
void* pq_parse_prolog_operand_or_arg_list(pq_parser* parser, const pq_priority priority, PQstr* node_type);
pq_list* pq_parse_prolog_arg_list(pq_parser* parser);
pq_term* pq_parse_prolog_arg(pq_parser* parser);
pq_list* pq_parse_prolog_items(pq_parser* parser);

static PQbool pq_syntax_name_is_spec_operator(const char* name, const pq_op_specifier spec)
{
    //brute force implementation for default operators, will replace with a hash table in the future.
    switch(spec)
    {
    case PQ_OP_XFX:
        if(!strcmp(name, ":-") || !strcmp(name, "-->")
        || !strcmp(name, "=") || !strcmp(name, "\\=")
        || !strcmp(name, "==") || !strcmp(name, "\\==") || !strcmp(name, "@<") || !strcmp(name, "@=<") || !strcmp(name, "@>") || !strcmp(name, "@>=")
        || !strcmp(name, "==..")
        || !strcmp(name, "is") || !strcmp(name, "=:=") || !strcmp(name, "=\\=") || !strcmp(name, "<") || !strcmp(name, "=<") || !strcmp(name, ">") || !strcmp(name, ">=")
        || !strcmp(name, "**"))
            return PQ_TRUE;
        break;
    
    case PQ_OP_XFY:
        if(!strcmp(name, ";") || !strcmp(name, "->") || !strcmp(name, ",") || !strcmp(name, "^"))
            return PQ_TRUE;
        break;

    case PQ_OP_YFX:
        if(!strcmp(name, "+") || !strcmp(name, "-") || !strcmp(name, "/\\") || !strcmp(name, "\\/")
        || !strcmp(name, "*") || !strcmp(name, "/") || !strcmp(name, "//") || !strcmp(name, "rem") || !strcmp(name, "mod") || !strcmp(name, "<<") || !strcmp(name, ">>"))
            return PQ_TRUE;
        break;

    case PQ_OP_FX:
        if(!strcmp(name, ":-") || !strcmp(name, "?-"))
            return PQ_TRUE;
        break;

    case PQ_OP_FY:
        if(!strcmp(name, "\\+") || !strcmp(name, "-") || !strcmp(name, "\\"))
            return PQ_TRUE;
        break;
    
    default:
        break;
    }

    return PQ_FALSE;
}

static PQbool pq_syntax_name_is_prefix_operator(const char* name)
{
    return pq_syntax_name_is_spec_operator(name, PQ_OP_FX)
    || pq_syntax_name_is_spec_operator(name, PQ_OP_FY);
}

static PQbool pq_syntax_name_is_operator(const char* name)
{
    return pq_syntax_name_is_spec_operator(name, PQ_OP_FX)
    || pq_syntax_name_is_spec_operator(name, PQ_OP_FY)
    || pq_syntax_name_is_spec_operator(name, PQ_OP_XFX)
    || pq_syntax_name_is_spec_operator(name, PQ_OP_XFY)
    || pq_syntax_name_is_spec_operator(name, PQ_OP_YFX)
    || pq_syntax_name_is_spec_operator(name, PQ_OP_XF)
    || pq_syntax_name_is_spec_operator(name, PQ_OP_YF);
}

// dead code atm, useful later
// static pq_priority pq_syntax_get_op_priority(const char* name, const pq_op_specifier spec)
// {
//     switch(spec)
//     {
//     case PQ_OP_XFX:
//         if(!strcmp(name, ":-") || !strcmp(name, "-->"))
//             return 1200;
//         if(!strcmp(name, "=") || !strcmp(name, "\\=")
//         || !strcmp(name, "==") || !strcmp(name, "\\==") || !strcmp(name, "@<") || !strcmp(name, "@=<") || !strcmp(name, "@>") || !strcmp(name, "@>=")
//         || !strcmp(name, "==..")
//         || !strcmp(name, "is") || !strcmp(name, "=:=") || !strcmp(name, "=\\=") || !strcmp(name, "<") || !strcmp(name, "=<") || !strcmp(name, ">") || !strcmp(name, ">="))
//             return 700;
//         if(!strcmp(name, "**"))
//             return 200;
//         break;
    
//     case PQ_OP_XFY:
//         if(!strcmp(name, ";")) return 1100;
//         if(!strcmp(name, "->")) return 1050;
//         if(!strcmp(name, ",")) return 1000;
//         if(!strcmp(name, "^")) return 200;
//         break;

//     case PQ_OP_YFX:
//         if(!strcmp(name, "+") || !strcmp(name, "-") || !strcmp(name, "/\\") || !strcmp(name, "\\/"))
//             return 500;
//         if(!strcmp(name, "*") || !strcmp(name, "/") || !strcmp(name, "//") || !strcmp(name, "rem") || !strcmp(name, "mod") || !strcmp(name, "<<") || !strcmp(name, ">>"))
//             return 400;
//         break;

//     case PQ_OP_FX:
//         if(!strcmp(name, ":-") || !strcmp(name, "?-")) return 1200;
//         break;

//     case PQ_OP_FY:
//         if(!strcmp(name, "\\+")) return 900;
//         if(!strcmp(name, "-") || !strcmp(name, "\\")) return 200;
//         break;
    
//     default:
//         break;
//     }
    
//     return 0;
// }

static inline void pq_parser_next_token(pq_parser* parser)
{
    parser->curr_tok = pq_scanner_next_token(parser->scanner, &parser->err); 
}

pq_syntax_tree* pq_parser_parse(pq_parser* parser)
{
    pq_syntax_tree* tree = pq_new_syntax_tree();
    pq_parser_next_token(parser);
    pq_parse_prolog_text(parser);
    return tree;
}

pq_syntax_tree_node* pq_parse_prolog_text(pq_parser* parser)
{
    
    if(!parser->curr_tok)
    {   //represents the <prolog-text> ::= <EOR> production
        return NULL;
    }
    else
    {   //represents 1 of the 2 productions:
        //<prolog-text> ::= <directive-term> <prolog-text>
        //<prolog-text> ::= <clause-term> <prolog-text>

        //first perform a <term> <end> check, b.c. <directive-term> and <clause-term> requires it.
        pq_syntax_tree_node* term_node = pq_parse_prolog_term(parser, 1200);
        if(parser->err) return NULL;

        if(parser->curr_tok && parser->curr_tok->tag == PQ_END_TOK)
        {
            pq_parser_next_token(parser);
            if(parser->err) return NULL;

            /*
                todo: determine whether the term is a directive or clause term.
            */

            //performs the right-recursive <prolog-text> after.
            pq_syntax_tree_add_right_sibling_node(term_node, pq_parse_prolog_text(parser));
            return term_node;
        }
        else
        {   //syntax error: expected end token.
            parser->err = "syntax error: expected an end token.";
            return NULL;
        }
    }
    
    parser->err = "syntax error: expected an empty line.";
    return NULL;
}

pq_syntax_tree_node* pq_parse_prolog_term(pq_parser* parser, pq_priority priority)
{
    if(!parser->curr_tok) 
    {   //needs at least 1 token.
        parser->err = "syntax error: expected a term.";
        return NULL;
    }

    if(priority > 0)
    {   //parses operator notation
        if(parser->curr_tok->tag == PQ_NAME_TOK && pq_syntax_name_is_prefix_operator(parser->curr_tok->val.s))
        {   //can either be a prefix operator with a right operand, a functor with an arg list of 2 or more, or the operator by itself as an atom
            return pq_parse_prolog_prefix_op_or_functor_or_atom(parser, priority);
        }

        //must be a term with 0 priority, original priority resets to 1200
        priority = 1200;
        pq_syntax_tree_node* left_operand_node = pq_parse_prolog_term(parser, 0);
        if(parser->err) return NULL;
        if(!parser->curr_tok)
        {   //left operand is just a term by itself
            pq_term* term = left_operand_node->item;
            term->priority = 1201;
            return left_operand_node;
        }
        else if(parser->curr_tok->tag == PQ_END_TOK)
        {
            return left_operand_node;
        }

        //check for an infix/postfix operator or just returns the left operand as a term
        const char* op_name = parser->curr_tok->val.s;
        if(pq_syntax_name_is_spec_operator(op_name, PQ_OP_XFX))
        {
            pq_parser_next_token(parser);
            pq_syntax_tree_node* right_operand_node = pq_parse_prolog_term(parser, priority-1);
            if(parser->err) return NULL;

            pq_syntax_tree_node* op_node = pq_new_syntax_tree_node(pq_new_operator_term(op_name, 1201, PQ_OP_XFX));
            pq_syntax_tree_add_right_sibling_node(left_operand_node, op_node);
            pq_syntax_tree_add_right_sibling_node(op_node, right_operand_node);
            return left_operand_node;
        }
        else if(pq_syntax_name_is_spec_operator(op_name, PQ_OP_XFY))
        {
            pq_parser_next_token(parser);
            pq_syntax_tree_node* right_operand_node = pq_parse_prolog_term(parser, priority);
            if(parser->err) return NULL;

            pq_syntax_tree_node* op_node = pq_new_syntax_tree_node(pq_new_operator_term(op_name, 1201, PQ_OP_XFY));
            pq_syntax_tree_add_right_sibling_node(left_operand_node, op_node);
            pq_syntax_tree_add_right_sibling_node(op_node, right_operand_node);
            return left_operand_node;
        }
        else if(pq_syntax_name_is_spec_operator(op_name, PQ_OP_YFX))
        {
            pq_parser_next_token(parser);
            pq_syntax_tree_node* right_operand_node = pq_parse_prolog_term(parser, priority);
            if(parser->err) return NULL;

            pq_syntax_tree_node* op_node = pq_new_syntax_tree_node(pq_new_operator_term(op_name, 1201, PQ_OP_YFX));
            pq_syntax_tree_add_right_sibling_node(left_operand_node, op_node);
            pq_syntax_tree_add_right_sibling_node(op_node, right_operand_node);
            return left_operand_node;
        }
        else if(pq_syntax_name_is_spec_operator(op_name, PQ_OP_XF))
        {
            pq_parser_next_token(parser);
            pq_syntax_tree_node* op_node = pq_new_syntax_tree_node(pq_new_operator_term(op_name, 1201, PQ_OP_XF));
            pq_syntax_tree_add_right_sibling_node(left_operand_node, op_node);
            return left_operand_node;
        }
        else if(pq_syntax_name_is_spec_operator(op_name, PQ_OP_YF))
        {
            pq_parser_next_token(parser);
            pq_syntax_tree_node* op_node = pq_new_syntax_tree_node(pq_new_operator_term(op_name, 1201, PQ_OP_YF));
            pq_syntax_tree_add_right_sibling_node(left_operand_node, op_node);
            return left_operand_node;
        }
        else
        {   //left operand is just a term by itself
            pq_term* term = left_operand_node->item;
            term->priority = 1201;
            return left_operand_node;
        } 
    }
    else
    {
        switch(parser->curr_tok->tag)
        {
        case PQ_LPAR_TOK:
        {   //represents the <term> ::= <open-par> <term> <close-par> production
            pq_parser_next_token(parser);
            pq_syntax_tree_node* term_node = pq_parse_prolog_term(parser, 1201);
            if(parser->err) return NULL;
            if(!parser->curr_tok || parser->curr_tok->tag != PQ_RPAR_TOK)
            {
                parser->err = "syntax error: expected a closing parenthesis";
                return NULL;
            }
            pq_parser_next_token(parser);
            pq_term* term = (pq_term*)term_node->item;
            term->priority = 0;
            return term_node;
        }

        //parses numeric constants
        case PQ_INT_TOK:
        {   //represents the <term> ::= <integer> production
            pq_syntax_tree_node* term_node = pq_new_syntax_tree_node(pq_new_integer_term(parser->curr_tok->val.i));
            pq_parser_next_token(parser);
            return term_node;
        }

        case PQ_FLT_TOK:
        {   //represents the <term> ::= <float-number> production
            pq_syntax_tree_node* term_node = pq_new_syntax_tree_node(pq_new_float_term(parser->curr_tok->val.f));
            pq_parser_next_token(parser);
            return term_node;
        }

        //parses variables
        case PQ_VAR_TOK:
        {   //represents the <term> ::= <variable> production
            pq_syntax_tree_node* term_node = pq_new_syntax_tree_node(pq_new_variable_term(parser->curr_tok->val.s));
            pq_parser_next_token(parser);
            return term_node;
        }

        //parses list notation
        case PQ_LLIST_TOK:
            pq_parser_next_token(parser);
            if(parser->err) return NULL;
            if(!parser->curr_tok)
            {
                parser->err = "syntax error: expected the end of the list";
                return NULL;
            }

            if(parser->curr_tok->tag == PQ_RLIST_TOK)
            {   //represents the <atom> ::= <open-list> <close-list> production
                pq_parser_next_token(parser);
                return pq_new_syntax_tree_node(pq_new_list_term(NULL));
            }
            else
            {   //represents the <term> ::= <open-list> <items> <close-list> production
                pq_list* items = pq_parse_prolog_items(parser);
                if(parser->err) return NULL;
                if(parser->curr_tok && parser->curr_tok->tag == PQ_RLIST_TOK)
                {
                    pq_parser_next_token(parser);
                    return pq_new_syntax_tree_node(pq_new_list_term(items));
                }
                else
                {
                    parser->err = "syntax error: expected the end of the list";
                    return NULL;
                }
            }
            break;

        //parses curly bracket notation
        case PQ_LCURLY_TOK:
            pq_parser_next_token(parser);
            if(parser->err) return NULL;
            if(!parser->curr_tok)
            {
                parser->err = "syntax error: expected a closing curly bracket";;
                return NULL;
            }

            if(parser->curr_tok->tag == PQ_RCURLY_TOK)
            {   //represents the <atom> ::= <open-curly> <close-curly> production
                pq_parser_next_token(parser);
                pq_syntax_tree_node* term_node = pq_new_syntax_tree_node(pq_new_atom_term("{}", 0));
                return term_node;
            }
            else
            {   //represents the <term> ::= <open-curly> <term> <close-curly> production
                pq_parser_next_token(parser);

                pq_syntax_tree_node* term_node = pq_parse_prolog_term(parser, 1201);
                if(parser->err) return NULL;

                if(!parser->curr_tok || parser->curr_tok->tag != PQ_RCURLY_TOK)
                {
                    parser->err = "syntax error: expected a closing curly bracket";
                    return NULL;
                }
                pq_parser_next_token(parser);

                pq_list* arg_list = pq_new_list();
                pq_list_push_back(arg_list, term_node->item);
                return pq_new_syntax_tree_node(pq_new_functor_term("{}", 0, arg_list));
            }
            break;

        //parses functional notation or atom
        case PQ_NAME_TOK:
        {
            PQstr atom_id = parser->curr_tok->val.s;
            pq_priority atom_priority;
            if(pq_syntax_name_is_operator(atom_id))
            {   //represents the <term> ::= <atom> production, where <atom> is an operator
                atom_priority = 1201;
            }
            else
            {   //represents the <term> ::= <atom> production, where <atom> is not an operator
                atom_priority = 0;
            }
            pq_parser_next_token(parser);

            //Checks for a potential functor
            if(parser->curr_tok && parser->curr_tok->tag == PQ_LPAR_TOK)
            {   //represents the <term> ::= <atom> <open-par> <arg-list> <close-par> production
                pq_parser_next_token(parser);

                pq_list* args = pq_parse_prolog_arg_list(parser);
                if(parser->err) return NULL;

                if(!parser->curr_tok || parser->curr_tok->tag != PQ_RPAR_TOK)
                {
                    parser->err = "syntax error: expected a closing parenthesis";
                    return NULL;
                }
                pq_parser_next_token(parser);
                return pq_new_syntax_tree_node(pq_new_functor_term(atom_id, 0, args));
            }
            else
            {   //not a functor, returns the atom alone.
                return pq_new_syntax_tree_node(pq_new_atom_term(atom_id, atom_priority));
            }
        }

        default:
            break;
        }
    }

    parser->err = "syntax error: expected a term.";
    return NULL;
}

pq_syntax_tree_node* pq_parse_prolog_prefix_op_or_functor_or_atom(pq_parser* parser, pq_priority priority)
{
    if(!parser->curr_tok || parser->curr_tok->tag != PQ_NAME_TOK || !pq_syntax_name_is_prefix_operator(parser->curr_tok->val.s))
    {
        parser->err = "syntax error: expected a prefix operator";
        return NULL;
    }

    PQstr op_name = parser->curr_tok->val.s;
    pq_op_specifier op_spec;
    pq_priority operand_priority;

    if(pq_syntax_name_is_spec_operator(op_name, PQ_OP_FX))
    {
        op_spec = PQ_OP_FX;
        operand_priority = priority - 1;
    }
    else
    {
        op_spec = PQ_OP_FY;
        operand_priority = priority;
    }
    pq_parser_next_token(parser);
    if(parser->err) return NULL;

    if(!parser->curr_tok)
    {   //represents the <term> ::= <atom> production where <atom> is the prefix operator
        return pq_new_syntax_tree_node(pq_new_atom_term(op_name, 1201));
    }

    switch(parser->curr_tok->tag)
    {
    case PQ_LPAR_TOK:
    {
        //represents the right operand or a functor's arg list
        PQstr node_type;
        void* syntax_node = pq_parse_prolog_operand_or_arg_list(parser, operand_priority, &node_type);
        if(parser->err) return NULL;

        if(!strcmp(node_type, "arg-list"))
        {   //returns a functor
            pq_list* arg_list = syntax_node;
            return pq_new_syntax_tree_node(pq_new_functor_term(op_name, 0, arg_list));
        }
        else
        {   //returns the prefix op with right operand
            pq_syntax_tree_node* right_operand_node = syntax_node;
            pq_term* right_operand = (pq_term*)right_operand_node->item;

            //check for negative numeric constant
            if(!strcmp(op_name, "-"))
            {
                if(right_operand->types & PQ_TERM_INTEGER_TYPE)
                    right_operand->data.int_val *= -1;
                else if(right_operand->types & PQ_TERM_FLOAT_TYPE)
                    right_operand->data.float_val *= -1.0;
                return right_operand_node;
            }

            pq_syntax_tree_node* term_node = pq_new_syntax_tree_node(pq_new_operator_term(op_name, 1201, op_spec));
            pq_syntax_tree_add_right_sibling_node(term_node, right_operand_node);
            return term_node;
        }
        return NULL;
    }

    case PQ_NAME_TOK:
    case PQ_INT_TOK:
    case PQ_FLT_TOK:
    case PQ_VAR_TOK:
    case PQ_LLIST_TOK:
    case PQ_LCURLY_TOK:
    {
        //represents the right operand
        pq_syntax_tree_node* right_operand_node = pq_parse_prolog_term(parser, operand_priority);
        if(parser->err) return NULL;
        pq_term* right_operand = (pq_term*)right_operand_node->item;

        //check for negative numeric constant
        if(!strcmp(op_name, "-"))
        {
            if(right_operand->types & PQ_TERM_INTEGER_TYPE)
                right_operand->data.int_val *= -1;
            else if(right_operand->types & PQ_TERM_FLOAT_TYPE)
                right_operand->data.float_val *= -1.0;
            return right_operand_node;
        }

        pq_syntax_tree_node* term_node = pq_new_syntax_tree_node(pq_new_operator_term(op_name, 1201, op_spec));
        pq_syntax_tree_add_right_sibling_node(term_node, right_operand_node);
        return term_node;
    }

    default:
        //represents the <term> ::= <atom> production where <atom> is the prefix operator
        return pq_new_syntax_tree_node(pq_new_atom_term(op_name, 1201));
    }
}

void* pq_parse_prolog_operand_or_arg_list(pq_parser* parser, const pq_priority priority, PQstr* node_type)
{
    pq_syntax_tree_node* syntax_node = pq_parse_prolog_term(parser, 1201);
    if(parser->err) return NULL;

    if(parser->curr_tok && parser->curr_tok->tag == PQ_COMMA_TOK)
    {   //start an arg list
        *node_type = "arg-list";
        pq_term* arg = (pq_term*)syntax_node->item;
        arg->priority = 999;
        
        pq_list* arg_list = pq_new_list();
        pq_list_push_back(arg_list, arg);

        while(parser->curr_tok && parser->curr_tok->tag == PQ_COMMA_TOK)
        {   
            pq_parser_next_token(parser);
            
            arg = pq_parse_prolog_arg(parser);
            if(parser->err) return NULL;
            pq_list_push_back(arg_list, arg);
        }

        return arg_list;
    }
    else
    {   //return as an operand
        *node_type = "operand";
        pq_term* operand = (pq_term*)syntax_node->item;
        operand->priority = priority;
        return syntax_node;
    }
}

pq_list* pq_parse_prolog_arg_list(pq_parser* parser)
{   //represents one of the two productions:
    //<arg-list> ::= <arg>
    //<arg-list> ::= <arg> <comma> <arg-list>

    pq_term* arg = pq_parse_prolog_arg(parser);
    if(parser->err) return NULL;

    pq_list* arg_list = pq_new_list();
    pq_list_push_back(arg_list, arg);

    while(parser->curr_tok && parser->curr_tok->tag == PQ_COMMA_TOK)
    {   
        pq_parser_next_token(parser);
        
        arg = pq_parse_prolog_arg(parser);
        if(parser->err) return NULL;
        pq_list_push_back(arg_list, arg);
    }

    return arg_list;
}

pq_term* pq_parse_prolog_arg(pq_parser* parser)
{
    if(!parser->curr_tok)
    {
        parser->err = "syntax error: expected an argument";
        return NULL;
    }

    if(parser->curr_tok->tag == PQ_NAME_TOK && (parser->curr_tok->val.s))
    {   //represents the <term> ::= <atom> production, where <atom> is an operator
        PQstr atom_id = parser->curr_tok->val.s;
        pq_parser_next_token(parser);
        return pq_new_atom_term(atom_id, 1201);
    }
    else
    {
        pq_syntax_tree_node* term_node = pq_parse_prolog_term(parser, 999);
        if(parser->err) return NULL;

        return (pq_term*)term_node->item;
    }
}

pq_list* pq_parse_prolog_items(pq_parser* parser)
{
    pq_term* arg = pq_parse_prolog_arg(parser);
    if(parser->err) return NULL;

    pq_list* items = pq_new_list();
    pq_list_push_back(items, arg);

    for(;;)
    {
        if(parser->curr_tok) switch(parser->curr_tok->tag)
        {
        case PQ_COMMA_TOK:
            pq_parser_next_token(parser);
            arg = pq_parse_prolog_arg(parser);
            if(parser->err) return NULL;
            pq_list_push_back(items, arg);
            break;

        case PQ_HT_SEP_TOK:
            pq_parser_next_token(parser);
            arg = pq_parse_prolog_arg(parser);
            if(parser->err) return NULL;
            pq_list_push_back(items, arg);
            return items;

        default:
            return items;
        }
        else break;
    }

    return items;
}