/**
 * @file pq_unicode.h
 * @author Brandon Foster
 * @brief unicode support functions header.
 * these functions favor utf8 encodings whenever possible.
 * these functions are primarily used and will be optimized specifically for the poqer-lang.
 * these functions are not recommended for use in the general case.
 * 
 * @version 0.003
 * @date 10-12-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_UNICODE_H
#define _PQ_UNICODE_H
#include "pq_globals.h"
#include <string.h>
#include <wchar.h>
#include <stdlib.h>

//Maximum characters required to store any null-terminated utf8 character sequence.
#define PQ_UTF8_LEN_MAX 5

//Maximum characters required to store any unicode character as a null-terminated wide c-string.
#define PQ_WCHAR_LEN_MAX (sizeof(wchar_t) + 1)

/**
 * @brief Checks if a character is the first code of a utf8 character.
 * 
 * @param byte The character to be checked.
 * @return true if the character appears as the first code of a utf8 character else false.
 */
static inline const int8_t pq_is_utf8_1st_byte(const char byte)
{
    const uint8_t b = byte;
    return b <= 0x7F || (0xC0 <= b && b <= 0xF7);
}

/**
 * @brief Checks if a character is a code in a utf8 character, but not the first code.
 * 
 * @param byte The character to be checked.
 * @return true if the character appears as a non-first code of a utf8 character else false.
 */
static inline const int8_t pq_is_utf8_non_1st_byte(const char byte)
{
    const uint8_t b = byte;
    return 0x7F < b && b < 0xC0;
}

/**
 * @brief Checks if the first wide character for a unicode character requires a second wide character.
 * A second one is required for 4 coded unicode characters if wide characters with size of 2 bytes are used.
 * 
 * @param wc1 The first wide character for a unicode character to be checked.
 * @return true if a second wide character is required else false.
 */
static inline const int8_t pq_wcs_need_2codes(const wchar_t wc1)
{
    return sizeof(wchar_t) == 2 && 0xD800 <= wc1 && wc1 <= 0xDBFF;
}

/**
 * @brief Converts a wide c-string to their respective unicode codepoint.
 * 
 * @param wstr The wide c-string to be checked.
 * @return The unicode codepoint for the wide c-string or -1 if the wide c-string is an invalid codepoint.
 */
static inline const int32_t pq_wcs_to_cp(const wchar_t* wstr)
{
    const uint32_t wc1 = wstr[0];
    if(sizeof(wchar_t) == 2)
    {   //for wide character with size of 2 bytes, 2 wide characters are checked for 4 coded unicode codepoint.
        if(wc1 <= 0xD7FF || (0xE000 <= wc1 && wc1 <= 0xFFFF ))
            return (int32_t) wc1;
        else if(0xD800 <= wc1 && wc1 <= 0xDBFF)
        {
            uint32_t wc2 = wstr[1];
            return ((wc1-0xD800)<<10) + (wc2-0xDC00) + 0x10000;
        }
    }
    else if(wc1 <= 0xD7FF || (0xE000 <= wc1 && wc1 <= 0x10FFFF))
        return (int32_t)wstr[0];
    return -1;
}

/**
 * @brief Converts a codepoint to a null-terminated utf8 char sequence then stores it into a c-string.
 * The c-string should have enough bytes allocated to store all unicode chars with the null character.
 * To ensure this constraint, please use PQ_UTF8_LEN_MAX for the c-string.
 * 
 * @param dest The c-string that will store the null-terminated utf8 char sequence.
 * @param cp The codepoint to be converted.
 * @return The number of bytes in the utf8 character or -1 for invalid codepoints.
 */
static inline const int8_t pq_cp_to_utf8(char* dest, const uint32_t cp)
{
    if(cp <= 0x007F)
    {   //unicode 1 byte codepoint
        dest[0] = (char)cp;
        dest[1] = '\0';
        return 1;
    }
    else if(cp <= 0x07FF)
    {   //unicode 2 bytes codepoint
        const uint32_t byte1_r6 = (cp>>6);

        dest[0] = (char)(byte1_r6+0xC0);
        dest[1] = (char)((cp-(byte1_r6<<6))+0x80);
        dest[2] = '\0';
        return 2;
    }
    else if(cp <= 0xFFFF)
    {   //unicode 3 bytes codepoint
        const uint32_t byte1_r6 = (cp>>6);
        const uint32_t byte1_r12 = (byte1_r6>>6);

        dest[0] = (char)(byte1_r12+0xE0);
        dest[1] = (char)(((cp-(byte1_r12<<12))>>6)+0x80);
        dest[2] = (char)((cp-(byte1_r6<<6))+0x80);
        dest[3] = '\0';
        return 3;
    }
    else if(cp <= 0x10FFFF)
    {   //unicode 4 bytes codepoint
        const uint32_t byte1_r6 = (cp>>6);
        const uint32_t byte1_r12 = (byte1_r6>>6);
        const uint32_t byte1_r18 = (byte1_r12>>6);

        dest[0] = (char)(byte1_r18+0xF0);
        dest[1] = (char)(((cp-(byte1_r18<<18))>>12)+0x80);
        dest[2] = (char)(((cp-(byte1_r12<<12))>>6)+0x80);
        dest[3] = (char)((cp-(byte1_r6<<6))+0x80);
        dest[4] = '\0';
        return 4;
    }

    //not defined
    return -1;
}

/**
 * @brief Converts a utf8 character sequence to a codepoint.
 * 
 * @param cp The codepoint will be stored here.
 * @param src The utf8 character sequence to be checked.
 * @return The number of bytes in the utf8 character or -1 for invalid codepoints.
 */
static inline const int8_t pq_utf8_to_cp(uint32_t* cp, const char* src)
{
    //utf8 char sequence, used for better readability than const char*
    const uint8_t* utf8 = (const uint8_t*)src;

    if(utf8[0] < 0x80)
    {   //utf8 1 byte char
        *cp = (uint32_t)utf8[0];
        return 1;
    }
    else if(0xBF < utf8[0])
    {
        if(utf8[0] < 0xE0)
        {   //utf8 2 bytes char
            if(!pq_is_utf8_non_1st_byte(utf8[1])) return -1;
            *cp = (uint32_t)(((utf8[0]-0xC0)<<6) + (utf8[1]-0x80));
            return 2;
        }
        else if(utf8[0] < 0xF0)
        {   //utf8 3 bytes char
            if(!pq_is_utf8_non_1st_byte(utf8[1]) || !pq_is_utf8_non_1st_byte(utf8[2])) return -1;
            *cp = (uint32_t)(((utf8[0]-0xE0)<<12) + ((utf8[1]-0x80)<<6) + (utf8[2]-0x80));
            return 3;
        }
        else if(utf8[0] < 0xF8)
        {   //utf8 4 bytes char
            if(!pq_is_utf8_non_1st_byte(utf8[1]) || !pq_is_utf8_non_1st_byte(utf8[2]) || !pq_is_utf8_non_1st_byte(utf8[3])) return -1;
            *cp = (uint32_t)(((utf8[0]-0xF0)<<18) + ((utf8[1]-0x80)<<12) + ((utf8[2]-0x80)<<6) + (utf8[3]-0x80));
            return 4;
        }
    }
    
    //not defined
    return -1;
}

/**
 * @brief Converts a utf8 character sequence to a null-terminated wide c-string.
 * The wide c-string should have enough wide characters to store all unicode chars with the null character.
 * To ensure this constraint, please use PQ_WCHAR_LEN_MAX for the wide c-string.
 * 
 * @param dest The null-terminated wide c-string is stored here.
 * @param src The utf8 character sequence to be checked.
 * @return The number of bytes in the utf8 character or -1 for invalid codepoints.
 */
static inline const int8_t pq_utf8_to_wcs(wchar_t* dest, const char* src)
{
    uint32_t cp;
    int8_t bytes = pq_utf8_to_cp(&cp, src);
    if(sizeof(wchar_t) == 2 && 4 == bytes)
    {   //for wide character with size of 2 bytes, the utf16 encoding is followed to get the 2 wide characters needed.
        uint32_t cp_prime = cp-0x10000;
        uint32_t cp_r10 = (cp_prime>>10);
        dest[0] = cp_r10 + 0xD800;
        dest[1] = (cp_prime-(cp_r10<<10)) + 0xDC00;
        dest[2] = L'\0';
    }
    else
    {
        dest[0] = cp;
        dest[1] = L'\0';
    }
    return bytes;
}

/**
 * @brief Converts a null-terminate wide c-string to a null-terminated utf8 c-string.
 * The behavior is undefined if not enough bytes were allocated to the destination.
 * 
 * @param dest The null-terminated utf8 c-string will be stored here.
 * @param src The null-terminated wide c-string to be checked.
 * @param len The maximum wide characters for the null-terminated wide c-string.
 * @return The total number of bytes used in the destination, including the null character.
 */
static inline const size_t pq_wcs_to_utf8s(char* dest, const wchar_t* src, const size_t len)
{
    char utf8ch[PQ_UTF8_LEN_MAX]; //holds a single null-terminated utf8 character sequence for reuse.
    char* d_curr = dest; //current position in the dest.
    const wchar_t* s_curr = src; //current position in the src.
    size_t tot_size = 0;

    if(sizeof(wchar_t) == 2)
    {   
        int8_t bytes;
        while(tot_size < len)
        {
            if((bytes = pq_cp_to_utf8(utf8ch, pq_wcs_to_cp(s_curr))) == -1) break; //invalid  conversion
            if(tot_size + bytes < len)
            {
                strcpy(d_curr, utf8ch);
                d_curr += bytes;
                tot_size += bytes;
                if(utf8ch[0] == '\0') break;
                s_curr += (4 == bytes) ? 2 : 1; //requires 2 wchar_t for a utf8 char that require 4 codes
            }
            else
            {   //not enough space for the utf8 character.
                *d_curr = '\0';
                tot_size++;
                break;
            }
        }
    }
    else
    {   //can store all utf8 chars in a wchar_t
        int8_t bytes;
        while(tot_size < len)
        {
            if((bytes = pq_cp_to_utf8(utf8ch, pq_wcs_to_cp(s_curr))) == -1) break;
            if(tot_size + bytes < len)
            {
                strcpy(d_curr, utf8ch);
                d_curr += bytes;
                tot_size += bytes;
                if(utf8ch[0] == '\0') break;
                s_curr++;
            }
            else
            {   //not enough space for the utf8 character.
                *d_curr = '\0';
                tot_size++;
                break;
            }
        }
    }
    return tot_size;
}

/**
 * @brief Converts a null-terminate utf8 c-string to a null-terminated wide c-string.
 * The behavior is undefined if not enough bytes were allocated to the destination.
 * 
 * @param dest The null-terminated wide c-string will be stored here.
 * @param src The null-terminated utf8 c-string to be checked.
 * @param len The maximum bytes for the null-terminated utf8 c-string.
 * @return The total number of wide characters used in the destination, including the null character.
 */
static inline const size_t pq_utf8s_to_wcs(wchar_t* dest, const char* src, const size_t len)
{
    wchar_t wcs[PQ_WCHAR_LEN_MAX]; //holds a single null-terminated wide character for reuse.
    wchar_t* d_curr = dest; //current position in the dest.
    const char* s_curr = src; //current position in the src.
    size_t tot_size = 0;
    
    if(sizeof(wchar_t) == 2)
    {   //requires 2 wchar_t for a utf8 char that require 4 codes
        int8_t bytes;
        while(tot_size < len)
        {
            if((bytes = pq_utf8_to_wcs(wcs, s_curr)) == -1) break;
            s_curr += bytes;
            if(4 == bytes)
            {   //requires 2 wchar_t for a utf8 char that require 4 codes
                if(tot_size + 1 < len)
                {
                    wcscpy(d_curr, wcs);
                    d_curr += 2;
                    tot_size += 2;
                }
                else
                {
                    *d_curr = L'\0';
                    tot_size++;
                    break;
                }
            }
            else
            {
                *d_curr++ = wcs[0];
                tot_size++;
            }
            if(wcs[0] == L'\0') break;
        }
    }
    else
    {   //can store all utf8 chars in a wchar_t
        int8_t bytes;
        while(tot_size < len)
        {
            if((bytes = pq_utf8_to_wcs(wcs, s_curr)) == -1) break;
            *d_curr++ = wcs[0];
            tot_size++;
            if(wcs[0] == L'\0') break;
            s_curr += bytes;
        }
    }
    return tot_size;
}

/**
 * @brief Converts a null-terminate utf8 c-string to a newly allocated null-terminated wide c-string.
 * 
 * @param src The null-terminated utf8 c-string to be checked.
 * @return The newly allocated null-terminated wide c-string or NULL if not enough space is available.
 */
static inline const wchar_t* pq_utf8s_to_new_wcs(const char* src)
{
    size_t len = strlen(src) + 1;
    wchar_t* dest = malloc(sizeof(wchar_t)*len);
    if(dest == NULL) return 0;
    pq_utf8s_to_wcs(dest, src, len);

    return dest;
}

//Prolog token character classifications that conforms to the ISO/IEC 13211-1 tokens with the inclusion of utf-8 characters.

const int pq_is_unicode_solo_char(const uint32_t cp);
const int pq_is_unicode_single_quoted_token_char(const uint32_t cp);
const int pq_is_unicode_single_quoted_token_esc_char(const uint32_t cp);
const int pq_is_unicode_control_esc_char(const uint32_t cp);
const int pq_is_unicode_meta_esc_char(const uint32_t cp);
const int pq_is_unicode_graphic_token_char(const uint32_t cp);
const int pq_is_unicode_graphic_char(const uint32_t cp);
const int pq_is_unicode_newline_char(const uint32_t cp);
const int pq_is_unicode_layout_char(const uint32_t cp);
const int pq_is_unicode_alnum_char(const uint32_t cp);
const int pq_is_unicode_upper_char(const uint32_t cp);
const int pq_is_unicode_lower_char(const uint32_t cp);
const int pq_is_unicode_bin_char(const uint32_t cp);
const int pq_is_unicode_oct_char(const uint32_t cp);
const int pq_is_unicode_dec_char(const uint32_t cp);
const int pq_is_unicode_hex_char(const uint32_t cp);
const int pq_is_unicode_other_char(const uint32_t cp);

#endif