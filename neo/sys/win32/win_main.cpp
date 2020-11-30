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

#include <SDL_main.h>

idCVar Win32Vars_t::win_outputDebugString( "win_outputDebugString", "0", CVAR_SYSTEM | CVAR_BOOL, "" );
idCVar Win32Vars_t::win_outputEditString( "win_outputEditString", "1", CVAR_SYSTEM | CVAR_BOOL, "" );
idCVar Win32Vars_t::win_viewlog( "win_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "" );

Win32Vars_t	win32;

#ifdef ID_ALLOW_TOOLS
/* These are required for tools (DG: taken from SteelStorm2) */

static HMODULE hOpenGL_DLL;

typedef int  (WINAPI * PWGLCHOOSEPIXELFORMAT) (HDC, CONST PIXELFORMATDESCRIPTOR *);
typedef int   (WINAPI * PWGLDESCRIBEPIXELFORMAT) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
typedef int   (WINAPI * PWGLGETPIXELFORMAT)(HDC);
typedef BOOL(WINAPI * PWGLSETPIXELFORMAT)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
typedef BOOL(WINAPI * PWGLSWAPBUFFERS)(HDC);

PWGLCHOOSEPIXELFORMAT	qwglChoosePixelFormat;
PWGLDESCRIBEPIXELFORMAT	qwglDescribePixelFormat;
PWGLGETPIXELFORMAT		qwglGetPixelFormat;
PWGLSETPIXELFORMAT		qwglSetPixelFormat;
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

	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr);

	printf("%s", text);

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
#define MAXPRINTMSG 4096
void Sys_Printf( const char *fmt, ... ) {
	char		msg[MAXPRINTMSG];

	va_list argptr;
	va_start(argptr, fmt);
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	va_end(argptr);
	msg[sizeof(msg)-1] = '\0';

	printf("%s", msg);

	if ( win32.win_outputDebugString.GetBool() ) {
		OutputDebugString( msg );
	}
	if ( win32.win_outputEditString.GetBool() ) {
		Conbuf_AppendText( msg );
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
To be used with Sys_DefaultSavePath(), so savegames, screenshots etc will be
saved to the users files instead of systemwide.

Based on (with kind permission) Yamagi Quake II's Sys_GetHomeDir()

Returns the number of characters written to dst
==============
 */
extern "C" { // DG: I need this in SDL_win32_main.c
	int Sys_GetHomeDir(char *dst, size_t size)
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
		if (Sys_GetHomeDir(buf, sizeof(buf)) < 1) {
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
		// TODO
	}
	return (uintptr_t)libHandle;
}

/*
=====================
Sys_DLL_GetProcAddress
=====================
*/
void *Sys_DLL_GetProcAddress( uintptr_t dllHandle, const char *procName ) {
	return (void *)GetProcAddress( (HINSTANCE)dllHandle, procName );
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
	qwglChoosePixelFormat = NULL;
	qwglDescribePixelFormat = NULL;
	qwglGetPixelFormat = NULL;
	qwglSetPixelFormat = NULL;
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
}

// code that tells windows we're High DPI aware so it doesn't scale our windows
// taken from Yamagi Quake II

typedef enum D3_PROCESS_DPI_AWARENESS {
	D3_PROCESS_DPI_UNAWARE = 0,
	D3_PROCESS_SYSTEM_DPI_AWARE = 1,
	D3_PROCESS_PER_MONITOR_DPI_AWARE = 2
} YQ2_PROCESS_DPI_AWARENESS;

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
		SetProcessDpiAwareness = (HRESULT(WINAPI *)(YQ2_PROCESS_DPI_AWARENESS))
									GetProcAddress(shcoreDLL, "SetProcessDpiAwareness");
	}


	if (SetProcessDpiAwareness) {
		SetProcessDpiAwareness(D3_PROCESS_PER_MONITOR_DPI_AWARE);
	}
	else if (SetProcessDPIAware) {
		SetProcessDPIAware();
	}
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
	qwglChoosePixelFormat = ChoosePixelFormat;
	qwglDescribePixelFormat = DescribePixelFormat;
	qwglGetPixelFormat = GetPixelFormat;
	qwglSetPixelFormat = SetPixelFormat;
	qwglSwapBuffers = SwapBuffers;
}
#endif

/*
==================
WinMain
==================
*/
int main(int argc, char *argv[]) {
	const HCURSOR hcurSave = ::SetCursor( LoadCursor( 0, IDC_WAIT ) );

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

	// ::SetCursor( hcurSave ); // DG: I think SDL handles the cursor fine..

	// Launch the script debugger
	if ( strstr( GetCommandLine(), "+debugger" ) ) {
		// DebuggerClientInit( lpCmdLine );
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
			else if (com_editors & EDITOR_MATERIAL ) {
				//BSM Nerve: Add support for the material editor
				MaterialEditorRun();
			}
			else {
				if ( com_editors & EDITOR_LIGHT ) {
					// in-game Light Editor
					LightEditorRun();
				}
				if ( com_editors & EDITOR_SOUND ) {
					// in-game Sound Editor
					SoundEditorRun();
				}
				if ( com_editors & EDITOR_DECL ) {
					// in-game Declaration Browser
					DeclBrowserRun();
				}
				if ( com_editors & EDITOR_AF ) {
					// in-game Articulated Figure Editor
					AFEditorRun();
				}
				if ( com_editors & EDITOR_PARTICLE ) {
					// in-game Particle Editor
					ParticleEditorRun();
				}
				if ( com_editors & EDITOR_SCRIPT ) {
					// in-game Script Editor
					ScriptEditorRun();
				}
				if ( com_editors & EDITOR_PDA ) {
					// in-game PDA Editor
					PDAEditorRun();
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

	if( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		common->Error( "Could not start process: '%s' ", szPathOrig );
	    return;
	}

	if ( doexit ) {
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}
