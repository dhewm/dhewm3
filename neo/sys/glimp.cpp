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

#include <SDL.h>

#include "sys/platform.h"
#include "framework/Licensee.h"

#include "renderer/tr_local.h"

#if defined(_WIN32) && defined(ID_ALLOW_TOOLS)
#include "sys/win32/win_local.h"
#include <SDL_syswm.h>

// from SDL_windowsopengl.h (internal SDL2 header)
#ifndef WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WGL_ARB_multisample
#define WGL_SAMPLE_BUFFERS_ARB         0x2041
#define WGL_SAMPLES_ARB                0x2042
#endif

#endif // _WIN32 and ID_ALLOW_TOOLS


#if SDL_VERSION_ATLEAST(2, 0, 0)
static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;
#else
static SDL_Surface *window = NULL;
#define SDL_WINDOW_OPENGL SDL_OPENGL
#define SDL_WINDOW_FULLSCREEN SDL_FULLSCREEN
#endif

static void SetSDLIcon()
{
	Uint32 rmask, gmask, bmask, amask;

	// ok, the following is pretty stupid.. SDL_CreateRGBSurfaceFrom() pretends to use a void* for the data,
	// but it's really treated as endian-specific Uint32* ...
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	#include "doom_icon.h" // contains the struct d3_icon

	SDL_Surface* icon = SDL_CreateRGBSurfaceFrom((void*)d3_icon.pixel_data, d3_icon.width, d3_icon.height,
			d3_icon.bytes_per_pixel*8, d3_icon.bytes_per_pixel*d3_icon.width,
			rmask, gmask, bmask, amask);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetWindowIcon(window, icon);
#else
	SDL_WM_SetIcon(icon, NULL);
#endif

	SDL_FreeSurface(icon);
}

/*
===================
GLimp_Init
===================
*/
bool GLimp_Init(glimpParms_t parms) {
	common->Printf("Initializing OpenGL subsystem\n");

	assert(SDL_WasInit(SDL_INIT_VIDEO));

	Uint32 flags = SDL_WINDOW_OPENGL;

	if (parms.fullScreen == 1)
	{
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if(r_fullscreenDesktop.GetBool())
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		else
#endif
			flags |= SDL_WINDOW_FULLSCREEN;
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	/* Doom3 has the nasty habit of modifying the default framebuffer's alpha channel and then
	 * relying on those modifications in blending operations (using GL_DST_(ONE_MINUS_)ALPHA).
	 * So far that hasn't been much of a problem, because Windows, macOS, X11 etc
	 * just ignore the alpha chan (unless maybe you explicitly tell a window it should be transparent).
	 * Unfortunately, Wayland by default *does* use the alpha channel, which often leads to
	 * rendering bugs (the window is partly transparent or very white in areas with low alpha).
	 * Mesa introduced an EGL extension that's supposed to fix that (EGL_EXT_present_opaque)
	 * and newer SDL2 versions use it by default (in the Wayland backend).
	 * Unfortunately, the implementation of that extension is (currently?) broken (at least
	 * in Mesa), seems like they just give you a visual without any alpha chan - which doesn't
	 * work for Doom3, as it needs a functioning alpha chan for blending operations, see above.
	 * See also: https://gitlab.freedesktop.org/mesa/mesa/-/issues/5886
	 *
	 * So to make sure dhewm3 (finally) works as expected on Wayland, we tell SDL2 to
	 * allow transparency and then fill the alpha-chan ourselves in RB_SwapBuffers()
	 * (unless the user disables that with r_fillWindowAlphaChan 0) */
  #ifdef SDL_HINT_VIDEO_EGL_ALLOW_TRANSPARENCY
	SDL_SetHint(SDL_HINT_VIDEO_EGL_ALLOW_TRANSPARENCY, "1");
  #else // little hack so this works if the SDL2 version used for building is older than runtime version
	SDL_SetHint("SDL_VIDEO_EGL_ALLOW_TRANSPARENCY", "1");
  #endif
#endif

	int colorbits = 24;
	int depthbits = 24;
	int stencilbits = 8;

	for (int i = 0; i < 16; i++) {

		int multisamples = parms.multiSamples;

		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if ((i % 4) == 0 && i) {
			// one pass, reduce
			switch (i / 4) {
			case 2 :
				if (colorbits == 24)
					colorbits = 16;
				break;
			case 1 :
				if (depthbits == 24)
					depthbits = 16;
				else if (depthbits == 16)
					depthbits = 8;
			case 3 :
				if (stencilbits == 24)
					stencilbits = 16;
				else if (stencilbits == 16)
					stencilbits = 8;
			}
		}

		int tcolorbits = colorbits;
		int tdepthbits = depthbits;
		int tstencilbits = stencilbits;

		if ((i % 4) == 3) {
			// reduce colorbits
			if (tcolorbits == 24)
				tcolorbits = 16;
		}

		if ((i % 4) == 2) {
			// reduce depthbits
			if (tdepthbits == 24)
				tdepthbits = 16;
			else if (tdepthbits == 16)
				tdepthbits = 8;
		}

		if ((i % 4) == 1) {
			// reduce stencilbits
			if (tstencilbits == 24)
				tstencilbits = 16;
			else if (tstencilbits == 16)
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}

		int channelcolorbits = 4;
		if (tcolorbits == 24)
			channelcolorbits = 8;

		int talphabits = channelcolorbits;

try_again:

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, channelcolorbits);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, channelcolorbits);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, channelcolorbits);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, tdepthbits);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, tstencilbits);

		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, talphabits);

		SDL_GL_SetAttribute(SDL_GL_STEREO, parms.stereo ? 1 : 0);

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, (multisamples > 1) ? 1 : 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisamples);

#if SDL_VERSION_ATLEAST(2, 0, 0)

		const char* windowMode = "";
		if(r_fullscreen.GetBool()) {
			windowMode = r_fullscreenDesktop.GetBool() ? "desktop-fullscreen-" : "fullscreen-";
		}

		common->Printf("Will create a %swindow with resolution %dx%d (r_mode = %d)\n",
		               windowMode, parms.width, parms.height, r_mode.GetInteger());

		int displayIndex = 0;
#if SDL_VERSION_ATLEAST(2, 0, 4)
		// try to put the window on the display the mousecursor currently is on
		{
			int x, y;
			SDL_GetGlobalMouseState(&x, &y);

			int numDisplays = SDL_GetNumVideoDisplays();
			common->Printf("SDL detected %d displays: \n", numDisplays);
			bool found = false;
			for ( int j=0; j<numDisplays; ++j ) {
				SDL_Rect rect;
				if (SDL_GetDisplayBounds(j, &rect) == 0) {
					common->Printf(" %d: %dx%d at (%d, %d) to (%d, %d)\n", j, rect.w, rect.h,
					               rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
					if ( !found && x >= rect.x && x < rect.x + rect.w
						&& y >= rect.y && y < rect.y + rect.h )
					{
						displayIndex = j;
						found = true;
					}
				}
			}
			common->Printf("Will use display %d because mouse cursor is at (%d, %d).\n",
			               displayIndex, x, y);
		}
#endif

		window = SDL_CreateWindow(ENGINE_VERSION,
									SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayIndex),
									SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayIndex),
									parms.width, parms.height, flags);

		if (!window) {
			common->Warning("Couldn't set GL mode %d/%d/%d with %dx MSAA: %s",
							channelcolorbits, tdepthbits, tstencilbits, parms.multiSamples, SDL_GetError());

			// before trying to reduce color channel size or whatever, first try reducing MSAA, if possible
			if(multisamples > 1) {
				multisamples = (multisamples <= 2) ? 0 : (multisamples/2);

				// using goto because enhancing that logic which reduces attributes
				// based on i (so it'd first try reducing MSAA) would be too painful
				goto try_again;
			}

			continue;
		} else {
			// creating the window succeeded, so adjust r_multiSamples to the value that was actually used
			parms.multiSamples = multisamples;
			r_multiSamples.SetInteger(multisamples);
		}

		/* Check if we're really in the requested display mode. There is
		   (or was) an SDL bug were SDL switched into the wrong mode
		   without giving an error code. See the bug report for details:
		   https://bugzilla.libsdl.org/show_bug.cgi?id=4700 */
		if ((flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) == SDL_WINDOW_FULLSCREEN)
		{
			SDL_DisplayMode real_mode;
			if (SDL_GetWindowDisplayMode(window, &real_mode) != 0)
			{
				SDL_DestroyWindow(window);
				window = NULL;
				common->Warning("Can't get display mode: %s\n", SDL_GetError());
				return false; // trying other color depth etc is unlikely to help with this issue
			}
			if ((real_mode.w != parms.width) || (real_mode.h != parms.height))
			{
				common->Warning("Current display mode isn't requested display mode\n");
				common->Warning("Likely SDL bug #4700, trying to work around it..\n");
				int dIdx = SDL_GetWindowDisplayIndex(window);
				if(dIdx != displayIndex) {
					common->Warning("Window's display index is %d, but we wanted %d!\n", dIdx, displayIndex);
				}

				/* Mkay, try to hack around that. */
				SDL_DisplayMode wanted_mode = {};

				wanted_mode.w = parms.width;
				wanted_mode.h = parms.height;

				if (SDL_SetWindowDisplayMode(window, &wanted_mode) != 0)
				{
					SDL_DestroyWindow(window);
					window = NULL;

					common->Warning("Can't force resolution to %ix%i: %s\n", parms.width, parms.height, SDL_GetError());

					return false; // trying other color depth etc is unlikely to help with this issue
				}

				/* The SDL doku says, that SDL_SetWindowSize() shouldn't be
				   used on fullscreen windows. But at least in my test with
				   SDL 2.0.9 the subsequent SDL_GetWindowDisplayMode() fails
				   if I don't call it. */
				SDL_SetWindowSize(window, wanted_mode.w, wanted_mode.h);

				if (SDL_GetWindowDisplayMode(window, &real_mode) != 0)
				{
					SDL_DestroyWindow(window);
					window = NULL;

					common->Warning("Can't get display mode: %s\n", SDL_GetError());

					return false; // trying other color depth etc is unlikely to help with this issue
				}

				if ((real_mode.w != parms.width) || (real_mode.h != parms.height))
				{
					SDL_DestroyWindow(window);
					window = NULL;

					common->Warning("Still in wrong display mode: %ix%i instead of %ix%i\n",
					                real_mode.w, real_mode.h, parms.width, parms.height);

					return false; // trying other color depth etc is unlikely to help with this issue
				}
				common->Warning("Now we have the requested resolution (%d x %d)\n", parms.width, parms.height);
			}
		}

		context = SDL_GL_CreateContext(window);

		if (SDL_GL_SetSwapInterval(r_swapInterval.GetInteger()) < 0)
			common->Warning("SDL_GL_SWAP_CONTROL not supported");

		SDL_GetWindowSize(window, &glConfig.vidWidth, &glConfig.vidHeight);

		SetSDLIcon(); // for SDL2  this must be done after creating the window

		glConfig.isFullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN;
#else
		SDL_WM_SetCaption(ENGINE_VERSION, ENGINE_VERSION);

		SetSDLIcon(); // for SDL1.2  this must be done before creating the window

		if (SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, r_swapInterval.GetInteger()) < 0)
			common->Warning("SDL_GL_SWAP_CONTROL not supported");

		window = SDL_SetVideoMode(parms.width, parms.height, colorbits, flags);
		if (!window) {
			common->DPrintf("Couldn't set GL mode %d/%d/%d: %s",
							channelcolorbits, tdepthbits, tstencilbits, SDL_GetError());

			// before trying to reduce color channel size or whatever, first try reducing MSAA, if possible
			if(multisamples > 1) {
				multisamples = (multisamples <= 2) ? 0 : (multisamples/2);

				// using goto because enhancing that logic which reduces attributes
				// based on i (so it'd first try reducing MSAA) would be too painful
				goto try_again;
			}

			continue;
		} else {
			// creating the window succeeded, so adjust r_multiSamples to the value that was actually used
			parms.multiSamples = multisamples;
			r_multiSamples.SetInteger(multisamples);
		}

		glConfig.vidWidth = window->w;
		glConfig.vidHeight = window->h;

		glConfig.isFullscreen = (window->flags & SDL_FULLSCREEN) == SDL_FULLSCREEN;
#endif

#if defined(_WIN32) && defined(ID_ALLOW_TOOLS)

#if ! SDL_VERSION_ATLEAST(2, 0, 0)
	#error "dhewm3 only supports the tools with SDL2, not SDL1!"
#endif

		// The tools are Win32 specific.  If building the tools
		// then we know we are win32 and we have to include this
		// config to get the editors to work.

		// Get the HWND for later use.
		SDL_SysWMinfo sdlinfo;
		SDL_version sdlver;
		SDL_VERSION(&sdlver);
		sdlinfo.version = sdlver;
		if (SDL_GetWindowWMInfo(window, &sdlinfo) && sdlinfo.subsystem == SDL_SYSWM_WINDOWS) {
			win32.hWnd = sdlinfo.info.win.window;
			win32.hDC = sdlinfo.info.win.hdc;
			// NOTE: hInstance is set in main()
			win32.hGLRC = qwglGetCurrentContext();

			int pfIdx = GetPixelFormat(win32.hDC);
			PIXELFORMATDESCRIPTOR src = {};
			if (DescribePixelFormat(win32.hDC, pfIdx, sizeof(PIXELFORMATDESCRIPTOR), &win32.pfd) == 0)
			{
				common->Warning("DescribePixelFormat() failed: %d!\n", GetLastError());
				PIXELFORMATDESCRIPTOR src =
				{
					sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
					1,								// version number
					PFD_DRAW_TO_WINDOW |			// support window
					PFD_SUPPORT_OPENGL |			// support OpenGL
					PFD_DOUBLEBUFFER,				// double buffered
					PFD_TYPE_RGBA,					// RGBA type
					32,								// 32-bit color depth
					0, 0, 0, 0, 0, 0,				// color bits ignored
					8,								// 8 bit destination alpha
					0,								// shift bit ignored
					0,								// no accumulation buffer
					0, 0, 0, 0, 					// accum bits ignored
					24,								// 24-bit z-buffer
					8,								// 8-bit stencil buffer
					0,								// no auxiliary buffer
					PFD_MAIN_PLANE,					// main layer
					0,								// reserved
					0, 0, 0							// layer masks ignored
				};
				memcpy(&win32.pfd, &src, sizeof(PIXELFORMATDESCRIPTOR));
			}
			
			win32.piAttribIList = NULL;

			win32.wglGetPixelFormatAttribivARB = (BOOL(WINAPI*)(HDC,int,int,UINT,const int*,int*))SDL_GL_GetProcAddress("wglGetPixelFormatAttribivARB");
			win32.wglChoosePixelFormatARB = (BOOL(WINAPI*)(HDC,const int*,const FLOAT*,UINT,int*piFormats,UINT*))SDL_GL_GetProcAddress("wglChoosePixelFormatARB");

			if(win32.wglGetPixelFormatAttribivARB != NULL && win32.wglChoosePixelFormatARB != NULL) {
				const int queryAttributes[] = {
					// equivalents of all the SDL_GL_* attributes we set above (and ones set implicitly)
					WGL_DRAW_TO_WINDOW_ARB,
					WGL_RED_BITS_ARB,
					WGL_GREEN_BITS_ARB,
					WGL_BLUE_BITS_ARB,
					WGL_ALPHA_BITS_ARB,
					WGL_DOUBLE_BUFFER_ARB,
					WGL_DEPTH_BITS_ARB,
					WGL_STENCIL_BITS_ARB,
					// WGL_ACCUM_*_BITS_ARB - not used
					WGL_STEREO_ARB,
					WGL_SAMPLE_BUFFERS_ARB,
					WGL_SAMPLES_ARB,
					// WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB - not used
					WGL_ACCELERATION_ARB,
				};
				enum { NUM_ATTRIBUTES = sizeof(queryAttributes)/sizeof(queryAttributes[0]) };
				int queryResults[NUM_ATTRIBUTES] = {};
				
				win32.wglGetPixelFormatAttribivARB(win32.hDC, pfIdx, PFD_MAIN_PLANE, NUM_ATTRIBUTES, queryAttributes, queryResults);
				
				static int attribIList[2*NUM_ATTRIBUTES+2] = {}; // +2 for terminating 0, 0 pair
				for(int i=0; i<NUM_ATTRIBUTES; ++i) {
					attribIList[i*2] = queryAttributes[i];
					attribIList[i*2+1] = queryResults[i];
				}
				win32.piAttribIList = attribIList;
			}
		} else {
			// TODO: can we just disable them?
			common->Error("SDL_GetWindowWMInfo(), which is needed for Tools to work, failed!");
		}		
#endif // defined(_WIN32) && defined(ID_ALLOW_TOOLS)

		common->Printf("Requested %d color bits per chan, %d alpha %d depth, %d stencil\n",
						channelcolorbits, talphabits, tdepthbits, tstencilbits);

		{
			int r, g, b, a, d, s;
			SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
			SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
			SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
			SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &a);
			SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &d);
			SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &s);

			common->Printf("Got %d stencil bits, %d depth bits, color bits: r%d g%d b%d a%d\n", s, d, r, g, b, a);

			glConfig.colorBits = r+g+b; // a bit imprecise, but seems to be used only in GfxInfo_f()
			glConfig.alphabits = a;
			glConfig.depthBits = d;
			glConfig.stencilBits = s;
		}

		glConfig.displayFrequency = 0;

		// for r_fillWindowAlphaChan -1, see also the big comment above
		glConfig.shouldFillWindowAlpha = false;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		const char* videoDriver = SDL_GetCurrentVideoDriver();
		if (idStr::Icmp(videoDriver, "wayland") == 0) {
			glConfig.shouldFillWindowAlpha = true;
		}
#endif

		break;
	}

	if (!window) {
		common->Warning("No usable GL mode found: %s", SDL_GetError());
		return false;
	}

	return true;
}

/*
===================
GLimp_SetScreenParms
===================
*/
bool GLimp_SetScreenParms(glimpParms_t parms) {
	common->DPrintf("TODO: GLimp_ActivateContext\n");
	return true;
}

/*
===================
GLimp_Shutdown
===================
*/
void GLimp_Shutdown() {
	common->Printf("Shutting down OpenGL subsystem\n");

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (context) {
		SDL_GL_DeleteContext(context);
		context = NULL;
	}

	if (window) {
		SDL_DestroyWindow(window);
		window = NULL;
	}
#endif
}

/*
===================
GLimp_SwapBuffers
===================
*/
void GLimp_SwapBuffers() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_SwapWindow(window);
#else
	SDL_GL_SwapBuffers();
#endif
}

static bool gammaOrigError = false;
static bool gammaOrigSet = false;
static unsigned short gammaOrigRed[256];
static unsigned short gammaOrigGreen[256];
static unsigned short gammaOrigBlue[256];

/*
=================
GLimp_SetGamma
=================
*/
void GLimp_SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]) {
	if (!window) {
		common->Warning("GLimp_SetGamma called without window");
		return;
	}

	if ( !gammaOrigSet ) {
		gammaOrigSet = true;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if ( SDL_GetWindowGammaRamp( window, gammaOrigRed, gammaOrigGreen, gammaOrigBlue ) == -1 ) {
#else
		if ( SDL_GetGammaRamp( gammaOrigRed, gammaOrigGreen, gammaOrigBlue ) == -1 ) {
#endif
			gammaOrigError = true;
			common->Warning( "Failed to get Gamma Ramp: %s\n", SDL_GetError() );
		}
	}


#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (SDL_SetWindowGammaRamp(window, red, green, blue))
#else
	if (SDL_SetGammaRamp(red, green, blue))
#endif
		common->Warning("Couldn't set gamma ramp: %s", SDL_GetError());
}

/*
=================
GLimp_ResetGamma

Restore original system gamma setting
=================
*/
void GLimp_ResetGamma() {
	if( gammaOrigError ) {
		common->Warning( "Can't reset hardware gamma because getting the Gamma Ramp at startup failed!\n" );
		common->Warning( "You might have to restart the game for gamma/brightness in shaders to work properly.\n" );
		return;
	}

	if( gammaOrigSet ) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_SetWindowGammaRamp( window, gammaOrigRed, gammaOrigGreen, gammaOrigBlue );
#else
		SDL_SetGammaRamp( gammaOrigRed, gammaOrigGreen, gammaOrigBlue );
#endif
	}
}


/*
=================
GLimp_ActivateContext
=================
*/
void GLimp_ActivateContext() {
	common->DPrintf("TODO: GLimp_ActivateContext\n");
}

/*
=================
GLimp_DeactivateContext
=================
*/
void GLimp_DeactivateContext() {
	common->DPrintf("TODO: GLimp_DeactivateContext\n");
}

/*
===================
GLimp_ExtensionPointer
===================
*/
GLExtension_t GLimp_ExtensionPointer(const char *name) {
	assert(SDL_WasInit(SDL_INIT_VIDEO));

	return (GLExtension_t)SDL_GL_GetProcAddress(name);
}

void GLimp_GrabInput(int flags) {
	if (!window) {
		common->Warning("GLimp_GrabInput called without window");
		return;
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_ShowCursor( (flags & GRAB_HIDECURSOR) ? SDL_DISABLE : SDL_ENABLE );
	SDL_SetRelativeMouseMode( (flags & GRAB_RELATIVEMOUSE) ? SDL_TRUE : SDL_FALSE );
	SDL_SetWindowGrab( window, (flags & GRAB_GRABMOUSE) ? SDL_TRUE : SDL_FALSE );
#else
	SDL_ShowCursor( (flags & GRAB_HIDECURSOR) ? SDL_DISABLE : SDL_ENABLE );
	// ignore GRAB_GRABMOUSE, SDL1.2 doesn't support grabbing without relative mode
	// so only grab if we want relative mode
	SDL_WM_GrabInput( (flags & GRAB_RELATIVEMOUSE) ? SDL_GRAB_ON : SDL_GRAB_OFF );
#endif
}
