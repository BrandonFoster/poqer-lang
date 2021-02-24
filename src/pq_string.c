/**
 * @file pq_string.c
 * @author Brandon Foster
 * @brief poqer-lang string implementation.
 * the implementation of the pq_string.
 * 
 * @version 0.001
 * @date 10-24-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#include "pq_string.h"

void pq_string_clear(pq_string* string)
{
    if(string->mem_size == PQ_STRING_MEM_OFFSET)
    {
        string->str_size = 0;
    }
    else
    {
        free(string->str);
        string->mem_size = PQ_STRING_MEM_OFFSET;
        string->str_size = 0;
        string->str = malloc(PQ_STRING_MEM_OFFSET);
    }
}

void pq_string_append_char(pq_string* dest, const char ch)
{
    if(dest->str_size - 1 >= dest->mem_size)
    {
        char *str = realloc(dest->str, (dest->mem_size += PQ_STRING_MEM_OFFSET));
        dest->str = str;
    }
    dest->str[dest->str_size++] = ch;
    dest->str[dest->str_size] = '\0';
}

void pq_string_append_str(pq_string* dest, const char* src)
{
    size_t len = strlen(src);
    if(!len) return;
    
    if(dest->str_size + len - 2 >= dest->mem_size)
    {
        char *str = realloc(dest->str, (dest->mem_size += PQ_STRING_MEM_OFFSET + len - 1));
        dest->str = str;
    }
    strcpy(dest->str + dest->str_size, src);
    dest->str[(dest->str_size += len)] = '\0';
}

void pq_string_append_string(pq_string* dest, const pq_string* src)
{
    if(!src->str_size) return;

    if(dest->str_size + src->str_size - 2 >= dest->mem_size)
    {
        char *str = realloc(dest->str, (dest->mem_size += PQ_STRING_MEM_OFFSET + src->str_size - 1));
        dest->str = str;
    }
    strcpy(dest->str + dest->str_size, src->str);
    dest->str[(dest->str_size += src->str_size)] = '\0';
}
