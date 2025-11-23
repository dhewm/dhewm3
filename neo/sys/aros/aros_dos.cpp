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

//#define DEBUG 1
#include <aros/debug.h>
#undef ASSERT

#include <proto/alib.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
#include <proto/socket.h>
#include <proto/openurl.h>

#include <dos/dos.h>
#include <dos/dosextens.h>

#include <proto/icon.h>
#include <intuition/intuition.h>
#include <workbench/startup.h>

#include "sys/aros/aros_fixup.h"

#include "sys/platform.h"
#include "idlib/containers/StrList.h"
#include "framework/Licensee.h"
#include "sys/sys_local.h"

#define ID_FTXT MAKE_ID('F','T','X','T')
#define ID_CHRS MAKE_ID('C','H','R','S')

extern idCVar   com_pid;

struct Library  *MiamiBase;
struct Library  *OpenURLBase;

char            chunk_buffer[1024];
extern char     launch_path[1024];

idStr           adoom3_basepath;
idStr           adoom3_savepath;

/*
================
Sys_GetSystemRam
returns in megabytes rounded to the nearest 16Mb
================
*/
int Sys_GetSystemRam( void ) {
    int mb;

    D( bug( "[ADoom3] %s()\n", __func__ ) );

    mb = ( ( AvailMem( MEMF_ANY ) / ( 1024 * 1024 ) ) + 8 ) & ~15;

    return mb;
}

/*
================
Sys_LockMemory
================
*/
bool Sys_LockMemory( void *ptr, int bytes ) {
    D( bug( "[ADoom3] %s( 0x%p, %u )\n", __func__, ptr, bytes ) );
    return true;
}

/*
================
Sys_UnlockMemory
================
*/
bool Sys_UnlockMemory( void *ptr, int bytes ) {
    D( bug( "[ADoom3] %s( 0x%p, %u )\n", __func__, ptr, bytes ) );
    return true;
}

/*
================
Sys_ListFiles
================
*/
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
    BPTR dirlock;
    struct FileInfoBlock *fib;
    char search[1024];

    bool dironly = false;

    D( bug("[ADoom3] %s()\n", __func__ ) );

    list.Clear();

    if ( !extension )
        extension = "";

    // passing a slash as extension will find directories
    if( extension[0] == '/' && extension[1] == 0 ) {
        extension = "";
        dironly = true;
    }

    // search
    // NOTE: case sensitivity of directory path can screw us up here
    if( ( dirlock = Lock( directory, SHARED_LOCK ) ) == BNULL ) {
        D( bug( "[ADoom3] %s: opendir '%s' failed\n", __func__, directory ) );
        return -1;
    }

    if( ( fib = (struct FileInfoBlock *)AllocDosObject( DOS_FIB, NULL ) ) != (struct FileInfoBlock *)NULL ) {
        if( Examine( dirlock, fib ) ) {
            while( ExNext( dirlock, fib ) ) {
                idStr filename( (const char *)fib->fib_FileName );

                idStr::snPrintf( search, sizeof( search ), "%s/%s", directory, fib->fib_FileName );
                if( !dironly ) {
                    idStr look( search );
                    idStr ext;
                    look.ExtractFileExtension( ext );
                    if( extension[0] != '\0' && ext.Icmp( &extension[1] ) != 0 ) {
                            continue;
                    }
                }
                if( ( dironly && ( fib->fib_DirEntryType != ST_USERDIR ) ) ||
                        ( !dironly && ( fib->fib_DirEntryType == ST_USERDIR ) ) )
                        continue;
                D( bug( "[ADoom3] %s:\tadding %s to list\n", __func__, filename.c_str() ) );
                list.Append( filename );
            }
        }
        FreeDosObject( DOS_FIB, fib );
    }

    UnLock( dirlock );

    D( bug( "[ADoom3] %s: %d entries in %s\n", __func__, list.Num(), directory ) );

    return list.Num();
}

/*
================
Sys_Mkdir
================
*/
void Sys_Mkdir( const char *path ) {
    BPTR dirlock;

    D( bug( "[ADoom3] %s('%s')\n", __func__, path ) );

    if( ( dirlock = CreateDir( path ) ) != BNULL ) {
        D( bug( "[ADoom3] %s: created\n", __func__ ) );
        UnLock( dirlock );
    }
}

char *Sys_GetClipboardData(void) {
    struct IFFHandle *IFFHandle;
    struct ContextNode  *cn;
    ULONG error, read = 0;

    D( bug( "[ADoom3] %s()\n", __func__ ) );

    if( ( IFFHandle = AllocIFF() ) ) {
        if( ( IFFHandle->iff_Stream = (IPTR)OpenClipboard( 0 ) ) ) {
            InitIFFasClip(IFFHandle);
            if( !OpenIFF( IFFHandle, IFFF_READ ) ) {
                if( !StopChunk( IFFHandle, ID_FTXT, ID_CHRS ) ) {
                    if( !( error = ParseIFF( IFFHandle, IFFPARSE_SCAN ) ) ) {
                        cn = CurrentChunk(IFFHandle);
                        if( cn && ( cn->cn_Type == ID_FTXT ) && ( cn->cn_ID == ID_CHRS ) ) {
                            read = ReadChunkBytes( IFFHandle, chunk_buffer, 1024 );
                        }
                    }
                }
                CloseIFF( IFFHandle );
            }
            CloseClipboard( (struct ClipboardHandle *)IFFHandle->iff_Stream );
        }
        FreeIFF( IFFHandle );
    }
    
    if( read > 0 ) {   
        return chunk_buffer;
    }

    return NULL;
}

void Sys_FreeClipboardData( char* data ) {
    // as Sys_GetClipboardData() returns a static buffer, there's nothing to free
}

void Sys_SetClipboardData( const char *string ) {
    struct	IFFHandle	*IFFHandle;
    BOOL	written = FALSE;

    D( bug( "[ADoom3] %s('%s')\n", __func__, string ) );

    if( ( string ) && ( strlen( string ) > 0 ) ) {
        if( ( IFFHandle = AllocIFF() ) ) {
            if( ( IFFHandle->iff_Stream = (IPTR)OpenClipboard( 0 ) ) ) {
                InitIFFasClip( IFFHandle );
                if( !OpenIFF( IFFHandle, IFFF_WRITE ) ) {
                    if( !PushChunk( IFFHandle, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN ) ) {
                        if( !PushChunk( IFFHandle, 0, ID_CHRS, IFFSIZE_UNKNOWN ) ) {
                            if( WriteChunkBytes( IFFHandle, (char *)string, strlen( string ) ) == strlen( string ) ) {
                                if( !PopChunk( IFFHandle ) )
                                    written = TRUE;
                            }
                        }
                        if( written )
                            PopChunk( IFFHandle );
                    }
                    CloseIFF( IFFHandle );
                }
                CloseClipboard( (struct ClipboardHandle *)IFFHandle->iff_Stream );
            }
            FreeIFF( IFFHandle );
        }
    }
}

/*
===========
Sys_GetDriveFreeSpace
return in MegaBytes
===========
*/
int Sys_GetDriveFreeSpace( const char *path ) {
    D( bug( "[ADoom3] %s('%s')\n", __func__, path ) );

    D( bug( "[ADoom3] ** TODO: Sys_GetDriveFreeSpace\n" ) );

    return 1000 * 1024;
}

/*
===============
AROS_InitLibs
===============
*/
void AROS_InitLibs( void ) {
    D( bug( "[ADoom3] %s()\n", __func__ ) );

    MiamiBase = OpenLibrary( "miami.library", 0 );
    OpenURLBase = OpenLibrary( "openurl.library", 0 );

    D(
        bug( "[ADoom3] %s: MiamiBase @ 0x%p\n", __func__, MiamiBase );
        bug( "[ADoom3] %s: OpenURLBase @ 0x%p\n", __func__, OpenURLBase );
    )
}

/*
=================
Sys_OpenURL
=================
*/
void AROS_OpenURL( const char *url ) {
    D( bug( "[ADoom3] %s( '%s' )\n", __func__, url ) );

    if( OpenURLBase ) {
        struct TagItem tags[2];

        tags[0].ti_Tag = TAG_DONE;
        tags[0].ti_Data = 0;

        URL_OpenA( (char *)url, tags );
    }
}

bool AROS_GetSavePath(char *buf, const size_t max)
{
    BPTR pathlock;
    bool ret = false;
    if( ( pathlock = Lock( "PROGDIR:", SHARED_LOCK ) ) != BNULL )
    {
        struct FileInfoBlock *fib;
        bool readonly = false;
        if( ( fib = (struct FileInfoBlock *)AllocDosObject( DOS_FIB, NULL ) ) ) {
            if( Examine( pathlock, fib ) )
                if( fib->fib_Protection & FIBF_WRITE )
                    readonly = TRUE;

            FreeDosObject( DOS_FIB, fib );
        }
        if( readonly ) {
            UnLock( pathlock );
            pathlock = Lock( "T:ADoom3", SHARED_LOCK );
            if( pathlock == BNULL ) {
                pathlock = CreateDir( "T:ADoom3" );
            }
        }
        if( pathlock ) {
            if( NameFromLock( pathlock, buf, max ) ) {
                D( bug( "[ADoom3] %s: using '%s'\n", __func__, buf ) );
                ret = true;
            }
            UnLock( pathlock );
        }
    }
    return ret;
}

bool Sys_GetPath(sysPath_t type, idStr &path) {
    char buf[1024];
    BPTR pathlock;
    bool ret = false;

    D( bug( "[ADoom3] %s(%d)\n", __func__, type ) );

    path.Clear();

    switch( type ) {
    case PATH_CONFIG:
        if( launch_path[0] != 0 ) {
            path = launch_path;
            ret = true;
            break;
        }
    case PATH_BASE:
    case PATH_SAVE:
        if( AROS_GetSavePath( buf, sizeof( buf ) ) ) {
            path = buf;
            ret = true;
        }
        break;

    case PATH_LAUNCH:
        if( launch_path[0] != 0 ) {
            path = launch_path;
            ret = true;
            break;
        }
    case PATH_EXE:
        if( ( pathlock = Lock( "PROGDIR:", SHARED_LOCK ) ) != BNULL ) {
            if( NameFromLock( pathlock, buf, sizeof( buf ) ) ) {
                struct Node *thisTask = (struct Node *)FindTask( NULL );

                if( thisTask->ln_Name )
                    AddPart( buf, FilePart( thisTask->ln_Name ), sizeof( buf ) );

                path = buf;
                ret = true;
            }
            UnLock( pathlock );
        }
        break;
    }
    D(
        if( path )
            bug( "[ADoom3] %s: using '%s'\n", __func__, path.c_str() );
        else
            bug( "[ADoom3] %s: returning 0\n", __func__);
    )
    return ret;
}
