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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pwd.h>
#include <pthread.h>

#include "sys/platform.h"
#include "framework/Common.h"

#include "sys/posix/posix_public.h"

#if defined(_DEBUG)
// #define ID_VERBOSE_PTHREADS
#endif

/*
======================================================
thread create and destroy
======================================================
*/

// not a hard limit, just what we keep track of for debugging
xthreadInfo *g_threads[MAX_THREADS];

int g_thread_count = 0;

typedef void *(*pthread_function_t) (void *);

/*
==================
Sys_CreateThread
==================
*/
void Sys_CreateThread( xthread_t function, void *parms, xthreadPriority priority, xthreadInfo& info, const char *name, xthreadInfo **threads, int *thread_count ) {
	Sys_EnterCriticalSection( );
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	if ( pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE ) != 0 ) {
		common->Error( "ERROR: pthread_attr_setdetachstate %s failed\n", name );
	}
	if ( pthread_create( ( pthread_t* )&info.threadHandle, &attr, function, parms ) != 0 ) {
		common->Error( "ERROR: pthread_create %s failed\n", name );
	}
	pthread_attr_destroy( &attr );
	info.name = name;
	if ( *thread_count < MAX_THREADS ) {
		threads[ ( *thread_count )++ ] = &info;
	} else {
		common->DPrintf( "WARNING: MAX_THREADS reached\n" );
	}
	Sys_LeaveCriticalSection( );
}

/*
==================
Sys_DestroyThread
==================
*/
void Sys_DestroyThread( xthreadInfo& info ) {
	// the target thread must have a cancelation point, otherwise pthread_cancel is useless
	assert( info.threadHandle );
	if ( pthread_cancel( ( pthread_t )info.threadHandle ) != 0 ) {
		common->Error( "ERROR: pthread_cancel %s failed\n", info.name );
	}
	if ( pthread_join( ( pthread_t )info.threadHandle, NULL ) != 0 ) {
		common->Error( "ERROR: pthread_join %s failed\n", info.name );
	}
	info.threadHandle = 0;
	Sys_EnterCriticalSection( );
	for( int i = 0 ; i < g_thread_count ; i++ ) {
		if ( &info == g_threads[ i ] ) {
			g_threads[ i ] = NULL;
			int j;
			for( j = i+1 ; j < g_thread_count ; j++ ) {
				g_threads[ j-1 ] = g_threads[ j ];
			}
			g_threads[ j-1 ] = NULL;
			g_thread_count--;
			break;
		}
	}
	Sys_LeaveCriticalSection( );
}

/*
==================
Sys_GetThreadName
find the name of the calling thread
==================
*/
const char* Sys_GetThreadName( int *index ) {
	Sys_EnterCriticalSection( );
	pthread_t thread = pthread_self();
	for( int i = 0 ; i < g_thread_count ; i++ ) {
		if ( thread == (pthread_t)g_threads[ i ]->threadHandle ) {
			if ( index ) {
				*index = i;
			}
			Sys_LeaveCriticalSection( );
			return g_threads[ i ]->name;
		}
	}
	if ( index ) {
		*index = -1;
	}
	Sys_LeaveCriticalSection( );
	return "main";
}

/*
==================
Posix_InitPThreads
==================
*/
void Posix_InitPThreads( ) {
	int i;

	// init threads table
	for ( i = 0; i < MAX_THREADS; i++ ) {
		g_threads[ i ] = NULL;
	}
}
