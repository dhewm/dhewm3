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
#include <SDL_syswm.h>

#include "sys/platform.h"
#include "framework/Licensee.h"

#include "renderer/tr_local.h"
#if defined(MACOS_X)
#include "sys/glimp_ati_fragment_shader.h"
#elif defined(_WIN32)
#include "sys/win32/win_local.h"
#endif

idCVar sys_videoRam("sys_videoRam", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "Texture memory on the video card (in megabytes) - 0: autodetect", 0, 512);

/*
===================
GLimp_Init
===================
*/
bool GLimp_Init(glimpParms_t parms) {
	common->Printf("Initializing OpenGL subsystem\n");

	assert(SDL_WasInit(SDL_INIT_VIDEO));

	Uint32 flags = SDL_OPENGL;

	if (parms.fullScreen)
		flags |= SDL_FULLSCREEN;

	SDL_Surface *surf = NULL;

	int colorbits = 24;
	int depthbits = 24;
	int stencilbits = 8;

	for (int i = 0; i < 16; i++) {
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

		SDL_WM_SetCaption(GAME_NAME, GAME_NAME);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, channelcolorbits);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, channelcolorbits);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, channelcolorbits);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, channelcolorbits);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, tdepthbits);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, tstencilbits);

		SDL_GL_SetAttribute(SDL_GL_STEREO, parms.stereo ? 1 : 0);

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, parms.multiSamples ? 1 : 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, parms.multiSamples);

		if (SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, r_swapInterval.GetInteger()) < 0)
			common->Warning("SDL_GL_SWAP_CONTROL not supported");

		surf = SDL_SetVideoMode(parms.width, parms.height, colorbits, flags);
		if (!surf) {
			common->DPrintf("Couldn't set GL mode %d/%d/%d: %s",
							channelcolorbits, tdepthbits, tstencilbits, SDL_GetError());
			continue;
		}

		common->Printf("Using %d color bits, %d depth, %d stencil display\n",
						channelcolorbits, tdepthbits, tstencilbits);

		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;

		glConfig.vidWidth = surf->w;
		glConfig.vidHeight = surf->h;

		glConfig.isFullscreen = (surf->flags & SDL_FULLSCREEN) == SDL_FULLSCREEN;
		glConfig.displayFrequency = 0;

		break;
	}

	if (!surf) {
		common->Warning("No usable GL mode found: %s", SDL_GetError());
		return false;
	}

	glConfig.wgl_extensions_string = "";

#ifdef _WIN32
	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)GLimp_ExtensionPointer("wglGetExtensionsStringARB");

	if (wglGetExtensionsStringARB)
		glConfig.wgl_extensions_string = (const char *)wglGetExtensionsStringARB(wglGetCurrentDC());

	// the editors still rely on these vars
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);

	if (SDL_GetWMInfo(&info)) {
		win32.hDC = wglGetCurrentDC();
		win32.hGLRC = info.hglrc;
	}

	win32.pixelformat = GetPixelFormat(win32.hDC);
	DescribePixelFormat(win32.hDC, win32.pixelformat, sizeof(win32.pfd), &win32.pfd);
#endif

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
}

/*
===================
GLimp_SwapBuffers
===================
*/
void GLimp_SwapBuffers() {
	SDL_GL_SwapBuffers();
}

/*
=================
GLimp_SetGamma
=================
*/
void GLimp_SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]) {
	if (SDL_SetGammaRamp(red, green, blue))
		common->Warning("Couldn't set gamma ramp: %s", SDL_GetError());
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

#ifdef MACOS_X
	// special case for ATI_fragment_shader calls to map to ATI_text_fragment_shader routines
	GLExtension_t res = GLimp_ExtensionPointer_ATI_fragment_shader(name);
	if (res)
		return res;
#endif

	return (GLExtension_t)SDL_GL_GetProcAddress(name);
}

/*
================
Sys_GetVideoRam
================
*/
int Sys_GetVideoRam() {
	assert(SDL_WasInit(SDL_INIT_VIDEO));

	if (sys_videoRam.GetInteger())
		return sys_videoRam.GetInteger();

	common->Printf("guessing video ram (use +set sys_videoRam to force)\n");

	const SDL_VideoInfo *vi = SDL_GetVideoInfo();

	return vi->video_mem;
}
