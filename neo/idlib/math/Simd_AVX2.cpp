/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

===========================================================================
*/

#include "sys/platform.h"
#include "idlib/geometry/DrawVert.h"
#include "idlib/geometry/JointTransform.h"
#include "idlib/math/Plane.h"
#include "idlib/math/Simd_AVX2.h"

#if D3_HAVE_AVX2_SIMD
#include <immintrin.h>

const char * idSIMD_AVX2::GetName( void ) const {
    return "AVX2 & FMA3";
}

/*
================
idSIMD_AVX2::Add

Add a scalar or a second array to eight floats at a time. The scalar
operation uses single-precision addition, matching the generic routine.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Add( float *dst, const float constant, const float *src, const int count ) {
    const __m256 value = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 input = _mm256_loadu_ps( src + i );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( input, value ) );
    }
    if ( i < count ) {
        idSIMD_SSE3::Add( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Add( float *dst, const float *src0, const float *src1, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src0 + i );
        const __m256 b = _mm256_loadu_ps( src1 + i );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( a, b ) );
    }
    if ( i < count ) {
        idSIMD_SSE3::Add( dst + i, src0 + i, src1 + i, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Sub( float *dst, const float *src0, const float *src1, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src0 + i );
        const __m256 b = _mm256_loadu_ps( src1 + i );
        _mm256_storeu_ps( dst + i, _mm256_sub_ps( a, b ) );
    }
    if ( i < count ) {
        idSIMD_SSE3::Sub( dst + i, src0 + i, src1 + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Mul

Multiply arrays or scale an array eight floats at a time.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Mul( float *dst, const float constant, const float *src, const int count ) {
    const __m256 scale = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( src + i );
        _mm256_storeu_ps( dst + i, _mm256_mul_ps( value, scale ) );
    }
    if ( i < count ) {
        idSIMD_SSE3::Mul( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Mul( float *dst, const float *src0, const float *src1, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src0 + i );
        const __m256 b = _mm256_loadu_ps( src1 + i );
        _mm256_storeu_ps( dst + i, _mm256_mul_ps( a, b ) );
    }
    if ( i < count ) {
        idSIMD_SSE3::Mul( dst + i, src0 + i, src1 + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Div

Divide arrays eight floats at a time. The scalar-numerator variant retains
idSIMD_Generic's double-precision numerator/division behavior by widening each
divisor to double before dividing.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Div( float *dst, const float constant, const float *divisor, const int count ) {
    const __m256d numerator = _mm256_set1_pd( (double)constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 input = _mm256_loadu_ps( divisor + i );
        const __m128 low = _mm256_castps256_ps128( input );
        const __m128 high = _mm256_extractf128_ps( input, 1 );
        const __m256d lowResult = _mm256_div_pd( numerator, _mm256_cvtps_pd( low ) );
        const __m256d highResult = _mm256_div_pd( numerator, _mm256_cvtps_pd( high ) );
        const __m128 lowFloats = _mm256_cvtpd_ps( lowResult );
        const __m128 highFloats = _mm256_cvtpd_ps( highResult );
        const __m256 result = _mm256_insertf128_ps( _mm256_castps128_ps256( lowFloats ), highFloats, 1 );
        _mm256_storeu_ps( dst + i, result );
    }
    if ( i < count ) {
        idSIMD_SSE3::Div( dst + i, constant, divisor + i, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Div( float *dst, const float *src0, const float *src1, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src0 + i );
        const __m256 b = _mm256_loadu_ps( src1 + i );
        _mm256_storeu_ps( dst + i, _mm256_div_ps( a, b ) );
    }
    if ( i < count ) {
        idSIMD_SSE3::Div( dst + i, src0 + i, src1 + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Clamp / ClampMin / ClampMax

Use ordered comparisons and blends rather than MIN/MAX instructions. This
preserves the generic ternary operators' behavior for NaNs and signed zero.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Clamp( float *dst, const float *src, const float min, const float max, const int count ) {
    const __m256 lower = _mm256_set1_ps( min );
    const __m256 upper = _mm256_set1_ps( max );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( src + i );
        const __m256 aboveMax = _mm256_cmp_ps( value, upper, _CMP_GT_OQ );
        const __m256 belowMin = _mm256_cmp_ps( value, lower, _CMP_LT_OQ );
        const __m256 upperClamped = _mm256_blendv_ps( value, upper, aboveMax );
        const __m256 result = _mm256_blendv_ps( upperClamped, lower, belowMin );
        _mm256_storeu_ps( dst + i, result );
    }
    if ( i < count ) {
        idSIMD_Generic::Clamp( dst + i, src + i, min, max, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::ClampMin( float *dst, const float *src, const float min, const int count ) {
    const __m256 lower = _mm256_set1_ps( min );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( src + i );
        const __m256 belowMin = _mm256_cmp_ps( value, lower, _CMP_LT_OQ );
        _mm256_storeu_ps( dst + i, _mm256_blendv_ps( value, lower, belowMin ) );
    }
    if ( i < count ) {
        idSIMD_Generic::ClampMin( dst + i, src + i, min, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::ClampMax( float *dst, const float *src, const float max, const int count ) {
    const __m256 upper = _mm256_set1_ps( max );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( src + i );
        const __m256 aboveMax = _mm256_cmp_ps( value, upper, _CMP_GT_OQ );
        _mm256_storeu_ps( dst + i, _mm256_blendv_ps( value, upper, aboveMax ) );
    }
    if ( i < count ) {
        idSIMD_Generic::ClampMax( dst + i, src + i, max, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::MulAdd / MulSub

Accumulate products into an existing destination array. The scalar variants
use double-precision intermediates, matching idSIMD_Generic. The array
variants retain the generic float multiply followed by float add/subtract.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MulAdd( float *dst, const float constant, const float *src, const int count ) {
    const __m256d scale = _mm256_set1_pd( (double)constant );
    int i = 0;
    for ( ; i + 4 <= count; i += 4 ) {
        const __m128 src4 = _mm_loadu_ps( src + i );
        const __m128 dst4 = _mm_loadu_ps( dst + i );
        const __m256d product = _mm256_mul_pd( scale, _mm256_cvtps_pd( src4 ) );
        const __m256d sum = _mm256_add_pd( _mm256_cvtps_pd( dst4 ), product );
        _mm_storeu_ps( dst + i, _mm256_cvtpd_ps( sum ) );
    }
    if ( i < count ) {
        idSIMD_Generic::MulAdd( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MulAdd( float *dst, const float *src0, const float *src1, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src0 + i );
        const __m256 b = _mm256_loadu_ps( src1 + i );
        const __m256 d = _mm256_loadu_ps( dst + i );
        const __m256 product = _mm256_mul_ps( a, b );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( d, product ) );
    }
    if ( i < count ) {
        idSIMD_Generic::MulAdd( dst + i, src0 + i, src1 + i, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MulSub( float *dst, const float constant, const float *src, const int count ) {
    const __m256d scale = _mm256_set1_pd( (double)constant );
    int i = 0;
    for ( ; i + 4 <= count; i += 4 ) {
        const __m128 src4 = _mm_loadu_ps( src + i );
        const __m128 dst4 = _mm_loadu_ps( dst + i );
        const __m256d product = _mm256_mul_pd( scale, _mm256_cvtps_pd( src4 ) );
        const __m256d difference = _mm256_sub_pd( _mm256_cvtps_pd( dst4 ), product );
        _mm_storeu_ps( dst + i, _mm256_cvtpd_ps( difference ) );
    }
    if ( i < count ) {
        idSIMD_Generic::MulSub( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MulSub( float *dst, const float *src0, const float *src1, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src0 + i );
        const __m256 b = _mm256_loadu_ps( src1 + i );
        const __m256 d = _mm256_loadu_ps( dst + i );
        const __m256 product = _mm256_mul_ps( a, b );
        _mm256_storeu_ps( dst + i, _mm256_sub_ps( d, product ) );
    }
    if ( i < count ) {
        idSIMD_Generic::MulSub( dst + i, src0 + i, src1 + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

  dst[i] = constant * src[i];

idVec3 is a tightly packed three-float structure. Gather lets us process
8 independent vectors without changing its AoS layout.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float *dst, const idVec3 &constant, const idVec3 *src, const int count ) {
    const float *base = reinterpret_cast<const float *>( src );
    const __m256i index = _mm256_setr_epi32( 0, 3, 6, 9, 12, 15, 18, 21 );
    const __m256 cx = _mm256_set1_ps( constant[0] );
    const __m256 cy = _mm256_set1_ps( constant[1] );
    const __m256 cz = _mm256_set1_ps( constant[2] );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const float *block = base + i * 3;
        const __m256 x = _mm256_i32gather_ps( block, index, 4 );
        const __m256 y = _mm256_i32gather_ps( block + 1, index, 4 );
        const __m256 z = _mm256_i32gather_ps( block + 2, index, 4 );
        const __m256 xy = _mm256_add_ps( _mm256_mul_ps( cx, x ), _mm256_mul_ps( cy, y ) );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( xy, _mm256_mul_ps( cz, z ) ) );
    }
    if ( i < count ) {
        idSIMD_Generic::Dot( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

  dst[i] = constant.Normal() * src[i] + constant[3];
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float *dst, const idPlane &constant, const idVec3 *src, const int count ) {
    const float *base = reinterpret_cast<const float *>( src );
    const __m256i index = _mm256_setr_epi32( 0, 3, 6, 9, 12, 15, 18, 21 );
    const __m256 cx = _mm256_set1_ps( constant[0] );
    const __m256 cy = _mm256_set1_ps( constant[1] );
    const __m256 cz = _mm256_set1_ps( constant[2] );
    const __m256 cw = _mm256_set1_ps( constant[3] );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const float *block = base + i * 3;
        const __m256 x = _mm256_i32gather_ps( block, index, 4 );
        const __m256 y = _mm256_i32gather_ps( block + 1, index, 4 );
        const __m256 z = _mm256_i32gather_ps( block + 2, index, 4 );
        const __m256 xy = _mm256_add_ps( _mm256_mul_ps( cx, x ), _mm256_mul_ps( cy, y ) );
        const __m256 xyz = _mm256_add_ps( xy, _mm256_mul_ps( cz, z ) );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( xyz, cw ) );
    }
    if ( i < count ) {
        idSIMD_Generic::Dot( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

  dst[i] = src0[i] * src1[i];

Both inputs are tightly packed idVec3 arrays. Gather the three components
from each array, then evaluate the dot product in the same left-to-right
order as the generic implementation.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float *dst, const idVec3 *src0, const idVec3 *src1, const int count ) {
    const float *base0 = reinterpret_cast<const float *>( src0 );
    const float *base1 = reinterpret_cast<const float *>( src1 );
    const __m256i index = _mm256_setr_epi32( 0, 3, 6, 9, 12, 15, 18, 21 );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const float *block0 = base0 + i * 3;
        const float *block1 = base1 + i * 3;
        const __m256 x0 = _mm256_i32gather_ps( block0, index, 4 );
        const __m256 y0 = _mm256_i32gather_ps( block0 + 1, index, 4 );
        const __m256 z0 = _mm256_i32gather_ps( block0 + 2, index, 4 );
        const __m256 x1 = _mm256_i32gather_ps( block1, index, 4 );
        const __m256 y1 = _mm256_i32gather_ps( block1 + 1, index, 4 );
        const __m256 z1 = _mm256_i32gather_ps( block1 + 2, index, 4 );
        const __m256 xy = _mm256_add_ps( _mm256_mul_ps( x0, x1 ), _mm256_mul_ps( y0, y1 ) );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( xy, _mm256_mul_ps( z0, z1 ) ) );
    }
    if ( i < count ) {
        idSIMD_Generic::Dot( dst + i, src0 + i, src1 + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

Compute the signed distance from a point to eight planes at a time. This
accelerates interaction-facing tests, where each plane is one triangle face.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float *dst, const idVec3 &constant, const idPlane *src, const int count ) {
    assert( sizeof( idPlane ) == 4 * sizeof( float ) );
    const __m256 nx = _mm256_set1_ps( constant[0] );
    const __m256 ny = _mm256_set1_ps( constant[1] );
    const __m256 nz = _mm256_set1_ps( constant[2] );
    int i = 0;

    for ( ; i + 8 <= count; i += 8 ) {
        float x[8], y[8], z[8], w[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            x[lane] = src[i + lane][0];
            y[lane] = src[i + lane][1];
            z[lane] = src[i + lane][2];
            w[lane] = src[i + lane][3];
        }
        __m256 value = _mm256_add_ps( _mm256_mul_ps( nx, _mm256_loadu_ps( x ) ), _mm256_loadu_ps( w ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( ny, _mm256_loadu_ps( y ) ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( nz, _mm256_loadu_ps( z ) ) );
        _mm256_storeu_ps( dst + i, value );
    }

    if ( i < count ) {
        idSIMD_SSE3::Dot( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

Compute constant-plane dot products for eight planes at a time. idPlane is
four contiguous floats, so AVX2 gathers each component into a vector before
the multiply/add sequence.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float *dst, const idPlane &constant, const idPlane *src, const int count ) {
    assert( sizeof( idPlane ) == 4 * sizeof( float ) );
    const float *base = reinterpret_cast<const float *>( src );
    const __m256 nx = _mm256_set1_ps( constant[0] );
    const __m256 ny = _mm256_set1_ps( constant[1] );
    const __m256 nz = _mm256_set1_ps( constant[2] );
    const __m256 nw = _mm256_set1_ps( constant[3] );
    const __m256i offsets = _mm256_setr_epi32( 0, 4, 8, 12, 16, 20, 24, 28 );
    int i = 0;

    for ( ; i + 8 <= count; i += 8 ) {
        const __m256i indices = _mm256_add_epi32( offsets, _mm256_set1_epi32( i * 4 ) );
        const __m256 x = _mm256_i32gather_ps( base, indices, 4 );
        const __m256 y = _mm256_i32gather_ps( base + 1, indices, 4 );
        const __m256 z = _mm256_i32gather_ps( base + 2, indices, 4 );
        const __m256 w = _mm256_i32gather_ps( base + 3, indices, 4 );
        __m256 value = _mm256_mul_ps( nx, x );
        value = _mm256_add_ps( value, _mm256_mul_ps( ny, y ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( nz, z ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( nw, w ) );
        _mm256_storeu_ps( dst + i, value );
    }

    if ( i < count ) {
        idSIMD_SSE3::Dot( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

Compute the dot product of a constant vector with eight DrawVert positions.
DrawVert is an AoS structure, so gather XYZ into temporary SoA arrays before
performing the arithmetic.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float *dst, const idVec3 &constant, const idDrawVert *src, const int count ) {
    const __m256 nx = _mm256_set1_ps( constant[0] );
    const __m256 ny = _mm256_set1_ps( constant[1] );
    const __m256 nz = _mm256_set1_ps( constant[2] );
    int i = 0;

    for ( ; i + 8 <= count; i += 8 ) {
        float x[8], y[8], z[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            x[lane] = src[i + lane].xyz.x;
            y[lane] = src[i + lane].xyz.y;
            z[lane] = src[i + lane].xyz.z;
        }
        __m256 value = _mm256_mul_ps( nx, _mm256_loadu_ps( x ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( ny, _mm256_loadu_ps( y ) ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( nz, _mm256_loadu_ps( z ) ) );
        _mm256_storeu_ps( dst + i, value );
    }

    if ( i < count ) {
        idSIMD_SSE3::Dot( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

Compute a scalar dot product of two float arrays. Products are calculated
in single precision, then accumulated in double precision to retain the
precision characteristics of the generic implementation.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float &dot, const float *src1, const float *src2, const int count ) {
    switch ( count ) {
        case 0:
            dot = 0.0f;
            return;
        case 1:
            dot = src1[0] * src2[0];
            return;
        case 2:
            dot = src1[0] * src2[0] + src1[1] * src2[1];
            return;
        case 3:
            dot = src1[0] * src2[0] + src1[1] * src2[1] + src1[2] * src2[2];
            return;
        default:
            break;
    }

    __m256d sumLo = _mm256_setzero_pd();
    __m256d sumHi = _mm256_setzero_pd();
    int i = 0;

    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 products = _mm256_mul_ps(
            _mm256_loadu_ps( src1 + i ), _mm256_loadu_ps( src2 + i ) );
        const __m128 productsLo = _mm256_castps256_ps128( products );
        const __m128 productsHi = _mm256_extractf128_ps( products, 1 );
        sumLo = _mm256_add_pd( sumLo, _mm256_cvtps_pd( productsLo ) );
        sumHi = _mm256_add_pd( sumHi, _mm256_cvtps_pd( productsHi ) );
    }

    ALIGN16( double laneLo[4] );
    ALIGN16( double laneHi[4] );
    _mm256_storeu_pd( laneLo, sumLo );
    _mm256_storeu_pd( laneHi, sumHi );

    double sum = ( laneLo[0] + laneLo[1] + laneLo[2] + laneLo[3] )
               + ( laneHi[0] + laneHi[1] + laneHi[2] + laneHi[3] );
    for ( ; i < count; ++i ) {
        sum += src1[i] * src2[i];
    }
    dot = (float)sum;
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Dot

Compute a plane distance for eight DrawVerts at a time. DrawVert is an
AoS structure (60 bytes), so gather XYZ into temporary SoA arrays before
performing the arithmetic. This is used by stencil-shadow point culling.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Dot( float *dst, const idPlane &constant, const idDrawVert *src, const int count ) {
    const __m256 nx = _mm256_set1_ps( constant[0] );
    const __m256 ny = _mm256_set1_ps( constant[1] );
    const __m256 nz = _mm256_set1_ps( constant[2] );
    const __m256 w  = _mm256_set1_ps( constant[3] );
    int i = 0;

    for ( ; i + 8 <= count; i += 8 ) {
        float x[8], y[8], z[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            x[lane] = src[i + lane].xyz.x;
            y[lane] = src[i + lane].xyz.y;
            z[lane] = src[i + lane].xyz.z;
        }
        __m256 value = _mm256_mul_ps( nx, _mm256_loadu_ps( x ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( ny, _mm256_loadu_ps( y ) ) );
        value = _mm256_add_ps( value, _mm256_mul_ps( nz, _mm256_loadu_ps( z ) ) );
        value = _mm256_add_ps( value, w );
        _mm256_storeu_ps( dst + i, value );
    }

    if ( i < count ) {
        idSIMD_SSE3::Dot( dst + i, constant, src + i, count - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::CmpGT / CmpLT

Set one culling bit per vertex. The comparison is ordered, matching the
scalar C++ semantics (NaNs do not compare greater/less than a value).
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::CmpGT( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
    const __m256 threshold = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 values = _mm256_loadu_ps( src0 + i );
        const int mask = _mm256_movemask_ps( _mm256_cmp_ps( values, threshold, _CMP_GT_OQ ) );
        for ( int lane = 0; lane < 8; ++lane ) {
            dst[i + lane] |= (byte)( ( ( mask >> lane ) & 1 ) << bitNum );
        }
    }
    for ( ; i < count; ++i ) {
        dst[i] |= (byte)( ( src0[i] > constant ) << bitNum );
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::CmpLT( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
    const __m256 threshold = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 values = _mm256_loadu_ps( src0 + i );
        const int mask = _mm256_movemask_ps( _mm256_cmp_ps( values, threshold, _CMP_LT_OQ ) );
        for ( int lane = 0; lane < 8; ++lane ) {
            dst[i + lane] |= (byte)( ( ( mask >> lane ) & 1 ) << bitNum );
        }
    }
    for ( ; i < count; ++i ) {
        dst[i] |= (byte)( ( src0[i] < constant ) << bitNum );
    }
    _mm256_zeroupper();
}



/*
================
idSIMD_AVX2::MinMax

Compute the bounds of a DrawVert array. DrawVert is an AoS structure, so
positions are gathered into eight-wide SoA vectors before min/max reduction.
Ordered comparisons preserve the generic implementation's behavior for NaNs.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MinMax( idVec3 &min, idVec3 &max, const idDrawVert *src, const int count ) {
    __m256 minX = _mm256_set1_ps( idMath::INFINITY );
    __m256 minY = minX;
    __m256 minZ = minX;
    __m256 maxX = _mm256_set1_ps( -idMath::INFINITY );
    __m256 maxY = maxX;
    __m256 maxZ = maxX;
    int i = 0;

    for ( ; i + 8 <= count; i += 8 ) {
        float x[8], y[8], z[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            x[lane] = src[i + lane].xyz.x;
            y[lane] = src[i + lane].xyz.y;
            z[lane] = src[i + lane].xyz.z;
        }
        const __m256 vx = _mm256_loadu_ps( x );
        const __m256 vy = _mm256_loadu_ps( y );
        const __m256 vz = _mm256_loadu_ps( z );

        minX = _mm256_blendv_ps( minX, vx, _mm256_cmp_ps( vx, minX, _CMP_LT_OQ ) );
        minY = _mm256_blendv_ps( minY, vy, _mm256_cmp_ps( vy, minY, _CMP_LT_OQ ) );
        minZ = _mm256_blendv_ps( minZ, vz, _mm256_cmp_ps( vz, minZ, _CMP_LT_OQ ) );
        maxX = _mm256_blendv_ps( maxX, vx, _mm256_cmp_ps( vx, maxX, _CMP_GT_OQ ) );
        maxY = _mm256_blendv_ps( maxY, vy, _mm256_cmp_ps( vy, maxY, _CMP_GT_OQ ) );
        maxZ = _mm256_blendv_ps( maxZ, vz, _mm256_cmp_ps( vz, maxZ, _CMP_GT_OQ ) );
    }

    ALIGN16( float laneMinX[8] );
    ALIGN16( float laneMinY[8] );
    ALIGN16( float laneMinZ[8] );
    ALIGN16( float laneMaxX[8] );
    ALIGN16( float laneMaxY[8] );
    ALIGN16( float laneMaxZ[8] );
    _mm256_store_ps( laneMinX, minX );
    _mm256_store_ps( laneMinY, minY );
    _mm256_store_ps( laneMinZ, minZ );
    _mm256_store_ps( laneMaxX, maxX );
    _mm256_store_ps( laneMaxY, maxY );
    _mm256_store_ps( laneMaxZ, maxZ );

    min[0] = min[1] = min[2] = idMath::INFINITY;
    max[0] = max[1] = max[2] = -idMath::INFINITY;
    for ( int lane = 0; lane < 8; ++lane ) {
        if ( laneMinX[lane] < min[0] ) min[0] = laneMinX[lane];
        if ( laneMinY[lane] < min[1] ) min[1] = laneMinY[lane];
        if ( laneMinZ[lane] < min[2] ) min[2] = laneMinZ[lane];
        if ( laneMaxX[lane] > max[0] ) max[0] = laneMaxX[lane];
        if ( laneMaxY[lane] > max[1] ) max[1] = laneMaxY[lane];
        if ( laneMaxZ[lane] > max[2] ) max[2] = laneMaxZ[lane];
    }

    for ( ; i < count; ++i ) {
        if ( src[i].xyz.x < min[0] ) min[0] = src[i].xyz.x;
        if ( src[i].xyz.y < min[1] ) min[1] = src[i].xyz.y;
        if ( src[i].xyz.z < min[2] ) min[2] = src[i].xyz.z;
        if ( src[i].xyz.x > max[0] ) max[0] = src[i].xyz.x;
        if ( src[i].xyz.y > max[1] ) max[1] = src[i].xyz.y;
        if ( src[i].xyz.z > max[2] ) max[2] = src[i].xyz.z;
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::CreateShadowCache

Compact the vertices marked by vertRemap and build the near/far vertex pairs
used by turbo stencil shadow volumes. The source DrawVerts are strided AoS;
AVX2 gathers eight selected XYZ positions, computes their light-relative
positions in parallel, and the final compacted writes preserve source order.
================
*/
D3_AVX2_TARGET static int R_CreateShadowCacheBatchAVX2( idVec4 *vertexCache, int *vertRemap,
                                                        const idDrawVert *verts, const int *sourceIndices,
                                                        const idVec3 &lightOrigin, const int outVerts ) {
    const int stride = (int)( sizeof( idDrawVert ) / sizeof( float ) );
    const float *base = verts[0].xyz.ToFloatPtr();
    const __m256i offsets = _mm256_setr_epi32(
        sourceIndices[0] * stride, sourceIndices[1] * stride,
        sourceIndices[2] * stride, sourceIndices[3] * stride,
        sourceIndices[4] * stride, sourceIndices[5] * stride,
        sourceIndices[6] * stride, sourceIndices[7] * stride );
    const __m256 x = _mm256_i32gather_ps( base, offsets, 4 );
    const __m256 y = _mm256_i32gather_ps( base + 1, offsets, 4 );
    const __m256 z = _mm256_i32gather_ps( base + 2, offsets, 4 );
    const __m256 dx = _mm256_sub_ps( x, _mm256_set1_ps( lightOrigin.x ) );
    const __m256 dy = _mm256_sub_ps( y, _mm256_set1_ps( lightOrigin.y ) );
    const __m256 dz = _mm256_sub_ps( z, _mm256_set1_ps( lightOrigin.z ) );

    float px[8], py[8], pz[8], fx[8], fy[8], fz[8];
    _mm256_storeu_ps( px, x );
    _mm256_storeu_ps( py, y );
    _mm256_storeu_ps( pz, z );
    _mm256_storeu_ps( fx, dx );
    _mm256_storeu_ps( fy, dy );
    _mm256_storeu_ps( fz, dz );

    for ( int lane = 0; lane < 8; ++lane ) {
        const int source = sourceIndices[lane];
        const int dst = outVerts + lane * 2;
        vertRemap[source] = dst;
        vertexCache[dst + 0].Set( px[lane], py[lane], pz[lane], 1.0f );
        vertexCache[dst + 1].Set( fx[lane], fy[lane], fz[lane], 0.0f );
    }
    return outVerts + 16;
}

D3_AVX2_TARGET int VPCALL idSIMD_AVX2::CreateShadowCache( idVec4 *vertexCache, int *vertRemap,
                                                           const idVec3 &lightOrigin, const idDrawVert *verts,
                                                           const int numVerts ) {
    if ( numVerts <= 0 ) {
        return 0;
    }

    int sourceIndices[8];
    int batchCount = 0;
    int outVerts = 0;
    for ( int i = 0; i < numVerts; ++i ) {
        if ( vertRemap[i] != 0 ) {
            continue;
        }
        sourceIndices[batchCount++] = i;
        if ( batchCount == 8 ) {
            outVerts = R_CreateShadowCacheBatchAVX2( vertexCache, vertRemap, verts,
                                                     sourceIndices, lightOrigin, outVerts );
            batchCount = 0;
        }
    }

    // Preserve the generic implementation's exact compacted ordering for
    // the final partial batch without issuing masked gathers.
    for ( int lane = 0; lane < batchCount; ++lane ) {
        const int source = sourceIndices[lane];
        const float *v = verts[source].xyz.ToFloatPtr();
        vertRemap[source] = outVerts;
        vertexCache[outVerts + 0].Set( v[0], v[1], v[2], 1.0f );
        vertexCache[outVerts + 1].Set( v[0] - lightOrigin.x,
                                       v[1] - lightOrigin.y,
                                       v[2] - lightOrigin.z, 0.0f );
        outVerts += 2;
    }
    _mm256_zeroupper();
    return outVerts;
}

/*
================
idSIMD_AVX2::ConvertJointQuatsToJointMats

Convert eight joint quaternions to 3x4 joint matrices in parallel. The
quaternion and matrix structures are arrays-of-structures, so input fields
are gathered into SoA lanes and the results are scattered back per joint.
The scalar tail uses the existing SSE3 implementation.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::ConvertJointQuatsToJointMats( idJointMat *jointMats, const idJointQuat *jointQuats, const int numJoints ) {
    assert( sizeof( idJointQuat ) == 7 * sizeof( float ) );
    assert( sizeof( idJointMat ) == 12 * sizeof( float ) );

    const __m256 one = _mm256_set1_ps( 1.0f );
    int i = 0;
    for ( ; i + 8 <= numJoints; i += 8 ) {
        float qx[8], qy[8], qz[8], qw[8];
        float tx[8], ty[8], tz[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            const idJointQuat &q = jointQuats[i + lane];
            qx[lane] = q.q[0];
            qy[lane] = q.q[1];
            qz[lane] = q.q[2];
            qw[lane] = q.q[3];
            tx[lane] = q.t[0];
            ty[lane] = q.t[1];
            tz[lane] = q.t[2];
        }

        const __m256 x = _mm256_loadu_ps( qx );
        const __m256 y = _mm256_loadu_ps( qy );
        const __m256 z = _mm256_loadu_ps( qz );
        const __m256 w = _mm256_loadu_ps( qw );
        const __m256 x2 = _mm256_add_ps( x, x );
        const __m256 y2 = _mm256_add_ps( y, y );
        const __m256 z2 = _mm256_add_ps( z, z );

        const __m256 xx = _mm256_mul_ps( x, x2 );
        const __m256 yy = _mm256_mul_ps( y, y2 );
        const __m256 zz = _mm256_mul_ps( z, z2 );
        const __m256 m00 = _mm256_sub_ps( _mm256_sub_ps( one, yy ), zz );
        const __m256 m11 = _mm256_sub_ps( _mm256_sub_ps( one, xx ), zz );
        const __m256 m22 = _mm256_sub_ps( _mm256_sub_ps( one, xx ), yy );

        const __m256 yz = _mm256_mul_ps( y, z2 );
        const __m256 wx = _mm256_mul_ps( w, x2 );
        const __m256 m21 = _mm256_sub_ps( yz, wx );
        const __m256 m12 = _mm256_add_ps( yz, wx );

        const __m256 xy = _mm256_mul_ps( x, y2 );
        const __m256 wz = _mm256_mul_ps( w, z2 );
        const __m256 m10 = _mm256_sub_ps( xy, wz );
        const __m256 m01 = _mm256_add_ps( xy, wz );

        const __m256 xz = _mm256_mul_ps( x, z2 );
        const __m256 wy = _mm256_mul_ps( w, y2 );
        const __m256 m02 = _mm256_sub_ps( xz, wy );
        const __m256 m20 = _mm256_add_ps( xz, wy );

        float out00[8], out01[8], out02[8];
        float out10[8], out11[8], out12[8];
        float out20[8], out21[8], out22[8];
        _mm256_storeu_ps( out00, m00 );
        _mm256_storeu_ps( out01, m01 );
        _mm256_storeu_ps( out02, m02 );
        _mm256_storeu_ps( out10, m10 );
        _mm256_storeu_ps( out11, m11 );
        _mm256_storeu_ps( out12, m12 );
        _mm256_storeu_ps( out20, m20 );
        _mm256_storeu_ps( out21, m21 );
        _mm256_storeu_ps( out22, m22 );

        for ( int lane = 0; lane < 8; ++lane ) {
            float *m = jointMats[i + lane].ToFloatPtr();
            m[0] = out00[lane]; m[1] = out01[lane]; m[2] = out02[lane]; m[3] = tx[lane];
            m[4] = out10[lane]; m[5] = out11[lane]; m[6] = out12[lane]; m[7] = ty[lane];
            m[8] = out20[lane]; m[9] = out21[lane]; m[10] = out22[lane]; m[11] = tz[lane];
        }
    }

    if ( i < numJoints ) {
        idSIMD_SSE3::ConvertJointQuatsToJointMats( jointMats + i, jointQuats + i, numJoints - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::ConvertJointMatsToJointQuats

Convert eight 3x4 joint matrices to quaternions in parallel. The trace and
largest-diagonal branches match idJointMat::ToJointQuat, with masked SIMD
selection. Two Newton refinements of rsqrt provide the reciprocal-square-root
precision needed by the existing joint conversion tests.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::ConvertJointMatsToJointQuats( idJointQuat *jointQuats,
        const idJointMat *jointMats, const int numJoints ) {
    assert( sizeof( idJointQuat ) == 7 * sizeof( float ) );
    assert( sizeof( idJointMat ) == 12 * sizeof( float ) );

    int i = 0;
    for ( ; i + 8 <= numJoints; i += 8 ) {
        ALIGN16( float m00v[8] ); ALIGN16( float m01v[8] ); ALIGN16( float m02v[8] );
        ALIGN16( float m10v[8] ); ALIGN16( float m11v[8] ); ALIGN16( float m12v[8] );
        ALIGN16( float m20v[8] ); ALIGN16( float m21v[8] ); ALIGN16( float m22v[8] );
        ALIGN16( float txv[8] ); ALIGN16( float tyv[8] ); ALIGN16( float tzv[8] );
        for ( int lane = 0; lane < 8; ++lane ) {
            const float *m = jointMats[i + lane].ToFloatPtr();
            m00v[lane] = m[0];  m01v[lane] = m[1];  m02v[lane] = m[2];
            m10v[lane] = m[4];  m11v[lane] = m[5];  m12v[lane] = m[6];
            m20v[lane] = m[8];  m21v[lane] = m[9];  m22v[lane] = m[10];
            txv[lane] = m[3]; tyv[lane] = m[7]; tzv[lane] = m[11];
        }
        const __m256 m00 = _mm256_loadu_ps( m00v );
        const __m256 m01 = _mm256_loadu_ps( m01v );
        const __m256 m02 = _mm256_loadu_ps( m02v );
        const __m256 m10 = _mm256_loadu_ps( m10v );
        const __m256 m11 = _mm256_loadu_ps( m11v );
        const __m256 m12 = _mm256_loadu_ps( m12v );
        const __m256 m20 = _mm256_loadu_ps( m20v );
        const __m256 m21 = _mm256_loadu_ps( m21v );
        const __m256 m22 = _mm256_loadu_ps( m22v );

        const __m256 trace = _mm256_add_ps( _mm256_add_ps( m00, m11 ), m22 );
        const __m256 traceT = _mm256_add_ps( trace, _mm256_set1_ps( 1.0f ) );
        const __m256 t0 = _mm256_add_ps( _mm256_sub_ps( m00, _mm256_add_ps( m11, m22 ) ), _mm256_set1_ps( 1.0f ) );
        const __m256 t1 = _mm256_add_ps( _mm256_sub_ps( m11, _mm256_add_ps( m22, m00 ) ), _mm256_set1_ps( 1.0f ) );
        const __m256 t2 = _mm256_add_ps( _mm256_sub_ps( m22, _mm256_add_ps( m00, m11 ) ), _mm256_set1_ps( 1.0f ) );

        const __m256 choose1 = _mm256_cmp_ps( m11, m00, _CMP_GT_OQ );
        const __m256 largest01 = _mm256_blendv_ps( m00, m11, choose1 );
        const __m256 choose2 = _mm256_cmp_ps( m22, largest01, _CMP_GT_OQ );
        const __m256 chooseOnly1 = _mm256_andnot_ps( choose2, choose1 );
        const __m256 traceMask = _mm256_cmp_ps( trace, _mm256_setzero_ps(), _CMP_GT_OQ );

        __m256 rTrace = _mm256_rsqrt_ps( traceT );
        __m256 r0 = _mm256_rsqrt_ps( t0 );
        __m256 r1 = _mm256_rsqrt_ps( t1 );
        __m256 r2 = _mm256_rsqrt_ps( t2 );
        const __m256 half = _mm256_set1_ps( 0.5f );
        const __m256 threeHalf = _mm256_set1_ps( 1.5f );
        for ( int refine = 0; refine < 2; ++refine ) {
            rTrace = _mm256_mul_ps( rTrace, _mm256_sub_ps( threeHalf, _mm256_mul_ps( _mm256_mul_ps( half, traceT ), _mm256_mul_ps( rTrace, rTrace ) ) ) );
            r0 = _mm256_mul_ps( r0, _mm256_sub_ps( threeHalf, _mm256_mul_ps( _mm256_mul_ps( half, t0 ), _mm256_mul_ps( r0, r0 ) ) ) );
            r1 = _mm256_mul_ps( r1, _mm256_sub_ps( threeHalf, _mm256_mul_ps( _mm256_mul_ps( half, t1 ), _mm256_mul_ps( r1, r1 ) ) ) );
            r2 = _mm256_mul_ps( r2, _mm256_sub_ps( threeHalf, _mm256_mul_ps( _mm256_mul_ps( half, t2 ), _mm256_mul_ps( r2, r2 ) ) ) );
        }
        const __m256 sTrace = _mm256_mul_ps( rTrace, half );
        const __m256 s0 = _mm256_mul_ps( r0, half );
        const __m256 s1 = _mm256_mul_ps( r1, half );
        const __m256 s2 = _mm256_mul_ps( r2, half );

        const __m256 tq0 = _mm256_mul_ps( sTrace, traceT );
        const __m256 tq1 = _mm256_mul_ps( _mm256_sub_ps( m12, m21 ), sTrace );
        const __m256 tq2 = _mm256_mul_ps( _mm256_sub_ps( m20, m02 ), sTrace );
        const __m256 tq3 = _mm256_mul_ps( _mm256_sub_ps( m01, m10 ), sTrace );

        const __m256 q0i0 = _mm256_mul_ps( s0, t0 );
        const __m256 q1i0 = _mm256_mul_ps( _mm256_add_ps( m01, m10 ), s0 );
        const __m256 q2i0 = _mm256_mul_ps( _mm256_add_ps( m02, m20 ), s0 );
        const __m256 q3i0 = _mm256_mul_ps( _mm256_sub_ps( m12, m21 ), s0 );

        const __m256 q0i1 = _mm256_mul_ps( _mm256_add_ps( m10, m01 ), s1 );
        const __m256 q1i1 = _mm256_mul_ps( s1, t1 );
        const __m256 q2i1 = _mm256_mul_ps( _mm256_add_ps( m12, m21 ), s1 );
        const __m256 q3i1 = _mm256_mul_ps( _mm256_sub_ps( m20, m02 ), s1 );

        const __m256 q0i2 = _mm256_mul_ps( _mm256_add_ps( m20, m02 ), s2 );
        const __m256 q1i2 = _mm256_mul_ps( _mm256_add_ps( m21, m12 ), s2 );
        const __m256 q2i2 = _mm256_mul_ps( s2, t2 );
        const __m256 q3i2 = _mm256_mul_ps( _mm256_sub_ps( m01, m10 ), s2 );

        const __m256 nq0 = _mm256_blendv_ps( _mm256_blendv_ps( q0i0, q0i1, chooseOnly1 ), q0i2, choose2 );
        const __m256 nq1 = _mm256_blendv_ps( _mm256_blendv_ps( q1i0, q1i1, chooseOnly1 ), q1i2, choose2 );
        const __m256 nq2 = _mm256_blendv_ps( _mm256_blendv_ps( q2i0, q2i1, chooseOnly1 ), q2i2, choose2 );
        const __m256 nq3 = _mm256_blendv_ps( _mm256_blendv_ps( q3i0, q3i1, chooseOnly1 ), q3i2, choose2 );
        const __m256 q0 = _mm256_blendv_ps( nq0, tq1, traceMask );
        const __m256 q1 = _mm256_blendv_ps( nq1, tq2, traceMask );
        const __m256 q2 = _mm256_blendv_ps( nq2, tq3, traceMask );
        const __m256 q3 = _mm256_blendv_ps( nq3, tq0, traceMask );

        ALIGN16( float q0v[8] ); ALIGN16( float q1v[8] ); ALIGN16( float q2v[8] ); ALIGN16( float q3v[8] );
        _mm256_storeu_ps( q0v, q0 ); _mm256_storeu_ps( q1v, q1 );
        _mm256_storeu_ps( q2v, q2 ); _mm256_storeu_ps( q3v, q3 );
        for ( int lane = 0; lane < 8; ++lane ) {
            jointQuats[i + lane].q[0] = q0v[lane];
            jointQuats[i + lane].q[1] = q1v[lane];
            jointQuats[i + lane].q[2] = q2v[lane];
            jointQuats[i + lane].q[3] = q3v[lane];
            jointQuats[i + lane].t[0] = txv[lane];
            jointQuats[i + lane].t[1] = tyv[lane];
            jointQuats[i + lane].t[2] = tzv[lane];
        }
    }
    if ( i < numJoints ) {
        idSIMD_Generic::ConvertJointMatsToJointQuats( jointQuats + i, jointMats + i, numJoints - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::TransformJoints

Apply parent transforms in hierarchy order. Two output rows (eight floats)
are evaluated together; the third row is handled with SSE. Each joint still
waits for its parent, preserving the skeleton dependency order.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::TransformJoints( idJointMat *jointMats,
        const int *parents, const int firstJoint, const int lastJoint ) {
    assert( sizeof( idJointMat ) == 48 );
    if ( lastJoint - firstJoint + 1 < 4 ) {
        idSIMD_Generic::TransformJoints( jointMats, parents, firstJoint, lastJoint );
        return;
    }
    for ( int i = firstJoint; i <= lastJoint; ++i ) {
        assert( parents[i] < i );
        float *m = jointMats[i].ToFloatPtr();
        const float *a = jointMats[parents[i]].ToFloatPtr();
        const __m128 child0 = _mm_loadu_ps( m );
        const __m128 child1 = _mm_loadu_ps( m + 4 );
        const __m128 child2 = _mm_loadu_ps( m + 8 );
        const __m256 c0 = _mm256_insertf128_ps( _mm256_castps128_ps256( child0 ), child0, 1 );
        const __m256 c1 = _mm256_insertf128_ps( _mm256_castps128_ps256( child1 ), child1, 1 );
        const __m256 c2 = _mm256_insertf128_ps( _mm256_castps128_ps256( child2 ), child2, 1 );
        const __m256 p0 = _mm256_setr_ps( a[0], a[0], a[0], a[0], a[4], a[4], a[4], a[4] );
        const __m256 p1 = _mm256_setr_ps( a[1], a[1], a[1], a[1], a[5], a[5], a[5], a[5] );
        const __m256 p2 = _mm256_setr_ps( a[2], a[2], a[2], a[2], a[6], a[6], a[6], a[6] );
        __m256 rows01 = _mm256_add_ps( _mm256_mul_ps( c0, p0 ), _mm256_mul_ps( c1, p1 ) );
        rows01 = _mm256_add_ps( rows01, _mm256_mul_ps( c2, p2 ) );
        rows01 = _mm256_add_ps( rows01, _mm256_setr_ps( 0, 0, 0, a[3], 0, 0, 0, a[7] ) );
        ALIGN16( float out01[8] );
        _mm256_store_ps( out01, rows01 );

        __m128 row2 = _mm_add_ps( _mm_mul_ps( child0, _mm_set1_ps( a[8] ) ),
                _mm_mul_ps( child1, _mm_set1_ps( a[9] ) ) );
        row2 = _mm_add_ps( row2, _mm_mul_ps( child2, _mm_set1_ps( a[10] ) ) );
        row2 = _mm_add_ps( row2, _mm_setr_ps( 0, 0, 0, a[11] ) );

        _mm_storeu_ps( m, _mm_load_ps( out01 ) );
        _mm_storeu_ps( m + 4, _mm_load_ps( out01 + 4 ) );
        _mm_storeu_ps( m + 8, row2 );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::UntransformJoints

Apply inverse parent transforms in reverse hierarchy order. As with
TransformJoints, two output rows are evaluated in AVX2 lanes and the third
row uses SSE. Translation is removed before applying the parent rotation,
matching idJointMat::operator/=.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::UntransformJoints( idJointMat *jointMats,
        const int *parents, const int firstJoint, const int lastJoint ) {
    assert( sizeof( idJointMat ) == 48 );
    if ( lastJoint - firstJoint + 1 < 4 ) {
        idSIMD_Generic::UntransformJoints( jointMats, parents, firstJoint, lastJoint );
        return;
    }
    for ( int i = lastJoint; i >= firstJoint; --i ) {
        assert( parents[i] < i );
        float *m = jointMats[i].ToFloatPtr();
        const float *a = jointMats[parents[i]].ToFloatPtr();
        __m128 child0 = _mm_loadu_ps( m );
        __m128 child1 = _mm_loadu_ps( m + 4 );
        __m128 child2 = _mm_loadu_ps( m + 8 );
        child0 = _mm_sub_ps( child0, _mm_setr_ps( 0, 0, 0, a[3] ) );
        child1 = _mm_sub_ps( child1, _mm_setr_ps( 0, 0, 0, a[7] ) );
        child2 = _mm_sub_ps( child2, _mm_setr_ps( 0, 0, 0, a[11] ) );
        const __m256 c0 = _mm256_insertf128_ps( _mm256_castps128_ps256( child0 ), child0, 1 );
        const __m256 c1 = _mm256_insertf128_ps( _mm256_castps128_ps256( child1 ), child1, 1 );
        const __m256 c2 = _mm256_insertf128_ps( _mm256_castps128_ps256( child2 ), child2, 1 );
        const __m256 p0 = _mm256_setr_ps( a[0], a[0], a[0], a[0], a[1], a[1], a[1], a[1] );
        const __m256 p1 = _mm256_setr_ps( a[4], a[4], a[4], a[4], a[5], a[5], a[5], a[5] );
        const __m256 p2 = _mm256_setr_ps( a[8], a[8], a[8], a[8], a[9], a[9], a[9], a[9] );
        __m256 rows01 = _mm256_add_ps( _mm256_mul_ps( c0, p0 ), _mm256_mul_ps( c1, p1 ) );
        rows01 = _mm256_add_ps( rows01, _mm256_mul_ps( c2, p2 ) );
        ALIGN16( float out01[8] );
        _mm256_storeu_ps( out01, rows01 );

        __m128 row2 = _mm_add_ps( _mm_mul_ps( child0, _mm_set1_ps( a[2] ) ),
                _mm_mul_ps( child1, _mm_set1_ps( a[6] ) ) );
        row2 = _mm_add_ps( row2, _mm_mul_ps( child2, _mm_set1_ps( a[10] ) ) );

        _mm_storeu_ps( m, _mm_load_ps( out01 ) );
        _mm_storeu_ps( m + 4, _mm_load_ps( out01 + 4 ) );
        _mm_storeu_ps( m + 8, row2 );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::TransformVerts

Skin vertices using the variable-length joint-weight lists. For each
influence, the first two 3x4 matrix rows are evaluated together in AVX2
lanes; the third row uses SSE arithmetic. The weight-list terminator and
byte-offset joint indices retain the original Doom 3 layout.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::TransformVerts( idDrawVert *verts, const int numVerts,
        const idJointMat *joints, const idVec4 *weights, const int *index, const int numWeights ) {
    assert( sizeof( idDrawVert ) == 60 );
    assert( sizeof( idVec4 ) == 16 );
    assert( sizeof( idJointMat ) == 48 );
    (void)numWeights;
    if ( numVerts < 8 ) {
        idSIMD_Generic::TransformVerts( verts, numVerts, joints, weights, index, numWeights );
        return;
    }

    const byte *jointBytes = reinterpret_cast<const byte *>( joints );
    int weightIndex = 0;
    for ( int vertex = 0; vertex < numVerts; ++vertex ) {
        float resultX = 0.0f;
        float resultY = 0.0f;
        float resultZ = 0.0f;
        for ( ;; ) {
            const float *m = reinterpret_cast<const float *>( jointBytes + index[weightIndex * 2] );
            const __m128 w = _mm_loadu_ps( weights[weightIndex].ToFloatPtr() );
            const __m256 w01 = _mm256_insertf128_ps( _mm256_castps128_ps256( w ), w, 1 );
            __m256 xy = _mm256_mul_ps( _mm256_loadu_ps( m ), w01 );
            xy = _mm256_hadd_ps( xy, xy );
            xy = _mm256_hadd_ps( xy, xy );
            const __m128 xyLow = _mm256_castps256_ps128( xy );
            const __m128 xyHigh = _mm256_extractf128_ps( xy, 1 );
            resultX += _mm_cvtss_f32( xyLow );
            resultY += _mm_cvtss_f32( xyHigh );

            __m128 z = _mm_mul_ps( _mm_loadu_ps( m + 8 ), w );
            z = _mm_hadd_ps( z, z );
            z = _mm_hadd_ps( z, z );
            resultZ += _mm_cvtss_f32( z );

            const bool lastWeight = index[weightIndex * 2 + 1] != 0;
            ++weightIndex;
            if ( lastWeight ) {
                break;
            }
        }
        verts[vertex].xyz.Set( resultX, resultY, resultZ );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::DeriveTriPlanes

Derive eight triangle planes per batch. Indexed AoS vertex positions are
loaded with AVX2 gathers, and the inverse-square-root approximation mirrors
idMath::RSqrt's bit seed and Newton step. The scalar tail uses the generic
implementation.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::DeriveTriPlanes( idPlane *planes, const idDrawVert *verts,
        const int numVerts, const int *indexes, const int numIndexes ) {
    assert( sizeof( idDrawVert ) == 60 );
    (void)numVerts;

    if ( numIndexes < 24 ) {
        idSIMD_Generic::DeriveTriPlanes( planes, verts, numVerts, indexes, numIndexes );
        return;
    }
    const float *position = verts[0].xyz.ToFloatPtr();
    const __m256 half = _mm256_set1_ps( 0.5f );
    const __m256 threeHalf = _mm256_set1_ps( 1.5f );
    const __m256i magic = _mm256_set1_epi32( 0x5f3759df );
    int i = 0;
    for ( ; i + 24 <= numIndexes; i += 24 ) {
        const __m256i ia = _mm256_setr_epi32( indexes[i+0], indexes[i+3], indexes[i+6], indexes[i+9], indexes[i+12], indexes[i+15], indexes[i+18], indexes[i+21] );
        const __m256i ib = _mm256_setr_epi32( indexes[i+1], indexes[i+4], indexes[i+7], indexes[i+10], indexes[i+13], indexes[i+16], indexes[i+19], indexes[i+22] );
        const __m256i ic = _mm256_setr_epi32( indexes[i+2], indexes[i+5], indexes[i+8], indexes[i+11], indexes[i+14], indexes[i+17], indexes[i+20], indexes[i+23] );
        const __m256i stride = _mm256_set1_epi32( 15 );
        const __m256i aBase = _mm256_mullo_epi32( ia, stride );
        const __m256i bBase = _mm256_mullo_epi32( ib, stride );
        const __m256i cBase = _mm256_mullo_epi32( ic, stride );

#define GATHER_POS(base, component) _mm256_i32gather_ps( position, _mm256_add_epi32( (base), _mm256_set1_epi32( component ) ), 4 )
        const __m256 ax = GATHER_POS( aBase, 0 );
        const __m256 ay = GATHER_POS( aBase, 1 );
        const __m256 az = GATHER_POS( aBase, 2 );
        const __m256 bx = GATHER_POS( bBase, 0 );
        const __m256 by = GATHER_POS( bBase, 1 );
        const __m256 bz = GATHER_POS( bBase, 2 );
        const __m256 cx = GATHER_POS( cBase, 0 );
        const __m256 cy = GATHER_POS( cBase, 1 );
        const __m256 cz = GATHER_POS( cBase, 2 );
#undef GATHER_POS

        const __m256 d0x = _mm256_sub_ps( bx, ax );
        const __m256 d0y = _mm256_sub_ps( by, ay );
        const __m256 d0z = _mm256_sub_ps( bz, az );
        const __m256 d1x = _mm256_sub_ps( cx, ax );
        const __m256 d1y = _mm256_sub_ps( cy, ay );
        const __m256 d1z = _mm256_sub_ps( cz, az );

        __m256 nx = _mm256_sub_ps( _mm256_mul_ps( d1y, d0z ), _mm256_mul_ps( d1z, d0y ) );
        __m256 ny = _mm256_sub_ps( _mm256_mul_ps( d1z, d0x ), _mm256_mul_ps( d1x, d0z ) );
        __m256 nz = _mm256_sub_ps( _mm256_mul_ps( d1x, d0y ), _mm256_mul_ps( d1y, d0x ) );
        const __m256 lengthSqr = _mm256_add_ps( _mm256_add_ps( _mm256_mul_ps( nx, nx ), _mm256_mul_ps( ny, ny ) ), _mm256_mul_ps( nz, nz ) );

        __m256 y = _mm256_mul_ps( lengthSqr, half );
        __m256i bits = _mm256_castps_si256( lengthSqr );
        bits = _mm256_sub_epi32( magic, _mm256_srli_epi32( bits, 1 ) );
        __m256 inv = _mm256_castsi256_ps( bits );
        const __m256 inv2 = _mm256_mul_ps( inv, inv );
        inv = _mm256_mul_ps( inv, _mm256_sub_ps( threeHalf, _mm256_mul_ps( inv2, y ) ) );

        nx = _mm256_mul_ps( nx, inv );
        ny = _mm256_mul_ps( ny, inv );
        nz = _mm256_mul_ps( nz, inv );
        const __m256 normalDotPoint = _mm256_add_ps(
                _mm256_add_ps( _mm256_mul_ps( nx, ax ), _mm256_mul_ps( ny, ay ) ),
                _mm256_mul_ps( nz, az ) );
        const __m256 nd = _mm256_sub_ps( _mm256_setzero_ps(), normalDotPoint );

        ALIGN16( float outX[8] );
        ALIGN16( float outY[8] );
        ALIGN16( float outZ[8] );
        ALIGN16( float outD[8] );
        _mm256_storeu_ps( outX, nx );
        _mm256_storeu_ps( outY, ny );
        _mm256_storeu_ps( outZ, nz );
        _mm256_storeu_ps( outD, nd );
        for ( int lane = 0; lane < 8; ++lane ) {
            planes[i / 3 + lane][0] = outX[lane];
            planes[i / 3 + lane][1] = outY[lane];
            planes[i / 3 + lane][2] = outZ[lane];
            planes[i / 3 + lane][3] = outD[lane];
        }
    }

    if ( i < numIndexes ) {
        idSIMD_Generic::DeriveTriPlanes( planes + i / 3, verts, numVerts, indexes + i, numIndexes - i );
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::DecalPointCull

Evaluate six decal clipping planes for eight vertices at a time. The
culling result is the inverse of each distance sign bit in the low six
bits, matching the generic implementation exactly.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::DecalPointCull( byte *cullBits, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {
    const __m256 nx[6] = {
        _mm256_set1_ps( planes[0][0] ), _mm256_set1_ps( planes[1][0] ),
        _mm256_set1_ps( planes[2][0] ), _mm256_set1_ps( planes[3][0] ),
        _mm256_set1_ps( planes[4][0] ), _mm256_set1_ps( planes[5][0] )
    };
    const __m256 ny[6] = {
        _mm256_set1_ps( planes[0][1] ), _mm256_set1_ps( planes[1][1] ),
        _mm256_set1_ps( planes[2][1] ), _mm256_set1_ps( planes[3][1] ),
        _mm256_set1_ps( planes[4][1] ), _mm256_set1_ps( planes[5][1] )
    };
    const __m256 nz[6] = {
        _mm256_set1_ps( planes[0][2] ), _mm256_set1_ps( planes[1][2] ),
        _mm256_set1_ps( planes[2][2] ), _mm256_set1_ps( planes[3][2] ),
        _mm256_set1_ps( planes[4][2] ), _mm256_set1_ps( planes[5][2] )
    };
    const __m256 d[6] = {
        _mm256_set1_ps( planes[0][3] ), _mm256_set1_ps( planes[1][3] ),
        _mm256_set1_ps( planes[2][3] ), _mm256_set1_ps( planes[3][3] ),
        _mm256_set1_ps( planes[4][3] ), _mm256_set1_ps( planes[5][3] )
    };
    const __m256 signMask = _mm256_castsi256_ps( _mm256_set1_epi32( (int)0x80000000 ) );
    int i = 0;

    for ( ; i + 8 <= numVerts; i += 8 ) {
        float x[8], y[8], z[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            x[lane] = verts[i + lane].xyz.x;
            y[lane] = verts[i + lane].xyz.y;
            z[lane] = verts[i + lane].xyz.z;
        }
        const __m256 vx = _mm256_loadu_ps( x );
        const __m256 vy = _mm256_loadu_ps( y );
        const __m256 vz = _mm256_loadu_ps( z );
        int signs[6];

        for ( int plane = 0; plane < 6; ++plane ) {
            __m256 distance = _mm256_mul_ps( nx[plane], vx );
            distance = _mm256_add_ps( distance, _mm256_mul_ps( ny[plane], vy ) );
            distance = _mm256_add_ps( distance, _mm256_mul_ps( nz[plane], vz ) );
            distance = _mm256_add_ps( distance, d[plane] );
            signs[plane] = _mm256_movemask_ps( _mm256_and_ps( distance, signMask ) );
        }

        for ( int lane = 0; lane < 8; ++lane ) {
            const int bit = 1 << lane;
            byte bits = 0;
            for ( int plane = 0; plane < 6; ++plane ) {
                bits |= (byte)( ((signs[plane] & bit) ? 0 : 1) << plane );
            }
            cullBits[i + lane] = bits;
        }
    }

    for ( ; i < numVerts; ++i ) {
        byte bits = 0;
        for ( int plane = 0; plane < 6; ++plane ) {
            const float distance = planes[plane].Distance( verts[i].xyz );
            bits |= (byte)( ( FLOATSIGNBITSET( distance ) ^ 1 ) << plane );
        }
        cullBits[i] = bits;
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::OverlayPointCull

Evaluate the two overlay projection planes for eight vertices at a time.
The projected coordinates are stored in their original interleaved layout,
and culling bits are generated from the sign bits of d0, d1, 1-d0, and 1-d1,
matching idSIMD_Generic::OverlayPointCull.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::OverlayPointCull( byte *cullBits, idVec2 *texCoords, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {
    const __m256 nx0 = _mm256_set1_ps( planes[0][0] );
    const __m256 ny0 = _mm256_set1_ps( planes[0][1] );
    const __m256 nz0 = _mm256_set1_ps( planes[0][2] );
    const __m256 d0  = _mm256_set1_ps( planes[0][3] );
    const __m256 nx1 = _mm256_set1_ps( planes[1][0] );
    const __m256 ny1 = _mm256_set1_ps( planes[1][1] );
    const __m256 nz1 = _mm256_set1_ps( planes[1][2] );
    const __m256 d1  = _mm256_set1_ps( planes[1][3] );
    const __m256 one = _mm256_set1_ps( 1.0f );
    const __m256 signMask = _mm256_castsi256_ps( _mm256_set1_epi32( (int)0x80000000 ) );
    int i = 0;

    for ( ; i + 8 <= numVerts; i += 8 ) {
        float x[8], y[8], z[8];
        float out0[8], out1[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            x[lane] = verts[i + lane].xyz.x;
            y[lane] = verts[i + lane].xyz.y;
            z[lane] = verts[i + lane].xyz.z;
        }
        const __m256 vx = _mm256_loadu_ps( x );
        const __m256 vy = _mm256_loadu_ps( y );
        const __m256 vz = _mm256_loadu_ps( z );

        __m256 value0 = _mm256_mul_ps( nx0, vx );
        value0 = _mm256_add_ps( value0, _mm256_mul_ps( ny0, vy ) );
        value0 = _mm256_add_ps( value0, _mm256_mul_ps( nz0, vz ) );
        value0 = _mm256_add_ps( value0, d0 );
        __m256 value1 = _mm256_mul_ps( nx1, vx );
        value1 = _mm256_add_ps( value1, _mm256_mul_ps( ny1, vy ) );
        value1 = _mm256_add_ps( value1, _mm256_mul_ps( nz1, vz ) );
        value1 = _mm256_add_ps( value1, d1 );

        const int sign0 = _mm256_movemask_ps( _mm256_and_ps( value0, signMask ) );
        const int sign1 = _mm256_movemask_ps( _mm256_and_ps( value1, signMask ) );
        const int sign2 = _mm256_movemask_ps( _mm256_and_ps( _mm256_sub_ps( one, value0 ), signMask ) );
        const int sign3 = _mm256_movemask_ps( _mm256_and_ps( _mm256_sub_ps( one, value1 ), signMask ) );
        _mm256_storeu_ps( out0, value0 );
        _mm256_storeu_ps( out1, value1 );

        for ( int lane = 0; lane < 8; ++lane ) {
            texCoords[i + lane][0] = out0[lane];
            texCoords[i + lane][1] = out1[lane];
            const int bit = 1 << lane;
            cullBits[i + lane] = (byte)(
                (((sign0 & bit) != 0) << 0) |
                (((sign1 & bit) != 0) << 1) |
                (((sign2 & bit) != 0) << 2) |
                (((sign3 & bit) != 0) << 3) );
        }
    }

    for ( ; i < numVerts; ++i ) {
        const idVec3 &v = verts[i].xyz;
        float value0 = planes[0].Distance( v );
        float value1 = planes[1].Distance( v );
        texCoords[i][0] = value0;
        texCoords[i][1] = value1;
        byte bits = (byte)( FLOATSIGNBITSET( value0 ) << 0 );
        bits |= (byte)( FLOATSIGNBITSET( value1 ) << 1 );
        value0 = 1.0f - value0;
        value1 = 1.0f - value1;
        bits |= (byte)( FLOATSIGNBITSET( value0 ) << 2 );
        bits |= (byte)( FLOATSIGNBITSET( value1 ) << 3 );
        cullBits[i] = bits;
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::TracePointCull

Evaluate the four trace planes for eight vertices at a time. DrawVert is
an AoS structure, so XYZ values are gathered into temporary SoA arrays.
The final bit packing deliberately uses sign bits, matching the generic
implementation including its treatment of signed zero and NaNs.
================
*/
/*
================
idSIMD_AVX2::CmpGE / CmpLE

Ordered comparisons preserve the scalar behavior for NaNs. The byte-output
variants replace each destination byte; the bit variants OR one result bit
into the existing culling mask.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::CmpGE( byte *dst, const float *src0, const float constant, const int count ) {
    const __m256 threshold = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const int mask = _mm256_movemask_ps( _mm256_cmp_ps( _mm256_loadu_ps( src0 + i ), threshold, _CMP_GE_OQ ) );
        for ( int lane = 0; lane < 8; ++lane ) dst[i + lane] = (byte)( ( mask >> lane ) & 1 );
    }
    for ( ; i < count; ++i ) dst[i] = (byte)( src0[i] >= constant );
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::CmpGE( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
    const __m256 threshold = _mm256_set1_ps( constant );
    for ( int i = 0; i + 8 <= count; i += 8 ) {
        const int mask = _mm256_movemask_ps( _mm256_cmp_ps( _mm256_loadu_ps( src0 + i ), threshold, _CMP_GE_OQ ) );
        for ( int lane = 0; lane < 8; ++lane ) dst[i + lane] |= (byte)( ( ( mask >> lane ) & 1 ) << bitNum );
    }
    int i = count & ~7;
    for ( ; i < count; ++i ) dst[i] |= (byte)( ( src0[i] >= constant ) << bitNum );
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::CmpLE( byte *dst, const float *src0, const float constant, const int count ) {
    const __m256 threshold = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const int mask = _mm256_movemask_ps( _mm256_cmp_ps( _mm256_loadu_ps( src0 + i ), threshold, _CMP_LE_OQ ) );
        for ( int lane = 0; lane < 8; ++lane ) dst[i + lane] = (byte)( ( mask >> lane ) & 1 );
    }
    for ( ; i < count; ++i ) dst[i] = (byte)( src0[i] <= constant );
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::CmpLE( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
    const __m256 threshold = _mm256_set1_ps( constant );
    for ( int i = 0; i + 8 <= count; i += 8 ) {
        const int mask = _mm256_movemask_ps( _mm256_cmp_ps( _mm256_loadu_ps( src0 + i ), threshold, _CMP_LE_OQ ) );
        for ( int lane = 0; lane < 8; ++lane ) dst[i + lane] |= (byte)( ( ( mask >> lane ) & 1 ) << bitNum );
    }
    int i = count & ~7;
    for ( ; i < count; ++i ) dst[i] |= (byte)( ( src0[i] <= constant ) << bitNum );
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::TracePointCull( byte *cullBits, byte &totalOr, const float radius, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {
    const __m256 nx0 = _mm256_set1_ps( planes[0][0] );
    const __m256 ny0 = _mm256_set1_ps( planes[0][1] );
    const __m256 nz0 = _mm256_set1_ps( planes[0][2] );
    const __m256 d0  = _mm256_set1_ps( planes[0][3] );
    const __m256 nx1 = _mm256_set1_ps( planes[1][0] );
    const __m256 ny1 = _mm256_set1_ps( planes[1][1] );
    const __m256 nz1 = _mm256_set1_ps( planes[1][2] );
    const __m256 d1  = _mm256_set1_ps( planes[1][3] );
    const __m256 nx2 = _mm256_set1_ps( planes[2][0] );
    const __m256 ny2 = _mm256_set1_ps( planes[2][1] );
    const __m256 nz2 = _mm256_set1_ps( planes[2][2] );
    const __m256 d2  = _mm256_set1_ps( planes[2][3] );
    const __m256 nx3 = _mm256_set1_ps( planes[3][0] );
    const __m256 ny3 = _mm256_set1_ps( planes[3][1] );
    const __m256 nz3 = _mm256_set1_ps( planes[3][2] );
    const __m256 d3  = _mm256_set1_ps( planes[3][3] );
    const __m256 r = _mm256_set1_ps( radius );
    const __m256 signMask = _mm256_castsi256_ps( _mm256_set1_epi32( (int)0x80000000 ) );
    byte aggregate = 0;
    int i = 0;

    for ( ; i + 8 <= numVerts; i += 8 ) {
        float x[8], y[8], z[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            x[lane] = verts[i + lane].xyz.x;
            y[lane] = verts[i + lane].xyz.y;
            z[lane] = verts[i + lane].xyz.z;
        }
        const __m256 vx = _mm256_loadu_ps( x );
        const __m256 vy = _mm256_loadu_ps( y );
        const __m256 vz = _mm256_loadu_ps( z );

        __m256 v0 = _mm256_mul_ps( nx0, vx );
        v0 = _mm256_add_ps( v0, _mm256_mul_ps( ny0, vy ) );
        v0 = _mm256_add_ps( v0, _mm256_mul_ps( nz0, vz ) );
        v0 = _mm256_add_ps( v0, d0 );
        __m256 v1 = _mm256_mul_ps( nx1, vx );
        v1 = _mm256_add_ps( v1, _mm256_mul_ps( ny1, vy ) );
        v1 = _mm256_add_ps( v1, _mm256_mul_ps( nz1, vz ) );
        v1 = _mm256_add_ps( v1, d1 );
        __m256 v2 = _mm256_mul_ps( nx2, vx );
        v2 = _mm256_add_ps( v2, _mm256_mul_ps( ny2, vy ) );
        v2 = _mm256_add_ps( v2, _mm256_mul_ps( nz2, vz ) );
        v2 = _mm256_add_ps( v2, d2 );
        __m256 v3 = _mm256_mul_ps( nx3, vx );
        v3 = _mm256_add_ps( v3, _mm256_mul_ps( ny3, vy ) );
        v3 = _mm256_add_ps( v3, _mm256_mul_ps( nz3, vz ) );
        v3 = _mm256_add_ps( v3, d3 );

        // Generic code sets low bits from !(signbit(distance + radius))
        // and high bits from signbit(distance - radius).
        const int lo0 = _mm256_movemask_ps( _mm256_and_ps( _mm256_add_ps( v0, r ), signMask ) );
        const int lo1 = _mm256_movemask_ps( _mm256_and_ps( _mm256_add_ps( v1, r ), signMask ) );
        const int lo2 = _mm256_movemask_ps( _mm256_and_ps( _mm256_add_ps( v2, r ), signMask ) );
        const int lo3 = _mm256_movemask_ps( _mm256_and_ps( _mm256_add_ps( v3, r ), signMask ) );
        const int hi0 = _mm256_movemask_ps( _mm256_and_ps( _mm256_sub_ps( v0, r ), signMask ) );
        const int hi1 = _mm256_movemask_ps( _mm256_and_ps( _mm256_sub_ps( v1, r ), signMask ) );
        const int hi2 = _mm256_movemask_ps( _mm256_and_ps( _mm256_sub_ps( v2, r ), signMask ) );
        const int hi3 = _mm256_movemask_ps( _mm256_and_ps( _mm256_sub_ps( v3, r ), signMask ) );

        for ( int lane = 0; lane < 8; ++lane ) {
            const int bit = 1 << lane;
            const byte bits = (byte)(
                (((lo0 & bit) ? 0 : 1) << 0) |
                (((lo1 & bit) ? 0 : 1) << 1) |
                (((lo2 & bit) ? 0 : 1) << 2) |
                (((lo3 & bit) ? 0 : 1) << 3) |
                (((hi0 & bit) ? 1 : 0) << 4) |
                (((hi1 & bit) ? 1 : 0) << 5) |
                (((hi2 & bit) ? 1 : 0) << 6) |
                (((hi3 & bit) ? 1 : 0) << 7) );
            cullBits[i + lane] = bits;
            aggregate |= bits;
        }
    }

    for ( ; i < numVerts; ++i ) {
        const idVec3 &v = verts[i].xyz;
        byte bits = 0;
        for ( int p = 0; p < 4; ++p ) {
            const float d = planes[p].Distance( v );
            const float lower = d + radius;
            const float upper = d - radius;
            bits |= (byte)( FLOATSIGNBITSET( lower ) << p );
            bits |= (byte)( FLOATSIGNBITSET( upper ) << ( p + 4 ) );
        }
        bits ^= 0x0F;
        cullBits[i] = bits;
        aggregate |= bits;
    }
    totalOr = aggregate;
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::DeriveTangents

Eight triangles are evaluated in parallel. Vertex gathers and the final
shared-vertex accumulation remain scalar because the input is AoS and
multiple triangles can reference the same vertex.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::DeriveTangents( idPlane *planes, idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) {
    if ( numVerts <= 0 || numIndexes <= 0 ) {
        return;
    }

    bool *used = (bool *)_alloca16( numVerts * sizeof( used[0] ) );
    memset( used, 0, numVerts * sizeof( used[0] ) );

    const __m256 signMask = _mm256_castsi256_ps( _mm256_set1_epi32( (int)0x80000000 ) );
    int tri = 0;
    const int numTriangles = numIndexes / 3;

    for ( ; tri + 8 <= numTriangles; tri += 8 ) {
        int v0[8], v1[8], v2[8];
        float d0x[8], d0y[8], d0z[8], d0s[8], d0t[8];
        float d1x[8], d1y[8], d1z[8], d1s[8], d1t[8];

        for ( int lane = 0; lane < 8; ++lane ) {
            v0[lane] = indexes[(tri + lane) * 3 + 0];
            v1[lane] = indexes[(tri + lane) * 3 + 1];
            v2[lane] = indexes[(tri + lane) * 3 + 2];
            const idDrawVert &a = verts[v0[lane]];
            const idDrawVert &b = verts[v1[lane]];
            const idDrawVert &c = verts[v2[lane]];
            d0x[lane] = b.xyz.x - a.xyz.x;
            d0y[lane] = b.xyz.y - a.xyz.y;
            d0z[lane] = b.xyz.z - a.xyz.z;
            d0s[lane] = b.st.x - a.st.x;
            d0t[lane] = b.st.y - a.st.y;
            d1x[lane] = c.xyz.x - a.xyz.x;
            d1y[lane] = c.xyz.y - a.xyz.y;
            d1z[lane] = c.xyz.z - a.xyz.z;
            d1s[lane] = c.st.x - a.st.x;
            d1t[lane] = c.st.y - a.st.y;
        }

        const __m256 x0 = _mm256_loadu_ps(d0x), y0 = _mm256_loadu_ps(d0y), z0 = _mm256_loadu_ps(d0z);
        const __m256 s0 = _mm256_loadu_ps(d0s), t0 = _mm256_loadu_ps(d0t);
        const __m256 x1 = _mm256_loadu_ps(d1x), y1 = _mm256_loadu_ps(d1y), z1 = _mm256_loadu_ps(d1z);
        const __m256 s1 = _mm256_loadu_ps(d1s), t1 = _mm256_loadu_ps(d1t);

        // Face normal, UV handedness, and tangent directions.
        __m256 nx = _mm256_fmsub_ps( y1, z0, _mm256_mul_ps( z1, y0 ) );
        __m256 ny = _mm256_fmsub_ps( z1, x0, _mm256_mul_ps( x1, z0 ) );
        __m256 nz = _mm256_fmsub_ps( x1, y0, _mm256_mul_ps( y1, x0 ) );
        __m256 n2 = _mm256_fmadd_ps( nx, nx, _mm256_fmadd_ps( ny, ny, _mm256_mul_ps( nz, nz ) ) );
        __m256 invN = _mm256_rsqrt_ps( n2 );
        nx = _mm256_mul_ps( nx, invN ); ny = _mm256_mul_ps( ny, invN ); nz = _mm256_mul_ps( nz, invN );

        const __m256 area = _mm256_fmsub_ps( s0, t1, _mm256_mul_ps( t0, s1 ) );
        const __m256 sign = _mm256_and_ps( area, signMask );
        __m256 tx = _mm256_fmsub_ps( x0, t1, _mm256_mul_ps( x1, t0 ) );
        __m256 ty = _mm256_fmsub_ps( y0, t1, _mm256_mul_ps( y1, t0 ) );
        __m256 tz = _mm256_fmsub_ps( z0, t1, _mm256_mul_ps( z1, t0 ) );
        __m256 t2 = _mm256_fmadd_ps( tx, tx, _mm256_fmadd_ps( ty, ty, _mm256_mul_ps( tz, tz ) ) );
        __m256 invT = _mm256_xor_ps( _mm256_rsqrt_ps( t2 ), sign );
        tx = _mm256_mul_ps( tx, invT ); ty = _mm256_mul_ps( ty, invT ); tz = _mm256_mul_ps( tz, invT );

        __m256 bx = _mm256_fmsub_ps( s0, x1, _mm256_mul_ps( x0, s1 ) );
        __m256 by = _mm256_fmsub_ps( s0, y1, _mm256_mul_ps( y0, s1 ) );
        __m256 bz = _mm256_fmsub_ps( s0, z1, _mm256_mul_ps( z0, s1 ) );
        __m256 b2 = _mm256_fmadd_ps( bx, bx, _mm256_fmadd_ps( by, by, _mm256_mul_ps( bz, bz ) ) );
        __m256 invB = _mm256_xor_ps( _mm256_rsqrt_ps( b2 ), sign );
        bx = _mm256_mul_ps( bx, invB ); by = _mm256_mul_ps( by, invB ); bz = _mm256_mul_ps( bz, invB );

        float fnx[8], fny[8], fnz[8], ftx[8], fty[8], ftz[8], fbx[8], fby[8], fbz[8];
        _mm256_storeu_ps( fnx, nx ); _mm256_storeu_ps( fny, ny ); _mm256_storeu_ps( fnz, nz );
        _mm256_storeu_ps( ftx, tx ); _mm256_storeu_ps( fty, ty ); _mm256_storeu_ps( ftz, tz );
        _mm256_storeu_ps( fbx, bx ); _mm256_storeu_ps( fby, by ); _mm256_storeu_ps( fbz, bz );

        for ( int lane = 0; lane < 8; ++lane ) {
            const idVec3 normal( fnx[lane], fny[lane], fnz[lane] );
            planes[tri + lane].SetNormal( normal );
            planes[tri + lane].FitThroughPoint( verts[v0[lane]].xyz );
            const idVec3 tangent0( ftx[lane], fty[lane], ftz[lane] );
            const idVec3 tangent1( fbx[lane], fby[lane], fbz[lane] );
            const int ids[3] = { v0[lane], v1[lane], v2[lane] };
            for ( int j = 0; j < 3; ++j ) {
                idDrawVert &v = verts[ids[j]];
                if ( used[ids[j]] ) {
                    v.normal += normal;
                    v.tangents[0] += tangent0;
                    v.tangents[1] += tangent1;
                } else {
                    v.normal = normal;
                    v.tangents[0] = tangent0;
                    v.tangents[1] = tangent1;
                    used[ids[j]] = true;
                }
            }
        }
    }

    // Preserve the reference implementation's exact accumulation order for
    // the remaining triangles (and avoid over-reading a partial SIMD batch).
    for ( ; tri < numTriangles; ++tri ) {
        idDrawVert *a = verts + indexes[tri * 3 + 0];
        idDrawVert *b = verts + indexes[tri * 3 + 1];
        idDrawVert *c = verts + indexes[tri * 3 + 2];
        const idVec3 d0 = b->xyz - a->xyz;
        const idVec3 d1 = c->xyz - a->xyz;
        const idVec2 st0 = b->st - a->st;
        const idVec2 st1 = c->st - a->st;
        idVec3 n = d1.Cross( d0 );
        n.NormalizeFast();
        planes[tri].SetNormal( n );
        planes[tri].FitThroughPoint( a->xyz );
        const float area = st0.x * st1.y - st0.y * st1.x;
        const float sign = area < 0.0f ? -1.0f : 1.0f;
        idVec3 t0 = d0 * st1.y - d1 * st0.y;
        t0 *= sign * idMath::RSqrt( t0.LengthSqr() );
        idVec3 t1 = d1 * st0.x - d0 * st1.x;
        t1 *= sign * idMath::RSqrt( t1.LengthSqr() );
        idDrawVert *v[3] = { a, b, c };
        for ( int j = 0; j < 3; ++j ) {
            int index = indexes[tri * 3 + j];
            if ( used[index] ) {
                v[j]->normal += n;
                v[j]->tangents[0] += t0;
                v[j]->tangents[1] += t1;
            } else {
                v[j]->normal = n;
                v[j]->tangents[0] = t0;
                v[j]->tangents[1] = t1;
                used[index] = true;
            }
        }
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::Zero16 / Negate16 / Copy16 / Add16 / Sub16 / Mul16

These fixed-width math helpers are also used by idMatX. Process the exact
requested element count, including a scalar tail, rather than relying on
padding or alignment assumptions.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Zero16( float *dst, const int count ) {
    const __m256 zero = _mm256_setzero_ps();
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) _mm256_storeu_ps( dst + i, zero );
    for ( ; i < count; ++i ) dst[i] = 0.0f;
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Negate16( float *dst, const int count ) {
    const __m256 sign = _mm256_castsi256_ps( _mm256_set1_epi32( (int)0x80000000u ) );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( dst + i );
        _mm256_storeu_ps( dst + i, _mm256_xor_ps( value, sign ) );
    }
    for ( ; i < count; ++i ) {
        unsigned int *bits = reinterpret_cast<unsigned int *>( dst + i );
        *bits ^= 0x80000000u;
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Copy16( float *dst, const float *src, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( src + i );
        _mm256_storeu_ps( dst + i, value );
    }
    for ( ; i < count; ++i ) dst[i] = src[i];
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Add16( float *dst, const float *src1, const float *src2, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src1 + i );
        const __m256 b = _mm256_loadu_ps( src2 + i );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( a, b ) );
    }
    for ( ; i < count; ++i ) dst[i] = src1[i] + src2[i];
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Sub16( float *dst, const float *src1, const float *src2, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( src1 + i );
        const __m256 b = _mm256_loadu_ps( src2 + i );
        _mm256_storeu_ps( dst + i, _mm256_sub_ps( a, b ) );
    }
    for ( ; i < count; ++i ) dst[i] = src1[i] - src2[i];
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::Mul16( float *dst, const float *src1, const float constant, const int count ) {
    const __m256 scale = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( src1 + i );
        _mm256_storeu_ps( dst + i, _mm256_mul_ps( value, scale ) );
    }
    for ( ; i < count; ++i ) dst[i] = src1[i] * constant;
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::AddAssign16( float *dst, const float *src, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( dst + i );
        const __m256 b = _mm256_loadu_ps( src + i );
        _mm256_storeu_ps( dst + i, _mm256_add_ps( a, b ) );
    }
    for ( ; i < count; ++i ) dst[i] += src[i];
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::SubAssign16( float *dst, const float *src, const int count ) {
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 a = _mm256_loadu_ps( dst + i );
        const __m256 b = _mm256_loadu_ps( src + i );
        _mm256_storeu_ps( dst + i, _mm256_sub_ps( a, b ) );
    }
    for ( ; i < count; ++i ) dst[i] -= src[i];
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MulAssign16( float *dst, const float constant, const int count ) {
    const __m256 scale = _mm256_set1_ps( constant );
    int i = 0;
    for ( ; i + 8 <= count; i += 8 ) {
        const __m256 value = _mm256_loadu_ps( dst + i );
        _mm256_storeu_ps( dst + i, _mm256_mul_ps( value, scale ) );
    }
    for ( ; i < count; ++i ) dst[i] *= constant;
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::NormalizeTangents

The per-vertex structure is gathered into SoA vectors, orthogonalized and
normalized eight vertices at a time, then written back to the original AoS.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::NormalizeTangents( idDrawVert *verts, const int numVerts ) {
    int i = 0;
    for ( ; i + 8 <= numVerts; i += 8 ) {
        float nx[8], ny[8], nz[8], ux[8], uy[8], uz[8], vx[8], vy[8], vz[8];
        for ( int lane = 0; lane < 8; ++lane ) {
            const idDrawVert &v = verts[i + lane];
            nx[lane] = v.normal.x; ny[lane] = v.normal.y; nz[lane] = v.normal.z;
            ux[lane] = v.tangents[0].x; uy[lane] = v.tangents[0].y; uz[lane] = v.tangents[0].z;
            vx[lane] = v.tangents[1].x; vy[lane] = v.tangents[1].y; vz[lane] = v.tangents[1].z;
        }
        __m256 x = _mm256_loadu_ps( nx ), y = _mm256_loadu_ps( ny ), z = _mm256_loadu_ps( nz );
        __m256 len = _mm256_fmadd_ps( x, x, _mm256_fmadd_ps( y, y, _mm256_mul_ps( z, z ) ) );
        __m256 inv = _mm256_rsqrt_ps( len );
        x = _mm256_mul_ps( x, inv ); y = _mm256_mul_ps( y, inv ); z = _mm256_mul_ps( z, inv );

        __m256 u0 = _mm256_loadu_ps( ux ), u1 = _mm256_loadu_ps( uy ), u2 = _mm256_loadu_ps( uz );
        __m256 v0 = _mm256_loadu_ps( vx ), v1 = _mm256_loadu_ps( vy ), v2 = _mm256_loadu_ps( vz );
        __m256 du = _mm256_fmadd_ps( u0, x, _mm256_fmadd_ps( u1, y, _mm256_mul_ps( u2, z ) ) );
        __m256 dv = _mm256_fmadd_ps( v0, x, _mm256_fmadd_ps( v1, y, _mm256_mul_ps( v2, z ) ) );
        u0 = _mm256_fnmadd_ps( du, x, u0 ); u1 = _mm256_fnmadd_ps( du, y, u1 ); u2 = _mm256_fnmadd_ps( du, z, u2 );
        v0 = _mm256_fnmadd_ps( dv, x, v0 ); v1 = _mm256_fnmadd_ps( dv, y, v1 ); v2 = _mm256_fnmadd_ps( dv, z, v2 );
        __m256 ul = _mm256_rsqrt_ps( _mm256_fmadd_ps( u0, u0, _mm256_fmadd_ps( u1, u1, _mm256_mul_ps( u2, u2 ) ) ) );
        __m256 vl = _mm256_rsqrt_ps( _mm256_fmadd_ps( v0, v0, _mm256_fmadd_ps( v1, v1, _mm256_mul_ps( v2, v2 ) ) ) );
        u0 = _mm256_mul_ps( u0, ul ); u1 = _mm256_mul_ps( u1, ul ); u2 = _mm256_mul_ps( u2, ul );
        v0 = _mm256_mul_ps( v0, vl ); v1 = _mm256_mul_ps( v1, vl ); v2 = _mm256_mul_ps( v2, vl );
        _mm256_storeu_ps( nx, x ); _mm256_storeu_ps( ny, y ); _mm256_storeu_ps( nz, z );
        _mm256_storeu_ps( ux, u0 ); _mm256_storeu_ps( uy, u1 ); _mm256_storeu_ps( uz, u2 );
        _mm256_storeu_ps( vx, v0 ); _mm256_storeu_ps( vy, v1 ); _mm256_storeu_ps( vz, v2 );
        for ( int lane = 0; lane < 8; ++lane ) {
            idDrawVert &v = verts[i + lane];
            v.normal.Set( nx[lane], ny[lane], nz[lane] );
            v.tangents[0].Set( ux[lane], uy[lane], uz[lane] );
            v.tangents[1].Set( vx[lane], vy[lane], vz[lane] );
        }
    }
    for ( ; i < numVerts; ++i ) {
        idVec3 &normal = verts[i].normal;
        normal.NormalizeFast();
        for ( int j = 0; j < 2; ++j ) {
            idVec3 &tangent = verts[i].tangents[j];
            tangent -= ( tangent * normal ) * normal;
            tangent.NormalizeFast();
        }
    }
    _mm256_zeroupper();
}

/*
================
idSIMD_AVX2::MatX_MultiplyVecX / MultiplyAddVecX / MultiplySubVecX

Vectorize across matrix rows. Each AVX lane owns one row, preserving the
per-row multiply/accumulation order of the generic implementation. Gather
handles idMatX's row-major layout; small matrices stay on the generic path.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_MultiplyVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) {
    const int rows = mat.GetNumRows(), cols = mat.GetNumColumns();
    assert( vec.GetSize() >= cols ); assert( dst.GetSize() >= rows );
    if ( rows < 8 || cols < 8 || dst.ToFloatPtr() == vec.ToFloatPtr() ) { idSIMD_Generic::MatX_MultiplyVecX( dst, mat, vec ); return; }
    const float *m = mat.ToFloatPtr(), *v = vec.ToFloatPtr(); float *d = dst.ToFloatPtr();
    const __m256i offsets = _mm256_setr_epi32( 0, cols, 2*cols, 3*cols, 4*cols, 5*cols, 6*cols, 7*cols );
    int row = 0;
    for ( ; row + 8 <= rows; row += 8 ) {
        const float *base = m + row * cols;
        __m256 sum = _mm256_mul_ps( _mm256_i32gather_ps( base, offsets, 4 ), _mm256_set1_ps( v[0] ) );
        for ( int col = 1; col < cols; ++col ) sum = _mm256_add_ps( sum, _mm256_mul_ps( _mm256_i32gather_ps( base + col, offsets, 4 ), _mm256_set1_ps( v[col] ) ) );
        _mm256_storeu_ps( d + row, sum );
    }
    for ( ; row < rows; ++row ) { const float *r = m + row * cols; float sum = r[0] * v[0]; for ( int col = 1; col < cols; ++col ) sum += r[col] * v[col]; d[row] = sum; }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_MultiplyAddVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) {
    const int rows = mat.GetNumRows(), cols = mat.GetNumColumns();
    assert( vec.GetSize() >= cols ); assert( dst.GetSize() >= rows );
    if ( rows < 8 || cols < 8 || dst.ToFloatPtr() == vec.ToFloatPtr() ) { idSIMD_Generic::MatX_MultiplyAddVecX( dst, mat, vec ); return; }
    const float *m = mat.ToFloatPtr(), *v = vec.ToFloatPtr(); float *d = dst.ToFloatPtr();
    const __m256i offsets = _mm256_setr_epi32( 0, cols, 2*cols, 3*cols, 4*cols, 5*cols, 6*cols, 7*cols );
    int row = 0;
    for ( ; row + 8 <= rows; row += 8 ) {
        const float *base = m + row * cols;
        __m256 sum = _mm256_mul_ps( _mm256_i32gather_ps( base, offsets, 4 ), _mm256_set1_ps( v[0] ) );
        for ( int col = 1; col < cols; ++col ) sum = _mm256_add_ps( sum, _mm256_mul_ps( _mm256_i32gather_ps( base + col, offsets, 4 ), _mm256_set1_ps( v[col] ) ) );
        _mm256_storeu_ps( d + row, _mm256_add_ps( _mm256_loadu_ps( d + row ), sum ) );
    }
    for ( ; row < rows; ++row ) { const float *r = m + row * cols; float sum = r[0] * v[0]; for ( int col = 1; col < cols; ++col ) sum += r[col] * v[col]; d[row] += sum; }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_MultiplySubVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) {
    const int rows = mat.GetNumRows(), cols = mat.GetNumColumns();
    assert( vec.GetSize() >= cols ); assert( dst.GetSize() >= rows );
    if ( rows < 8 || cols < 8 || dst.ToFloatPtr() == vec.ToFloatPtr() ) { idSIMD_Generic::MatX_MultiplySubVecX( dst, mat, vec ); return; }
    const float *m = mat.ToFloatPtr(), *v = vec.ToFloatPtr(); float *d = dst.ToFloatPtr();
    const __m256i offsets = _mm256_setr_epi32( 0, cols, 2*cols, 3*cols, 4*cols, 5*cols, 6*cols, 7*cols );
    int row = 0;
    for ( ; row + 8 <= rows; row += 8 ) {
        const float *base = m + row * cols;
        __m256 sum = _mm256_mul_ps( _mm256_i32gather_ps( base, offsets, 4 ), _mm256_set1_ps( v[0] ) );
        for ( int col = 1; col < cols; ++col ) sum = _mm256_add_ps( sum, _mm256_mul_ps( _mm256_i32gather_ps( base + col, offsets, 4 ), _mm256_set1_ps( v[col] ) ) );
        _mm256_storeu_ps( d + row, _mm256_sub_ps( _mm256_loadu_ps( d + row ), sum ) );
    }
    for ( ; row < rows; ++row ) { const float *r = m + row * cols; float sum = r[0] * v[0]; for ( int col = 1; col < cols; ++col ) sum += r[col] * v[col]; d[row] -= sum; }
    _mm256_zeroupper();
}


/*
================
idSIMD_AVX2::MatX_TransposeMultiplyVecX / MultiplyAddVecX / MultiplySubVecX

Vectorize across contiguous output columns. Each row contributes one
broadcast vector value, so the per-output accumulation order matches the
generic implementation. The row-major matrix layout allows ordinary
unaligned loads rather than gathers.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_TransposeMultiplyVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) {
    const int rows = mat.GetNumRows(), cols = mat.GetNumColumns();
    assert( vec.GetSize() >= rows ); assert( dst.GetSize() >= cols );
    if ( cols < 8 || dst.ToFloatPtr() == vec.ToFloatPtr() ) { idSIMD_Generic::MatX_TransposeMultiplyVecX( dst, mat, vec ); return; }
    const float *m = mat.ToFloatPtr(), *v = vec.ToFloatPtr(); float *d = dst.ToFloatPtr();
    int col = 0;
    for ( ; col + 8 <= cols; col += 8 ) {
        __m256 sum = _mm256_mul_ps( _mm256_loadu_ps( m + col ), _mm256_set1_ps( v[0] ) );
        for ( int row = 1; row < rows; ++row )
            sum = _mm256_add_ps( sum, _mm256_mul_ps( _mm256_loadu_ps( m + row * cols + col ), _mm256_set1_ps( v[row] ) ) );
        _mm256_storeu_ps( d + col, sum );
    }
    for ( ; col < cols; ++col ) {
        float sum = m[col] * v[0];
        for ( int row = 1; row < rows; ++row ) sum += m[row * cols + col] * v[row];
        d[col] = sum;
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_TransposeMultiplyAddVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) {
    const int rows = mat.GetNumRows(), cols = mat.GetNumColumns();
    assert( vec.GetSize() >= rows ); assert( dst.GetSize() >= cols );
    if ( cols < 8 || dst.ToFloatPtr() == vec.ToFloatPtr() ) { idSIMD_Generic::MatX_TransposeMultiplyAddVecX( dst, mat, vec ); return; }
    const float *m = mat.ToFloatPtr(), *v = vec.ToFloatPtr(); float *d = dst.ToFloatPtr();
    int col = 0;
    for ( ; col + 8 <= cols; col += 8 ) {
        __m256 sum = _mm256_mul_ps( _mm256_loadu_ps( m + col ), _mm256_set1_ps( v[0] ) );
        for ( int row = 1; row < rows; ++row )
            sum = _mm256_add_ps( sum, _mm256_mul_ps( _mm256_loadu_ps( m + row * cols + col ), _mm256_set1_ps( v[row] ) ) );
        _mm256_storeu_ps( d + col, _mm256_add_ps( _mm256_loadu_ps( d + col ), sum ) );
    }
    for ( ; col < cols; ++col ) {
        float sum = m[col] * v[0];
        for ( int row = 1; row < rows; ++row ) sum += m[row * cols + col] * v[row];
        d[col] += sum;
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_TransposeMultiplySubVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) {
    const int rows = mat.GetNumRows(), cols = mat.GetNumColumns();
    assert( vec.GetSize() >= rows ); assert( dst.GetSize() >= cols );
    if ( cols < 8 || dst.ToFloatPtr() == vec.ToFloatPtr() ) { idSIMD_Generic::MatX_TransposeMultiplySubVecX( dst, mat, vec ); return; }
    const float *m = mat.ToFloatPtr(), *v = vec.ToFloatPtr(); float *d = dst.ToFloatPtr();
    int col = 0;
    for ( ; col + 8 <= cols; col += 8 ) {
        __m256 sum = _mm256_mul_ps( _mm256_loadu_ps( m + col ), _mm256_set1_ps( v[0] ) );
        for ( int row = 1; row < rows; ++row )
            sum = _mm256_add_ps( sum, _mm256_mul_ps( _mm256_loadu_ps( m + row * cols + col ), _mm256_set1_ps( v[row] ) ) );
        _mm256_storeu_ps( d + col, _mm256_sub_ps( _mm256_loadu_ps( d + col ), sum ) );
    }
    for ( ; col < cols; ++col ) {
        float sum = m[col] * v[0];
        for ( int row = 1; row < rows; ++row ) sum += m[row * cols + col] * v[row];
        d[col] -= sum;
    }
    _mm256_zeroupper();
}



/*
================
idSIMD_AVX2::MatX_MultiplyMatX / MatX_TransposeMultiplyMatX

For the general-size matrix paths, accumulate four output columns in
parallel using double-precision lanes.  This matches the generic routines'
double accumulator semantics (used when the shared dimension is > 6), while
avoiding the narrow 6-column cases for which AVX2 would not be profitable.
================
*/
D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_MultiplyMatX( idMatX &dst, const idMatX &m1, const idMatX &m2 ) {
    assert( m1.GetNumColumns() == m2.GetNumRows() );
    const int rows = m1.GetNumRows();
    const int inner = m1.GetNumColumns();
    const int cols = m2.GetNumColumns();
    const float *a = m1.ToFloatPtr();
    const float *b = m2.ToFloatPtr();
    float *d = dst.ToFloatPtr();

    // The generic implementation has specialized, faster code for dimensions
    // 1..6.  Also preserve its behavior if the destination aliases an input.
    if ( inner <= 6 || d == a || d == b ) {
        idSIMD_SSE::MatX_MultiplyMatX( dst, m1, m2 );
        return;
    }

    for ( int i = 0; i < rows; ++i ) {
        int j = 0;
        for ( ; j + 4 <= cols; j += 4 ) {
            __m256d sum = _mm256_cvtps_pd( _mm_mul_ps(
                _mm_set1_ps( a[i * inner] ),
                _mm_loadu_ps( b + j ) ) );
            for ( int n = 1; n < inner; ++n ) {
                const __m128 product = _mm_mul_ps( _mm_set1_ps( a[i * inner + n] ), _mm_loadu_ps( b + n * cols + j ) );
                sum = _mm256_add_pd( sum, _mm256_cvtps_pd( product ) );
            }
            _mm_storeu_ps( d + i * cols + j, _mm256_cvtpd_ps( sum ) );
        }
        for ( ; j < cols; ++j ) {
            double sum = a[i * inner] * b[j];
            for ( int n = 1; n < inner; ++n ) {
                sum += a[i * inner + n] * b[n * cols + j];
            }
            d[i * cols + j] = static_cast<float>( sum );
        }
    }
    _mm256_zeroupper();
}

D3_AVX2_TARGET void VPCALL idSIMD_AVX2::MatX_TransposeMultiplyMatX( idMatX &dst, const idMatX &m1, const idMatX &m2 ) {
    assert( m1.GetNumRows() == m2.GetNumRows() );
    const int rows = m1.GetNumRows();
    const int cols1 = m1.GetNumColumns();
    const int cols2 = m2.GetNumColumns();
    const float *a = m1.ToFloatPtr();
    const float *b = m2.ToFloatPtr();
    float *d = dst.ToFloatPtr();

    if ( rows <= 6 || d == a || d == b ) {
        idSIMD_SSE::MatX_TransposeMultiplyMatX( dst, m1, m2 );
        return;
    }

    for ( int i = 0; i < cols1; ++i ) {
        int j = 0;
        for ( ; j + 4 <= cols2; j += 4 ) {
            __m256d sum = _mm256_cvtps_pd( _mm_mul_ps( _mm_set1_ps( a[i] ), _mm_loadu_ps( b + j ) ) );
            for ( int n = 1; n < rows; ++n ) {
                const __m128 product = _mm_mul_ps( _mm_set1_ps( a[n * cols1 + i] ), _mm_loadu_ps( b + n * cols2 + j ) );
                sum = _mm256_add_pd( sum, _mm256_cvtps_pd( product ) );
            }
            _mm_storeu_ps( d + i * cols2 + j, _mm256_cvtpd_ps( sum ) );
        }
        for ( ; j < cols2; ++j ) {
            double sum = a[i] * b[j];
            for ( int n = 1; n < rows; ++n ) {
                sum += a[n * cols1 + i] * b[n * cols2 + j];
            }
            d[i * cols2 + j] = static_cast<float>( sum );
        }
    }
    _mm256_zeroupper();
}

#endif /* D3_HAVE_AVX2_SIMD */
