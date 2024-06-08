/*
 * Copyright (c) 2018 Yamagi Burmeister
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * --------------------------------------------------------------------
 *
 * Some additional definitions required by minizip and not provided
 * by the miniz library.
 *
 * --------------------------------------------------------------------
 */

#ifndef COMMON_UNZIP_MINIZCONF_H
#define COMMON_UNZIP_MINIZCONF_H

#ifndef z_off_t
  #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #include <unistd.h>
    #define z_off_t off_t
  #elif defined(off_t) // maybe we're lucky :-p
    #define z_off_t off_t
  #else
    #define z_off_t long
  #endif
#endif

#ifndef ZEXPORT
 #define ZEXPORT
#endif

// FIXME: why not just set this to int64_t?
#if !defined(_WIN32) && defined(Z_LARGE64)
  #define z_off64_t off64_t
#else
  #if defined(_WIN32)
    #ifdef _MSC_VER
      #define z_off64_t __int64
    #else
      #include <stdint.h>
      #define z_off64_t int64_t
    #endif
  #else
    #define z_off64_t z_off_t
  #endif
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
static_assert( sizeof(z_off64_t) == 8, "z_off64_t should be a 64bit type" );
#endif

#endif
