/*
    utils.h -- Various useful shell utility functions.

    Copyright (C) 2016, 2021 Lawrence Sebald
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Count the number of whitespace characters in a given string.
   Note: This counts anything that would return true to isspace(). */
extern size_t count_spaces(const char *str);

/* Flush the given input stream up to EOF or the first newline character,
   discarding all input between the current file pointer and that EOF/newline
   character.
 */
extern void flush_input(FILE *fp);

/* Unescape a given string, transforming all escape sequences and quoted
   strings as appropriate. You probably want to pass the global variable
   "stderr" (defined in <stdio.h>, no quotes, obviously) for errf.
   Note: You are responsible for freeing the string returned by this function
         if it does not return NULL. */
extern char *unescape(const char *str, FILE *errf);

/* Find the first unquoted/unescaped space character in a given string and
   return its index.
   Note: Running this on a string returned by unescape will not do what you
         want in all likelihood. */
extern int first_unquoted_space(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* !UTILS_H */