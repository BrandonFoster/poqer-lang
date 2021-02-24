/**
 * @file pq_main.c
 * @author Brandon Foster
 * @brief poqer-lang interpreter program.
 * 
 * @version 0.005
 * @date 10-11-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#include "pq_utils.h"
#include "pq_unicode.h"
#include "pq_parser.h"
#include <stdio.h>

#ifdef PQ_OS_LINUX
  #define MAIN(X) main(X)

#elif PQ_OS_WINDOWS
  #define MAIN(X) wmain(X)

  #ifndef _UNICODE
  #define _UNICODE
  #endif

  #ifndef UNICODE
  #define UNICODE
  #endif
#endif

/**
 * @brief Creates and returns a null-terminated c-string line from stdin. 
 * The newline character is not included.
 * 
 * @return A null-terminated c-string line from stdin. 
 */
char* my_getline(void);

void print_all_tokens(pq_scanner* scanner, const char* line);

void debug_test_syntax_tree();

int MAIN(void)
{
    pq_init_utf_io();
    wprintf(L"poqer-lang interpreter(work in progress)\n");

    //Initialization

    pq_parser* parser = pq_new_parser();

    for(;;)
    {   //REPL

        //Command prefix
        wprintf(L"?- ");

        //Read input
        pq_parser_set_buffer(parser, my_getline());

        //Parse tokens into a Syntax Tree
        pq_syntax_tree* result = pq_parser_parse(parser);

        //Prints error if any
        if(parser->err)
        {
            wprintf(L"%ls", pq_utf8s_to_new_wcs(parser->err));
        }
        else wprintf(L"okay poqer syntax");
        
        wprintf(L"\n");
    }
    
    //Clean Up
    pq_del_parser(parser);

    return PQ_SUCCESS;
}

void print_all_tokens(pq_scanner* scanner, const char* line)
{
    pq_scanner_set_buffer(scanner, line);

    //prints the output of each token found in the buffer with error checking.
    int8_t found_atleast1 = 0;
    pq_tok* tok;
    char* err;
    while((tok = pq_scanner_next_token(scanner, &err)) != NULL)
    {
        found_atleast1 = 1;
        switch(tok->tag)
        {
        case PQ_NAME_TOK:
            wprintf(L"name{%ls} ", pq_utf8s_to_new_wcs(tok->val.s));
            break;
        case PQ_VAR_TOK:
            wprintf(L"var{%ls} ", pq_utf8s_to_new_wcs(tok->val.s));
            break;

        case PQ_INT_TOK:
            wprintf(L"int{%lld} ", tok->val.i);
            break;
        case PQ_FLT_TOK:
            wprintf(L"float{%f} ", tok->val.f);
            break;

        case PQ_LPAR_TOK:
        case PQ_RPAR_TOK:
        case PQ_LLIST_TOK:
        case PQ_RLIST_TOK:
        case PQ_LCURLY_TOK:
        case PQ_RCURLY_TOK:
        case PQ_HT_SEP_TOK:
        case PQ_COMMA_TOK:
            wprintf(L"%ls ", pq_utf8s_to_new_wcs(tok->val.s));
            break;
        case PQ_END_TOK:
            wprintf(L"%ls\n", pq_utf8s_to_new_wcs(tok->val.s));
            break;
        }
        
        free(tok);
    }

    if(err != NULL)
    {
        if(found_atleast1) wprintf(L"\n");
        wprintf(L"%ls", pq_utf8s_to_new_wcs(err));
    }
    wprintf(L"\n");
}

void debug_test_syntax_tree()
{
    wprintf(L"Syntax Tree Test:\n");
    pq_syntax_tree* tree = pq_new_syntax_tree();
    double* my_item = (double*)malloc(sizeof(double));
    *my_item = 0.5;

    double* my_item2 = (double*)malloc(sizeof(double));
    *my_item2 = 0.9;

    pq_syntax_tree_add_right_sibling(tree->children_nil, my_item);
    pq_syntax_tree_add_left_sibling(tree->children_nil->next, my_item2);

    wprintf(L"My Double: %f\n", *((double*)tree->children_nil->prev->item));
    wprintf(L"My Double: %f\n", *((double*)tree->children_nil->prev->prev->next->item));
    wprintf(L"My Double2: %f\n", *((double*)tree->children_nil->next->next->item));
    wprintf(L"My Double2: %f\n", *((double*)tree->children_nil->next->next->prev->item));
    wprintf(L"Is Leaf: %d\n", pq_syntax_tree_is_leaf(tree->children_nil));
    wprintf(L"\n");
}

char* my_getline(void)
{
    size_t max = 64; //current # of chars allocated for the char str.
    size_t len = 64; //current # of chars remaining in the char str.
    
    char* line; //will hold a null-terminated c-string line from stdin.
    char* curr; //the current position on the line.
    
    char utf8ch[PQ_UTF8_LEN_MAX]; //holds a single null-terminated utf8 character sequence for reuse.
    wchar_t wcs[PQ_WCHAR_LEN_MAX]; //holds a single null-terminated wide character sequence for reuse.

    line = (curr = malloc(max));
    if(NULL == line)
    {   //not enough space, stops.
        return NULL;
    }
    
    /* approach explanation
    the loop will first read user input one wide char at a time (sometimes two for certain wide chars on certain platforms).
    secondly, allocates more space to the line if needed.
    and finally, converts the wide char(s) read to utf8 char sequence(s) and appends them to the line.
    stops upon the first newline character, this character will not be included.

    note: upon on read error, the current line will be null-terminated then returned. */
    for(;;)
    {
        //grabs a wide char
        wint_t ch = fgetwc(stdin);
        if(WEOF == ch) break; //read error

        //builds the null-terminated wide char string
        wcs[0] = ch;
        if(pq_wcs_need_2codes(ch))
        {   //depending on the platform, for wide char that are 2 bytes in size,
            //2 wide char needs to be read for unicode char of 4 codes.
            ch = fgetwc(stdin);
            if(WEOF == ch) break; //read error
            wcs[1] = ch;
            wcs[2] = '\0';
        }
        else
        {   //wide char that are 4 bytes in size do.
            wcs[1] = '\0';
        }

        //stores wide char string as a utf8 char sequence.
        int8_t bytes = pq_cp_to_utf8(utf8ch, pq_wcs_to_cp(wcs));
        if(-1 == bytes) break; //conversion error
        
        //not enough bytes in line, request double.
        if((len -= bytes) <= 0)
        {
            len += max;
            size_t offset = curr - line;
            char* line_n = realloc(line, (max <<= 1));
            if(NULL == line_n)
            {   //no more space, free used space and give up.
                free(line);
                fflush(stdin);
                return NULL;
            }

            line = line_n;
            curr = line_n + offset;
        }
        
        if('\n' == utf8ch[0])
        {   //finished reading the line of user input.
            break;
        }

        //appends the ut8 char sequence to the line.
        strcpy(curr, utf8ch);
        curr += bytes;
    }
    *curr = '\0'; //null-terminates the c-string.
    fflush(stdin);
    
    return line;
}