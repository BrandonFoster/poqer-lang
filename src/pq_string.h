/**
 * @file pq_string.h
 * @author Brandon Foster
 * @brief poqer-lang string header.
 * the pq_string struct is used to manage variable-sized strings.
 * 
 * @version 0.001
 * @date 10-24-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_STRING_H
#define _PQ_STRING_H
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define PQ_STRING_MEM_OFFSET 32

typedef struct pq_string
{
    size_t mem_size;
    size_t str_size;
    char   *str;
} pq_string;

static inline pq_string* pq_new_string()
{
    pq_string *string = malloc(sizeof(pq_string));
    if(!string) return NULL;

    string->mem_size = PQ_STRING_MEM_OFFSET;
    string->str_size = 0;
    string->str = malloc(PQ_STRING_MEM_OFFSET);

    return string;
}

static inline void pq_del_string(pq_string* string)
{
    if(!string) return;
    free(string->str);
    free(string);
}

static inline void pq_string_assign_char(pq_string* string, const size_t index, const char ch)
{
    assert(index < string->str_size);
    string->str[index] = ch;
    if(ch == '\0') string->str_size = index;
}

void pq_string_clear(pq_string* string);
void pq_string_append_char(pq_string* string, const char ch);
void pq_string_append_str(pq_string* string, const char* str);
void pq_string_append_string(pq_string* string, const pq_string* string_src);

static inline const size_t pq_string_get_size(const pq_string* string)
{
    return string->str_size;
}

static inline const char* pq_string_get_cstr(const pq_string* string)
{
    return string->str;
}

static inline const char pq_string_get_char(const pq_string* string, const size_t index)
{
    return string->str[index];
}

#endif