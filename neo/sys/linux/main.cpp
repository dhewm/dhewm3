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

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef ID_MCHECK
#include <mcheck.h>
#endif

#include <SDL_main.h>

#include "sys/platform.h"
#include "framework/Licensee.h"
#include "framework/FileSystem.h"
#include "sys/posix/posix_public.h"
#include "sys/sys_local.h"

#include "sys/linux/local.h"

static idStr	basepath;
static idStr	savepath;

/*
 ==============
 Sys_DefaultSavePath
 ==============
 */
const char *Sys_DefaultSavePath(void) {
#if defined( ID_DEMO_BUILD )
	sprintf( savepath, "%s/.doom3-demo", getenv( "HOME" ) );
#else
	sprintf( savepath, "%s/.doom3", getenv( "HOME" ) );
#endif
	return savepath.c_str();
}
/*
==============
Sys_EXEPath
==============
*/
const char *Sys_EXEPath( void ) {
	static char	buf[ 1024 ];
	idStr		linkpath;
	int			len;

	buf[ 0 ] = '\0';
	sprintf( linkpath, "/proc/%d/exe", getpid() );
	len = readlink( linkpath.c_str(), buf, sizeof( buf ) );
	if ( len == -1 ) {
		Sys_Printf("couldn't stat exe path link %s\n", linkpath.c_str());
		buf[ 0 ] = '\0';
	}
	return buf;
}

/*
================
Sys_DefaultBasePath

Get the default base path
- binary image path
- current directory
- hardcoded
Try to be intelligent: if there is no BASE_GAMEDIR, try the next path
================
*/
const char *Sys_DefaultBasePath(void) {
	struct stat st;
	idStr testbase;
	basepath = Sys_EXEPath();
	if ( basepath.Length() ) {
		basepath.StripFilename();
		testbase = basepath; testbase += "/"; testbase += BASE_GAMEDIR;
		if ( stat( testbase.c_str(), &st ) != -1 && S_ISDIR( st.st_mode ) ) {
			return basepath.c_str();
		} else {
			common->Printf( "no '%s' directory in exe path %s, skipping\n", BASE_GAMEDIR, basepath.c_str() );
		}
	}
	if ( basepath != Posix_Cwd() ) {
		basepath = Posix_Cwd();
		testbase = basepath; testbase += "/"; testbase += BASE_GAMEDIR;
		if ( stat( testbase.c_str(), &st ) != -1 && S_ISDIR( st.st_mode ) ) {
			return basepath.c_str();
		} else {
			common->Printf("no '%s' directory in cwd path %s, skipping\n", BASE_GAMEDIR, basepath.c_str());
		}
	}
	common->Printf( "WARNING: using hardcoded default base path\n" );
	return LINUX_DEFAULT_PATH;
}

/*
===============
Sys_Shutdown
===============
*/
void Sys_Shutdown( void ) {
	basepath.Clear();
	savepath.Clear();
	Posix_Shutdown();
}

/*
===============
Sys_FPU_EnableExceptions
===============
*/
void Sys_FPU_EnableExceptions( int exceptions ) {
}

/*
===============
Sys_FPE_handler
===============
*/
void Sys_FPE_handler( int signum, siginfo_t *info, void *context ) {
	assert( signum == SIGFPE );
	Sys_Printf( "FPE\n" );
}

/*
================
Sys_GetSystemRam
returns in megabytes
================
*/
int Sys_GetSystemRam( void ) {
	long	count, page_size;
	int		mb;

	count = sysconf( _SC_PHYS_PAGES );
	if ( count == -1 ) {
		common->Printf( "GetSystemRam: sysconf _SC_PHYS_PAGES failed\n" );
		return 512;
	}
	page_size = sysconf( _SC_PAGE_SIZE );
	if ( page_size == -1 ) {
		common->Printf( "GetSystemRam: sysconf _SC_PAGE_SIZE failed\n" );
		return 512;
	}
	mb= (int)( (double)count * (double)page_size / ( 1024 * 1024 ) );
	// round to the nearest 16Mb
	mb = ( mb + 8 ) & ~15;
	return mb;
}

/*
==================
Sys_DoStartProcess
if we don't fork, this function never returns
the no-fork lets you keep the terminal when you're about to spawn an installer

if the command contains spaces, system() is used. Otherwise the more straightforward execl ( system() blows though )
==================
*/
void Sys_DoStartProcess( const char *exeName, bool dofork ) {
	bool use_system = false;
	if ( strchr( exeName, ' ' ) ) {
		use_system = true;
	} else {
		// set exec rights when it's about a single file to execute
		struct stat buf;
		if ( stat( exeName, &buf ) == -1 ) {
			printf( "stat %s failed: %s\n", exeName, strerror( errno ) );
		} else {
			if ( chmod( exeName, buf.st_mode | S_IXUSR ) == -1 ) {
				printf( "cmod +x %s failed: %s\n", exeName, strerror( errno ) );
			}
		}
	}
	if ( dofork ) {
		switch ( fork() ) {
		case -1:
			printf( "fork failed: %s\n", strerror( errno ) );
			break;
		case 0:
			if ( use_system ) {
				printf( "system %s\n", exeName );
				if (system( exeName ) == -1)
					printf( "system failed: %s\n", strerror( errno ) );
				_exit( 0 );
			} else {
				printf( "execl %s\n", exeName );
				execl( exeName, exeName, NULL );
				printf( "execl failed: %s\n", strerror( errno ) );
				_exit( -1 );
			}
			break;
		default:
			break;
		}
	} else {
		if ( use_system ) {
			printf( "system %s\n", exeName );
			if (system( exeName ) == -1)
				printf( "system failed: %s\n", strerror( errno ) );
			else
				sleep( 1 );	// on some systems I've seen that starting the new process and exiting this one should not be too close
		} else {
			printf( "execl %s\n", exeName );
			execl( exeName, exeName, NULL );
			printf( "execl failed: %s\n", strerror( errno ) );
		}
		// terminate
		_exit( 0 );
	}
}

/*
=================
Sys_OpenURL
=================
*/
void idSysLocal::OpenURL( const char *url, bool quit ) {
	const char	*script_path;
	idFile		*script_file;
	char		cmdline[ 1024 ];

	static bool	quit_spamguard = false;

	if ( quit_spamguard ) {
		common->DPrintf( "Sys_OpenURL: already in a doexit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf( "Open URL: %s\n", url );
	// opening an URL on *nix can mean a lot of things ..
	// just spawn a script instead of deciding for the user :-)

	// look in the savepath first, then in the basepath
	script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_savepath" ), "", "openurl.sh" );
	script_file = fileSystem->OpenExplicitFileRead( script_path );
	if ( !script_file ) {
		script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_basepath" ), "", "openurl.sh" );
		script_file = fileSystem->OpenExplicitFileRead( script_path );
	}
	if ( !script_file ) {
		common->Printf( "Can't find URL script 'openurl.sh' in either savepath or basepath\n" );
		common->Printf( "OpenURL '%s' failed\n", url );
		return;
	}
	fileSystem->CloseFile( script_file );

	// if we are going to quit, only accept a single URL before quitting and spawning the script
	if ( quit ) {
		quit_spamguard = true;
	}

	common->Printf( "URL script: %s\n", script_path );

	// StartProcess is going to execute a system() call with that - hence the &
	idStr::snPrintf( cmdline, 1024, "%s '%s' &",  script_path, url );
	sys->StartProcess( cmdline, quit );
}

/*
===============
mem consistency stuff
===============
*/

#ifdef ID_MCHECK

const char *mcheckstrings[] = {
	"MCHECK_DISABLED",
	"MCHECK_OK",
	"MCHECK_FREE",	// block freed twice
	"MCHECK_HEAD",	// memory before the block was clobbered
	"MCHECK_TAIL"	// memory after the block was clobbered
};

void abrt_func( mcheck_status status ) {
	Sys_Printf( "memory consistency failure: %s\n", mcheckstrings[ status + 1 ] );
	Posix_SetExit( EXIT_FAILURE );
	common->Quit();
}

#endif

/*
===============
main
===============
*/
int main(int argc, char **argv) {
#ifdef ID_MCHECK
	// must have -lmcheck linkage
	mcheck( abrt_func );
	Sys_Printf( "memory consistency checking enabled\n" );
#endif

	Posix_EarlyInit( );

	if ( argc > 1 ) {
		common->Init( argc-1, &argv[1] );
	} else {
		common->Init( 0, NULL );
	}

	Posix_LateInit( );

	while (1) {
		common->Frame();
	}
}
