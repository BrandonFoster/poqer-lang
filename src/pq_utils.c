/**
 * @file pq_utils.c
 * @author Brandon Foster
 * @brief utility functions implementation.
 * 
 * @version 0.003
 * @date 10-11-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#include "pq_utils.h"

#ifdef PQ_OS_LINUX
#include <locale.h>
#include <string.h>
#elif PQ_OS_WINDOWS
#include <stdio.h>
#include <fcntl.h>
#endif

int pq_init_utf_io(void)
{
#ifdef PQ_OS_LINUX
    //On GNU-Linux, attempts to set to locale to C.UTF-8
    return strcmp("C.UTF-8", setlocale(LC_ALL, "C.UTF-8")) ? PQ_SUCCESS : PQ_FAILURE;

#elif PQ_OS_WINDOWS
    //For Windows, the mode for stdin, stdout, and stderr are changed to process UTF-8.
    //If any stream fails to change, all of them are reverted to their original mode.
    int in_mode = _setmode(fileno(stdin), _O_U16TEXT);
    if(-1 == in_mode) return PQ_FAILURE;

    int out_mode = _setmode(fileno(stdout), _O_U16TEXT);
    if(-1 == out_mode)
    {   //Restores previous stdin mode.
        _setmode(fileno(stdin), in_mode);
        return PQ_FAILURE;
    }

    int err_mode = _setmode(fileno(stderr), _O_U16TEXT);
    if(-1 == err_mode)
    {   //Restores previous stdin and stdout mode.
        _setmode(fileno(stdin), in_mode);
        _setmode(fileno(stdout), out_mode);
        return PQ_FAILURE;
    }
    return PQ_SUCCESS;

#else
    //For unknown operating systems, the failure status here indicates that nothing was done.
    return PQ_FAILURE;
#endif
}