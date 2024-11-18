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
#include "Simd_AVX.h"

//===============================================================
//
//	AVX implementation of idSIMDProcessor
//
//===============================================================

// Revelator: these work whether in gcc clang or msvc in x86 or x64 (no inline assembly used)
#if defined(_MSC_VER) && ( defined(_M_X64) || defined(_M_IX86) ) || \
	defined(__GNUC__) && ( defined(__i386__) || defined (__x86_64__) ) && defined(__AVX__)

#include <immintrin.h>

#include "idlib/geometry/DrawVert.h"
#include "idlib/geometry/JointTransform.h"
#include "idlib/math/Vector.h"
#include "idlib/math/Plane.h"

/*
============
idSIMD_AVX::GetName
============
*/
const char *idSIMD_AVX::GetName( void ) const {
	return "MMX & SSE & SSE2 & SSE3 & AVX";
}

/*
============
idSIMD_AVX::CullByFrustum
============
*/
void VPCALL idSIMD_AVX::CullByFrustum( idDrawVert *verts, const int numVerts, const idPlane frustum[6], byte *pointCull, float epsilon ) {
	const __m256 fA = _mm256_set_ps( 0, 0, frustum[5][0], frustum[4][0], frustum[3][0], frustum[2][0], frustum[1][0], frustum[0][0] );
	const __m256 fB = _mm256_set_ps( 0, 0, frustum[5][1], frustum[4][1], frustum[3][1], frustum[2][1], frustum[1][1], frustum[0][1] );
	const __m256 fC = _mm256_set_ps( 0, 0, frustum[5][2], frustum[4][2], frustum[3][2], frustum[2][2], frustum[1][2], frustum[0][2] );
	const __m256 fD = _mm256_set_ps( 0, 0, frustum[5][3], frustum[4][3], frustum[3][3], frustum[2][3], frustum[1][3], frustum[0][3] );
	const __m256 eps = _mm256_set1_ps( epsilon );
	const byte mask6 = ( 1 << 6 ) - 1;

	for( int j = 0; j < numVerts; j++ ) {
		idVec3 &vec = verts[j].xyz;
		__m256 vX = _mm256_set1_ps( vec.x );
		__m256 vY = _mm256_set1_ps( vec.y );
		__m256 vZ = _mm256_set1_ps( vec.z );
		__m256 d = 
		_mm256_add_ps(
		    _mm256_add_ps(
		        _mm256_mul_ps( fA, vX ),
		        _mm256_mul_ps( fB, vY )
		    ),
		    _mm256_add_ps(
		        _mm256_mul_ps( fC, vZ ),
		        fD
		    )
		);
		int mask_lo = _mm256_movemask_ps( _mm256_cmp_ps( d, eps, _CMP_LT_OQ ) );
		pointCull[j] = ( byte )mask_lo & mask6;
	}
	_mm256_zeroupper();
}

/*
============
idSIMD_AVX::CullByFrustum2
============
*/
void VPCALL idSIMD_AVX::CullByFrustum2( idDrawVert *verts, const int numVerts, const idPlane frustum[6], unsigned short *pointCull, float epsilon ) {
	const __m256 fA = _mm256_set_ps( 0, 0, frustum[5][0], frustum[4][0], frustum[3][0], frustum[2][0], frustum[1][0], frustum[0][0] );
	const __m256 fB = _mm256_set_ps( 0, 0, frustum[5][1], frustum[4][1], frustum[3][1], frustum[2][1], frustum[1][1], frustum[0][1] );
	const __m256 fC = _mm256_set_ps( 0, 0, frustum[5][2], frustum[4][2], frustum[3][2], frustum[2][2], frustum[1][2], frustum[0][2] );
	const __m256 fD = _mm256_set_ps( 0, 0, frustum[5][3], frustum[4][3], frustum[3][3], frustum[2][3], frustum[1][3], frustum[0][3] );
	const __m256 eps = _mm256_set1_ps( epsilon );
	static const __m256 epsM = _mm256_set1_ps( -epsilon );
	const short mask6 = ( 1 << 6 ) - 1;

	for( int j = 0; j < numVerts; j++ ) {
		idVec3 &vec = verts[j].xyz;
		__m256 vX = _mm256_set1_ps( vec.x );
		__m256 vY = _mm256_set1_ps( vec.y );
		__m256 vZ = _mm256_set1_ps( vec.z );
		__m256 d = 
		_mm256_add_ps(
		    _mm256_add_ps(
		        _mm256_mul_ps( fA, vX ),
		        _mm256_mul_ps( fB, vY )
		    ),
		    _mm256_add_ps(
		        _mm256_mul_ps( fC, vZ ),
		        fD
		    )
		);
		int mask_lo = _mm256_movemask_ps( _mm256_cmp_ps( d, eps, _CMP_LT_OQ ) );
		int mask_hi = _mm256_movemask_ps( _mm256_cmp_ps( d, eps, _CMP_GT_OQ ) );
		pointCull[j] = ( unsigned short )( mask_lo & mask6 | ( mask_hi & mask6 ) << 6 );
	}
	_mm256_zeroupper();
}

#endif  /* _MSC_VER */
