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

#ifndef __MATH_SIMD_AVX2_H__
#define __MATH_SIMD_AVX2_H__

#include "idlib/math/Simd_SSE3.h"

// Keep AVX2 code isolated in target-specific functions. The rest of dhewm3
// is not compiled with -mavx/-mavx2 and remains runnable on older CPUs.
#if ( defined(__GNUC__) || defined(__clang__) ) && ( defined(__i386__) || defined(__x86_64__) )
#define D3_HAVE_AVX2_SIMD 1
#define D3_AVX2_TARGET __attribute__((target("avx2,fma")))

class idSIMD_AVX2 : public idSIMD_SSE3 {
public:
    using idSIMD_SSE::Dot;
    using idSIMD_Generic::Sub;
    using idSIMD_Generic::Mul;
    using idSIMD_Generic::Div;
    using idSIMD_Generic::MulAdd;
    using idSIMD_Generic::MulSub;
    using idSIMD_SSE::CmpGT;
    using idSIMD_SSE2::CmpLT;
    using idSIMD_SSE::MinMax;

    virtual const char * VPCALL GetName( void ) const;
    virtual void VPCALL Add( float *dst, const float constant, const float *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Add( float *dst, const float *src0, const float *src1, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Sub( float *dst, const float *src0, const float *src1, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Mul( float *dst, const float constant, const float *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Mul( float *dst, const float *src0, const float *src1, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Div( float *dst, const float constant, const float *divisor, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Div( float *dst, const float *src0, const float *src1, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Clamp( float *dst, const float *src, const float min, const float max, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL ClampMin( float *dst, const float *src, const float min, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL ClampMax( float *dst, const float *src, const float max, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL MulAdd( float *dst, const float constant, const float *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL MulAdd( float *dst, const float *src0, const float *src1, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL MulSub( float *dst, const float constant, const float *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL MulSub( float *dst, const float *src0, const float *src1, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float *dst, const idVec3 &constant, const idPlane *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float *dst, const idVec3 &constant, const idVec3 *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float *dst, const idPlane &constant, const idPlane *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float *dst, const idPlane &constant, const idVec3 *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float *dst, const idVec3 *src0, const idVec3 *src1, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float *dst, const idVec3 &constant, const idDrawVert *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float *dst, const idPlane &constant, const idDrawVert *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Dot( float &dot, const float *src1, const float *src2, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL CmpGT( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL CmpGE( byte *dst, const float *src0, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL CmpGE( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL CmpLE( byte *dst, const float *src0, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL CmpLE( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL CmpLT( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL TracePointCull( byte *cullBits, byte &totalOr, const float radius, const idPlane *planes, const idDrawVert *verts, const int numVerts ) D3_AVX2_TARGET;
    virtual void VPCALL DecalPointCull( byte *cullBits, const idPlane *planes, const idDrawVert *verts, const int numVerts ) D3_AVX2_TARGET;
    virtual void VPCALL OverlayPointCull( byte *cullBits, idVec2 *texCoords, const idPlane *planes, const idDrawVert *verts, const int numVerts ) D3_AVX2_TARGET;
    virtual void VPCALL DeriveTangents( idPlane *planes, idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) D3_AVX2_TARGET;
    virtual void VPCALL NormalizeTangents( idDrawVert *verts, const int numVerts ) D3_AVX2_TARGET;
    virtual void VPCALL MinMax( idVec3 &min, idVec3 &max, const idDrawVert *src, const int count ) D3_AVX2_TARGET;
    virtual int VPCALL CreateShadowCache( idVec4 *vertexCache, int *vertRemap, const idVec3 &lightOrigin, const idDrawVert *verts, const int numVerts ) D3_AVX2_TARGET;
    virtual void VPCALL Zero16( float *dst, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Negate16( float *dst, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Copy16( float *dst, const float *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Add16( float *dst, const float *src1, const float *src2, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Sub16( float *dst, const float *src1, const float *src2, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL Mul16( float *dst, const float *src1, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL AddAssign16( float *dst, const float *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL SubAssign16( float *dst, const float *src, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL MulAssign16( float *dst, const float constant, const int count ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_MultiplyVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_MultiplyAddVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_MultiplySubVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_TransposeMultiplyVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_TransposeMultiplyAddVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_TransposeMultiplySubVecX( idVecX &dst, const idMatX &mat, const idVecX &vec ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_MultiplyMatX( idMatX &dst, const idMatX &m1, const idMatX &m2 ) D3_AVX2_TARGET;
    virtual void VPCALL MatX_TransposeMultiplyMatX( idMatX &dst, const idMatX &m1, const idMatX &m2 ) D3_AVX2_TARGET;
    virtual void VPCALL ConvertJointQuatsToJointMats( idJointMat *jointMats, const idJointQuat *jointQuats, const int numJoints ) D3_AVX2_TARGET;
    virtual void VPCALL ConvertJointMatsToJointQuats( idJointQuat *jointQuats, const idJointMat *jointMats, const int numJoints ) D3_AVX2_TARGET;
    virtual void VPCALL TransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint ) D3_AVX2_TARGET;
    virtual void VPCALL UntransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint ) D3_AVX2_TARGET;
    virtual void VPCALL TransformVerts( idDrawVert *verts, const int numVerts, const idJointMat *joints, const idVec4 *weights, const int *index, const int numWeights ) D3_AVX2_TARGET;
    virtual void VPCALL DeriveTriPlanes( idPlane *planes, const idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) D3_AVX2_TARGET;
};
#else
#define D3_HAVE_AVX2_SIMD 0
#define D3_AVX2_TARGET
#endif

#endif /* !__MATH_SIMD_AVX2_H__ */
