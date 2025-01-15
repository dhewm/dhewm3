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
/*
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__

#if defined( ID_DEDICATED ) && defined( _WIN32 )
// to allow stubbing gl on windows, define WINGDIAPI to nothing - it would otherwise be
// extended to __declspec(dllimport) on MSVC (our stub is no dll.)
	#ifdef WINGDIAPI
		#pragma push_macro("WINGDIAPI")
		#undef WINGDIAPI
		#define WINGDIAPI
	#endif
#endif

#define GL_GLEXT_PROTOTYPES

#ifdef D3_SDL3
  #include <SDL3/SDL_opengl.h>
#else // SDL1.2 or SDL2
  #include <SDL_opengl.h>
#endif

#if defined( ID_DEDICATED ) && defined( _WIN32 )
// restore WINGDIAPI
	#ifdef WINGDIAPI
		#pragma pop_macro("WINGDIAPI")
	#endif
#endif

typedef void (*GLExtension_t)(void);

#ifdef __cplusplus
	extern "C" {
#endif

GLExtension_t GLimp_ExtensionPointer( const char *name );

#ifdef __cplusplus
	}
#endif

// declare qgl functions
#define QGLPROC(name, rettype, args) extern rettype (APIENTRYP q##name) args;
#include "renderer/qgl_proc.h"

// multitexture
extern	void ( APIENTRY * qglMultiTexCoord2fARB )( GLenum texture, GLfloat s, GLfloat t );
extern	void ( APIENTRY * qglMultiTexCoord2fvARB )( GLenum texture, GLfloat *st );
extern	void ( APIENTRY * qglActiveTextureARB )( GLenum texture );
extern	void ( APIENTRY * qglClientActiveTextureARB )( GLenum texture );

// ARB_MapBufferRange
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC qglFlushMappedBufferRange;
extern PFNGLMAPBUFFERRANGEPROC qglMapBufferRange;

// ARB_shading_language_100
extern PFNGLUNIFORM1FPROC qglUniform1f;
extern PFNGLUNIFORM1FVPROC qglUniform1fv;
extern PFNGLUNIFORM1IPROC qglUniform1i;
extern PFNGLUNIFORM1IVPROC qglUniform1iv;
extern PFNGLUNIFORM2FPROC qglUniform2f;
extern PFNGLUNIFORM2FVPROC qglUniform2fv;
extern PFNGLUNIFORM2IPROC qglUniform2i;
extern PFNGLUNIFORM2IVPROC qglUniform2iv;
extern PFNGLUNIFORM3FPROC qglUniform3f;
extern PFNGLUNIFORM3FVPROC qglUniform3fv;
extern PFNGLUNIFORM3IPROC qglUniform3i;
extern PFNGLUNIFORM3IVPROC qglUniform3iv;
extern PFNGLUNIFORM4FPROC qglUniform4f;
extern PFNGLUNIFORM4FVPROC qglUniform4fv;
extern PFNGLUNIFORM4IPROC qglUniform4i;
extern PFNGLUNIFORM4IVPROC qglUniform4iv;
extern PFNGLUNIFORMMATRIX2FVPROC qglUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC qglUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC qglUniformMatrix4fv;
extern PFNGLUNIFORMMATRIX2X3FVPROC qglUniformMatrix2x3fv;
extern PFNGLUNIFORMMATRIX2X4FVPROC qglUniformMatrix2x4fv;
extern PFNGLUNIFORMMATRIX3X2FVPROC qglUniformMatrix3x2fv;
extern PFNGLUNIFORMMATRIX3X4FVPROC qglUniformMatrix3x4fv;
extern PFNGLUNIFORMMATRIX4X2FVPROC qglUniformMatrix4x2fv;
extern PFNGLUNIFORMMATRIX4X3FVPROC qglUniformMatrix4x3fv;

extern PFNGLISPROGRAMPROC qglIsProgram;
extern PFNGLCREATEPROGRAMPROC qglCreateProgram;
extern PFNGLVALIDATEPROGRAMPROC qglValidateProgram;
extern PFNGLLINKPROGRAMPROC qglLinkProgram;

extern PFNGLISSHADERPROC qglIsShader;
extern PFNGLSHADERSOURCEPROC qglShaderSource;
extern PFNGLCREATESHADERPROC qglCreateShader;
extern PFNGLATTACHSHADERPROC qglAttachShader;
extern PFNGLBINDATTRIBLOCATIONPROC qglBindAttribLocation;
extern PFNGLCOMPILESHADERPROC qglCompileShader;
extern PFNGLDETACHSHADERPROC qglDetachShader;
extern PFNGLDELETESHADERPROC qglDeleteShader;

extern PFNGLGETPROGRAMINFOLOGPROC qglGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC qglGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC qglGetShaderInfoLog;
extern PFNGLGETSHADERSOURCEPROC qglGetShaderSource;
extern PFNGLGETSHADERIVPROC qglGetShaderiv;
extern PFNGLGETUNIFORMLOCATIONPROC qglGetUniformLocation;
extern PFNGLGETUNIFORMFVPROC qglGetUniformfv;
extern PFNGLGETUNIFORMIVPROC qglGetUniformiv;

// ARB_vertex_buffer_object
extern PFNGLBINDBUFFERARBPROC qglBindBufferARB;
extern PFNGLDELETEBUFFERSARBPROC qglDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC qglGenBuffersARB;
extern PFNGLISBUFFERARBPROC qglIsBufferARB;
extern PFNGLBUFFERDATAARBPROC qglBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC qglBufferSubDataARB;
extern PFNGLGETBUFFERSUBDATAARBPROC qglGetBufferSubDataARB;
extern PFNGLMAPBUFFERARBPROC qglMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC qglUnmapBufferARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC qglGetBufferParameterivARB;
extern PFNGLGETBUFFERPOINTERVARBPROC qglGetBufferPointervARB;

// 3D textures
extern void ( APIENTRY *qglTexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);

// shared texture palette
extern	void ( APIENTRY *qglColorTableEXT)( int, int, int, int, int, const void * );

// EXT_stencil_two_side
extern	PFNGLACTIVESTENCILFACEEXTPROC	qglActiveStencilFaceEXT;

// DG: couldn't find any extension for this, it's supported in GL2.0 and newer, incl OpenGL ES2.0
// SE: work around missing function definition on legacy Mac OS X versions
#if defined(OSX_TIGER) || defined(OSX_LEOPARD)
typedef void (APIENTRYP PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
#endif
extern PFNGLSTENCILOPSEPARATEPROC qglStencilOpSeparate;

// ARB_texture_compression
extern	PFNGLCOMPRESSEDTEXIMAGE2DARBPROC	qglCompressedTexImage2DARB;
extern	PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	qglGetCompressedTexImageARB;

// ARB_vertex_program / ARB_fragment_program
extern PFNGLVERTEXATTRIBPOINTERARBPROC		qglVertexAttribPointerARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC	qglEnableVertexAttribArrayARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	qglDisableVertexAttribArrayARB;
extern PFNGLPROGRAMSTRINGARBPROC			qglProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC				qglBindProgramARB;
extern PFNGLGENPROGRAMSARBPROC				qglGenProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC	qglProgramEnvParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC	qglProgramLocalParameter4fvARB;

// GL_EXT_depth_bounds_test
extern PFNGLDEPTHBOUNDSEXTPROC              qglDepthBoundsEXT;

// GL_ARB_debug_output
extern PFNGLDEBUGMESSAGECALLBACKARBPROC    qglDebugMessageCallbackARB;

#if defined( _WIN32 ) && defined(ID_ALLOW_TOOLS)

extern  BOOL(WINAPI * qwglSwapBuffers)(HDC);
extern int Win_ChoosePixelFormat(HDC hdc);

extern BOOL(WINAPI * qwglCopyContext)(HGLRC, HGLRC, UINT);
extern HGLRC(WINAPI * qwglCreateContext)(HDC);
extern HGLRC(WINAPI * qwglCreateLayerContext)(HDC, int);
extern BOOL(WINAPI * qwglDeleteContext)(HGLRC);
extern HGLRC(WINAPI * qwglGetCurrentContext)(VOID);
extern HDC(WINAPI * qwglGetCurrentDC)(VOID);
extern PROC(WINAPI * qwglGetProcAddress)(LPCSTR);
extern BOOL(WINAPI * qwglMakeCurrent)(HDC, HGLRC);
extern BOOL(WINAPI * qwglShareLists)(HGLRC, HGLRC);
extern BOOL(WINAPI * qwglUseFontBitmaps)(HDC, DWORD, DWORD, DWORD);

extern BOOL(WINAPI * qwglUseFontOutlines)(HDC, DWORD, DWORD, DWORD, FLOAT,
	FLOAT, int, LPGLYPHMETRICSFLOAT);

extern BOOL(WINAPI * qwglDescribeLayerPlane)(HDC, int, int, UINT,
	LPLAYERPLANEDESCRIPTOR);
extern int  (WINAPI * qwglSetLayerPaletteEntries)(HDC, int, int, int,
	CONST COLORREF *);
extern int  (WINAPI * qwglGetLayerPaletteEntries)(HDC, int, int, int,
	COLORREF *);
extern BOOL(WINAPI * qwglRealizeLayerPalette)(HDC, int, BOOL);
extern BOOL(WINAPI * qwglSwapLayerBuffers)(HDC, UINT);

#endif	// _WIN32 && ID_ALLOW_TOOLS

#endif
