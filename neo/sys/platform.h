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

#include "framework/BuildDefines.h"

/*
===============================================================================

	Non-portable system services.

===============================================================================
*/

// Win32
#if defined(WIN32) || defined(_WIN32)

#define	BUILD_STRING				"win-x86"
#define BUILD_OS_ID					0
#define	CPUSTRING					"x86"
#define CPU_EASYARGS				1

#define _alloca16( x )				((void *)((((uintptr_t)_alloca( (x)+15 )) + 15) & ~15))

#define PATHSEPERATOR_STR			"\\"
#define PATHSEPERATOR_CHAR			'\\'

#ifdef _MSC_VER
#define ALIGN16( x )				__declspec(align(16)) x
#define PACKED						__attribute__((packed))
#define ID_INLINE					__forceinline
#define ID_STATIC_TEMPLATE			static
#define assertmem( x, y )			assert( _CrtIsValidPointer( x, y, true ) )
#else
#define ALIGN16( x )				x __attribute__ ((aligned (16)))
#define PACKED
#define ID_INLINE					inline
#define ID_STATIC_TEMPLATE
#define assertmem( x, y )
#endif

#define THREAD_RETURN_TYPE			DWORD

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
	#define ID_LITTLE_ENDIAN			1
#endif

#endif


// Mac OSX
#if defined(MACOS_X) || defined(__APPLE__)

#define BUILD_STRING				"MacOSX-universal"
#define BUILD_OS_ID					1
#ifdef __ppc__
	#define	CPUSTRING				"ppc"
	#define CPU_EASYARGS			0
#elif defined(__i386__)
	#define	CPUSTRING				"x86"
	#define CPU_EASYARGS			1
#endif

#define ALIGN16( x )				x __attribute__ ((aligned (16)))

#ifdef __MWERKS__
#define PACKED
#include <alloca.h>
#else
#define PACKED						__attribute__((packed))
#endif

#define _alloca						alloca
#define _alloca16( x )				((void *)((((uintptr_t)alloca( (x)+15 )) + 15) & ~15))

#define PATHSEPERATOR_STR			"/"
#define PATHSEPERATOR_CHAR			'/'

#define __cdecl
#define ASSERT						assert

#define ID_INLINE					inline
#define ID_STATIC_TEMPLATE

#define assertmem( x, y )

#define THREAD_RETURN_TYPE			void *

#endif


// Unix
#ifdef __unix__

#define BUILD_OS_ID					2

#ifdef __linux__
	#define BUILD_OS "linux"
#elif defined(__FreeBSD__)
	#define BUILD_OS "FreeBSD"
#elif defined(__DragonFly__)
	#define BUILD_OS "DragonFly"
#elif defined(__OpenBSD__)
	#define BUILD_OS "OpenBSD"
#elif defined(__NetBSD__)
	#define BUILD_OS "NetBSD"
#else
	#error unknown operating system!
#endif

#ifdef __i386__
	#define CPUSTRING				"x86"
	#define CPU_EASYARGS			1
#elif defined(__x86_64__)
	#define CPUSTRING				"x86_64"
	#define CPU_EASYARGS			0
#elif defined(__ppc__)
	#define CPUSTRING				"ppc"
	#define CPU_EASYARGS			0
#else
	#error unknown cpu architecture!
#endif

#define	BUILD_STRING				(BUILD_OS "-" CPUSTRING)

#define _alloca						alloca
#define _alloca16( x )				((void *)((((uintptr_t)alloca( (x)+15 )) + 15) & ~15))

#define ALIGN16( x )				x
#define PACKED						__attribute__((packed))

#define PATHSEPERATOR_STR			"/"
#define PATHSEPERATOR_CHAR			'/'

#define __cdecl
#define ASSERT						assert

#define ID_INLINE					inline
#define ID_STATIC_TEMPLATE

#define assertmem( x, y )

#define THREAD_RETURN_TYPE			void *

#endif


#if !defined(ID_LITTLE_ENDIAN) && !defined(ID_BIG_ENDIAN)
	#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
		#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			#define ID_LITTLE_ENDIAN
		#endif
	#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
		#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			#define ID_BIG_ENDIAN
		#endif
	#endif
#endif

#if !defined(ID_LITTLE_ENDIAN) && !defined(ID_BIG_ENDIAN)
	#if defined(__i386__) || defined(__x86_64__)
		#define ID_LITTLE_ENDIAN		1
	#elif defined(__ppc__)
		#define ID_BIG_ENDIAN			1
	#endif
#endif

#ifdef __GNUC__
#define id_attribute(x) __attribute__(x)
#else
#define id_attribute(x)
#endif

#include <inttypes.h>
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
