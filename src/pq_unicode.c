/**
 * @file pq_unicode.c
 * @author Brandon Foster
 * @brief unicode support functions implementation.
 * the internal implementation of the pq_is_unicode_* functions are documented below.
 * 
 * @version 0.003
 * @date 10-15-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#include "pq_unicode.h"
#include <wchar.h>
#ifdef PQ_OS_LINUX
#include <wctype.h>
#endif

const int pq_is_unicode_solo_char(const uint32_t cp)
{
    switch(cp)
    {
    case '!':
    case '(':
    case ')':
    case ',':
    case ';':
    case '[':
    case ']':
    case '{':
    case '}':
    case '|':
    case '.':
        return 1;

    default:
        return 0;
    }
}

const int pq_is_unicode_single_quoted_token_char(const uint32_t cp)
{
    return iswprint(cp);
}

const int pq_is_unicode_single_quoted_token_esc_char(const uint32_t cp)
{
    return pq_is_unicode_control_esc_char(cp) || pq_is_unicode_meta_esc_char(cp);
}

const int pq_is_unicode_control_esc_char(const uint32_t cp)
{
    switch(cp)
    {
    case 'a':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
        return 1;
    
    default:
        return 0;
    }
}

const int pq_is_unicode_meta_esc_char(const uint32_t cp)
{
    switch(cp)
    {
    case '\\':
    case '\'':
    case  '"':
    case  '`':
        return 1;

    default:
        return 0;
    }
}

const int pq_is_unicode_graphic_token_char(const uint32_t cp)
{
    return pq_is_unicode_graphic_char(cp) || '\\' == cp;
}

const int pq_is_unicode_graphic_char(const uint32_t cp)
{
    return (iswgraph(cp) && !pq_is_unicode_alnum_char(cp) && !pq_is_unicode_solo_char(cp)) || '.' == cp;
}

const int pq_is_unicode_newline_char(const uint32_t cp)
{   //poqer-defined newline
    switch(cp)
    {
    case  ' ':
    case '\t':
        return 0;
    }

    return pq_is_unicode_layout_char(cp);
}

const int pq_is_unicode_layout_char(const uint32_t cp)
{
    return iswspace(cp);
}

const int pq_is_unicode_alnum_char(const uint32_t cp)
{
    return iswalnum(cp) || '_' == cp;
}

const int pq_is_unicode_upper_char(const uint32_t cp)
{
    return iswupper(cp);
}

const int pq_is_unicode_lower_char(const uint32_t cp)
{
    return iswalpha(cp) && !iswupper(cp);
}

const int pq_is_unicode_bin_char(const uint32_t cp)
{
    switch(cp)
    {
    case '0':
    case '1':
        return 1;
    
    default:
        return 0;
    }
}

const int pq_is_unicode_oct_char(const uint32_t cp)
{
    switch(cp)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        return 1;
    
    default:
        return 0;
    }
}

const int pq_is_unicode_dec_char(const uint32_t cp)
{
    switch(cp)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return 1;

    default:
        return 0;
    }
}

const int pq_is_unicode_hex_char(const uint32_t cp)
{
    switch(cp)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
        return 1;

    default:
        return 0;
    }
}

const int pq_is_unicode_other_char(const uint32_t cp)
{
    switch(cp)
    {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '|':
    case ',':
    case '.':
        return 1;

    default:
        return 0;
    }
}