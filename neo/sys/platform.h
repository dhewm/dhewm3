/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __PLATFORM__
#define __PLATFORM__

#include "config.h"
#include "framework/BuildDefines.h"

#ifdef _WIN32
#include <malloc.h> // _alloca()
#endif

// NOTE: By default Win32 uses a 1MB stack. Doom3 1.3.1 uses 4MB (probably set after compiling with EDITBIN /STACK
// dhewm3 now uses a 8MB stack, set with a linker flag in CMakeLists.txt (/STACK:8388608 for MSVC, -Wl,--stack,8388608 for mingw)
// Linux has a 8MB stack by default, and so does macOS, at least for the main thread
// anyway, a 2MB limit alloca should be safe even when using it multiple times in the same function
#define ID_MAX_ALLOCA_SIZE 2097152 // 2MB

/*
===============================================================================

	Non-portable system services.

===============================================================================
*/

// AROS
#if defined(__AROS__)

#define _alloca						alloca
#define _alloca16( x )				((void *)((((uintptr_t)alloca( (x)+15 )) + 15) & ~15))

#ifdef GAME_DLL
#define ID_GAME_API					__attribute__((visibility ("default")))
#else
#define ID_GAME_API
#endif

#define ALIGN16( x )				x __attribute__ ((aligned (16)))
#define PACKED						__attribute__((packed))

#define PATHSEPERATOR_STR			"/"
#define PATHSEPERATOR_CHAR			'/'

#define __cdecl
#define ASSERT						assert

#define ID_INLINE					inline
#define ID_STATIC_TEMPLATE

#define assertmem( x, y )

#endif

// Win32
#if defined(WIN32) || defined(_WIN32)

#ifdef __MINGW32__
  #undef _alloca // in mingw _alloca is a #define
  #define _alloca16( x )			( (assert((x)<ID_MAX_ALLOCA_SIZE)), __builtin_alloca_with_align( (x), 16*8 ) )
  #define _alloca( x )				( (assert((x)<ID_MAX_ALLOCA_SIZE)), __builtin_alloca( (x) ) )
#else
  #define _alloca16( x )			( (void *) ( (assert((x)<ID_MAX_ALLOCA_SIZE)), ((((uintptr_t)_alloca( (x)+15 )) + 15) & ~15) ) )
  #define _alloca( x )				( (void *) ( (assert((x)<ID_MAX_ALLOCA_SIZE)), _alloca( (x) ) ) )
#endif

#define PATHSEPERATOR_STR			"\\"
#define PATHSEPERATOR_CHAR			'\\'

#ifdef _MSC_VER
#ifdef GAME_DLL
#define ID_GAME_API					__declspec(dllexport)
#else
#define ID_GAME_API
#endif
#define ALIGN16( x )				__declspec(align(16)) x
#define PACKED
#define ID_INLINE					__forceinline
// DG: alternative to forced inlining of ID_INLINE for functions that do alloca()
//     and are called in a loop so inlining them might cause stack overflow
#define ID_MAYBE_INLINE				__inline
#define ID_STATIC_TEMPLATE			static
#define assertmem( x, y )			assert( _CrtIsValidPointer( x, y, true ) )
#else
#ifdef GAME_DLL
#define ID_GAME_API					__attribute__((visibility ("default")))
#else
#define ID_GAME_API
#endif
#define ALIGN16( x )				x __attribute__ ((aligned (16)))
#define PACKED						__attribute__((packed))
#define ID_INLINE					inline
#define ID_STATIC_TEMPLATE
#define assertmem( x, y )
#endif

#endif

// Setting D3_ARCH for VisualC++ from CMake doesn't work when using VS integrated CMake
// so set it in code instead
#ifdef _MSC_VER

#ifdef D3_ARCH
  #undef D3_ARCH
#endif // D3_ARCH

#ifdef _M_X64
  // this matches AMD64 and ARM64EC (but not regular ARM64), but they're supposed to be binary-compatible somehow, so whatever
  #define D3_ARCH "x86_64"
#elif defined(_M_ARM64)
  #define D3_ARCH "arm64"
#elif defined(_M_ARM)
  #define D3_ARCH "arm"
#elif defined(_M_IX86)
  #define D3_ARCH "x86"
#else
  // if you're not targeting one of the aforementioned architectures,
  // check https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros
  // to find out how to detect yours and add it here - and please send a patch :)
  #error "Unknown CPU architecture!"
  // (for a quick and dirty solution, comment out the previous line, but keep in mind
  //  that savegames may not be compatible with other builds of dhewm3)
  #define D3_ARCH "UNKNOWN"
#endif // _M_X64 etc

#endif // _MSC_VER


// Mac OSX
#if defined(MACOS_X) || defined(__APPLE__)

#ifdef GAME_DLL
#define ID_GAME_API					__attribute__((visibility ("default")))
#else
#define ID_GAME_API
#endif

#define ALIGN16( x )				x __attribute__ ((aligned (16)))

#define PACKED						__attribute__((packed))

#define _alloca						alloca
#define _alloca16( x )				((void *)((((uintptr_t)alloca( (x)+15 )) + 15) & ~15))

#define PATHSEPERATOR_STR			"/"
#define PATHSEPERATOR_CHAR			'/'

#define __cdecl
#define ASSERT						assert

#define ID_INLINE					inline
#define ID_STATIC_TEMPLATE

#define assertmem( x, y )

#endif


// Unix
#ifdef __unix__

#if !defined(__GNUC__) || (defined(__MCST__) && __LCC__ < 128)
  // MCST-LCC < 1.28 does not support __builtin_alloca_with_align()
  #define _alloca( x )				(({assert( (x)<ID_MAX_ALLOCA_SIZE );}), alloca( (x) ))
  #define _alloca16( x )			(({assert( (x)<ID_MAX_ALLOCA_SIZE );}),((void *)((((uintptr_t)alloca( (x)+15 )) + 15) & ~15)))
#else
  // GCC, CLANG, MCST-LCC >= 1.28
  #define _alloca16( x )			( ({assert((x)<ID_MAX_ALLOCA_SIZE);}), __builtin_alloca_with_align( (x), 16*8 ) )
  #define _alloca( x )				( ({assert((x)<ID_MAX_ALLOCA_SIZE);}), __builtin_alloca( (x) ) )
#endif

#ifdef GAME_DLL
#define ID_GAME_API					__attribute__((visibility ("default")))
#else
#define ID_GAME_API
#endif

#define ALIGN16( x )				x
#define PACKED						__attribute__((packed))

#define PATHSEPERATOR_STR			"/"
#define PATHSEPERATOR_CHAR			'/'

#define __cdecl
#define ASSERT						assert

#define ID_INLINE					inline
#define ID_STATIC_TEMPLATE

#define assertmem( x, y )

#endif

#ifndef ID_MAYBE_INLINE
// for MSVC it's __inline, otherwise just inline should work
#define ID_MAYBE_INLINE inline
#endif // ID_MAYBE_INLINE

#ifdef __GNUC__
#define id_attribute(x) __attribute__(x)
#else
#define id_attribute(x)
#endif

#if !defined(_MSC_VER)
	// MSVC does not provide this C99 header
	#include <inttypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <cstddef>
#include <typeinfo>
#include <errno.h>
#include <math.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef FindText								// stupid namespace poluting Microsoft monkeys
#endif

// Apple legacy
#ifdef __APPLE__
#include <Availability.h>
#ifdef __MAC_OS_X_VERSION_MIN_REQUIRED
#if __MAC_OS_X_VERSION_MIN_REQUIRED == 1040
#define OSX_TIGER
#elif __MAC_OS_X_VERSION_MIN_REQUIRED < 1060
#define OSX_LEOPARD
#endif
#endif
#endif

#define ID_TIME_T time_t

typedef unsigned char			byte;		// 8 bits
typedef unsigned short			word;		// 16 bits
typedef unsigned int			dword;		// 32 bits
typedef unsigned int			uint;
typedef unsigned long			ulong;

typedef int						qhandle_t;

#ifndef NULL
#define NULL					((void *)0)
#endif

#ifndef BIT
#define BIT( num )				( 1 << ( num ) )
#endif

#define	MAX_STRING_CHARS		1024		// max length of a string

// maximum world size
#define MAX_WORLD_COORD			( 128 * 1024 )
#define MIN_WORLD_COORD			( -128 * 1024 )
#define MAX_WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

#endif
