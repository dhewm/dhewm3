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

#ifndef __SYS_AROS__
#define __SYS_AROS__

#include <signal.h>

#include "sys/sys_public.h"

const char*	AROS_Cwd( void );

// called first thing. does InitSigs and various things
void		AROS_EarlyInit( );
// called after common has been initialized
void		AROS_LateInit( );

void		AROS_InitLibs( );
void		AROS_InitSigs( );
void		AROS_ClearSigs( );

void		AROS_Exit( int ret );
void		AROS_SetExit(int ret); // override the exit code
void		AROS_SetExitSpawn( const char *exeName ); // set the process to be spawned when we quit

void		AROS_InitConsoleInput( void );
void		AROS_Shutdown( void );

void		AROS_OpenURL( const char *url );

void		Sys_FPE_handler( int signum, siginfo_t *info, void *context );
void		Sys_DoStartProcess( const char *exeName, bool dofork = true ); // if not forking, current process gets replaced

#endif
