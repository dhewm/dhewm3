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

#include "sys/win32/win_local.h"

/*
===============================================================================

	FPU

===============================================================================
*/

typedef struct bitFlag_s {
	const char *name;
	int			bit;
} bitFlag_t;

static byte fpuState[128], *statePtr = fpuState;
static char fpuString[2048];
static bitFlag_t controlWordFlags[] = {
	{ "Invalid operation", 0 },
	{ "Denormalized operand", 1 },
	{ "Divide-by-zero", 2 },
	{ "Numeric overflow", 3 },
	{ "Numeric underflow", 4 },
	{ "Inexact result (precision)", 5 },
	{ "Infinity control", 12 },
	{ "", 0 }
};
static const char *precisionControlField[] = {
	"Single Precision (24-bits)",
	"Reserved",
	"Double Precision (53-bits)",
	"Double Extended Precision (64-bits)"
};
static const char *roundingControlField[] = {
	"Round to nearest",
	"Round down",
	"Round up",
	"Round toward zero"
};
static bitFlag_t statusWordFlags[] = {
	{ "Invalid operation", 0 },
	{ "Denormalized operand", 1 },
	{ "Divide-by-zero", 2 },
	{ "Numeric overflow", 3 },
	{ "Numeric underflow", 4 },
	{ "Inexact result (precision)", 5 },
	{ "Stack fault", 6 },
	{ "Error summary status", 7 },
	{ "FPU busy", 15 },
	{ "", 0 }
};

/*
===============
Sys_FPU_PrintStateFlags
===============
*/
int Sys_FPU_PrintStateFlags( char *ptr, int ctrl, int stat, int tags, int inof, int inse, int opof, int opse ) {
#ifdef _MSC_VER
	int i, length = 0;

	length += sprintf( ptr+length,	"CTRL = %08x\n"
									"STAT = %08x\n"
									"TAGS = %08x\n"
									"INOF = %08x\n"
									"INSE = %08x\n"
									"OPOF = %08x\n"
									"OPSE = %08x\n"
									"\n",
									ctrl, stat, tags, inof, inse, opof, opse );

	length += sprintf( ptr+length, "Control Word:\n" );
	for ( i = 0; controlWordFlags[i].name[0]; i++ ) {
		length += sprintf( ptr+length, "  %-30s = %s\n", controlWordFlags[i].name, ( ctrl & ( 1 << controlWordFlags[i].bit ) ) ? "true" : "false" );
	}
	length += sprintf( ptr+length, "  %-30s = %s\n", "Precision control", precisionControlField[(ctrl>>8)&3] );
	length += sprintf( ptr+length, "  %-30s = %s\n", "Rounding control", roundingControlField[(ctrl>>10)&3] );

	length += sprintf( ptr+length, "Status Word:\n" );
	for ( i = 0; statusWordFlags[i].name[0]; i++ ) {
		ptr += sprintf( ptr+length, "  %-30s = %s\n", statusWordFlags[i].name, ( stat & ( 1 << statusWordFlags[i].bit ) ) ? "true" : "false" );
	}
	length += sprintf( ptr+length, "  %-30s = %d%d%d%d\n", "Condition code", (stat>>8)&1, (stat>>9)&1, (stat>>10)&1, (stat>>14)&1 );
	length += sprintf( ptr+length, "  %-30s = %d\n", "Top of stack pointer", (stat>>11)&7 );

	return length;
#else
	return 0;
#endif
}

/*
===============
Sys_FPU_StackIsEmpty
===============
*/
bool Sys_FPU_StackIsEmpty( void ) {
#ifdef _MSC_VER
	__asm {
		mov			eax, statePtr
		fnstenv		[eax]
		mov			eax, [eax+8]
		xor			eax, 0xFFFFFFFF
		and			eax, 0x0000FFFF
		jz			empty
	}
	return false;
empty:
	return true;
#else
	return true;
#endif
}

/*
===============
Sys_FPU_GetState

  gets the FPU state without changing the state
===============
*/
const char *Sys_FPU_GetState( void ) {
#ifdef _MSC_VER
	double fpuStack[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double *fpuStackPtr = fpuStack;
	int i, numValues;
	char *ptr;

	__asm {
		mov			esi, statePtr
		mov			edi, fpuStackPtr
		fnstenv		[esi]
		mov			esi, [esi+8]
		xor			esi, 0xFFFFFFFF
		mov			edx, (3<<14)
		xor			eax, eax
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fst			qword ptr [edi+0]
		inc			eax
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(1)
		fst			qword ptr [edi+8]
		inc			eax
		fxch		st(1)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(2)
		fst			qword ptr [edi+16]
		inc			eax
		fxch		st(2)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(3)
		fst			qword ptr [edi+24]
		inc			eax
		fxch		st(3)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(4)
		fst			qword ptr [edi+32]
		inc			eax
		fxch		st(4)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(5)
		fst			qword ptr [edi+40]
		inc			eax
		fxch		st(5)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(6)
		fst			qword ptr [edi+48]
		inc			eax
		fxch		st(6)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(7)
		fst			qword ptr [edi+56]
		inc			eax
		fxch		st(7)
	done:
		mov			numValues, eax
	}

	int ctrl = *(int *)&fpuState[0];
	int stat = *(int *)&fpuState[4];
	int tags = *(int *)&fpuState[8];
	int inof = *(int *)&fpuState[12];
	int inse = *(int *)&fpuState[16];
	int opof = *(int *)&fpuState[20];
	int opse = *(int *)&fpuState[24];

	ptr = fpuString;
	ptr += sprintf( ptr,"FPU State:\n"
						"num values on stack = %d\n", numValues );
	for ( i = 0; i < 8; i++ ) {
		ptr += sprintf( ptr, "ST%d = %1.10e\n", i, fpuStack[i] );
	}

	Sys_FPU_PrintStateFlags( ptr, ctrl, stat, tags, inof, inse, opof, opse );

	return fpuString;
#else
	return "";
#endif
}

/*
===============
Sys_FPU_EnableExceptions
===============
*/
void Sys_FPU_EnableExceptions( int exceptions ) {
#ifdef _MSC_VER
	__asm {
		mov			eax, statePtr
		mov			ecx, exceptions
		and			cx, 63
		not			cx
		fnstcw		word ptr [eax]
		mov			bx, word ptr [eax]
		or			bx, 63
		and			bx, cx
		mov			word ptr [eax], bx
		fldcw		word ptr [eax]
	}
#endif
}

/*
===============
Sys_FPU_SetPrecision
===============
*/
void Sys_FPU_SetPrecision( int precision ) {
#ifdef _MSC_VER
	short precisionBitTable[4] = { 0, 1, 3, 0 };
	short precisionBits = precisionBitTable[precision & 3] << 8;
	short precisionMask = ~( ( 1 << 9 ) | ( 1 << 8 ) );

	__asm {
		mov			eax, statePtr
		mov			cx, precisionBits
		fnstcw		word ptr [eax]
		mov			bx, word ptr [eax]
		and			bx, precisionMask
		or			bx, cx
		mov			word ptr [eax], bx
		fldcw		word ptr [eax]
	}
#endif
}

/*
================
Sys_FPU_SetRounding
================
*/
void Sys_FPU_SetRounding( int rounding ) {
#ifdef _MSC_VER
	short roundingBitTable[4] = { 0, 1, 2, 3 };
	short roundingBits = roundingBitTable[rounding & 3] << 10;
	short roundingMask = ~( ( 1 << 11 ) | ( 1 << 10 ) );

	__asm {
		mov			eax, statePtr
		mov			cx, roundingBits
		fnstcw		word ptr [eax]
		mov			bx, word ptr [eax]
		and			bx, roundingMask
		or			bx, cx
		mov			word ptr [eax], bx
		fldcw		word ptr [eax]
	}
#endif
}
