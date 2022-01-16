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

#define DEBUG 1

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

// undefine - conflict with ID functions
#undef Remove
#undef Insert
#undef Read
#undef Write
#undef Seek
#undef Flush
#undef Close
#undef Allocate
#undef Printf
#undef VPrintf

#include "sys/platform.h"
#include "idlib/containers/StrList.h"
#include "framework/Licensee.h"
#include "sys/sys_local.h"

#define ID_FTXT	MAKE_ID('F','T','X','T')
#define ID_CHRS	MAKE_ID('C','H','R','S')

extern idCVar com_pid;

struct Library *MiamiBase;
struct Library *OpenURLBase;

char chunk_buffer[1024];

idStr	adoom3_basepath;
idStr	adoom3_savepath;

/*
================
Sys_GetSystemRam
returns in megabytes rounded to the nearest 16Mb
================
*/
int Sys_GetSystemRam( void ) {
    int		mb;

    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    mb = ( ( AvailMem( MEMF_ANY ) / ( 1024 * 1024 ) ) + 8 ) & ~15;

    return mb;
}

/*
================
Sys_LockMemory
================
*/
bool Sys_LockMemory( void *ptr, int bytes ) {
    D(bug("[ADoom3] Sys_LockMemory( 0x%p, %u )\n", ptr, bytes));
    return true;
}

/*
================
Sys_UnlockMemory
================
*/
bool Sys_UnlockMemory( void *ptr, int bytes ) {
    D(bug("[ADoom3] Sys_UnlockMemory( 0x%p, %u )\n", ptr, bytes));
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

    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    list.Clear();

    if (!extension)
            extension = "";

    // passing a slash as extension will find directories
    if (extension[0] == '/' && extension[1] == 0) {
            extension = "";
            dironly = true;
    }

    // search
    // NOTE: case sensitivity of directory path can screw us up here
    if ((dirlock = Lock(directory, SHARED_LOCK)) == BNULL) {
            D(bug("[ADoom3] Sys_ListFiles: opendir '%s' failed\n", directory));
            return -1;
    }

    if ((fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL)) != (struct FileInfoBlock *)NULL)
    {
        if (Examine(dirlock, fib))
        {
            while(ExNext(dirlock, fib))
            {
                idStr filename((const char *)fib->fib_FileName);

                idStr::snPrintf(search, sizeof(search), "%s/%s", directory, fib->fib_FileName);
                if (!dironly) {
                    idStr look(search);
                    idStr ext;
                    look.ExtractFileExtension(ext);
                    if (extension[0] != '\0' && ext.Icmp(&extension[1]) != 0) {
                            continue;
                    }
                }
                if ((dironly && (fib->fib_DirEntryType != ST_USERDIR)) ||
                        (!dironly && (fib->fib_DirEntryType == ST_USERDIR)))
                        continue;
                D(bug("[ADoom3] Sys_ListFiles:\tadding %s to list\n", filename.c_str()));
                list.Append(filename);
            }
        }
        FreeDosObject(DOS_FIB, fib);
    }

    UnLock(dirlock);

    D(bug( "[ADoom3] Sys_ListFiles: %d entries in %s\n", list.Num(), directory ));

    return list.Num();
}

/*
================
Sys_Mkdir
================
*/
void Sys_Mkdir( const char *path ) {
    BPTR dirlock;

    D(bug("[ADoom3] Sys_Mkdir('%s')\n", path));

    if ((dirlock = CreateDir(path)) != BNULL)
    {
        D(bug("[ADoom3] Sys_Mkdir: created\n"));
        UnLock(dirlock);
    }
}

char *Sys_GetClipboardData(void) {
    struct IFFHandle *IFFHandle;
    struct ContextNode  *cn;
    ULONG error, read = 0;

    D(bug("[ADoom3] Sys_GetClipboardData()\n"));

    if ((IFFHandle = AllocIFF()))
    {
        if ((IFFHandle->iff_Stream = (IPTR)OpenClipboard(0)))
        {
            InitIFFasClip(IFFHandle);

            if (!OpenIFF(IFFHandle, IFFF_READ))
            {
                if (!StopChunk(IFFHandle, ID_FTXT, ID_CHRS))
                {
                    if (!(error = ParseIFF(IFFHandle, IFFPARSE_SCAN)))
                    {
                        cn = CurrentChunk(IFFHandle);

                        if (cn && (cn->cn_Type == ID_FTXT) && (cn->cn_ID == ID_CHRS))
                        {
                            read = ReadChunkBytes(IFFHandle, chunk_buffer, 1024);
                        }
                    }
                }
                CloseIFF(IFFHandle);
            }
            CloseClipboard((struct ClipboardHandle *)IFFHandle->iff_Stream);
        }
        FreeIFF(IFFHandle);
    }

    
    if (read > 0)
    {   
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

    D(bug("[ADoom3] Sys_SetClipboardData('%s')\n", string));

    if ((string) && (strlen(string) > 0))
    {
        if((IFFHandle = AllocIFF()))
        {
            if((IFFHandle->iff_Stream = (IPTR)OpenClipboard(0)))
            {
                InitIFFasClip(IFFHandle);

                if(!OpenIFF(IFFHandle, IFFF_WRITE))
                {
                    if(!PushChunk(IFFHandle, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN))
                    {
                        if(!PushChunk(IFFHandle, 0, ID_CHRS, IFFSIZE_UNKNOWN))
                        {
                            if(WriteChunkBytes(IFFHandle, (char *)string, strlen(string)) == strlen(string))
                            {
                                if(!PopChunk(IFFHandle))
                                    written = TRUE;
                            }
                        }
                        if(written)
                            PopChunk(IFFHandle);
                    }
                    CloseIFF(IFFHandle);
                }
                CloseClipboard((struct ClipboardHandle *)IFFHandle->iff_Stream);
            }
            FreeIFF(IFFHandle);
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
    D(bug("[ADoom3] Sys_GetDriveFreeSpace('%s')\n", path));

    D(bug("[ADoom3] ** TODO: Sys_GetDriveFreeSpace\n"));

    return 1000 * 1024;
}

/*
===============
AROS_InitLibs
===============
*/
void AROS_InitLibs( void ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    MiamiBase = OpenLibrary("miami.library", 0);
    OpenURLBase = OpenLibrary("openurl.library", 0);
}

#if (0)
/*
==============
Sys_EXEPath
==============
*/
const char *Sys_EXEPath( void ) {
    static char	buf[ 1024 ];
    BPTR                  pathlock;

    D(bug("[ADoom3] Sys_EXEPath()\n"));

    if ((pathlock = Lock("PROGDIR:", SHARED_LOCK)) != BNULL)
    {
        if ( NameFromLock( pathlock, buf, sizeof( buf ) ) )
        {
            UnLock(pathlock);
            struct Node *thisTask = (struct Node *)FindTask(NULL);

            AddPart(buf, thisTask->ln_Name, 1024);

            D(bug("[ADoom3] Sys_EXEPath: using '%s'\n", buf));
            return buf;
        }
        UnLock(pathlock);
    }
    D(bug("[ADoom3] Sys_EXEPath: faling back to PROGDIR\n"));

    return "PROGDIR:ADoom3";
}


/*
================
AROS_Cwd
================
*/
const char *AROS_Cwd( void ) {
    static char	buf[ 1024 ];
    struct Process *thisTask = (struct Process *)FindTask(NULL);
    D(bug("[ADoom3] AROS_Cwd()\n"));

    if (thisTask->pr_CurrentDir != BNULL)
    {
        if ( NameFromLock( thisTask->pr_CurrentDir, buf, sizeof( buf ) ) )
        {
            D(bug("[ADoom3] AROS_Cwd: '%s'\n", buf));
            return buf;
        }
    }
    return "PROGDIR:";
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
    struct FileInfoBlock *fib;
    BPTR pathLock = BNULL;
    idStr testbase;

    D(bug("[ADoom3] Sys_DefaultBasePath()\n"));

    adoom3_basepath = Sys_EXEPath();
    if ( adoom3_basepath.Length() ) {
            adoom3_basepath.StripFilename();
            testbase = adoom3_basepath; testbase += "/"; testbase += BASE_GAMEDIR;
            if ((fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL)) != (struct FileInfoBlock *)NULL)
            {
                if ((pathLock = Lock(testbase.c_str(), SHARED_LOCK)) != BNULL)
                {
                    if (Examine(pathLock, fib))
                    {
                        if (fib->fib_DirEntryType == ST_USERDIR)
                        {
                            UnLock(pathLock);
                            FreeDosObject(DOS_FIB, fib);
                            return adoom3_basepath.c_str();
                        }
                    }
                    UnLock(pathLock);
                }
                FreeDosObject(DOS_FIB, fib);
            }
            D(bug( "[ADoom3] Sys_DefaultBasePath: no '%s' directory in exe path %s, skipping\n", BASE_GAMEDIR, adoom3_basepath.c_str() ));
    }
    if ( adoom3_basepath != AROS_Cwd() ) {
            adoom3_basepath = AROS_Cwd();
            testbase = adoom3_basepath; testbase += "/"; testbase += BASE_GAMEDIR;
            if ((fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL)) != (struct FileInfoBlock *)NULL)
            {
                if ((pathLock = Lock(testbase.c_str(), SHARED_LOCK)) != BNULL)
                {
                    if (Examine(pathLock, fib))
                    {
                        if (fib->fib_DirEntryType == ST_USERDIR)
                        {
                            UnLock(pathLock);
                            FreeDosObject(DOS_FIB, fib);
                            return adoom3_basepath.c_str();
                        }
                    }
                    UnLock(pathLock);
                }
                FreeDosObject(DOS_FIB, fib);
            }
            D(bug( "[ADoom3] Sys_DefaultBasePath: no '%s' directory in cwd path %s, skipping\n", BASE_GAMEDIR, adoom3_basepath.c_str()));
    }
    return "PROGDIR:";
}

/*
 ==============
 Sys_DefaultSavePath
 ==============
 */
const char *Sys_DefaultSavePath(void) {
    static char	buf[ 1024 ];
    BPTR                  pathlock;

    D(bug("[ADoom3] Sys_DefaultSavePath()\n"));

    if ((pathlock = Lock("ENVARC:", SHARED_LOCK)) != BNULL)
    {
        if ( NameFromLock( pathlock, buf, sizeof( buf ) ) )
        {
            UnLock(pathlock);

#if defined( ID_DEMO_BUILD )
            AddPart(buf, ".doom3-demo", sizeof( buf ));
#else
            AddPart(buf, ".doom3", sizeof( buf ));
#endif
            D(bug("[ADoom3] Sys_DefaultSavePath: using '%s'\n", buf));
            return buf;
        }
        UnLock(pathlock);
    }

    D(bug("[ADoom3] Sys_DefaultSavePath: faling back to DefaultBasePath\n"));

    sprintf(buf, "%s/.doom3", Sys_DefaultBasePath());
    return buf;
}
#endif

/*
=================
Sys_OpenURL
=================
*/
void AROS_OpenURL( const char *url ) {
    struct TagItem tags[2];

    tags[0].ti_Tag = TAG_DONE; tags[0].ti_Data = 0;

    D(bug("[ADoom3] OpenURL( '%s' )\n", url));

    URL_OpenA( (char *)url, tags );
}

bool AROS_GetSavePath(char buf[1024])
{
	static const size_t bufSize = 1024; // NOTE: keep in sync with caller/function sig!
	BPTR pathlock;
	bool ret = false;
	if ((pathlock = Lock("PROGDIR:", SHARED_LOCK)) != BNULL)
	{
		if ( NameFromLock( pathlock, buf, bufSize ) )
		{
			D(bug("[ADoom3] Sys_GetPath: using '%s'\n", buf));
			ret = true;
		}
		UnLock(pathlock);
	}
	return ret;
}

bool Sys_GetPath(sysPath_t type, idStr &path) {
    char buf[1024];
    BPTR pathlock;
    bool ret = false;

    D(bug("[ADoom3] Sys_GetPath(%d)\n", type));

    path.Clear();

    switch(type) {
    case PATH_BASE:
    case PATH_CONFIG:
    case PATH_SAVE:
            if(AROS_GetSavePath(buf)) {
                path = buf;
                ret = true;
            }
            break;

    case PATH_EXE:
            if ((pathlock = Lock("PROGDIR:", SHARED_LOCK)) != BNULL)
            {
                if ( NameFromLock( pathlock, buf, sizeof( buf ) ) )
                {
                    struct Node *thisTask = (struct Node *)FindTask(NULL);

                    AddPart(buf, thisTask->ln_Name, 1024);

                    D(bug("[ADoom3] Sys_GetPath: using '%s'\n", buf));
                    path = buf;
                    ret = true;
                }
                UnLock(pathlock);
            }
            break;
    }

    return ret;
}
