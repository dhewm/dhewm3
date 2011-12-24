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

// -*- mode: objc -*-
#import <unistd.h>

#import <Foundation/Foundation.h>
#import <Carbon/Carbon.h>
#import <AppKit/AppKit.h>
#import <OpenGL/gl.h>

#import <fenv.h>
#import <sys/ucontext.h>
#import <mach/thread_status.h>

#import <SDL_main.h>

#import "sys/platform.h"
#import "idlib/Str.h"
#import "framework/Licensee.h"
#import "framework/Common.h"
#import "sys/posix/posix_public.h"

#import "DOOMController.h"

static idStr			savepath;

#define TEST_FPU_EXCEPTIONS			\
FPU_EXCEPTION_INVALID_OPERATION |		\
FPU_EXCEPTION_DENORMALIZED_OPERAND |	\
FPU_EXCEPTION_DIVIDE_BY_ZERO |			\
/* FPU_EXCEPTION_NUMERIC_OVERFLOW |	*/		\
/* FPU_EXCEPTION_NUMERIC_UNDERFLOW | */		\
/* FPU_EXCEPTION_INEXACT_RESULT | */		\
0

@interface DOOMController (Private)
- (void)quakeMain;
- (BOOL)checkOS;
@end

@implementation DOOMController

/*
+ (void)initialize;
{
	static bool initialized = NO;

	[super initialize];
	if ( initialized ) {
		return;
	}
	initialized = YES;
}
*/

#define MAX_ARGC 1024

- (void)applicationDidFinishLaunching:(NSNotification *)notification;
{
	NS_DURING {
		NSAssert(sizeof(bool) == 1, @"sizeof(bool) should equal 1 byte");
		[self quakeMain];
	} NS_HANDLER {
		Sys_Error( (const char *)[ [ localException reason ] cString ] );
	} NS_ENDHANDLER;
	Sys_Quit();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	common->Quit();
	return NSTerminateLater;	// we never reach this
}

#if 0
// Actions

- (IBAction)paste:(id)sender;
{
	int shiftWasDown, insertWasDown;
	unsigned int currentTime;

	currentTime = Sys_Milliseconds();
	// Save the original keyboard state
	shiftWasDown = keys[K_SHIFT].down;
	insertWasDown = keys[K_INS].down;
	// Fake a Shift-Insert keyboard event
	keys[K_SHIFT].down = true;
	Posix_QueEvent(currentTime, SE_KEY, K_INS, true, 0, NULL);
	Posix_QueEvent(currentTime, SE_KEY, K_INS, false, 0, NULL);
	// Restore the original keyboard state
	keys[K_SHIFT].down = shiftWasDown;
	keys[K_INS].down = insertWasDown;
}

extern void CL_Quit_f(void);


- (IBAction)requestTerminate:(id)sender;
{
	common->Quit();
}

- (void)showBanner;
{
	static bool hasShownBanner = NO;

	if (!hasShownBanner) {
		//cvar_t *showBanner;

		hasShownBanner = YES;
		//showBanner = Cvar_Get("cl_showBanner", "1", 0);
		//if ( showBanner->integer != 0 ) {
		if ( true ) {
			NSPanel *splashPanel;
			NSImage *bannerImage;
			NSRect bannerRect;
			NSImageView *bannerImageView;

			bannerImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForImageResource:@"banner.jpg"]];
			bannerRect = NSMakeRect(0.0, 0.0, [bannerImage size].width, [bannerImage size].height);

			splashPanel = [[NSPanel alloc] initWithContentRect:bannerRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];

			bannerImageView = [[NSImageView alloc] initWithFrame:bannerRect];
			[bannerImageView setImage:bannerImage];
			[splashPanel setContentView:bannerImageView];
			[bannerImageView release];

			[splashPanel center];
			[splashPanel setHasShadow:YES];
			[splashPanel orderFront: nil];
			[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.5]];
			[splashPanel close];

			[bannerImage release];
		}
	}
}

// Services

- (void)connectToServer:(NSPasteboard *)pasteboard userData:(NSString *)data error:(NSString **)error;
{
	NSArray *pasteboardTypes;

	pasteboardTypes = [pasteboard types];
	if ([pasteboardTypes containsObject:NSStringPboardType]) {
		NSString *requestedServer;

		requestedServer = [pasteboard stringForType:NSStringPboardType];
		if (requestedServer) {
			Cbuf_AddText( va( "connect %s\n", [requestedServer cString]));
			return;
		}
	}
	*error = @"Unable to connect to server:  could not find string on pasteboard";
}

- (void)performCommand:(NSPasteboard *)pasteboard userData:(NSString *)data error:(NSString **)error;
{
	NSArray *pasteboardTypes;

	pasteboardTypes = [pasteboard types];
	if ([pasteboardTypes containsObject:NSStringPboardType]) {
		NSString *requestedCommand;

		requestedCommand = [pasteboard stringForType:NSStringPboardType];
		if (requestedCommand) {
			Cbuf_AddText(va("%s\n", [requestedCommand cString]));
			return;
		}
	}
	*error = @"Unable to perform command:  could not find string on pasteboard";
}

#endif // commented out all the banners and actions

@end

@implementation DOOMController (Private)

- (void)quakeMain
{
	NSAutoreleasePool *pool;
	int argc = 0;
	char *argv[MAX_ARGC];
	NSProcessInfo *processInfo;
	NSArray *arguments;
	unsigned int argumentIndex, argumentCount;
	//const char *cddir;
	//NSFileManager *defaultManager;
	//bool tryAgain;

	pool = [[NSAutoreleasePool alloc] init];

	[NSApp setServicesProvider:self];

	processInfo = [NSProcessInfo processInfo];
	arguments = [processInfo arguments];
	argumentCount = [arguments count];
	for (argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
		argv[argc++] = strdup([[arguments objectAtIndex:argumentIndex] cString]);
	}
	if (![[NSFileManager defaultManager] changeCurrentDirectoryPath:[[NSBundle mainBundle] resourcePath]]) {
		Sys_Error("Could not access application resources");
	}
	//cddir = macosx_scanForLibraryDirectory();
	/*
	do {
		tryAgain = NO;
		defaultManager = [NSFileManager defaultManager];
		if (![defaultManager fileExistsAtPath:@"./base/default.cfg"] && (!cddir || *cddir == '\0' || ![defaultManager fileExistsAtPath:[NSString stringWithFormat:@"%s/baseq3/pak0.pk3", cddir]])) {
			NSString *message;

			if (!cddir || *cddir == '\0') {
				message = [NSString stringWithFormat:@"Could not find DOOM levels."];
			} else if (![defaultManager fileExistsAtPath:[NSString stringWithFormat:@"%s", cddir]]) {
				message = [NSString stringWithFormat:@"Could not find DOOM levels:  '%s' does not exist.", cddir];
			} else {
				message = [NSString stringWithFormat:@"Could not find DOOM levels:  '%s' is not a complete DOOM installation.", cddir];
			}
			switch (NSRunAlertPanel(@"DOOM", @"%@", @"Quit", @"Find...", nil, message)) {
				case NSAlertDefaultReturn:
				default:
					Sys_Quit();
					break;
				case NSAlertAlternateReturn:
					tryAgain = YES;
					break;
			}
			if (tryAgain) {
				NSOpenPanel *openPanel;
				int result;

				openPanel = [NSOpenPanel openPanel];
				[openPanel setAllowsMultipleSelection:NO];
				[openPanel setCanChooseDirectories:YES];
				[openPanel setCanChooseFiles:NO];
				result = [openPanel runModalForDirectory:nil file:nil];
				if (result == NSOKButton) {
					NSArray *filenames;

					filenames = [openPanel filenames];
					if ([filenames count] == 1) {
						NSString *cdPath;

						cdPath = [filenames objectAtIndex:0];
						[[NSUserDefaults standardUserDefaults] setObject:cdPath forKey:@"CDPath"];
						cddir = strdup([cdPath cString]);
					}
				}
			}
		}
	} while (tryAgain);
	*/
/*
	if (cddir && *cddir != '\0') {
		SetProgramPath([[[NSString stringWithCString:cddir] stringByAppendingPathComponent:@"/x"] cString]);
	}
*/

	//Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );

	Posix_EarlyInit( );

#ifndef _DEBUG
	if ( [self checkOS] == FALSE) {
		common->Quit();
	}

	if ( [self checkDVD] == FALSE) {
		common->Quit();
	}
#endif

	// need strncmp, can't use idlib before init
#undef strncmp
	// Finder passes the process serial number as only argument after the program path
	// nuke it if we see it
	if ( argc > 1 && strncmp( argv[ 1 ], "-psn", 4 ) ) {
		common->Init( argc-1, &argv[1] );
	} else {
		common->Init( 0, NULL );
	}

	Posix_LateInit( );

	[NSApp activateIgnoringOtherApps:YES];

	while (1) {
		// maintain exceptions in case system calls are turning them off (is that needed)
		//Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );

		common->Frame();

		// We should think about doing this less frequently than every frame
		[pool release];
		pool = [[NSAutoreleasePool alloc] init];
	}

	[pool release];
}

- (BOOL)checkOS
{
	OSErr	err;
	long gestaltOSVersion;
	err = Gestalt(gestaltSystemVersion, &gestaltOSVersion);
	if ( err || gestaltOSVersion < 0x1038 ) {
		NSBundle *thisBundle = [ NSBundle mainBundle ];
		NSString *messsage = [ thisBundle localizedStringForKey:@"InsufficientOS" value:@"No translation" table:nil ];
		NSRunAlertPanel(@GAME_NAME, messsage, nil, nil, nil);
		return FALSE;
	}
	return TRUE;
}

- (BOOL)checkDVD
{
	return TRUE;
}

@end

/*
==============
Sys_EXEPath
==============
*/
const char *Sys_EXEPath( void ) {
	static char exepath[ 1024 ];
	strncpy( exepath, [ [ [ NSBundle mainBundle ] bundlePath ] cString ], 1024 );
	return exepath;
}

/*
 ==========
 Sys_DefaultSavePath
 ==========
 */
const char *Sys_DefaultSavePath(void) {
#if defined( ID_DEMO_BUILD )
	sprintf( savepath, "%s/Library/Application Support/Doom 3 Demo", [NSHomeDirectory() cString] );
#else
	sprintf( savepath, "%s/Library/Application Support/Doom 3", [NSHomeDirectory() cString] );
#endif
	return savepath.c_str();
}

/*
==========
Sys_DefaultBasePath
==========
*/
const char *Sys_DefaultBasePath(void) {
	static char basepath[ 1024 ];
	strncpy( basepath, [ [ [ NSBundle mainBundle ] bundlePath ] cString ], 1024 );
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
	savepath.Clear();
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
	return NSApplicationMain( argc, (const char **)argv );
}
