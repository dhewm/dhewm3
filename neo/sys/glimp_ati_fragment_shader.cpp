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
#include "framework/Common.h"

#include "sys/glimp_ati_fragment_shader.h"

static GLuint sGeneratingProgram = 0;
static int sCurrentPass;
static char sConstString[4096];
static char sPassString[2][4096];
static int sOpUsed;
static int sConstUsed;
static int sConst[8];
static GLfloat sConstVal[8][4];

static void _endPass (void) {
	if (!sOpUsed) return;
	sOpUsed = 0;
	sCurrentPass ++;
}

static GLuint glGenFragmentShadersATI (GLuint ID) {
	qglGenProgramsARB(1, &ID);
	return ID;
}

static void glBindFragmentShaderATI (GLuint ID) {
	qglBindProgramARB(GL_TEXT_FRAGMENT_SHADER_ATI, ID);
}

static void glDeleteFragmentShaderATI (GLuint ID) {
//	qglDeleteProgramsARB(1, &ID);
}

static void glBeginFragmentShaderATI (void) {
	int i;

	sConstString[0] = 0;
	for (i = 0; i < 8; i ++)
		sConst[i] = 0;

	sOpUsed = 0;
	sPassString[0][0] = 0;
	sPassString[1][0] = 0;

	sCurrentPass = 0;
	sGeneratingProgram = 1;
}

static void glEndFragmentShaderATI (void) {
	GLint errPos;
	int i;
	char fragString[4096];

	sGeneratingProgram = 0;

	// header
	strcpy(fragString, "!!ATIfs1.0\n");

	// constants
	if (sConstString[0] || sConstUsed) {
		strcat (fragString, "StartConstants;\n");
		if (sConstUsed) {
			for (i = 0; i < 8; i ++) {
				if (sConst[i] == 1) {
					char str[128];
					sprintf (str, "    CONSTANT c%d = program.env[%d];\n", i, i);
					strcat (fragString, str);
				}
			}
		}
		if (sConstString[0]) {
			strcat (fragString, sConstString);
		}
		strcat (fragString, "EndConstants;\n\n");
	}

	if (sCurrentPass == 0) {
		strcat(fragString, "StartOutputPass;\n");
		strcat(fragString, sPassString[0]);
		strcat(fragString, "EndPass;\n");
	} else {
		strcat(fragString, "StartPrelimPass;\n");
		strcat(fragString, sPassString[0]);
		strcat(fragString, "EndPass;\n\n");

		strcat(fragString, "StartOutputPass;\n");
		strcat(fragString, sPassString[1]);
		strcat(fragString, "EndPass;\n");
	}

	qglProgramStringARB(GL_TEXT_FRAGMENT_SHADER_ATI, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(fragString), fragString);
	qglGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errPos );
	if(errPos != -1) {
		const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		common->Warning("WARNING: glError at %d:%s when compiling atiFragmentShader %s", errPos, errString, fragString);
	}
}


static void glSetFragmentShaderConstantATI (GLuint num, const GLfloat *val) {
	int constNum = num-GL_CON_0_ATI;
	if (sGeneratingProgram) {
		char str[128];
		sprintf (str, "    CONSTANT c%d = { %f, %f, %f, %f };\n", constNum, val[0], val[1], val[2], val[3]);
		strcat (sConstString, str);
		sConst[constNum] = 2;
	}
	else {
		// According to Duane, frequent setting of fragment shader constants, even if they contain
		// the same value, will cause a performance hit.
		// According to Chris Bentley at ATI, this performance hit appears if you are using
		// many different fragment shaders in each scene.
		// So, we cache those values and only set the constants if they are different.
		if (memcmp (val, sConstVal[constNum], sizeof(GLfloat)*8*4) != 0)
		{
			qglProgramEnvParameter4fvARB (GL_TEXT_FRAGMENT_SHADER_ATI, num-GL_CON_0_ATI, val);
			memcpy (sConstVal[constNum], val, sizeof(GLfloat)*8*4);
		}
	}
}

static char *makeArgStr(GLuint arg) {
	// Since we return "str", it needs to be static to ensure that it retains
	// its value outside this routine.
	static char str[128];

	strcpy (str, "");

	if ( arg >= GL_REG_0_ATI && arg <= GL_REG_5_ATI ) {
		sprintf(str, "r%d", arg - GL_REG_0_ATI);
	} else if(arg >= GL_CON_0_ATI && arg <= GL_CON_7_ATI) {
		if(!sConst[arg - GL_CON_0_ATI]) {
			sConstUsed = 1;
			sConst[arg - GL_CON_0_ATI] = 1;
		}
		sprintf(str, "c%d", arg - GL_CON_0_ATI);
	} else if( arg >= GL_TEXTURE0_ARB && arg <= GL_TEXTURE31_ARB ) {
		sprintf(str, "t%d", arg - GL_TEXTURE0_ARB);
	} else if( arg == GL_PRIMARY_COLOR_ARB ) {
		strcpy(str, "color0");
	} else if(arg == GL_SECONDARY_INTERPOLATOR_ATI) {
		strcpy(str, "color1");
	} else if (arg == GL_ZERO) {
		strcpy(str, "0");
	} else if (arg == GL_ONE) {
		strcpy(str, "1");
	} else {
		common->Warning("makeArgStr: bad arg value\n");
	}
	return str;
}

static void glPassTexCoordATI (GLuint dst, GLuint coord, GLenum swizzle) {
	char str[128] = "\0";
	_endPass();

	switch(swizzle) {
		case GL_SWIZZLE_STR_ATI:
			sprintf(str, "    PassTexCoord r%d, %s.str;\n", dst - GL_REG_0_ATI, makeArgStr(coord));
			break;
		case GL_SWIZZLE_STQ_ATI:
			sprintf(str, "    PassTexCoord r%d, %s.stq;\n", dst - GL_REG_0_ATI, makeArgStr(coord));
			break;
		case GL_SWIZZLE_STR_DR_ATI:
			sprintf(str, "    PassTexCoord r%d, %s.str_dr;\n", dst - GL_REG_0_ATI, makeArgStr(coord));
			break;
		case GL_SWIZZLE_STQ_DQ_ATI:
			sprintf(str, "    PassTexCoord r%d, %s.stq_dq;\n", dst - GL_REG_0_ATI, makeArgStr(coord));
			break;
		default:
			common->Warning("glPassTexCoordATI invalid swizzle;");
			break;
	}
	strcat(sPassString[sCurrentPass], str);
}

void glSampleMapATI (GLuint dst, GLuint interp, GLenum swizzle) {
	char str[128] = "\0";
	_endPass();

	switch(swizzle) {
		case GL_SWIZZLE_STR_ATI:
			sprintf(str, "    SampleMap r%d, %s.str;\n", dst - GL_REG_0_ATI, makeArgStr(interp));
			break;
		case GL_SWIZZLE_STQ_ATI:
			sprintf(str, "    SampleMap r%d, %s.stq;\n", dst - GL_REG_0_ATI, makeArgStr(interp));
			break;
		case GL_SWIZZLE_STR_DR_ATI:
			sprintf(str, "    SampleMap r%d, %s.str_dr;\n", dst - GL_REG_0_ATI, makeArgStr(interp));
			break;
		case GL_SWIZZLE_STQ_DQ_ATI:
			sprintf(str, "    SampleMap r%d, %s.stq_dq;\n", dst - GL_REG_0_ATI, makeArgStr(interp));
			break;
		default:
			common->Warning("glSampleMapATI invalid swizzle;");
			break;
	}
	strcat(sPassString[sCurrentPass], str);
}

static char *makeMaskStr(GLuint mask) {
	// Since we return "str", it needs to be static to ensure that it retains
	// its value outside this routine.
	static char str[128];

	strcpy (str, "");

	switch (mask) {
		case GL_NONE:
			str[0] = '\0';
			break;
		case GL_RGBA:
			strcpy(str, ".rgba");
			break;
		case GL_RGB:
			strcpy(str, ".rgb");
			break;
		case GL_RED:
			strcpy(str, ".r");
			break;
		case GL_GREEN:
			strcpy(str, ".g");
			break;
		case GL_BLUE:
			strcpy(str, ".b");
			break;
		case GL_ALPHA:
			strcpy(str, ".a");
			break;
		default:
			strcpy(str, ".");
			if( mask & GL_RED_BIT_ATI )
				strcat(str, "r");
			if( mask & GL_GREEN_BIT_ATI )
				strcat(str, "g");
			if( mask & GL_BLUE_BIT_ATI )
				strcat(str, "b");
			break;
	}

	return str;
}

static char *makeDstModStr(GLuint mod) {
	// Since we return "str", it needs to be static to ensure that it retains
	// its value outside this routine.
	static char str[128];

	strcpy (str, "");

	if( mod == GL_NONE) {
		str[0] = '\0';
		return str;
	}
	if( mod & GL_2X_BIT_ATI) {
		strcat(str, ".2x");
	}

	if( mod & GL_4X_BIT_ATI) {
		strcat(str, ".4x");
	}

	if( mod & GL_8X_BIT_ATI) {
		strcat(str, ".8x");
	}

	if( mod & GL_SATURATE_BIT_ATI) {
		strcat(str, ".sat");
	}

	if( mod & GL_HALF_BIT_ATI) {
		strcat(str, ".half");
	}

	if( mod & GL_QUARTER_BIT_ATI) {
		strcat(str, ".quarter");
	}

	if( mod & GL_EIGHTH_BIT_ATI) {
		strcat(str, ".eighth");
	}

	return str;
}

static char *makeArgModStr(GLuint mod) {
	// Since we return "str", it needs to be static to ensure that it retains
	// its value outside this routine.
	static char str[128];

	strcpy (str, "");

	if( mod == GL_NONE) {
		str[0] = '\0';
		return str;
	}
	if( mod & GL_NEGATE_BIT_ATI) {
		strcat(str, ".neg");
	}

	if( mod & GL_2X_BIT_ATI) {
		strcat(str, ".2x");
	}

	if( mod & GL_BIAS_BIT_ATI) {
		strcat(str, ".bias");
	}

	if( mod & GL_COMP_BIT_ATI) {
		strcat(str, ".comp");
	}

	return str;
}

static void glColorFragmentOp1ATI (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod) {
	char str[128] = "\0";

	sOpUsed = 1;

	switch(op) {
		// Unary operators
		case GL_MOV_ATI:
			sprintf(str, "    MOV r%d", dst - GL_REG_0_ATI);
			break;
		default:
			common->Warning("glColorFragmentOp1ATI invalid op;\n");
			break;
	}
	if(dstMask != GL_NONE)  {
		strcat(str, makeMaskStr(dstMask));
	}
	else {
		strcat(str, ".rgb" );
	}

	if(dstMod != GL_NONE) {
		strcat(str, makeDstModStr(dstMod));
	}
	strcat(str, ", ");

	strcat(str, makeArgStr(arg1));
	if(arg1Rep != GL_NONE)  {
		strcat(str, makeMaskStr(arg1Rep));
	}
	if(arg1Mod != GL_NONE) {
		strcat(str, makeArgModStr(arg1Mod));
	}
	strcat(str, ";\n");

	strcat(sPassString[sCurrentPass], str);
}

static void glColorFragmentOp2ATI (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod) {
	char str[128] = "\0";

	if (!sOpUsed)
		sprintf(str,"\n");
	sOpUsed = 1;

	switch(op) {
		// Unary operators - fall back to Op1 routine.
		case GL_MOV_ATI:
			glColorFragmentOp1ATI(op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod);
			return;

		// Binary operators
		case GL_ADD_ATI:
			sprintf(str, "    ADD r%d", dst - GL_REG_0_ATI);
			break;
		case GL_MUL_ATI:
			sprintf(str, "    MUL r%d", dst - GL_REG_0_ATI);
			break;
		case GL_SUB_ATI:
			sprintf(str, "    SUB r%d", dst - GL_REG_0_ATI);
			break;
		case GL_DOT3_ATI:
			sprintf(str, "    DOT3 r%d", dst - GL_REG_0_ATI);
			break;
		case GL_DOT4_ATI:
			sprintf(str, "    DOT4 r%d", dst - GL_REG_0_ATI);
			break;
		default:
			common->Warning("glColorFragmentOp2ATI invalid op;");
			break;
	}
	if(dstMask != GL_NONE)  {
		strcat(str, makeMaskStr(dstMask));
	}
	else {
		strcat(str, ".rgb" );
	}
	if(dstMod != GL_NONE) {
		strcat(str, makeDstModStr(dstMod));
	}
	strcat(str, ", ");

	strcat(str, makeArgStr(arg1));
//	if(arg1Rep != GL_NONE)
		strcat(str, makeMaskStr(arg1Rep));
	if(arg1Mod != GL_NONE) {
		strcat(str, makeArgModStr(arg1Mod));
	}
	strcat(str, ", ");

	strcat(str, makeArgStr(arg2));
//	if(arg2Rep != GL_NONE)
		strcat(str, makeMaskStr(arg2Rep));
	if(arg2Mod != GL_NONE) {
		strcat(str, makeArgModStr(arg2Mod));
	}
	strcat(str, ";\n");

	strcat(sPassString[sCurrentPass], str);
}

static void glColorFragmentOp3ATI (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod) {
	char str[128] = "\0";

	sOpUsed = 1;

	switch(op) {
		// Unary operators - fall back to Op1 routine.
		case GL_MOV_ATI:
			glColorFragmentOp1ATI(op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod);
			return;

		// Binary operators - fall back to Op2 routine.
		case GL_ADD_ATI:
		case GL_MUL_ATI:
		case GL_SUB_ATI:
		case GL_DOT3_ATI:
		case GL_DOT4_ATI:
			glColorFragmentOp2ATI(op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod);
			break;

		// Ternary operators
		case GL_MAD_ATI:
			sprintf(str, "    MAD r%d", dst - GL_REG_0_ATI);
			break;
		case GL_LERP_ATI:
			sprintf(str, "    LERP r%d", dst - GL_REG_0_ATI);
			break;
		case GL_CND_ATI:
			sprintf(str, "    CND r%d", dst - GL_REG_0_ATI);
			break;
		case GL_CND0_ATI:
			sprintf(str, "    CND0 r%d", dst - GL_REG_0_ATI);
			break;
		case GL_DOT2_ADD_ATI:
			sprintf(str, "    DOT2ADD r%d", dst - GL_REG_0_ATI);
			break;
		default:
			common->Warning("glColorFragmentOp3ATI invalid op;");
			break;
	}

	if(dstMask != GL_NONE)  {
		strcat(str, makeMaskStr(dstMask));
	}
	else {
		strcat(str, ".rgb" );
	}
	if(dstMod != GL_NONE) {
		strcat(str, makeDstModStr(dstMod));
	}
	strcat(str, ", ");

	strcat(str, makeArgStr(arg1));
	if(arg1Rep != GL_NONE)  {
		strcat(str, makeMaskStr(arg1Rep));
	}
	if(arg1Mod != GL_NONE) {
		strcat(str, makeArgModStr(arg1Mod));
	}
	strcat(str, ", ");

	strcat(str, makeArgStr(arg2));
	if(arg2Rep != GL_NONE)  {
		strcat(str, makeMaskStr(arg2Rep));
	}
	if(arg2Mod != GL_NONE) {
		strcat(str, makeArgModStr(arg2Mod));
	}
	strcat(str, ", ");

	strcat(str, makeArgStr(arg3));
	if(arg3Rep != GL_NONE)  {
		strcat(str, makeMaskStr(arg3Rep));
	}
	if(arg3Mod != GL_NONE) {
		strcat(str, makeArgModStr(arg3Mod));
	}
	strcat(str, ";\n");

	strcat(sPassString[sCurrentPass], str);
}

static void glAlphaFragmentOp1ATI (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod) {
	glColorFragmentOp1ATI ( op, dst, GL_ALPHA, dstMod, arg1, arg1Rep, arg1Mod);
}

static void glAlphaFragmentOp2ATI (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod) {
	glColorFragmentOp2ATI ( op, dst, GL_ALPHA, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod);
}

static void glAlphaFragmentOp3ATI (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod) {
	glColorFragmentOp3ATI ( op, dst, GL_ALPHA, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod);
}

GLExtension_t GLimp_ExtensionPointer_ATI_fragment_shader(const char *name) {
	if (!strcmp(name, "glGenFragmentShadersATI"))
		return (GLExtension_t)glGenFragmentShadersATI;
	if (!strcmp(name, "glBindFragmentShaderATI"))
		return (GLExtension_t)glBindFragmentShaderATI;
	if (!strcmp(name, "glDeleteFragmentShaderATI"))
		return (GLExtension_t)glDeleteFragmentShaderATI;
	if (!strcmp(name, "glBeginFragmentShaderATI"))
		return (GLExtension_t)glBeginFragmentShaderATI;
	if (!strcmp(name, "glEndFragmentShaderATI"))
		return (GLExtension_t)glEndFragmentShaderATI;
	if (!strcmp(name, "glPassTexCoordATI"))
		return (GLExtension_t)glPassTexCoordATI;
	if (!strcmp(name, "glSampleMapATI"))
		return (GLExtension_t)glSampleMapATI;
	if (!strcmp(name, "glColorFragmentOp1ATI"))
		return (GLExtension_t)glColorFragmentOp1ATI;
	if (!strcmp(name, "glColorFragmentOp2ATI"))
		return (GLExtension_t)glColorFragmentOp2ATI;
	if (!strcmp(name, "glColorFragmentOp3ATI"))
		return (GLExtension_t)glColorFragmentOp3ATI;
	if (!strcmp(name, "glAlphaFragmentOp1ATI"))
		return (GLExtension_t)glAlphaFragmentOp1ATI;
	if (!strcmp(name, "glAlphaFragmentOp2ATI"))
		return (GLExtension_t)glAlphaFragmentOp2ATI;
	if (!strcmp(name, "glAlphaFragmentOp3ATI"))
		return (GLExtension_t)glAlphaFragmentOp3ATI;
	if (!strcmp(name, "glSetFragmentShaderConstantATI"))
		return (GLExtension_t)glSetFragmentShaderConstantATI;

	return (GLExtension_t)NULL;
}
