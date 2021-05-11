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

#ifndef __MATH_SIMD_SSE2_H__
#define __MATH_SIMD_SSE2_H__

#include "idlib/math/Simd_SSE.h"

/*
===============================================================================

	SSE2 implementation of idSIMDProcessor

===============================================================================
*/

#ifdef __SSE2__
# include <emmintrin.h>
#endif


// DG: _CRT_ALIGN seems to be MSVC specific, so provide implementation..
#ifndef _CRT_ALIGN
#if defined(__GNUC__) // also applies for clang
#define _CRT_ALIGN(x) __attribute__ ((__aligned__ (x)))
#elif defined(_MSC_VER) // also for MSVC, just to be sure
#define _CRT_ALIGN(x) __declspec(align(x))
#endif
#endif

// DG: make sure __declspec(intrin_type) is only used on MSVC (it's not available on GCC etc
#ifdef _MSC_VER
#define DECLSPEC_INTRINTYPE __declspec( intrin_type )
#else
#define DECLSPEC_INTRINTYPE
#endif
// DG end

#if defined(__GNUC__) && defined(__SSE2__)
/* Helper Functions */
// make the intrinsics "type unsafe"
typedef union DECLSPEC_INTRINTYPE _CRT_ALIGN( 16 ) __m128c
{
	__m128c() {}
	__m128c( __m128 f )
	{
		m128 = f;
	}
	__m128c( __m128i i )
	{
		m128i = i;
	}
	operator	__m128()
	{
		return m128;
	}
	operator	__m128i()
	{
		return m128i;
	}
	__m128		m128;
	__m128i		m128i;
} __m128c;

/* Some Helper intrinsics */
#define _mm_madd_ps( a, b, c ) _mm_add_ps( _mm_mul_ps( (a), (b) ), (c) )
#define _mm_nmsub_ps( a, b, c ) _mm_sub_ps( (c), _mm_mul_ps( (a), (b) ) )
#define _mm_splat_ps( x, i ) __m128c( _mm_shuffle_epi32( __m128c( x ), _MM_SHUFFLE( i, i, i, i ) ) )
#define _mm_perm_ps( x, perm ) __m128c( _mm_shuffle_epi32( __m128c( x ), perm ) )
#define _mm_sel_ps( a, b, c ) _mm_or_ps( _mm_andnot_ps( __m128c( c ), a ), _mm_and_ps( __m128c( c ), b ) )
#define _mm_sel_si128( a, b, c ) _mm_or_si128( _mm_andnot_si128( __m128c( c ), a ), _mm_and_si128( __m128c( c ), b ) )


/*
================================================================================================

	Scalar single precision floating-point intrinsics

================================================================================================
*/

ID_INLINE float __fmuls( float a, float b )
{
	return ( a * b );
}
ID_INLINE float __fmadds( float a, float b, float c )
{
	return ( a * b + c );
}
ID_INLINE float __fnmsubs( float a, float b, float c )
{
	return ( c - a * b );
}
ID_INLINE float __fsels( float a, float b, float c )
{
	return ( a >= 0.0f ) ? b : c;
}
ID_INLINE float __frcps( float x )
{
	return ( 1.0f / x );
}
ID_INLINE float __fdivs( float x, float y )
{
	return ( x / y );
}
ID_INLINE float __frsqrts( float x )
{
	return ( 1.0f / sqrtf( x ) );
}
ID_INLINE float __frcps16( float x )
{
	return ( 1.0f / x );
}
ID_INLINE float __fdivs16( float x, float y )
{
	return ( x / y );
}
ID_INLINE float __frsqrts16( float x )
{
	return ( 1.0f / sqrtf( x ) );
}
ID_INLINE float __frndz( float x )
{
	return ( float )( ( int )( x ) );
}

#endif

class idSIMD_SSE2 : public idSIMD_SSE {
public:
#if defined(__GNUC__) && defined(__SSE2__)
	using idSIMD_SSE::CmpLT;

	virtual const char * VPCALL GetName( void ) const;
	virtual void VPCALL CmpLT( byte *dst,			const byte bitNum,		const float *src0,		const float constant,	const int count );
	virtual void VPCALL BlendJoints( idJointQuat *joints, const idJointQuat *blendJoints, const float lerp, const int *index, const int numJoints );
	virtual void VPCALL BlendJointsFast( idJointQuat *joints, const idJointQuat *blendJoints, const float lerp, const int *index, const int numJoints );
	virtual void VPCALL ConvertJointQuatsToJointMats( idJointMat *jointMats, const idJointQuat *jointQuats, const int numJoints );
	virtual void VPCALL ConvertJointMatsToJointQuats( idJointQuat *jointQuats, const idJointMat *jointMats, const int numJoints );
	virtual void VPCALL TransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint );
	virtual void VPCALL UntransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint );
#elif defined(_MSC_VER) && defined(_M_IX86)
//#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
	virtual const char * VPCALL GetName( void ) const;

	//virtual void VPCALL MatX_LowerTriangularSolve( const idMatX &L, float *x, const float *b, const int n, int skip = 0 );
	//virtual void VPCALL MatX_LowerTriangularSolveTranspose( const idMatX &L, float *x, const float *b, const int n );

	virtual void VPCALL MixedSoundToSamples( short *samples, const float *mixBuffer, const int numSamples );

#endif
};

#endif /* !__MATH_SIMD_SSE2_H__ */
