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

#include "sys/platform.h"
#include "idlib/CmdArgs.h"
#include "framework/async/AsyncNetwork.h"
#include "framework/Licensee.h"
#include "framework/UsercmdGen.h"
#include "renderer/tr_local.h"
#include "sys/win32/rc/CreateResourceIDs.h"
#include "sys/sys_local.h"

#include "sys/win32/win_local.h"

#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <shellapi.h>
#include <shlobj.h>

#ifndef __MRC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "tools/edit_public.h"

#undef strcmp // get rid of "#define strcmp idStr::Cmp", it conflicts with SDL headers

#include "sys/sys_sdl.h"

#ifdef D3_SDL3
  #define SDL_MAIN_HANDLED // dhewm3 implements WinMain() itself
  #include <SDL3/SDL_main.h>
#else // SDL1.2 or SDL2
  #include <SDL_main.h>
#endif


idCVar Win32Vars_t::win_outputDebugString( "win_outputDebugString", "0", CVAR_SYSTEM | CVAR_BOOL, "" );
idCVar Win32Vars_t::win_outputEditString( "win_outputEditString", "1", CVAR_SYSTEM | CVAR_BOOL, "" );
idCVar Win32Vars_t::win_viewlog( "win_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "" );

Win32Vars_t	win32;

#ifdef ID_ALLOW_TOOLS
/* These are required for tools (DG: taken from SteelStorm2) */

static HMODULE hOpenGL_DLL;

typedef BOOL(WINAPI * PWGLSWAPBUFFERS)(HDC);

PWGLSWAPBUFFERS			qwglSwapBuffers;

typedef BOOL(WINAPI * PWGLCOPYCONTEXT)(HGLRC, HGLRC, UINT);
typedef HGLRC(WINAPI * PWGLCREATECONTEXT)(HDC);
typedef HGLRC(WINAPI * PWGLCREATELAYERCONTEXT)(HDC, int);
typedef BOOL(WINAPI * PWGLDELETECONTEXT)(HGLRC);
typedef HGLRC(WINAPI * PWGLGETCURRENTCONTEXT)(VOID);
typedef HDC(WINAPI * PWGLGETCURRENTDC)(VOID);
typedef PROC(WINAPI * PWGLGETPROCADDRESS)(LPCSTR);
typedef BOOL(WINAPI * PWGLMAKECURRENT)(HDC, HGLRC);
typedef BOOL(WINAPI * PWGLSHARELISTS)(HGLRC, HGLRC);
typedef BOOL(WINAPI * PWGLUSEFONTBITMAPS)(HDC, DWORD, DWORD, DWORD);


PWGLCOPYCONTEXT			qwglCopyContext;
PWGLCREATECONTEXT		qwglCreateContext;
PWGLCREATELAYERCONTEXT	qwglCreateLayerContext;
PWGLDELETECONTEXT		qwglDeleteContext;
PWGLGETCURRENTCONTEXT	qwglGetCurrentContext;
PWGLGETCURRENTDC		qwglGetCurrentDC;
PWGLGETPROCADDRESS		qwglGetProcAddress;
PWGLMAKECURRENT			qwglMakeCurrent;
PWGLSHARELISTS			qwglShareLists;
PWGLUSEFONTBITMAPS		qwglUseFontBitmaps;

typedef BOOL(WINAPI * PWGLUSEFONTOUTLINES)(HDC, DWORD, DWORD, DWORD, FLOAT,
	FLOAT, int, LPGLYPHMETRICSFLOAT);
typedef BOOL(WINAPI * PWGLDESCRIBELAYERPLANE)(HDC, int, int, UINT,
	LPLAYERPLANEDESCRIPTOR);
typedef int  (WINAPI * PWGLSETLAYERPALETTEENTRIES)(HDC, int, int, int,
	CONST COLORREF *);
typedef int  (WINAPI * PWGLGETLAYERPALETTEENTRIES)(HDC, int, int, int,
	COLORREF *);
typedef BOOL(WINAPI * PWGLREALIZELAYERPALETTE)(HDC, int, BOOL);
typedef BOOL(WINAPI * PWGLSWAPLAYERBUFFERS)(HDC, UINT);

PWGLUSEFONTOUTLINES			qwglUseFontOutlines;
PWGLDESCRIBELAYERPLANE		qwglDescribeLayerPlane;
PWGLSETLAYERPALETTEENTRIES	qwglSetLayerPaletteEntries;
PWGLGETLAYERPALETTEENTRIES	qwglGetLayerPaletteEntries;
PWGLREALIZELAYERPALETTE		qwglRealizeLayerPalette;
PWGLSWAPLAYERBUFFERS		qwglSwapLayerBuffers;

#endif /* End stuff required for tools */

static bool hadError = false;
static char errorText[4096];

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error( const char *error, ... ) {
	va_list		argptr;
	char		text[4096];
	MSG        msg;

	if ( !Sys_IsMainThread() ) {
		// to avoid deadlocks we mustn't call Conbuf_AppendText() etc if not in main thread!
		va_start(argptr, error);
		vsprintf(errorText, error, argptr);
		va_end(argptr);

		printf("%s", errorText);
		OutputDebugString( errorText );

		hadError = true;
		return;
	}

	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr);

	if ( !hadError ) {
		// if we had an error in another thread, printf() and OutputDebugString() has already been called for this
		printf( "%s", text );
		OutputDebugString( text );
	}

	Conbuf_AppendText( text );
	Conbuf_AppendText( "\n" );

	Win_SetErrorText( text );
	Sys_ShowConsole( 1, true );

	timeEndPeriod( 1 );

	Sys_ShutdownInput();

	GLimp_Shutdown();

	// wait for the user to quit
	while ( 1 ) {
		if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
			common->Quit();
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	Sys_DestroyConsole();

	exit (1);
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit( void ) {
#ifdef ID_ALLOW_TOOLS
	// Free OpenGL DLL.
	if (hOpenGL_DLL)
	{
		FreeLibrary(hOpenGL_DLL);
	}
#endif

	timeEndPeriod( 1 );
	Sys_ShutdownInput();
	Sys_DestroyConsole();
	ExitProcess( 0 );
}


/*
==============
Sys_Printf
==============
*/

enum {
	MAXPRINTMSG = 4096,
	MAXNUMBUFFEREDLINES = 16
};

static char bufferedPrintfLines[MAXNUMBUFFEREDLINES][MAXPRINTMSG];
static int curNumBufferedPrintfLines = 0;
static CRITICAL_SECTION printfCritSect;

void Sys_Printf( const char *fmt, ... ) {
	char		msg[MAXPRINTMSG];

	va_list argptr;
	va_start(argptr, fmt);
	int len = idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	va_end(argptr);
	msg[sizeof(msg)-1] = '\0';

	printf("%s", msg);

	if ( win32.win_outputDebugString.GetBool() ) {
		OutputDebugString( msg );
	}
	if ( win32.win_outputEditString.GetBool() ) {
		if ( Sys_IsMainThread() ) {
			Conbuf_AppendText( msg );
		} else {
			EnterCriticalSection( &printfCritSect );
			int idx = curNumBufferedPrintfLines++;
			if ( idx < MAXNUMBUFFEREDLINES ) {
				if ( len >= MAXPRINTMSG )
					len = MAXPRINTMSG - 1;
				memcpy( bufferedPrintfLines[idx], msg, len + 1 );
			}
			LeaveCriticalSection( &printfCritSect );
		}
	}
}

/*
==============
Sys_DebugPrintf
==============
*/
#define MAXPRINTMSG 4096
void Sys_DebugPrintf( const char *fmt, ... ) {
	char msg[MAXPRINTMSG];

	va_list argptr;
	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	msg[ sizeof(msg)-1 ] = '\0';
	va_end( argptr );

	printf("%s", msg);

	OutputDebugString( msg );
}

/*
==============
Sys_DebugVPrintf
==============
*/
void Sys_DebugVPrintf( const char *fmt, va_list arg ) {
	char msg[MAXPRINTMSG];

	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, arg );
	msg[ sizeof(msg)-1 ] = '\0';

	printf("%s", msg);

	OutputDebugString( msg );
}

/*
==============
Sys_ShowWindow
==============
*/
void Sys_ShowWindow( bool show ) {
	::ShowWindow( win32.hWnd, show ? SW_SHOW : SW_HIDE );
}

/*
==============
Sys_IsWindowVisible
==============
*/
bool Sys_IsWindowVisible( void ) {
	return ( ::IsWindowVisible( win32.hWnd ) != 0 );
}

/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir( const char *path ) {
	_mkdir (path);
}

/*
=================
Sys_FileTimeStamp
=================
*/
ID_TIME_T Sys_FileTimeStamp( FILE *fp ) {
	struct _stat st;
	_fstat( _fileno( fp ), &st );
	return (long) st.st_mtime;
}

/*
==============
Sys_Cwd
==============
*/
const char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

static int WPath2A(char *dst, size_t size, const WCHAR *src) {
	int len;
	BOOL default_char = FALSE;

	// test if we can convert lossless
	len = WideCharToMultiByte(CP_ACP, 0, src, -1, dst, size, NULL, &default_char);

	if (default_char) {
		/* The following lines implement a horrible
		   hack to connect the UTF-16 WinAPI to the
		   ASCII doom3 strings. While this should work in
		   most cases, it'll fail if the "Windows to
		   DOS filename translation" is switched off.
		   In that case the function will return NULL
		   and no homedir is used. */
		WCHAR w[MAX_OSPATH];
		len = GetShortPathNameW(src, w, sizeof(w));

		if (len == 0)
			return 0;

		/* Since the DOS path contains no UTF-16 characters, convert it to the system's default code page */
		len = WideCharToMultiByte(CP_ACP, 0, w, len, dst, size - 1, NULL, NULL);
	}

	if (len == 0)
		return 0;

	dst[len] = 0;
	/* Replace backslashes by slashes */
	for (int i = 0; i < len; ++i)
		if (dst[i] == '\\')
			dst[i] = '/';

	// cut trailing slash
	if (dst[len - 1] == '/') {
		dst[len - 1] = 0;
		len--;
	}

	return len;
}

/*
==============
Returns "My Documents"/My Games/dhewm3 directory (or equivalent - "CSIDL_PERSONAL").
To be used with Sys_GetPath(PATH_SAVE), so savegames, screenshots etc will be
saved to the users files instead of systemwide.

Based on (with kind permission) Yamagi Quake II's Sys_GetHomeDir()

Returns the number of characters written to dst
==============
 */
extern "C" { // DG: I need this in SDL_win32_main.c
	int Win_GetHomeDir(char *dst, size_t size)
	{
		int len;
		WCHAR profile[MAX_OSPATH];

		/* Get the path to "My Documents" directory */
		SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, profile);

		len = WPath2A(dst, size, profile);
		if (len == 0)
			return 0;

		idStr::Append(dst, size, "/My Games/dhewm3");

		return len;
	}
}

static int GetRegistryPath(char *dst, size_t size, const WCHAR *subkey, const WCHAR *name) {
	WCHAR w[MAX_OSPATH];
	DWORD len = sizeof(w);
	HKEY res;
	DWORD sam = KEY_QUERY_VALUE
#ifdef _WIN64
		| KEY_WOW64_32KEY
#endif
		;
	DWORD type;

	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, sam, &res) != ERROR_SUCCESS)
		return 0;

	if (RegQueryValueExW(res, name, NULL, &type, (LPBYTE)w, &len) != ERROR_SUCCESS) {
		RegCloseKey(res);
		return 0;
	}

	RegCloseKey(res);

	if (type != REG_SZ)
		return 0;

	return WPath2A(dst, size, w);
}

bool Sys_GetPath(sysPath_t type, idStr &path) {
	char buf[MAX_OSPATH];
	struct _stat st;
	idStr s;

	switch(type) {
	case PATH_BASE:
		// try <path to exe>/base first
		if (Sys_GetPath(PATH_EXE, path)) {
			path.StripFilename();

			s = path;
			s.AppendPath(BASE_GAMEDIR);
			if (_stat(s.c_str(), &st) != -1 && (st.st_mode & _S_IFDIR)) {
				common->Warning("using path of executable: %s", path.c_str());
				return true;
			} else {
				s = path + "/demo/demo00.pk4";
				if (_stat(s.c_str(), &st) != -1 && (st.st_mode & _S_IFREG)) {
					common->Warning("using path of executable (seems to contain demo game data): %s ", path.c_str());
					return true;
				}
			}

			common->Warning("base path '%s' does not exist", s.c_str());
		}

		// Note: apparently there is no registry entry for the Doom 3 Demo

		// fallback to vanilla doom3 cd install
		if (GetRegistryPath(buf, sizeof(buf), L"SOFTWARE\\id\\Doom 3", L"InstallPath") > 0) {
			path = buf;
			return true;
		}

		// fallback to steam doom3 install
		if (GetRegistryPath(buf, sizeof(buf), L"SOFTWARE\\Valve\\Steam", L"InstallPath") > 0) {
			path = buf;
			path.AppendPath("steamapps\\common\\doom 3");

			if (_stat(path.c_str(), &st) != -1 && st.st_mode & _S_IFDIR)
				return true;
		}

		common->Warning("vanilla doom3 path not found either");

		return false;

	case PATH_CONFIG:
	case PATH_SAVE:
		if (Win_GetHomeDir(buf, sizeof(buf)) < 1) {
			Sys_Error("ERROR: Couldn't get dir to home path");
			return false;
		}

		path = buf;
		return true;

	case PATH_EXE:
		GetModuleFileName(NULL, buf, sizeof(buf) - 1);
		path = buf;
		path.BackSlashesToSlashes();
		return true;
	}

	return false;
}

/*
==============
Sys_ListFiles
==============
*/
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
	idStr		search;
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;

	if ( !extension) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	sprintf( search, "%s\\*%s", directory, extension );

	// search
	list.Clear();

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		return -1;
	}

	do {
		if ( flag ^ ( findinfo.attrib & _A_SUBDIR ) ) {
			list.Append( findinfo.name );
		}
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	_findclose( findhandle );

	return list.Num();
}


/*
================
Sys_GetClipboardData
================
*/
char *Sys_GetClipboardData( void ) {
	char *data = NULL;
	char *cliptext;

	if ( OpenClipboard( NULL ) != 0 ) {
		HANDLE hClipboardData;

		if ( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 ) {
			if ( ( cliptext = (char *)GlobalLock( hClipboardData ) ) != 0 ) {
				data = (char *)Mem_Alloc( GlobalSize( hClipboardData ) + 1 );
				strcpy( data, cliptext );
				GlobalUnlock( hClipboardData );

				strtok( data, "\n\r\b" );
			}
		}
		CloseClipboard();
	}
	return data;
}

void Sys_FreeClipboardData( char* data ) {
	Mem_Free( data );
}

/*
================
Sys_SetClipboardData
================
*/
void Sys_SetClipboardData( const char *string ) {
	HGLOBAL HMem;
	char *PMem;

	// allocate memory block
	HMem = (char *)::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, strlen( string ) + 1 );
	if ( HMem == NULL ) {
		return;
	}
	// lock allocated memory and obtain a pointer
	PMem = (char *)::GlobalLock( HMem );
	if ( PMem == NULL ) {
		return;
	}
	// copy text into allocated memory block
	lstrcpy( PMem, string );
	// unlock allocated memory
	::GlobalUnlock( HMem );
	// open Clipboard
	if ( !OpenClipboard( 0 ) ) {
		::GlobalFree( HMem );
		return;
	}
	// remove current Clipboard contents
	EmptyClipboard();
	// supply the memory handle to the Clipboard
	SetClipboardData( CF_TEXT, HMem );
	HMem = 0;
	// close Clipboard
	CloseClipboard();
}

/*
========================================================================

DLL Loading

========================================================================
*/

/*
=====================
Sys_DLL_Load
=====================
*/
uintptr_t Sys_DLL_Load( const char *dllName ) {
	HINSTANCE	libHandle;
	libHandle = LoadLibrary( dllName );
	if ( libHandle ) {
		// since we can't have LoadLibrary load only from the specified path, check it did the right thing
		char loadedPath[ MAX_OSPATH ];
		GetModuleFileName( libHandle, loadedPath, sizeof( loadedPath ) - 1 );
		if ( idStr::IcmpPath( dllName, loadedPath ) ) {
			Sys_Printf( "ERROR: LoadLibrary '%s' wants to load '%s'\n", dllName, loadedPath );
			Sys_DLL_Unload( (uintptr_t)libHandle );
			return 0;
		}
	} else {
		DWORD e = GetLastError();

		if ( e ==  0x7E ) {
			// 0x7E is "The specified module could not be found."
			// don't print a warning for that error, it's expected
			// when trying different possible paths for a DLL
			return 0;
		}

		if ( e == 0xC1) {
			// "[193 (0xC1)] is not a valid Win32 application"
			// probably going to be common. Lets try to be less cryptic.
			common->Warning( "LoadLibrary( \"%s\" ) Failed ! [%i (0x%X)]\tprobably the DLL is of the wrong architecture, "
			                 "like x64 instead of x86 (this build of dhewm3 expects %s)",
			                 dllName, e, e, D3_ARCH );
			return 0;
		}

		// for all other errors, print whatever FormatMessage() gives us
		LPVOID msgBuf = NULL;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			e,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&msgBuf,
			0, NULL);

		common->Warning( "LoadLibrary( \"%s\" ) Failed ! [%i (0x%X)]\t%s", dllName, e, e, msgBuf );

		::LocalFree( msgBuf );
	}
	return (uintptr_t)libHandle;
}

/*
=====================
Sys_DLL_GetProcAddress
=====================
*/
void *Sys_DLL_GetProcAddress( uintptr_t dllHandle, const char *procName ) {
	void * adr = (void*)GetProcAddress((HINSTANCE)dllHandle, procName);
	if (!adr)
	{
		DWORD e = GetLastError();
		LPVOID msgBuf = NULL;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			e,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&msgBuf,
			0, NULL);

		idStr errorStr = va("[%i (0x%X)]\t%s", e, e, msgBuf);

		if (errorStr.Length())
			common->Warning("GetProcAddress( %i %s) Failed ! %s", dllHandle, procName, errorStr.c_str());

		::LocalFree(msgBuf);
	}
	return adr;
}

/*
=====================
Sys_DLL_Unload
=====================
*/
void Sys_DLL_Unload( uintptr_t dllHandle ) {
	if ( !dllHandle ) {
		return;
	}
	if ( FreeLibrary( (HINSTANCE)dllHandle ) == 0 ) {
		int lastError = GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
		    NULL,
			lastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
		);
		Sys_Error( "Sys_DLL_Unload: FreeLibrary failed - %s (%d)", lpMsgBuf, lastError );
	}
}

/*
================
Sys_Init

The cvar system must already be setup
================
*/
void Sys_Init( void ) {

	CoInitialize( NULL );

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod( 1 );

	// get WM_TIMER messages pumped every millisecond
//	SetTimer( NULL, 0, 100, NULL );

#ifdef DEBUG
	cmdSystem->AddCommand( "createResourceIDs", CreateResourceIDs_f, CMD_FL_TOOL, "assigns resource IDs in _resouce.h files" );
#endif
#if 0
	cmdSystem->AddCommand( "setAsyncSound", Sys_SetAsyncSound_f, CMD_FL_SYSTEM, "set the async sound option" );
#endif
	{
		idStr savepath;
		Sys_GetPath( PATH_SAVE, savepath );
		common->Printf( "Logging console output to %s/dhewm3log.txt\n", savepath.c_str() );
	}

	//
	// Windows version
	//
	win32.osversion.dwOSVersionInfoSize = sizeof( win32.osversion );

	if ( !GetVersionEx( (LPOSVERSIONINFO)&win32.osversion ) )
		Sys_Error( "Couldn't get OS info" );

	if ( win32.osversion.dwMajorVersion < 4 ) {
		Sys_Error( GAME_NAME " requires Windows version 4 (NT) or greater" );
	}
	if ( win32.osversion.dwPlatformId == VER_PLATFORM_WIN32s ) {
		Sys_Error( GAME_NAME " doesn't run on Win32s" );
	}

	common->Printf( "%d MB System Memory\n", Sys_GetSystemRam() );
}

/*
================
Sys_Shutdown
================
*/
void Sys_Shutdown( void ) {
#ifdef ID_ALLOW_TOOLS
	qwglCopyContext = NULL;
	qwglCreateContext = NULL;
	qwglCreateLayerContext = NULL;
	qwglDeleteContext = NULL;
	qwglDescribeLayerPlane = NULL;
	qwglGetCurrentContext = NULL;
	qwglGetCurrentDC = NULL;
	qwglGetLayerPaletteEntries = NULL;
	qwglGetProcAddress = NULL;
	qwglMakeCurrent = NULL;
	qwglRealizeLayerPalette = NULL;
	qwglSetLayerPaletteEntries = NULL;
	qwglShareLists = NULL;
	qwglSwapLayerBuffers = NULL;
	qwglUseFontBitmaps = NULL;
	qwglUseFontOutlines = NULL;
	qwglSwapBuffers = NULL;
#endif // ID_ALLOW_TOOLS

	CoUninitialize();
}

//=======================================================================

//#define SET_THREAD_AFFINITY


/*
====================
Win_Frame
====================
*/
void Win_Frame( void ) {
	// if "viewlog" has been modified, show or hide the log console
	if ( win32.win_viewlog.IsModified() ) {
		if ( !com_skipRenderer.GetBool() && idAsyncNetwork::serverDedicated.GetInteger() != 1 ) {
			Sys_ShowConsole( win32.win_viewlog.GetInteger(), false );
		}
		win32.win_viewlog.ClearModified();
	}

	if ( curNumBufferedPrintfLines > 0 ) {
		// if Sys_Printf() had been called in another thread, add those lines to the windows console now
		EnterCriticalSection( &printfCritSect );
		int n = Min( curNumBufferedPrintfLines, (int)MAXNUMBUFFEREDLINES );
		for ( int i = 0; i < n; ++i ) {
			Conbuf_AppendText( bufferedPrintfLines[i] );
		}
		curNumBufferedPrintfLines = 0;
		LeaveCriticalSection( &printfCritSect );
	}

	if ( hadError ) {
		// if Sys_Error() had been called in another thread, handle it now
		Sys_Error( "%s", errorText );
	}
}

// the MFC tools use Win_GetWindowScalingFactor() for High-DPI support
#ifdef ID_ALLOW_TOOLS

typedef enum D3_MONITOR_DPI_TYPE {
	D3_MDT_EFFECTIVE_DPI = 0,
	D3_MDT_ANGULAR_DPI = 1,
	D3_MDT_RAW_DPI = 2,
	D3_MDT_DEFAULT = D3_MDT_EFFECTIVE_DPI
} D3_MONITOR_DPI_TYPE;

// https://docs.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getdpiformonitor
// GetDpiForMonitor() - Win8.1+, shellscalingapi.h, Shcore.dll
static HRESULT (STDAPICALLTYPE *D3_GetDpiForMonitor)(HMONITOR hmonitor, D3_MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY) = NULL;

// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow
// GetDpiForWindow() - Win10 1607+, winuser.h/Windows.h, User32.dll
static UINT(WINAPI *D3_GetDpiForWindow)(HWND hwnd) = NULL;

float Win_GetWindowScalingFactor(HWND window)
{
	// the best way - supported by Win10 1607 and newer
	if ( D3_GetDpiForWindow != NULL ) {
		UINT dpi = D3_GetDpiForWindow(window);
		return static_cast<float>(dpi) / 96.0f;
	}

	// probably second best, supported by Win8.1 and newer
	if ( D3_GetDpiForMonitor != NULL ) {
		HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);
		UINT dpiX = 96, dpiY;
		D3_GetDpiForMonitor(monitor, D3_MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
		return static_cast<float>(dpiX) / 96.0f;
	}

	// on older versions of windows, DPI was system-wide (not per monitor)
	// and changing DPI required logging out and in again (AFAIK), so we only need to get it once
	static float scaling_factor = -1.0f;
	if ( scaling_factor == -1.0f ) {
		HDC hdc = GetDC(window);
		if (hdc == NULL) {
			return 1.0f;
		}
		// "Number of pixels per logical inch along the screen width. In a system with multiple display monitors, this value is the same for all monitors."
		int ppi = GetDeviceCaps(hdc, LOGPIXELSX);
		scaling_factor = static_cast<float>(ppi) / 96.0f;
	}
	return scaling_factor;
}

#endif // ID_ALLOW_TOOLS

// code that tells windows we're High DPI aware so it doesn't scale our windows
// taken from Yamagi Quake II

typedef enum D3_PROCESS_DPI_AWARENESS {
	D3_PROCESS_DPI_UNAWARE = 0,
	D3_PROCESS_SYSTEM_DPI_AWARE = 1,
	D3_PROCESS_PER_MONITOR_DPI_AWARE = 2
} D3_PROCESS_DPI_AWARENESS;

static void setHighDPIMode(void)
{
	/* For Vista, Win7 and Win8 */
	BOOL(WINAPI *SetProcessDPIAware)(void) = NULL;

	/* Win8.1 and later */
	HRESULT(WINAPI *SetProcessDpiAwareness)(D3_PROCESS_DPI_AWARENESS dpiAwareness) = NULL;

	HINSTANCE userDLL = LoadLibrary("USER32.DLL");

	if (userDLL)
	{
		SetProcessDPIAware = (BOOL(WINAPI *)(void)) GetProcAddress(userDLL, "SetProcessDPIAware");
	}

	HINSTANCE shcoreDLL = LoadLibrary("SHCORE.DLL");

	if (shcoreDLL)
	{
		SetProcessDpiAwareness = (HRESULT(WINAPI *)(D3_PROCESS_DPI_AWARENESS))
									GetProcAddress(shcoreDLL, "SetProcessDpiAwareness");
	}

	if (SetProcessDpiAwareness) {
		SetProcessDpiAwareness(D3_PROCESS_PER_MONITOR_DPI_AWARE);
	}
	else if (SetProcessDPIAware) {
		SetProcessDPIAware();
	}

#ifdef ID_ALLOW_TOOLS // also init function pointers for Win_GetWindowScalingFactor() here
	if (userDLL) {
		D3_GetDpiForWindow = (UINT(WINAPI *)(HWND))GetProcAddress(userDLL, "GetDpiForWindow");
	}
	if (shcoreDLL) {
		D3_GetDpiForMonitor = (HRESULT (STDAPICALLTYPE *)(HMONITOR, D3_MONITOR_DPI_TYPE, UINT *, UINT *))
		                          GetProcAddress(shcoreDLL, "GetDpiForMonitor");
	}
#endif // ID_ALLOW_TOOLS
}

#ifdef ID_ALLOW_TOOLS
static void loadWGLpointers() {
	if (hOpenGL_DLL == NULL)
	{
		// Load OpenGL DLL.
		hOpenGL_DLL = LoadLibrary("opengl32.dll");
		if (hOpenGL_DLL == NULL) {
			Sys_Error(GAME_NAME " Cannot Load opengl32.dll - Disabling TOOLS");
			return;
		}
	}
	// opengl32.dll found... grab the addresses.

	qwglGetProcAddress = (PWGLGETPROCADDRESS)GetProcAddress(hOpenGL_DLL, "wglGetProcAddress");

	// Context controls
	qwglCopyContext = (PWGLCOPYCONTEXT)GetProcAddress(hOpenGL_DLL, "wglCopyContext");
	qwglCreateContext = (PWGLCREATECONTEXT)GetProcAddress(hOpenGL_DLL, "wglCreateContext");
	qwglCreateLayerContext = (PWGLCREATELAYERCONTEXT)GetProcAddress(hOpenGL_DLL, "wglCreateLayerContext");
	qwglDeleteContext = (PWGLDELETECONTEXT)GetProcAddress(hOpenGL_DLL, "wglDeleteContext");
	qwglGetCurrentContext = (PWGLGETCURRENTCONTEXT)GetProcAddress(hOpenGL_DLL, "wglGetCurrentContext");
	qwglGetCurrentDC = (PWGLGETCURRENTDC)GetProcAddress(hOpenGL_DLL, "wglGetCurrentDC");
	qwglMakeCurrent = (PWGLMAKECURRENT)GetProcAddress(hOpenGL_DLL, "wglMakeCurrent");
	qwglShareLists = (PWGLSHARELISTS)GetProcAddress(hOpenGL_DLL, "wglShareLists");

	// Fonts
	qwglUseFontBitmaps = (PWGLUSEFONTBITMAPS)GetProcAddress(hOpenGL_DLL, "wglUseFontBitmapsA");
	qwglUseFontOutlines = (PWGLUSEFONTOUTLINES)GetProcAddress(hOpenGL_DLL, "wglUseFontOutlinesA");

	// Layers.
	qwglDescribeLayerPlane = (PWGLDESCRIBELAYERPLANE)GetProcAddress(hOpenGL_DLL, "wglDescribeLayerPlane");
	qwglSwapLayerBuffers = (PWGLSWAPLAYERBUFFERS)GetProcAddress(hOpenGL_DLL, "wglSwapLayerBuffers");

	// Palette controls
	qwglGetLayerPaletteEntries = (PWGLGETLAYERPALETTEENTRIES)GetProcAddress(hOpenGL_DLL, "wglGetLayerPaletteEntries");
	qwglRealizeLayerPalette = (PWGLREALIZELAYERPALETTE)GetProcAddress(hOpenGL_DLL, "wglRealizeLayerPalette");
	qwglSetLayerPaletteEntries = (PWGLSETLAYERPALETTEENTRIES)GetProcAddress(hOpenGL_DLL, "wglSetLayerPaletteEntries");


	// These by default exist in windows
	qwglSwapBuffers = SwapBuffers;
}

// calls wglChoosePixelFormatARB() or ChoosePixelFormat() matching the main window from SDL
int Win_ChoosePixelFormat(HDC hdc)
{
	if (win32.wglChoosePixelFormatARB != NULL && win32.piAttribIList != NULL) {
		int formats[4];
		UINT numFormats = 0;
		if (win32.wglChoosePixelFormatARB(hdc, win32.piAttribIList, NULL, 4, formats, &numFormats) && numFormats > 0) {
			return formats[0];
		}
		static bool haveWarned = false;
		if(!haveWarned) {
			common->Warning("wglChoosePixelFormatARB() failed, falling back to ChoosePixelFormat()!\n");
			haveWarned = true;
		}
	}
	// fallback to normal ChoosePixelFormats() - doesn't support MSAA!
	return ChoosePixelFormat(hdc, &win32.pfd);
}
#endif


// ---------- Time Stuff -------------

// D3_CpuPause() abstracts a CPU pause instruction, to make busy waits a bit less power-hungry
// (code taken from Yamagi Quake II)
#ifdef SDL_CPUPauseInstruction
  #define D3_CpuPause() SDL_CPUPauseInstruction()
#elif defined(__GNUC__)
  #if (__i386 || __x86_64__)
    #define D3_CpuPause() asm volatile("pause")
  #elif defined(__aarch64__) || (defined(__ARM_ARCH) && __ARM_ARCH >= 7) || defined(__ARM_ARCH_6K__)
    #define D3_CpuPause() asm volatile("yield")
  #elif defined(__powerpc__) || defined(__powerpc64__)
    #define D3_CpuPause() asm volatile("or 27,27,27")
  #elif defined(__riscv) && __riscv_xlen == 64
    #define D3_CpuPause() asm volatile(".insn i 0x0F, 0, x0, x0, 0x010");
  #endif
#elif defined(_MSC_VER)
  #if defined(_M_IX86) || defined(_M_X64)
    #define D3_CpuPause() _mm_pause()
  #elif defined(_M_ARM) || defined(_M_ARM64)
    #define D3_CpuPause() __yield()
  #endif
#endif

#ifndef D3_CpuPause
  #warning "No D3_CpuPause implementation for this platform/architecture! Will busy-wait sometimes!"
  // TODO: something that prevents the loop from being optimized away?
  //#define D3_CpuPause()
#endif

static double perfCountToMS = 0.0; // set in initTime()
static LARGE_INTEGER firstCount = { 0 };

static size_t pauseLoopsPer5usec = 100; // set in initTime()

static void Win_InitTime() {
	LARGE_INTEGER freq = { 0 };
	QueryPerformanceFrequency(&freq); // in Hz
	perfCountToMS = 1000.0 / (double)freq.QuadPart; // 1/freq would be factor for seconds, we want milliseconds
	QueryPerformanceCounter(&firstCount);
	firstCount.QuadPart -= 1.5 / perfCountToMS; // make sure Sys_MillisecondsPrecise() always returns value >= 1

	double before = Sys_MillisecondsPrecise();
	for ( int i=0; i < 1000; ++i ) {
		// volatile so the call isn't optimized away
		volatile double x = Sys_MillisecondsPrecise();
		(void)x;
	}
	double after = Sys_MillisecondsPrecise();
	double callDiff = after - before;

#ifdef D3_CpuPause
	// figure out how long D3_CpuPause() instructions take
	before = Sys_MillisecondsPrecise();
	for( int i=0; i < 1000000; ++i ) {
		// call it 4 times per loop, so the ratio between pause and loop-instructions is better
		D3_CpuPause(); D3_CpuPause(); D3_CpuPause(); D3_CpuPause();
	}
	after = Sys_MillisecondsPrecise();
	double diff = after - before;
	double onePauseIterTime = diff / 1000000;
	if ( onePauseIterTime > 0.00000001 ) {
		double loopsPer10usec = 0.005 / onePauseIterTime;
		pauseLoopsPer5usec = loopsPer10usec;
		printf( "Win_InitTime(): A call to Sys_MillisecondsPrecise() takes about %g nsec; 1mio pause loops took %g ms => pauseLoopsPer5usec = %zd\n",
		        callDiff*1000.0, diff, pauseLoopsPer5usec );
		if ( pauseLoopsPer5usec == 0 )
			pauseLoopsPer5usec = 1;
	} else {
		assert( 0 && "apparently 1mio pause loops are so fast we can't even measure it?!" );
		pauseLoopsPer5usec = 1000000;
	}
	// Note: Due to CPU frequency scaling this is not super precise, but it should be within
	//   an order of magnitude of the real current value, I think, which should suffice for our purposes
#else
	printf( "Win_InitTime(): A call to Sys_MillisecondsPrecise() takes about %g nsecs\n", callDiff*1000.0 );
#endif
}

/*
=======================
Sys_MillisecondsPrecise
=======================
*/
double Sys_MillisecondsPrecise() {
	LARGE_INTEGER cur;
	QueryPerformanceCounter(&cur);

	double ret = cur.QuadPart - firstCount.QuadPart;
	ret *= perfCountToMS;
	return ret;
}

/*
=====================
Sys_SleepUntilPrecise
=====================
*/
void Sys_SleepUntilPrecise( double targetTimeMS ) {
	double msec = targetTimeMS - Sys_MillisecondsPrecise();
	if ( msec < 0.01 ) // don't bother for less than 10usec
		return;

	if ( msec > 2.0 ) {
		// Note: Theoretically one could use SetWaitableTimer() and WaitForSingleObject()
		//   for higher precision, but last time I tested (on Win10),
		//   in practice that also only had millisecond-precision
		dword sleepMS = msec - 1.0; // wait for last MS or so in busy(-ish) loop below
		Sleep( sleepMS );
	}

	// wait for the remaining time with a busy loop, as that has higher precision
	do {
#ifdef D3_CpuPause
		for ( size_t i=0; i < pauseLoopsPer5usec; ++i ) {
			// call it 4 times per loop, so the ratio between pause and loop-instructions is better
			D3_CpuPause(); D3_CpuPause(); D3_CpuPause(); D3_CpuPause();
		}
#endif

		msec = targetTimeMS - Sys_MillisecondsPrecise();
	} while ( msec >= 0.01 );
}


// stdout/stderr redirection, originally from SDL_win32_main.c

/* The standard output files */
#define STDOUT_FILE	TEXT("dhewm3log.txt") /* DG: renamed this */
#define STDERR_FILE	TEXT("stderr.txt")

/* Set a variable to tell if the stdio redirect has been enabled. */
static int stdioRedirectEnabled = 0;
static char stdoutPath[MAX_PATH];
static char stderrPath[MAX_PATH];
#define DIR_SEPERATOR TEXT("/")


/* Remove the output files if there was no output written */
static void cleanup_output(void) {
	FILE *file;
	int empty;

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

	/* Without redirection we're done */
	if (!stdioRedirectEnabled) {
		return;
	}

	/* See if the files have any output in them */
	if ( stdoutPath[0] ) {
		file = fopen(stdoutPath, TEXT("rb"));
		if ( file ) {
			empty = (fgetc(file) == EOF) ? 1 : 0;
			fclose(file);
			if ( empty ) {
				remove(stdoutPath);
			}
		}
	}
	if ( stderrPath[0] ) {
		file = fopen(stderrPath, TEXT("rb"));
		if ( file ) {
			empty = (fgetc(file) == EOF) ? 1 : 0;
			fclose(file);
			if ( empty ) {
				remove(stderrPath);
			}
		}
	}
}

/* Redirect the output (stdout and stderr) to a file */
static void redirect_output(void)
{
	char path[MAX_PATH];
	struct _stat st;

	/* DG: use "My Documents/My Games/dhewm3" to write stdout.txt and stderr.txt
	*     instead of the binary, which might not be writable */
	Win_GetHomeDir(path, sizeof(path));

	if (_stat(path, &st) == -1) {
		/* oops, "My Documents/My Games/dhewm3" doesn't exist - does My Games/ at least exist? */
		char myGamesPath[MAX_PATH];
		char* lastslash;
		memcpy(myGamesPath, path, MAX_PATH);
		lastslash = strrchr(myGamesPath, '/');
		if (lastslash != NULL) {
			*lastslash = '\0';
		}
		if (_stat(myGamesPath, &st) == -1) {
			/* if My Documents/My Games/ doesn't exist, create it */
			if( _mkdir(myGamesPath) != 0 && errno != EEXIST ) {
				char msg[2048];
				D3_snprintfC99( msg, sizeof(msg), "Failed to create '%s',\n error number is %d (%s).\nPermission problem?",
				                myGamesPath, errno, strerror(errno) );
				MessageBox( NULL, msg, "Can't create 'My Games' directory!", MB_OK | MB_ICONERROR );
				exit(1);
			}
		}
		/* create My Documents/My Games/dhewm3/ */
		if( _mkdir(path) != 0 && errno != EEXIST ) {
			char msg[2048];
			D3_snprintfC99( msg, sizeof(msg), "Failed to create '%s'\n(for savegames, configs and logs),\n error number is %d (%s)\nIs Documents/My Games/ write protected?",
			                path, errno, strerror(errno) );
			MessageBox( NULL, msg, "Can't create 'My Games/dhewm3' directory!", MB_OK | MB_ICONERROR );
			exit(1);
		}
	}

	FILE *newfp;

#if 0 /* DG: don't do this anymore. */
	DWORD pathlen;
	pathlen = GetModuleFileName(NULL, path, SDL_arraysize(path));
	while ( pathlen > 0 && path[pathlen] != '\\' ) {
		--pathlen;
	}
	path[pathlen] = '\0';
#endif

	SDL_strlcpy( stdoutPath, path, SDL_arraysize(stdoutPath) );
	SDL_strlcat( stdoutPath, DIR_SEPERATOR STDOUT_FILE, SDL_arraysize(stdoutPath) );

	{ /* DG: rename old stdout log */
		char stdoutPathBK[MAX_PATH];
		SDL_strlcpy( stdoutPathBK, path, SDL_arraysize(stdoutPath) );
		SDL_strlcat( stdoutPathBK, DIR_SEPERATOR TEXT("dhewm3log-old.txt"), SDL_arraysize(stdoutPath) );
		rename( stdoutPath, stdoutPathBK );
	} /* DG end */

	  /* Redirect standard input and standard output */
	newfp = freopen(stdoutPath, TEXT("w"), stdout);

	if ( newfp == NULL ) {	/* This happens on NT */
#if !defined(stdout)
		stdout = fopen(stdoutPath, TEXT("w"));
#else
		newfp = fopen(stdoutPath, TEXT("w"));
		if ( newfp ) {
			*stdout = *newfp;
		} else {
			char msg[2048];
			D3_snprintfC99( msg, sizeof(msg), "Failed to create '%s',\n error number is %d (%s)\nIs Documents/My Games/dhewm3/\n or dhewm3log.txt write protected?",
			                stdoutPath, errno, strerror(errno) );
			MessageBox( NULL, msg, "Can't create dhewm3log.txt!", MB_OK | MB_ICONERROR );
			exit(1);
		}
#endif
	}

	SDL_strlcpy( stderrPath, path, SDL_arraysize(stderrPath) );
	SDL_strlcat( stderrPath, DIR_SEPERATOR STDERR_FILE, SDL_arraysize(stderrPath) );

	newfp = freopen(stderrPath, TEXT("w"), stderr);
	if ( newfp == NULL ) {	/* This happens on NT */
#if !defined(stderr)
		stderr = fopen(stderrPath, TEXT("w"));
#else
		newfp = fopen(stderrPath, TEXT("w"));
		if ( newfp ) {
			*stderr = *newfp;
		} else {
			char msg[2048];
			D3_snprintfC99( msg, sizeof(msg), "Failed to create '%s',\n error number is %d (%s)\nIs Documents/My Games/dhewm3/ write protected?",
			                stderrPath, errno, strerror(errno) );
			MessageBox( NULL, msg, "Can't create stderr.txt!", MB_OK | MB_ICONERROR );
			exit(1);
		}
#endif
	}

	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);	/* Line buffered */
	setbuf(stderr, NULL);			/* No buffering */
	stdioRedirectEnabled = 1;
}

// end of stdout/stderr redirection code from old SDL

/*
==================
The pseudo-main function called from real main (either in SDL_win32_main.c or WinMain() below)
NOTE: Currently argv[] are ANSI strings, not UTF-8 strings as usual in SDL2 and SDL3!
==================
*/
int SDL_main(int argc, char *argv[]) {
	// as the very first thing, redirect stdout to dhewm3log.txt (and stderr to stderr.txt)
	// so we can log
	redirect_output();
	atexit(cleanup_output);

	// now that stdout is redirected to dhewm3log.txt,
	// log its (approx.) creation time before anything else is logged:
	{
		time_t tt = time(NULL);
		const struct tm* tms = localtime(&tt);
		char timeStr[64] = {};
		strftime(timeStr, sizeof(timeStr), "%F %H:%M:%S", tms);
		printf("Opened this log at %s\n", timeStr);
	}

	InitializeCriticalSection( &printfCritSect );

	Win_InitTime();

#ifdef ID_DEDICATED
	MSG msg;
#else
	// tell windows we're high dpi aware, otherwise display scaling screws up the game
	setHighDPIMode();
#endif

	Sys_SetPhysicalWorkMemory( 192 << 20, 1024 << 20 );

	win32.hInstance = GetModuleHandle(NULL);

	// done before Com/Sys_Init since we need this for error output
	Sys_CreateConsole();

	// no abort/retry/fail errors
	SetErrorMode( SEM_FAILCRITICALERRORS );

#ifdef DEBUG
	// disable the painfully slow MS heap check every 1024 allocs
	_CrtSetDbgFlag( 0 );
#endif

#ifdef ID_ALLOW_TOOLS
	loadWGLpointers();
#endif

	if ( argc > 1 ) {
		common->Init( argc-1, &argv[1] );
	} else {
		common->Init( 0, NULL );
	}

	// hide or show the early console as necessary
	if ( win32.win_viewlog.GetInteger() || com_skipRenderer.GetBool() || idAsyncNetwork::serverDedicated.GetInteger() ) {
		Sys_ShowConsole( 1, true );
	} else {
		Sys_ShowConsole( 0, false );
	}

#ifdef SET_THREAD_AFFINITY
	// give the main thread an affinity for the first cpu
	SetThreadAffinityMask( GetCurrentThread(), 1 );
#endif

	// Launch the script debugger
	if ( strstr( GetCommandLine(), "+debugger" ) ) {

#ifdef ID_ALLOW_TOOLS
		DebuggerClientInit(GetCommandLine());
#endif
		return 0;
	}

	// ::SetFocus( win32.hWnd ); // DG: let SDL handle focus, otherwise input is fucked up! (#100)

	// main game loop
	while( 1 ) {
#if ID_DEDICATED
		// Since this is a Dedicated Server, process all Windowing Messages
		// Now.
		while(PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Give the OS a little time to recuperate.
		Sleep(10);
#endif

		Win_Frame();

#ifdef ID_ALLOW_TOOLS
		if ( com_editors ) {
			if ( com_editors & EDITOR_GUI ) {
				// GUI editor
				GUIEditorRun();
			} else if ( com_editors & EDITOR_RADIANT ) {
				// Level Editor
				RadiantRun();
			}
			else {
#ifdef IMGUI_DISABLE // DG: unless ImGui is disabled, the ImGui-based versions are used instead
				if ( com_editors & EDITOR_MATERIAL ) {
					//BSM Nerve: Add support for the material editor
					MaterialEditorRun();
				}
				if ( com_editors & EDITOR_LIGHT ) {
					// in-game Light Editor
					LightEditorRun();
				}
				if ( com_editors & EDITOR_AF ) {
					// in-game Articulated Figure Editor
					AFEditorRun();
				}
				if ( com_editors & EDITOR_PDA ) {
					// in-game PDA Editor
					PDAEditorRun();
				}
#endif
				if ( com_editors & EDITOR_SOUND ) {
					// in-game Sound Editor
					SoundEditorRun();
				}
				if ( com_editors & EDITOR_DECL ) {
					// in-game Declaration Browser
					DeclBrowserRun();
				}

				if ( com_editors & EDITOR_PARTICLE ) {
					// in-game Particle Editor
					ParticleEditorRun();
				}
				if ( com_editors & EDITOR_SCRIPT ) {
					// in-game Script Editor
					ScriptEditorRun();
				}
			}
		}
#endif
		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}

/*
==================
idSysLocal::OpenURL
==================
*/
void idSysLocal::OpenURL( const char *url, bool doexit ) {
	static bool doexit_spamguard = false;
	HWND wnd;

	if (doexit_spamguard) {
		common->DPrintf( "OpenURL: already in an exit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf("Open URL: %s\n", url);

	if ( !ShellExecute( NULL, "open", url, NULL, NULL, SW_RESTORE ) ) {
		common->Error( "Could not open url: '%s' ", url );
		return;
	}

	wnd = GetForegroundWindow();
	if ( wnd ) {
		ShowWindow( wnd, SW_MAXIMIZE );
	}

	if ( doexit ) {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

/*
==================
idSysLocal::StartProcess
==================
*/
void idSysLocal::StartProcess( const char *exePath, bool doexit ) {
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strncpy( szPathOrig, exePath, _MAX_PATH );
	szPathOrig[_MAX_PATH-1] = 0;

	if( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		common->Error( "Could not start process: '%s' ", szPathOrig );
	    return;
	}

	if ( doexit ) {
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

// the actual WinMain(), based on SDL2_main and SDL3's SDL_main_impl.h + SDL_RunApp()
// but modified to pass ANSI strings to SDL_main() instead of UTF-8,
// because dhewm3 doesn't use Unicode internally (except for Dear ImGui,
// which doesn't use commandline arguments)
// for SDL1.2, SDL_win32_main.c is still used instead
#if SDL_VERSION_ATLEAST(2, 0, 0)

/* Pop up an out of memory message, returns to Windows */
static BOOL OutOfMemory(void)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", "Out of memory - aborting", NULL);
	return -1;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
	(void)hInst;
	(void)hPrev;
	(void)szCmdLine;
	(void)sw;

	LPWSTR *argvw;
	char **argv;
	int i, argc, result;

	argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (!argvw) {
		return OutOfMemory();
	}

	/* Note that we need to be careful about how we allocate/free memory here.
	* If the application calls SDL_SetMemoryFunctions(), we can't rely on
	* SDL_free() to use the same allocator after SDL_main() returns.
	*/

	/* Parse it into argv and argc */
	argv = (char **)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (argc + 1) * sizeof(*argv));
	if (!argv) {
		return OutOfMemory();
	}
	for (i = 0; i < argc; ++i) {
		// NOTE: SDL2+ uses CP_UTF8 instead of CP_ACP here (and in the other call below)
		//       but Doom3 needs ANSI strings on Windows (so paths work with the Windows ANSI APIs)
		const int ansiSize = WideCharToMultiByte(CP_ACP, 0, argvw[i], -1, NULL, 0, NULL, NULL);
		if (!ansiSize) {  // uhoh?
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", "Error processing command line arguments", NULL);
			return -1;
		}

		argv[i] = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ansiSize);  // this size includes the null-terminator character.
		if (!argv[i]) {
			return OutOfMemory();
		}

		if (WideCharToMultiByte(CP_ACP, 0, argvw[i], -1, argv[i], ansiSize, NULL, NULL) == 0) {  // failed? uhoh!
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", "Error processing command line arguments", NULL);
			return -1;
		}
	}
	argv[i] = NULL;
	LocalFree(argvw);

	SDL_SetMainReady();

	// Run the application main() code
	result = SDL_main(argc, argv);

	// Free argv, to avoid memory leak
	for (i = 0; i < argc; ++i) {
		HeapFree(GetProcessHeap(), 0, argv[i]);
	}
	HeapFree(GetProcessHeap(), 0, argv);

	return result;
}
#endif
