/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
******************************************************************************/

#ifndef __MATH_SIMD_AVX_H__
#define __MATH_SIMD_AVX_H__

#include "idlib/math/Simd_SSE3.h"

/*
===============================================================================

	AVX implementation of idSIMDProcessor

===============================================================================
*/

class idSIMD_AVX : public idSIMD_SSE3 {
public:
	// Revelator: these work whether in gcc clang or msvc x86 or x64 (no inline assembly used)
	virtual const char *VPCALL GetName( void ) const;
	virtual void VPCALL CullByFrustum( idDrawVert *verts, const int numVerts, const idPlane frustum[6], byte *pointCull, float epsilon );
	virtual void VPCALL CullByFrustum2( idDrawVert *verts, const int numVerts, const idPlane frustum[6], unsigned short *pointCull, float epsilon );
};

#endif /* !__MATH_SIMD_AVX_H__ */
