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

#ifndef __WIN_LOCAL_H__
#define __WIN_LOCAL_H__

#if defined(_MFC_VER) && !defined(_D3SDK) && !defined(GAME_DLL)
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// prevent auto literal to string conversion
#include "tools/comafx/StdAfx.h"
#endif

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <objbase.h>

#include "framework/CVarSystem.h"
#include "sys/sys_public.h"

#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE | WS_THICKFRAME)

void	Sys_CreateConsole( void );
void	Sys_DestroyConsole( void );

void	Win_SetErrorText( const char *text );

const unsigned char *Win_GetScanTable( void );
int		Win_MapKey (int key);

void Conbuf_AppendText( const char *msg );

struct Win32Vars_t {
	HWND			hWnd;
	HINSTANCE		hInstance;

	OSVERSIONINFOEX	osversion;

#ifdef ID_ALLOW_TOOLS
	HDC				hDC; // handle to device
	HGLRC			hGLRC; // handle to GL rendering context
	PIXELFORMATDESCRIPTOR pfd;
#endif

	static idCVar	win_outputDebugString;
	static idCVar	win_outputEditString;
	static idCVar	win_viewlog;
};

extern Win32Vars_t	win32;

#endif /* !__WIN_LOCAL_H__ */
