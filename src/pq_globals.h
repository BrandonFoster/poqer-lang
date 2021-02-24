/**
 * @file pq_globals.h
 * @author Brandon Foster
 * @brief poqer-lang global declarations.
 * this header file is included in all poqer-lang headers.
 * 
 * @version 0.002
 * @date 10-11-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_GLOBALS_H
#define _PQ_GLOBALS_H
#include <stdint.h>

#ifdef __gnu_linux__
#define PQ_OS_LINUX 1
#elif _WIN32
#define PQ_OS_WINDOWS 1
#endif

//Boolean for better readability
#define PQ_TRUE 1
#define PQ_FALSE 0
typedef uint8_t PQbool;

enum pq_status_code { PQ_SUCCESS, PQ_FAILURE };

//poqer-lang datatypes.

typedef const char* PQstr;
typedef double      PQflt;
typedef int64_t     PQint;

//poqer-lang extra typedefs
typedef uint16_t pq_priority;

#endif