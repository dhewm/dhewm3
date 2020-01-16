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

/*
===============================================================================

	Non-portable system services.

===============================================================================
*/

// Win32
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

#define _alloca16( x )				((void *)((((uintptr_t)_alloca( (x)+15 )) + 15) & ~15))

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

#define _alloca( x )				(({assert( (x)<600000 );}), alloca( (x) ))
#define _alloca16( x )				(({assert( (x)<600000 );}),((void *)((((uintptr_t)alloca( (x)+15 )) + 15) & ~15)))

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
#if defined(__MINGW32__)
	#include <malloc.h>
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
