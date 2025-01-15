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

#ifndef QGLPROC
#error "you must define QGLPROC before including this file"
#endif

QGLPROC(glAccum, void, (GLenum op, GLfloat value))
QGLPROC(glAlphaFunc, void, (GLenum func, GLclampf ref))
QGLPROC(glAreTexturesResident, GLboolean, (GLsizei n, const GLuint *textures, GLboolean *residences))
QGLPROC(glArrayElement, void, (GLint i))
QGLPROC(glBegin, void, (GLenum mode))
QGLPROC(glBindTexture, void, (GLenum target, GLuint texture))
QGLPROC(glBitmap, void, (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap))
QGLPROC(glBlendFunc, void, (GLenum sfactor, GLenum dfactor))
QGLPROC(glBlendEquation, void, (GLenum mode))
QGLPROC(glCallList, void, (GLuint list))
QGLPROC(glCallLists, void, (GLsizei n, GLenum type, const GLvoid *lists))
QGLPROC(glClear, void, (GLbitfield mask))
QGLPROC(glClearAccum, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
QGLPROC(glClearColor, void, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha))
QGLPROC(glClearDepth, void, (GLclampd depth))
QGLPROC(glClearIndex, void, (GLfloat c))
QGLPROC(glClearStencil, void, (GLint s))
QGLPROC(glClipPlane, void, (GLenum plane, const GLdouble *equation))
QGLPROC(glColor3b, void, (GLbyte red, GLbyte green, GLbyte blue))
QGLPROC(glColor3bv, void, (const GLbyte *v))
QGLPROC(glColor3d, void, (GLdouble red, GLdouble green, GLdouble blue))
QGLPROC(glColor3dv, void, (const GLdouble *v))
QGLPROC(glColor3f, void, (GLfloat red, GLfloat green, GLfloat blue))
QGLPROC(glColor3fv, void, (const GLfloat *v))
QGLPROC(glColor3i, void, (GLint red, GLint green, GLint blue))
QGLPROC(glColor3iv, void, (const GLint *v))
QGLPROC(glColor3s, void, (GLshort red, GLshort green, GLshort blue))
QGLPROC(glColor3sv, void, (const GLshort *v))
QGLPROC(glColor3ub, void, (GLubyte red, GLubyte green, GLubyte blue))
QGLPROC(glColor3ubv, void, (const GLubyte *v))
QGLPROC(glColor3ui, void, (GLuint red, GLuint green, GLuint blue))
QGLPROC(glColor3uiv, void, (const GLuint *v))
QGLPROC(glColor3us, void, (GLushort red, GLushort green, GLushort blue))
QGLPROC(glColor3usv, void, (const GLushort *v))
QGLPROC(glColor4b, void, (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha))
QGLPROC(glColor4bv, void, (const GLbyte *v))
QGLPROC(glColor4d, void, (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha))
QGLPROC(glColor4dv, void, (const GLdouble *v))
QGLPROC(glColor4f, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
QGLPROC(glColor4fv, void, (const GLfloat *v))
QGLPROC(glColor4i, void, (GLint red, GLint green, GLint blue, GLint alpha))
QGLPROC(glColor4iv, void, (const GLint *v))
QGLPROC(glColor4s, void, (GLshort red, GLshort green, GLshort blue, GLshort alpha))
QGLPROC(glColor4sv, void, (const GLshort *v))
QGLPROC(glColor4ub, void, (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha))
QGLPROC(glColor4ubv, void, (const GLubyte *v))
QGLPROC(glColor4ui, void, (GLuint red, GLuint green, GLuint blue, GLuint alpha))
QGLPROC(glColor4uiv, void, (const GLuint *v))
QGLPROC(glColor4us, void, (GLushort red, GLushort green, GLushort blue, GLushort alpha))
QGLPROC(glColor4usv, void, (const GLushort *v))
QGLPROC(glColorMask, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))
QGLPROC(glColorMaterial, void, (GLenum face, GLenum mode))
QGLPROC(glColorPointer, void, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
QGLPROC(glCopyPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type))
QGLPROC(glCopyTexImage1D, void, (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border))
QGLPROC(glCopyTexImage2D, void, (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border))
QGLPROC(glCopyTexSubImage1D, void, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width))
QGLPROC(glCopyTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height))
QGLPROC(glCullFace, void, (GLenum mode))
QGLPROC(glDeleteLists, void, (GLuint list, GLsizei range))
QGLPROC(glDeleteProgram, void, (GLuint program))
QGLPROC(glDeleteTextures, void, (GLsizei n, const GLuint *textures))
QGLPROC(glDepthFunc, void, (GLenum func))
QGLPROC(glDepthMask, void, (GLboolean flag))
QGLPROC(glDepthRange, void, (GLclampd zNear, GLclampd zFar))
QGLPROC(glDisable, void, (GLenum cap))
QGLPROC(glDisableClientState, void, (GLenum array))
QGLPROC(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))
QGLPROC(glDrawBuffer, void, (GLenum mode))
QGLPROC(glDrawElements, void, (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices))
QGLPROC(glDrawPixels, void, (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels))
QGLPROC(glEdgeFlag, void, (GLboolean flag))
QGLPROC(glEdgeFlagPointer, void, (GLsizei stride, const GLvoid *pointer))
QGLPROC(glEdgeFlagv, void, (const GLboolean *flag))
QGLPROC(glEnable, void, (GLenum cap))
QGLPROC(glEnableClientState, void, (GLenum array))
QGLPROC(glEnd, void, (void))
QGLPROC(glEndList, void, (void))
QGLPROC(glEvalCoord1d, void, (GLdouble u))
QGLPROC(glEvalCoord1dv, void, (const GLdouble *u))
QGLPROC(glEvalCoord1f, void, (GLfloat u))
QGLPROC(glEvalCoord1fv, void, (const GLfloat *u))
QGLPROC(glEvalCoord2d, void, (GLdouble u, GLdouble v))
QGLPROC(glEvalCoord2dv, void, (const GLdouble *u))
QGLPROC(glEvalCoord2f, void, (GLfloat u, GLfloat v))
QGLPROC(glEvalCoord2fv, void, (const GLfloat *u))
QGLPROC(glEvalMesh1, void, (GLenum mode, GLint i1, GLint i2))
QGLPROC(glEvalMesh2, void, (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2))
QGLPROC(glEvalPoint1, void, (GLint i))
QGLPROC(glEvalPoint2, void, (GLint i, GLint j))
QGLPROC(glFeedbackBuffer, void, (GLsizei size, GLenum type, GLfloat *buffer))
QGLPROC(glFinish, void, (void))
QGLPROC(glFlush, void, (void))
QGLPROC(glFogf, void, (GLenum pname, GLfloat param))
QGLPROC(glFogfv, void, (GLenum pname, const GLfloat *params))
QGLPROC(glFogi, void, (GLenum pname, GLint param))
QGLPROC(glFogiv, void, (GLenum pname, const GLint *params))
QGLPROC(glFrontFace, void, (GLenum mode))
QGLPROC(glFrustum, void, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar))
QGLPROC(glGenLists, GLuint, (GLsizei range))
QGLPROC(glGenTextures, void, (GLsizei n, GLuint *textures))
QGLPROC(glGetBooleanv, void, (GLenum pname, GLboolean *params))
QGLPROC(glGetClipPlane, void, (GLenum plane, GLdouble *equation))
QGLPROC(glGetDoublev, void, (GLenum pname, GLdouble *params))
QGLPROC(glGetError, GLenum, (void))
QGLPROC(glGetFloatv, void, (GLenum pname, GLfloat *params))
QGLPROC(glGetIntegerv, void, (GLenum pname, GLint *params))
QGLPROC(glGetLightfv, void, (GLenum light, GLenum pname, GLfloat *params))
QGLPROC(glGetLightiv, void, (GLenum light, GLenum pname, GLint *params))
QGLPROC(glGetMapdv, void, (GLenum target, GLenum query, GLdouble *v))
QGLPROC(glGetMapfv, void, (GLenum target, GLenum query, GLfloat *v))
QGLPROC(glGetMapiv, void, (GLenum target, GLenum query, GLint *v))
QGLPROC(glGetMaterialfv, void, (GLenum face, GLenum pname, GLfloat *params))
QGLPROC(glGetMaterialiv, void, (GLenum face, GLenum pname, GLint *params))
QGLPROC(glGetPixelMapfv, void, (GLenum map, GLfloat *values))
QGLPROC(glGetPixelMapuiv, void, (GLenum map, GLuint *values))
QGLPROC(glGetPixelMapusv, void, (GLenum map, GLushort *values))
QGLPROC(glGetPointerv, void, (GLenum pname, GLvoid* *params))
QGLPROC(glGetPolygonStipple, void, (GLubyte *mask))
QGLPROC(glGetString, const GLubyte *, (GLenum name))
QGLPROC(glGetTexEnvfv, void, (GLenum target, GLenum pname, GLfloat *params))
QGLPROC(glGetTexEnviv, void, (GLenum target, GLenum pname, GLint *params))
QGLPROC(glGetTexGendv, void, (GLenum coord, GLenum pname, GLdouble *params))
QGLPROC(glGetTexGenfv, void, (GLenum coord, GLenum pname, GLfloat *params))
QGLPROC(glGetTexGeniv, void, (GLenum coord, GLenum pname, GLint *params))
QGLPROC(glGetTexImage, void, (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels))
QGLPROC(glGetTexLevelParameterfv, void, (GLenum target, GLint level, GLenum pname, GLfloat *params))
QGLPROC(glGetTexLevelParameteriv, void, (GLenum target, GLint level, GLenum pname, GLint *params))
QGLPROC(glGetTexParameterfv, void, (GLenum target, GLenum pname, GLfloat *params))
QGLPROC(glGetTexParameteriv, void, (GLenum target, GLenum pname, GLint *params))
QGLPROC(glHint, void, (GLenum target, GLenum mode))
QGLPROC(glIndexMask, void, (GLuint mask))
QGLPROC(glIndexPointer, void, (GLenum type, GLsizei stride, const GLvoid *pointer))
QGLPROC(glIndexd, void, (GLdouble c))
QGLPROC(glIndexdv, void, (const GLdouble *c))
QGLPROC(glIndexf, void, (GLfloat c))
QGLPROC(glIndexfv, void, (const GLfloat *c))
QGLPROC(glIndexi, void, (GLint c))
QGLPROC(glIndexiv, void, (const GLint *c))
QGLPROC(glIndexs, void, (GLshort c))
QGLPROC(glIndexsv, void, (const GLshort *c))
QGLPROC(glIndexub, void, (GLubyte c))
QGLPROC(glIndexubv, void, (const GLubyte *c))
QGLPROC(glInitNames, void, (void))
QGLPROC(glInterleavedArrays, void, (GLenum format, GLsizei stride, const GLvoid *pointer))
QGLPROC(glIsEnabled, GLboolean, (GLenum cap))
QGLPROC(glIsList, GLboolean, (GLuint list))
QGLPROC(glIsTexture, GLboolean, (GLuint texture))
QGLPROC(glLightModelf, void, (GLenum pname, GLfloat param))
QGLPROC(glLightModelfv, void, (GLenum pname, const GLfloat *params))
QGLPROC(glLightModeli, void, (GLenum pname, GLint param))
QGLPROC(glLightModeliv, void, (GLenum pname, const GLint *params))
QGLPROC(glLightf, void, (GLenum light, GLenum pname, GLfloat param))
QGLPROC(glLightfv, void, (GLenum light, GLenum pname, const GLfloat *params))
QGLPROC(glLighti, void, (GLenum light, GLenum pname, GLint param))
QGLPROC(glLightiv, void, (GLenum light, GLenum pname, const GLint *params))
QGLPROC(glLineStipple, void, (GLint factor, GLushort pattern))
QGLPROC(glLineWidth, void, (GLfloat width))
QGLPROC(glListBase, void, (GLuint base))
QGLPROC(glLoadIdentity, void, (void))
QGLPROC(glLoadMatrixd, void, (const GLdouble *m))
QGLPROC(glLoadMatrixf, void, (const GLfloat *m))
QGLPROC(glLoadName, void, (GLuint name))
QGLPROC(glLogicOp, void, (GLenum opcode))
QGLPROC(glMap1d, void, (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points))
QGLPROC(glMap1f, void, (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points))
QGLPROC(glMap2d, void, (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points))
QGLPROC(glMap2f, void, (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points))
QGLPROC(glMapGrid1d, void, (GLint un, GLdouble u1, GLdouble u2))
QGLPROC(glMapGrid1f, void, (GLint un, GLfloat u1, GLfloat u2))
QGLPROC(glMapGrid2d, void, (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2))
QGLPROC(glMapGrid2f, void, (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2))
QGLPROC(glMaterialf, void, (GLenum face, GLenum pname, GLfloat param))
QGLPROC(glMaterialfv, void, (GLenum face, GLenum pname, const GLfloat *params))
QGLPROC(glMateriali, void, (GLenum face, GLenum pname, GLint param))
QGLPROC(glMaterialiv, void, (GLenum face, GLenum pname, const GLint *params))
QGLPROC(glMatrixMode, void, (GLenum mode))
QGLPROC(glMultMatrixd, void, (const GLdouble *m))
QGLPROC(glMultMatrixf, void, (const GLfloat *m))
QGLPROC(glNewList, void, (GLuint list, GLenum mode))
QGLPROC(glNormal3b, void, (GLbyte nx, GLbyte ny, GLbyte nz))
QGLPROC(glNormal3bv, void, (const GLbyte *v))
QGLPROC(glNormal3d, void, (GLdouble nx, GLdouble ny, GLdouble nz))
QGLPROC(glNormal3dv, void, (const GLdouble *v))
QGLPROC(glNormal3f, void, (GLfloat nx, GLfloat ny, GLfloat nz))
QGLPROC(glNormal3fv, void, (const GLfloat *v))
QGLPROC(glNormal3i, void, (GLint nx, GLint ny, GLint nz))
QGLPROC(glNormal3iv, void, (const GLint *v))
QGLPROC(glNormal3s, void, (GLshort nx, GLshort ny, GLshort nz))
QGLPROC(glNormal3sv, void, (const GLshort *v))
QGLPROC(glNormalPointer, void, (GLenum type, GLsizei stride, const GLvoid *pointer))
QGLPROC(glOrtho, void, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar))
QGLPROC(glPassThrough, void, (GLfloat token))
QGLPROC(glPixelMapfv, void, (GLenum map, GLsizei mapsize, const GLfloat *values))
QGLPROC(glPixelMapuiv, void, (GLenum map, GLsizei mapsize, const GLuint *values))
QGLPROC(glPixelMapusv, void, (GLenum map, GLsizei mapsize, const GLushort *values))
QGLPROC(glPixelStoref, void, (GLenum pname, GLfloat param))
QGLPROC(glPixelStorei, void, (GLenum pname, GLint param))
QGLPROC(glPixelTransferf, void, (GLenum pname, GLfloat param))
QGLPROC(glPixelTransferi, void, (GLenum pname, GLint param))
QGLPROC(glPixelZoom, void, (GLfloat xfactor, GLfloat yfactor))
QGLPROC(glPointSize, void, (GLfloat size))
QGLPROC(glPolygonMode, void, (GLenum face, GLenum mode))
QGLPROC(glPolygonOffset, void, (GLfloat factor, GLfloat units))
QGLPROC(glPolygonStipple, void, (const GLubyte *mask))
QGLPROC(glPopAttrib, void, (void))
QGLPROC(glPopClientAttrib, void, (void))
QGLPROC(glPopMatrix, void, (void))
QGLPROC(glPopName, void, (void))
QGLPROC(glPrioritizeTextures, void, (GLsizei n, const GLuint *textures, const GLclampf *priorities))
QGLPROC(glPushAttrib, void, (GLbitfield mask))
QGLPROC(glPushClientAttrib, void, (GLbitfield mask))
QGLPROC(glPushMatrix, void, (void))
QGLPROC(glPushName, void, (GLuint name))
QGLPROC(glRasterPos2d, void, (GLdouble x, GLdouble y))
QGLPROC(glRasterPos2dv, void, (const GLdouble *v))
QGLPROC(glRasterPos2f, void, (GLfloat x, GLfloat y))
QGLPROC(glRasterPos2fv, void, (const GLfloat *v))
QGLPROC(glRasterPos2i, void, (GLint x, GLint y))
QGLPROC(glRasterPos2iv, void, (const GLint *v))
QGLPROC(glRasterPos2s, void, (GLshort x, GLshort y))
QGLPROC(glRasterPos2sv, void, (const GLshort *v))
QGLPROC(glRasterPos3d, void, (GLdouble x, GLdouble y, GLdouble z))
QGLPROC(glRasterPos3dv, void, (const GLdouble *v))
QGLPROC(glRasterPos3f, void, (GLfloat x, GLfloat y, GLfloat z))
QGLPROC(glRasterPos3fv, void, (const GLfloat *v))
QGLPROC(glRasterPos3i, void, (GLint x, GLint y, GLint z))
QGLPROC(glRasterPos3iv, void, (const GLint *v))
QGLPROC(glRasterPos3s, void, (GLshort x, GLshort y, GLshort z))
QGLPROC(glRasterPos3sv, void, (const GLshort *v))
QGLPROC(glRasterPos4d, void, (GLdouble x, GLdouble y, GLdouble z, GLdouble w))
QGLPROC(glRasterPos4dv, void, (const GLdouble *v))
QGLPROC(glRasterPos4f, void, (GLfloat x, GLfloat y, GLfloat z, GLfloat w))
QGLPROC(glRasterPos4fv, void, (const GLfloat *v))
QGLPROC(glRasterPos4i, void, (GLint x, GLint y, GLint z, GLint w))
QGLPROC(glRasterPos4iv, void, (const GLint *v))
QGLPROC(glRasterPos4s, void, (GLshort x, GLshort y, GLshort z, GLshort w))
QGLPROC(glRasterPos4sv, void, (const GLshort *v))
QGLPROC(glReadBuffer, void, (GLenum mode))
QGLPROC(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels))
QGLPROC(glRectd, void, (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2))
QGLPROC(glRectdv, void, (const GLdouble *v1, const GLdouble *v2))
QGLPROC(glRectf, void, (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2))
QGLPROC(glRectfv, void, (const GLfloat *v1, const GLfloat *v2))
QGLPROC(glRecti, void, (GLint x1, GLint y1, GLint x2, GLint y2))
QGLPROC(glRectiv, void, (const GLint *v1, const GLint *v2))
QGLPROC(glRects, void, (GLshort x1, GLshort y1, GLshort x2, GLshort y2))
QGLPROC(glRectsv, void, (const GLshort *v1, const GLshort *v2))
QGLPROC(glRenderMode, GLint, (GLenum mode))
QGLPROC(glRotated, void, (GLdouble angle, GLdouble x, GLdouble y, GLdouble z))
QGLPROC(glRotatef, void, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z))
QGLPROC(glScaled, void, (GLdouble x, GLdouble y, GLdouble z))
QGLPROC(glScalef, void, (GLfloat x, GLfloat y, GLfloat z))
QGLPROC(glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height))
QGLPROC(glSelectBuffer, void, (GLsizei size, GLuint *buffer))
QGLPROC(glShadeModel, void, (GLenum mode))
QGLPROC(glStencilFunc, void, (GLenum func, GLint ref, GLuint mask))
QGLPROC(glStencilMask, void, (GLuint mask))
QGLPROC(glStencilOp, void, (GLenum fail, GLenum zfail, GLenum zpass))
QGLPROC(glTexCoord1d, void, (GLdouble s))
QGLPROC(glTexCoord1dv, void, (const GLdouble *v))
QGLPROC(glTexCoord1f, void, (GLfloat s))
QGLPROC(glTexCoord1fv, void, (const GLfloat *v))
QGLPROC(glTexCoord1i, void, (GLint s))
QGLPROC(glTexCoord1iv, void, (const GLint *v))
QGLPROC(glTexCoord1s, void, (GLshort s))
QGLPROC(glTexCoord1sv, void, (const GLshort *v))
QGLPROC(glTexCoord2d, void, (GLdouble s, GLdouble t))
QGLPROC(glTexCoord2dv, void, (const GLdouble *v))
QGLPROC(glTexCoord2f, void, (GLfloat s, GLfloat t))
QGLPROC(glTexCoord2fv, void, (const GLfloat *v))
QGLPROC(glTexCoord2i, void, (GLint s, GLint t))
QGLPROC(glTexCoord2iv, void, (const GLint *v))
QGLPROC(glTexCoord2s, void, (GLshort s, GLshort t))
QGLPROC(glTexCoord2sv, void, (const GLshort *v))
QGLPROC(glTexCoord3d, void, (GLdouble s, GLdouble t, GLdouble r))
QGLPROC(glTexCoord3dv, void, (const GLdouble *v))
QGLPROC(glTexCoord3f, void, (GLfloat s, GLfloat t, GLfloat r))
QGLPROC(glTexCoord3fv, void, (const GLfloat *v))
QGLPROC(glTexCoord3i, void, (GLint s, GLint t, GLint r))
QGLPROC(glTexCoord3iv, void, (const GLint *v))
QGLPROC(glTexCoord3s, void, (GLshort s, GLshort t, GLshort r))
QGLPROC(glTexCoord3sv, void, (const GLshort *v))
QGLPROC(glTexCoord4d, void, (GLdouble s, GLdouble t, GLdouble r, GLdouble q))
QGLPROC(glTexCoord4dv, void, (const GLdouble *v))
QGLPROC(glTexCoord4f, void, (GLfloat s, GLfloat t, GLfloat r, GLfloat q))
QGLPROC(glTexCoord4fv, void, (const GLfloat *v))
QGLPROC(glTexCoord4i, void, (GLint s, GLint t, GLint r, GLint q))
QGLPROC(glTexCoord4iv, void, (const GLint *v))
QGLPROC(glTexCoord4s, void, (GLshort s, GLshort t, GLshort r, GLshort q))
QGLPROC(glTexCoord4sv, void, (const GLshort *v))
QGLPROC(glTexCoordPointer, void, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
QGLPROC(glTexEnvf, void, (GLenum target, GLenum pname, GLfloat param))
QGLPROC(glTexEnvfv, void, (GLenum target, GLenum pname, const GLfloat *params))
QGLPROC(glTexEnvi, void, (GLenum target, GLenum pname, GLint param))
QGLPROC(glTexEnviv, void, (GLenum target, GLenum pname, const GLint *params))
QGLPROC(glTexGend, void, (GLenum coord, GLenum pname, GLdouble param))
QGLPROC(glTexGendv, void, (GLenum coord, GLenum pname, const GLdouble *params))
QGLPROC(glTexGenf, void, (GLenum coord, GLenum pname, GLfloat param))
QGLPROC(glTexGenfv, void, (GLenum coord, GLenum pname, const GLfloat *params))
QGLPROC(glTexGeni, void, (GLenum coord, GLenum pname, GLint param))
QGLPROC(glTexGeniv, void, (GLenum coord, GLenum pname, const GLint *params))
QGLPROC(glTexImage1D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels))
QGLPROC(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels))
QGLPROC(glTexParameterf, void, (GLenum target, GLenum pname, GLfloat param))
QGLPROC(glTexParameterfv, void, (GLenum target, GLenum pname, const GLfloat *params))
QGLPROC(glTexParameteri, void, (GLenum target, GLenum pname, GLint param))
QGLPROC(glTexParameteriv, void, (GLenum target, GLenum pname, const GLint *params))
QGLPROC(glTexSubImage1D, void, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels))
QGLPROC(glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels))
QGLPROC(glTranslated, void, (GLdouble x, GLdouble y, GLdouble z))
QGLPROC(glTranslatef, void, (GLfloat x, GLfloat y, GLfloat z))
QGLPROC(glUseProgram, void,(GLuint program))
QGLPROC(glVertex2d, void, (GLdouble x, GLdouble y))
QGLPROC(glVertex2dv, void, (const GLdouble *v))
QGLPROC(glVertex2f, void, (GLfloat x, GLfloat y))
QGLPROC(glVertex2fv, void, (const GLfloat *v))
QGLPROC(glVertex2i, void, (GLint x, GLint y))
QGLPROC(glVertex2iv, void, (const GLint *v))
QGLPROC(glVertex2s, void, (GLshort x, GLshort y))
QGLPROC(glVertex2sv, void, (const GLshort *v))
QGLPROC(glVertex3d, void, (GLdouble x, GLdouble y, GLdouble z))
QGLPROC(glVertex3dv, void, (const GLdouble *v))
QGLPROC(glVertex3f, void, (GLfloat x, GLfloat y, GLfloat z))
QGLPROC(glVertex3fv, void, (const GLfloat *v))
QGLPROC(glVertex3i, void, (GLint x, GLint y, GLint z))
QGLPROC(glVertex3iv, void, (const GLint *v))
QGLPROC(glVertex3s, void, (GLshort x, GLshort y, GLshort z))
QGLPROC(glVertex3sv, void, (const GLshort *v))
QGLPROC(glVertex4d, void, (GLdouble x, GLdouble y, GLdouble z, GLdouble w))
QGLPROC(glVertex4dv, void, (const GLdouble *v))
QGLPROC(glVertex4f, void, (GLfloat x, GLfloat y, GLfloat z, GLfloat w))
QGLPROC(glVertex4fv, void, (const GLfloat *v))
QGLPROC(glVertex4i, void, (GLint x, GLint y, GLint z, GLint w))
QGLPROC(glVertex4iv, void, (const GLint *v))
QGLPROC(glVertex4s, void, (GLshort x, GLshort y, GLshort z, GLshort w))
QGLPROC(glVertex4sv, void, (const GLshort *v))
QGLPROC(glVertexPointer, void, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
QGLPROC(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height))

#undef QGLPROC
