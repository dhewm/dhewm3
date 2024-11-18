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
along with Doom 3 Source Code.	If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include <float.h>

#ifdef D3_SDL3
#include <SDL3/SDL_version.h>
#include <SDL3/SDL_cpuinfo.h>
#else // SDL1.2 or SDL2
#include <SDL_version.h>
#include <SDL_cpuinfo.h>
#endif

// hack add missing instructions to SDL1.2
#if SDL_VERSION_ATLEAST(2, 0, 0)
#define SDL_SUPPORTS_ABOVE_SSE2
#endif

// MSVC header intrin.h uses strcmp and errors out when not set
#define IDSTR_NO_REDIRECT

#include "sys/platform.h"
#include "framework/Common.h"
#include "sys/sys_public.h"

#define _REG_EAX		0
#define _REG_EBX		1
#define _REG_ECX		2
#define _REG_EDX		3

#if defined(__GNUC__) && (defined(__i386__) || defined (__x86_64__))
#include <cpuid.h>
// yes cpuid has been around for a long time
static inline void CPUid( int func, unsigned regs[4] ) {
	// stgatilov: use macro from cpuid.h on GCC (and possibly Clang)
	__cpuid( func, regs[_REG_EAX], regs[_REG_EBX], regs[_REG_ECX], regs[_REG_EDX] );
}
#elif defined(_MSC_VER)
// intrin.h allready included by SDL_cpuinfo.h
static inline void CPUid( int func, unsigned regs[4] ) {
	// greebo: Use intrinsics on VC++
	int values[4] = { 0, 0, 0, 0 };

	__cpuid( values, func );

	regs[_REG_EAX] = values[0];
	regs[_REG_EBX] = values[1];
	regs[_REG_ECX] = values[2];
	regs[_REG_EDX] = values[3];
}
#else
#error unsupported compiler
#endif

/*
================
IsAMD
================
*/
static bool IsAMD( void ) {
	char pstring[16];
	char processorString[13];

	// get name of processor
	CPUid( 0, ( unsigned int * )pstring );
	processorString[0] = pstring[4];
	processorString[1] = pstring[5];
	processorString[2] = pstring[6];
	processorString[3] = pstring[7];
	processorString[4] = pstring[12];
	processorString[5] = pstring[13];
	processorString[6] = pstring[14];
	processorString[7] = pstring[15];
	processorString[8] = pstring[8];
	processorString[9] = pstring[9];
	processorString[10] = pstring[10];
	processorString[11] = pstring[11];
	processorString[12] = 0;

	if ( strcmp( processorString, "AuthenticAMD" ) == 0 ) {
		return true;
	}
	return false;
}

// SDL 1.2 only supports up to SSE2 so we Add our own.
#ifndef SDL_SUPPORTS_ABOVE_SSE2

/*
================
SDL_HasSSE3
================
*/
static inline bool SDL_HasSSE3( void ) {
	unsigned regs[4] = { 0, 0, 0, 0 };

	// get CPU feature bits
	CPUid( 1, regs );

	// bit 9 of ECX denotes SSE3 existence
	if ( regs[_REG_ECX] & ( 1 << 0 ) ) {
		return true;
	}
	return false;
}

/*
================
SDL_HasSSE41
================
*/
static inline bool SDL_HasSSE41( void ) {
	unsigned regs[4] = { 0, 0, 0, 0 };

	// get CPU feature bits
	CPUid( 1, regs );

	// bit 19 of ECX denotes SSE4.1 existence
	if ( regs[_REG_ECX] & ( 1 << 19 ) ) {
		return true;
	}
	return false;
}

/*
================
SDL_HasAVX
================
*/
static inline bool SDL_HasAVX( void ) {
	unsigned regs[4] = { 0, 0, 0, 0 };

	// get CPU feature bits
	CPUid( 1, regs );

	//check if CPU supports AVX instructions
	bool cpuAVXSupport = ( regs[_REG_ECX] & ( 1 << 28 ) ) != 0;

	//check if xsave/xrstor instructions are enabled by OS for context switches
	bool osUsesXsaveXrstor = ( regs[_REG_ECX] & ( 1 << 27 ) ) != 0;

	if ( cpuAVXSupport && osUsesXsaveXrstor ) {
#if defined(_MSC_VER)
		uint64_t xcrFeatureMask = _xgetbv( _XCR_XFEATURE_ENABLED_MASK );
#elif defined (__GNUC__) && (defined(__i386__) || defined (__x86_64__))
		// stgatilov: GCC did not have proper _xgetbv intrinsic until GCC 9
		// inline assembly taken from: https://gist.github.com/hi2p-perim/7855506
		int index = 0;	//_XCR_XFEATURE_ENABLED_MASK
		unsigned int eax, edx;
		__asm__ __volatile__(
		    "xgetbv;"
		    : "=a"( eax ), "=d"( edx )
		    : "c"( index )
		);
		uint64_t xcrFeatureMask = ( ( unsigned long long )edx << 32 ) | eax;
#endif
		//check if OS is configured to save/restore YMM registers on context switches
		if ( ( xcrFeatureMask & 0x6 ) == 0x6 ) {
			return true;
		}
	}
	return false;
}

/*
================
SDL_HasAVX2
================
*/
static inline bool SDL_HasAVX2( void ) {
	unsigned regs[4] = { 0, 0, 0, 0 };

	//check that cpuid instruction supports function 7
	CPUid( 0, regs );

	if ( regs[0] < 7 ) {
		return false;
	}

	// get CPU feature bits
	CPUid( 7, regs );

	// check if CPU supports AVX2 instructions
	bool cpuAVX2Support = ( regs[_REG_EBX] & ( 1 << 5 ) ) != 0;

	if ( cpuAVX2Support && HasAVX() ) {
		return true;
	}
	return false;
}

#endif /* SDL_HAS_ABOVE_SSE2_INSTRUCTIONS */

/*
================
SDL_HasFMA3
================
*/
static inline bool SDL_HasFMA3( void ) {
	unsigned regs[4] = { 0, 0, 0, 0 };

	// get CPU feature bits
	CPUid( 1, regs );

	// bit 12 of ECX denotes FMA3 support
	bool cpuFMA3Support = ( regs[_REG_ECX] & ( 1 << 12 ) ) != 0;

	// stgatilov: ensure that AVX2 is enabled too
	// (also checks for OS support of AVX registers)
	if ( cpuFMA3Support && SDL_HasAVX2() ) {
		return true;
	}
	return false;
}

// bitmasks for Denormals are zero and Flush to zero
#define MXCSR_DAZ	(1 << 6)
#define MXCSR_FTZ	(1 << 15)

/*
================
SDL_HasDAZ
================
*/
static inline bool SDL_HasDAZ( void ) {
	ALIGN16( unsigned char	FXSaveArea[512] );
	unsigned char			*FXArea = FXSaveArea;
	unsigned				regs[4] = { 0, 0, 0, 0 };

	// get CPU feature bits
	CPUid( 1, regs );

	// bit 24 of EDX denotes support for FXSAVE (possibly deprecated)
	if ( !( regs[_REG_EDX] & ( 1 << 24 ) ) ) {
		return false;
	}
	memset( FXArea, 0, sizeof( FXSaveArea ) );

// inline assembly not supported by msvc on 64 bit
#if defined(_MSC_VER) && defined(_M_X64) || \
	defined(__GNUC__) && (defined(__i386__) || defined (__x86_64__))
	_fxsave( FXArea );
#elif defined(_MSC_VER) && defined(_M_IX86)
	__asm {
		mov	   eax, FXArea
		FXSAVE[eax]
	}
#endif
	DWORD dwMask = *( DWORD * )&FXArea[28];				// Read the MXCSR Mask
	return ( ( dwMask & MXCSR_DAZ ) == MXCSR_DAZ );		// Return if the DAZ bit is set
}

/*
================
Sys_GetProcessorId
================
*/
int Sys_GetProcessorId( void ) {
	// unknown or unsupported cpu
	int flags = CPUID_GENERIC;

	// check for an AMD cpu default to Intel if not found.
	if ( IsAMD() ) {
		flags = CPUID_AMD;
	} else {
		flags = CPUID_INTEL;
	}

	// check for Multi Media Extensions
	if ( SDL_HasMMX() ) {
		flags |= CPUID_MMX;
	}

// SDL3 doesn't support detecting 3DNow, and current CPUs (even from AMD) don't support it either
#if !defined(D3_SDL3)
	// check for 3DNow! (deprecated and removed in SDL3)
	if ( SDL_Has3DNow() ) {
		flags |= CPUID_3DNOW;
	}
#endif

	// check for Streaming SIMD Extensions
	if ( SDL_HasSSE() ) {
		flags |= CPUID_SSE;
	}

	// check for Streaming SIMD Extensions 2
	if ( SDL_HasSSE2() ) {
		flags |= CPUID_SSE2;
	}

	// check for Streaming SIMD Extensions 3 aka Prescott's New Instructions
	if ( SDL_HasSSE3() ) {
		flags |= CPUID_SSE3;
	}

	// check for Streaming SIMD Extensions 4.1 (unused atm)
	if ( SDL_HasSSE41() ) {
		flags |= CPUID_SSE41;
	}

	// check for FMA3 (SDL2 does set the AVX2 flag without this but SDL1 does not even support AVX)
	if ( SDL_HasFMA3() ) {
		flags |= CPUID_FMA3;
	}

	// check for AVX
	if ( SDL_HasAVX() ) {
		flags |= CPUID_AVX;
	}

	// check for AVX2
	if ( SDL_HasAVX2() ) {
		flags |= CPUID_AVX2;
	}

	// check for Denormals-Are-Zero mode
	if ( SDL_HasDAZ() ) {
		flags |= CPUID_DAZ;
	}

	// check for ALTIVEC (apple in my eye...)
	if ( SDL_HasAltiVec() ) {
		flags |= CPUID_ALTIVEC;
	}
	return flags;
}

// inline assembly not supported by msvc 64 bit
// so use the standard intel functions to get the value there.
// removed unused macros and added some that also work on 64 bit msvc
// btw the return values are unsigned not plain int.
#if defined(_MSC_VER) && defined(_M_IX86)
#define STREFLOP_STMXCSR( cw ) do { unsigned int tmp = 0; __asm { stmxcsr tmp }; ( cw ) = tmp; } while (0)
#define STREFLOP_LDMXCSR( cw ) do { unsigned int tmp = ( cw ); __asm { ldmxcsr tmp }; } while (0)
#elif defined(_MSC_VER) && defined(_M_X64)
#define STREFLOP_STMXCSR( cw ) do { unsigned int tmp = 0; tmp = _mm_getcsr(); ( cw ) = tmp; } while (0)
#define STREFLOP_LDMXCSR( cw ) do { unsigned int tmp = ( cw ); _mm_setcsr( tmp ); } while (0)
#else /* __GNUC__? */
#define STREFLOP_STMXCSR( cw ) do { asm volatile ( "stmxcsr %0" : "=m" ( cw ) : ); } while (0)
#define STREFLOP_LDMXCSR( cw ) do { asm volatile ( "ldmxcsr %0" : : "m" ( cw ) ); } while (0)
#endif

/*
================
EnableMXCSRFlag

Ignored anyway if the CPU does not support SSE
================
*/
static inline void EnableMXCSRFlag( unsigned int flag, bool enable, const char *name ) {
	unsigned int sse_mode = 0;

	// stgatilov: note that MXCSR affects only SSE+ arithmetic
	// old x87 FPU has its own control register, but we don't care about it =)
	STREFLOP_STMXCSR( sse_mode );

	if ( enable && ( sse_mode & flag ) == flag ) {
		common->Printf( "%s mode is already enabled\n", name );
		return;
	}

	if ( !enable && ( sse_mode & flag ) == 0 ) {
		common->Printf( "%s mode is already disabled\n", name );
		return;
	}

	if ( enable ) {
		common->Printf( "enabling %s mode\n", name );
		sse_mode |= flag;
	} else {
		common->Printf( "disabling %s mode\n", name );
		sse_mode &= ~flag;
	}
	STREFLOP_LDMXCSR( sse_mode );
}

/*
================
Sys_FPU_SetDAZ
================
*/
void Sys_FPU_SetDAZ( bool enable ) {
	int cpuid = Sys_GetProcessorId();

	if ( ( cpuid & CPUID_DAZ ) == 0 ) {
		common->Printf( "this CPU doesn't support Denormals-Are-Zero\n" );
		return;
	}
	EnableMXCSRFlag( MXCSR_DAZ, enable, "Denormals-Are-Zero" );
}

/*
================
Sys_FPU_SetFTZ
================
*/
void Sys_FPU_SetFTZ( bool enable ) {
	int cpuid = Sys_GetProcessorId();

	if ( ( cpuid & CPUID_SSE ) == 0 ) {
		common->Printf( "this CPU doesn't support Flush-To-Zero\n" );
		return;
	}
	EnableMXCSRFlag( MXCSR_FTZ, enable, "Flush-To-Zero" );
}

/*
===============
Sys_FPU_SetPrecision
===============
*/
void Sys_FPU_SetPrecision() {
#if defined(_MSC_VER) && defined(_M_IX86)
	_controlfp( _PC_64, _MCW_PC );
#endif
}
