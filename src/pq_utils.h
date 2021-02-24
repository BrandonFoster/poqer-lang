/**
 * @file pq_utils.h
 * @author Brandon Foster
 * @brief utility functions header.
 * these functions are not internally used in the library.
 * these functions are used by the interpreter and any poqer-lang related program. provided here.
 * 
 * @version 0.001
 * @date 10-11-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_UTILS_H
#define _PQ_UTILS_H
#include "pq_globals.h"

/**
 * @brief Tries to set stdin, stdout, and stderr to use UTF-8 (Linux) or UTF-16 (Windows).
 * 
 * @return PQ_SUCCESS if all streams now use a UTF encoding else PQ_FAILURE.
 */
int pq_init_utf_io(void);

#endif