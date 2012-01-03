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

#include <sys/param.h>
#include <sys/ucontext.h>
#include <unistd.h>
#include <fenv.h>
#include <mach/thread_status.h>
#include <AppKit/AppKit.h>

#include <SDL_main.h>

#include "sys/platform.h"
#include "idlib/Str.h"
#include "framework/Common.h"

#include "sys/posix/posix_public.h"

#define TEST_FPU_EXCEPTIONS			\
FPU_EXCEPTION_INVALID_OPERATION |		\
FPU_EXCEPTION_DENORMALIZED_OPERAND |	\
FPU_EXCEPTION_DIVIDE_BY_ZERO |			\
/* FPU_EXCEPTION_NUMERIC_OVERFLOW |	*/		\
/* FPU_EXCEPTION_NUMERIC_UNDERFLOW | */		\
/* FPU_EXCEPTION_INEXACT_RESULT | */		\
0

/*
==============
Sys_EXEPath
==============
*/
const char *Sys_EXEPath( void ) {
	static char exepath[ MAXPATHLEN ];
	strncpy( exepath, [ [ [ NSBundle mainBundle ] bundlePath ] cString ], MAXPATHLEN );
	return exepath;
}

/*
==========
Sys_DefaultSavePath
==========
*/
const char *Sys_DefaultSavePath(void) {
	static char savepath[ MAXPATHLEN ];
#if defined( ID_DEMO_BUILD )
	sprintf( savepath, "%s/Library/Application Support/Doom 3 Demo", [NSHomeDirectory() cString] );
#else
	sprintf( savepath, "%s/Library/Application Support/Doom 3", [NSHomeDirectory() cString] );
#endif
	return savepath;
}

/*
==========
Sys_DefaultBasePath
==========
*/
const char *Sys_DefaultBasePath(void) {
	static char basepath[ MAXPATHLEN ];

	strncpy( basepath, [ [ [ NSBundle mainBundle ] bundlePath ] cString ], MAXPATHLEN );
	char *snap = strrchr( basepath, '/' );
	if ( snap ) {
		*snap = '\0';
	}

	return basepath;
}

/*
===============
Sys_Shutdown
===============
*/
void Sys_Shutdown( void ) {
	Posix_Shutdown();
}

/*
===============
Sys_FPU_EnableExceptions
http://developer.apple.com/documentation/mac/PPCNumerics/PPCNumerics-154.html
http://developer.apple.com/documentation/Performance/Conceptual/Mac_OSX_Numerics/Mac_OSX_Numerics.pdf
===============
*/

#define fegetenvd(x) asm volatile( "mffs %0" : "=f" (x) );
#define fesetenvd(x) asm volatile( "mtfsf 255,%0" : : "f" (x) );
enum {
	FE_ENABLE_INEXACT		= 0x8,
	FE_ENABLE_DIVBYZERO		= 0x10,
	FE_ENABLE_UNDERFLOW		= 0x20,
	FE_ENABLE_OVERFLOW		= 0x40,
	FE_ENABLE_INVALID		= 0x80,
	FE_ENABLE_ALL_EXCEPT	= 0xF8
};

typedef union {
	struct {
		unsigned long hi;
		unsigned long lo;
	} i;
	double d;
} hexdouble;

static int exception_mask = 0;

void Sys_FPU_EnableExceptions( int exceptions ) {
#if 0
	if ( exceptions & ( FPU_EXCEPTION_INVALID_OPERATION | FPU_EXCEPTION_DENORMALIZED_OPERAND ) ) {
		// clear the flag before enabling the exception
		asm( "mtfsb0 2" );
		asm( "mtfsb0 7" );
		asm( "mtfsb0 8" );
		asm( "mtfsb0 9" );
		asm( "mtfsb0 10" );
		asm( "mtfsb0 11" );
		asm( "mtfsb0 12" );
		asm( "mtfsb0 21" );
		asm( "mtfsb0 22" );
		asm( "mtfsb0 23" );
		// enable
		asm( "mtfsb1 24" );
	} else {
		asm( "mtfsb0 24" );
	}
	if ( exceptions & FPU_EXCEPTION_DIVIDE_BY_ZERO ) {
		asm( "mtfsb0 5" );
		asm( "mtfsb1 27" );
	} else {
		asm( "mtfsb0 27" );
	}
	if ( exceptions & FPU_EXCEPTION_NUMERIC_OVERFLOW ) {
		asm( "mtfsb0 3" );
		asm( "mtfsb1 25" );
	} else {
		asm( "mtfsb0 25" );
	}
	if ( exceptions & FPU_EXCEPTION_NUMERIC_UNDERFLOW ) {
		asm( "mtfsb0 4" );
		asm( "mtfsb1 26" );
	} else {
		asm( "mtfsb0 26" );
	}
	if ( exceptions & FPU_EXCEPTION_INEXACT_RESULT ) {
		asm( "mtfsb0 6" );
		asm( "mtfsb0 13" );
		asm( "mtfsb0 14" );
		asm( "mtfsb1 28" );
	} else {
		asm( "mtfsb0 28" );
	}
#elif defined(__ppc__)
	hexdouble t;
	exception_mask = 0;
	if ( exceptions & ( FPU_EXCEPTION_INVALID_OPERATION | FPU_EXCEPTION_DENORMALIZED_OPERAND ) ) {
		exception_mask |= FE_ENABLE_INVALID;
	}
	if ( exceptions & FPU_EXCEPTION_DIVIDE_BY_ZERO ) {
		exception_mask |= FE_ENABLE_DIVBYZERO;
	}
	if ( exceptions & FPU_EXCEPTION_NUMERIC_OVERFLOW ) {
		exception_mask |= FE_ENABLE_OVERFLOW;
	}
	if ( exceptions & FPU_EXCEPTION_NUMERIC_UNDERFLOW ) {
		exception_mask |= FE_ENABLE_UNDERFLOW;
	}
	if ( exceptions & FPU_EXCEPTION_INEXACT_RESULT ) {
		exception_mask |= FE_ENABLE_INVALID;
	}
	Sys_Printf( "Sys_FPUEnableExceptions: 0x%x\n", exception_mask );
	// clear the exception flags
	feclearexcept( FE_ALL_EXCEPT );
	// set the enable flags on the exceptions we want
	fegetenvd( t.d );
	t.i.lo &= ~FE_ENABLE_ALL_EXCEPT;
	t.i.lo |= exception_mask;
	fesetenvd( t.d );
	Sys_Printf( "done\n" );
#endif
}

/*
===============
Sys_FPE_handler
===============
*/
void Sys_FPE_handler( int signum, siginfo_t *info, void *context ) {
#if defined(__ppc__)
	int ret;
	ppc_float_state_t *fs;
	ppc_thread_state_t *ss;

	fs = &( (struct ucontext *)context )->uc_mcontext->fs;
	ss = &( (struct ucontext *)context )->uc_mcontext->ss;

	Sys_Printf( "FPE at 0x%x:\n", info->si_addr );

	ret = fetestexcept( FE_ALL_EXCEPT );
	if ( ret & FE_INEXACT ) {
		Sys_Printf( "FE_INEXACT " );
	}
	if ( ret & FE_DIVBYZERO ) {
		Sys_Printf( "FE_DIVBYZERO " );
	}
	if ( ret & FE_UNDERFLOW ) {
		Sys_Printf( "FE_UNDERFLOW " );
	}
	if ( ret & FE_OVERFLOW ) {
		Sys_Printf( "FE_OVERFLOW " );
	}
	if ( ret & FE_INVALID ) {
		Sys_Printf( "FE_INVALID " );
	}
	Sys_Printf( "\n" );
	// clear the exception flags
	feclearexcept( FE_ALL_EXCEPT );
	// re-arm
	fs->fpscr &= exception_mask;
	ss->srr0 += 4;
#endif
}

/*
================
Sys_GetSystemRam
returns in megabytes
================
*/
int Sys_GetSystemRam( void ) {
	long ramSize;

	if ( Gestalt( gestaltPhysicalRAMSize, &ramSize ) == noErr ) {
		return ramSize / (1024*1024);
	}
	else
		return 1024;
}

bool OSX_GetCPUIdentification( int& cpuId, bool& oldArchitecture )
{
	long cpu;
	Gestalt(gestaltNativeCPUtype, &cpu);

	cpuId = cpu;
	oldArchitecture = cpuId < gestaltCPU970;
	return true;
}

void OSX_GetVideoCard( int& outVendorId, int& outDeviceId )
{
	kern_return_t err;
	mach_port_t masterPort;
	io_iterator_t itThis;
	io_service_t service;

	outVendorId = -1;
	outDeviceId = -1;

	// Get a mach port for us and check for errors
	err = IOMasterPort(MACH_PORT_NULL, &masterPort);
	if(err)
		return;
	// Grab all the PCI devices out of the registry
	err = IOServiceGetMatchingServices(masterPort, IOServiceMatching("IOPCIDevice"), &itThis);
	if(err)
		return;

	// Yank everything out of the iterator
	// We could walk through all devices and try to determine the best card. But for now,
	// we'll just look at the first card.
	while(1)
	{
		service = IOIteratorNext(itThis);
		io_name_t dName;

		// Make sure we have a valid service
		if(service)
		{
			// Get the classcode so we know what we're looking at
			CFDataRef classCode =  (CFDataRef)IORegistryEntryCreateCFProperty(service,CFSTR("class-code"),kCFAllocatorDefault,0);
			// Only accept devices that are
			// PCI Spec - 0x00030000 is a display device
			if((*(UInt32*)CFDataGetBytePtr(classCode) & 0x00ff0000) == 0x00030000)
			{
				// Get the name of the service (hw)
				IORegistryEntryGetName(service, dName);

			    CFDataRef vendorID, deviceID;

				// Get the information for the device we've selected from the list
			    vendorID = (CFDataRef)IORegistryEntryCreateCFProperty(service, CFSTR("vendor-id"),kCFAllocatorDefault,0);
			    deviceID = (CFDataRef)IORegistryEntryCreateCFProperty(service, CFSTR("device-id"),kCFAllocatorDefault,0);

			    outVendorId = *((long*)CFDataGetBytePtr(vendorID));
			    outDeviceId = *((long*)CFDataGetBytePtr(deviceID));

				CFRelease(vendorID);
				CFRelease(deviceID);
			}
			CFRelease(classCode);

			// Stop after finding the first device
			if (outVendorId != -1)
				break;
		}
		else
			break;
	}
}

/*
===============
main
===============
*/
int main( int argc, char *argv[] ) {
	NSAutoreleasePool *pool;

	pool = [[NSAutoreleasePool alloc] init];

	if (![[NSFileManager defaultManager] changeCurrentDirectoryPath:[[NSBundle mainBundle] resourcePath]])
		Sys_Error("Could not access application resources");

	//Sys_FPU_EnableExceptions(TEST_FPU_EXCEPTIONS);

	Posix_EarlyInit();

	if (argc > 1)
		common->Init(argc - 1, &argv[1]);
	else
		common->Init(0, NULL);

	Posix_LateInit();

	[NSApp activateIgnoringOtherApps:YES];

	while (1) {
		// maintain exceptions in case system calls are turning them off (is that needed)
		//Sys_FPU_EnableExceptions(TEST_FPU_EXCEPTIONS);

		common->Frame();

		// We should think about doing this less frequently than every frame
		[pool release];
		pool = [[NSAutoreleasePool alloc] init];
	}

	[pool release];
}
