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

#include "precompiled.h"
#pragma hdrstop

#if defined( MACOS_X )
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/*
===============================================================================

	idLib

===============================================================================
*/

idSys *			idLib::sys			= NULL;
idCommon *		idLib::common		= NULL;
idCVarSystem *	idLib::cvarSystem	= NULL;
idFileSystem *	idLib::fileSystem	= NULL;
int				idLib::frameNumber	= 0;

/*
================
idLib::Init
================
*/
void idLib::Init( void ) {

	assert( sizeof( bool ) == 1 );

	// assumptions from the scripting compiler/interpreter
	assert( sizeof( float ) == sizeof( int ) );
	assert( sizeof( idVec3 ) == sizeof( float ) * 3 );

	// initialize memory manager
	Mem_Init();

	// init string memory allocator
	idStr::InitMemory();

	// initialize generic SIMD implementation
	idSIMD::Init();

	// initialize math
	idMath::Init();

	// test idMatX
	//idMatX::Test();

	// test idPolynomial
	idPolynomial::Test();

	// initialize the dictionary string pools
	idDict::Init();
}

/*
================
idLib::ShutDown
================
*/
void idLib::ShutDown( void ) {

	// shut down the dictionary string pools
	idDict::Shutdown();

	// shut down the string memory allocator
	idStr::ShutdownMemory();

	// shut down the SIMD engine
	idSIMD::Shutdown();

	// shut down the memory manager
	Mem_Shutdown();
}


/*
===============================================================================

	Colors

===============================================================================
*/

idVec4	colorBlack	= idVec4( 0.00f, 0.00f, 0.00f, 1.00f );
idVec4	colorWhite	= idVec4( 1.00f, 1.00f, 1.00f, 1.00f );
idVec4	colorRed	= idVec4( 1.00f, 0.00f, 0.00f, 1.00f );
idVec4	colorGreen	= idVec4( 0.00f, 1.00f, 0.00f, 1.00f );
idVec4	colorBlue	= idVec4( 0.00f, 0.00f, 1.00f, 1.00f );
idVec4	colorYellow	= idVec4( 1.00f, 1.00f, 0.00f, 1.00f );
idVec4	colorMagenta= idVec4( 1.00f, 0.00f, 1.00f, 1.00f );
idVec4	colorCyan	= idVec4( 0.00f, 1.00f, 1.00f, 1.00f );
idVec4	colorOrange	= idVec4( 1.00f, 0.50f, 0.00f, 1.00f );
idVec4	colorPurple	= idVec4( 0.60f, 0.00f, 0.60f, 1.00f );
idVec4	colorPink	= idVec4( 0.73f, 0.40f, 0.48f, 1.00f );
idVec4	colorBrown	= idVec4( 0.40f, 0.35f, 0.08f, 1.00f );
idVec4	colorLtGrey	= idVec4( 0.75f, 0.75f, 0.75f, 1.00f );
idVec4	colorMdGrey	= idVec4( 0.50f, 0.50f, 0.50f, 1.00f );
idVec4	colorDkGrey	= idVec4( 0.25f, 0.25f, 0.25f, 1.00f );

static dword colorMask[2] = { 255, 0 };

/*
================
ColorFloatToByte
================
*/
ID_INLINE static byte ColorFloatToByte( float c ) {
	return (byte) ( ( (dword) ( c * 255.0f ) ) & colorMask[FLOATSIGNBITSET(c)] );
}

/*
================
PackColor
================
*/
dword PackColor( const idVec4 &color ) {
	dword dw, dx, dy, dz;

	dx = ColorFloatToByte( color.x );
	dy = ColorFloatToByte( color.y );
	dz = ColorFloatToByte( color.z );
	dw = ColorFloatToByte( color.w );

#if defined(ID_LITTLE_ENDIAN)
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 ) | ( dw << 24 );
#elif defined(ID_BIG_ENDIAN)
	return ( dx << 24 ) | ( dy << 16 ) | ( dz << 8 ) | ( dw << 0 );
#else
#error unknown endianness!
#endif
}

/*
================
UnpackColor
================
*/
void UnpackColor( const dword color, idVec4 &unpackedColor ) {
#if defined(ID_LITTLE_ENDIAN)
	unpackedColor.Set( ( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 24 ) & 255 ) * ( 1.0f / 255.0f ) );
#elif defined(ID_BIG_ENDIAN)
	unpackedColor.Set( ( ( color >> 24 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ) );
#else
#error unknown endianness!
#endif
}

/*
================
PackColor
================
*/
dword PackColor( const idVec3 &color ) {
	dword dx, dy, dz;

	dx = ColorFloatToByte( color.x );
	dy = ColorFloatToByte( color.y );
	dz = ColorFloatToByte( color.z );

#if defined(ID_LITTLE_ENDIAN)
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 );
#elif defined(ID_BIG_ENDIAN)
	return ( dy << 16 ) | ( dz << 8 ) | ( dx << 0 );
#else
#error unknown endianness!
#endif
}

/*
================
UnpackColor
================
*/
void UnpackColor( const dword color, idVec3 &unpackedColor ) {
#if defined(ID_LITTLE_ENDIAN)
	unpackedColor.Set( ( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ) );
#elif defined(ID_BIG_ENDIAN)
	unpackedColor.Set( ( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ) );
#else
#error unknown endianness!
#endif
}

/*
===============
idLib::Error
===============
*/
void idLib::Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	common->Error( "%s", text );
}

/*
===============
idLib::Warning
===============
*/
void idLib::Warning( const char *fmt, ... ) {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	common->Warning( "%s", text );
}

/*
===============================================================================

	Byte order functions

===============================================================================
*/

/*
================
ShortSwap
================
*/
ID_INLINE static short ShortSwap( short l ) {
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

/*
================
LongSwap
================
*/
ID_INLINE static int LongSwap ( int l ) {
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

/*
================
FloatSwap
================
*/
ID_INLINE static float FloatSwap( float f ) {
	union {
		float	f;
		byte	b[4];
	} dat1, dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

/*
=====================================================================
RevBytesSwap

Reverses byte order in place.

INPUTS
   bp       bytes to reverse
   elsize   size of the underlying data type
   elcount  number of elements to swap

RESULTS
   Reverses the byte order in each of elcount elements.
===================================================================== */
ID_INLINE static void RevBytesSwap( void *bp, int elsize, int elcount ) {
	register unsigned char *p, *q;

	p = ( unsigned char * ) bp;

	if ( elsize == 2 ) {
		q = p + 1;
		while ( elcount-- ) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			p += 2;
			q += 2;
		}
		return;
	}

	while ( elcount-- ) {
		q = p + elsize - 1;
		while ( p < q ) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			++p;
			--q;
		}
		p += elsize >> 1;
	}
}

/*
 =====================================================================
 RevBytesSwap

 Reverses byte order in place, then reverses bits in those bytes

 INPUTS
 bp       bitfield structure to reverse
 elsize   size of the underlying data type

 RESULTS
 Reverses the bitfield of size elsize.
 ===================================================================== */
ID_INLINE static void RevBitFieldSwap( void *bp, int elsize) {
	int i;
	unsigned char *p, t, v;

	LittleRevBytes( bp, elsize, 1 );

	p = (unsigned char *) bp;
	while ( elsize-- ) {
		v = *p;
		t = 0;
		for (i = 7; i; i--) {
			t <<= 1;
			v >>= 1;
			t |= v & 1;
		}
		*p++ = t;
	}
}

/*
================
SixtetsForIntLittle
================
*/
ID_INLINE static void SixtetsForIntLittle( byte *out, int src) {
	byte *b = (byte *)&src;
	out[0] = ( b[0] & 0xfc ) >> 2;
	out[1] = ( ( b[0] & 0x3 ) << 4 ) + ( ( b[1] & 0xf0 ) >> 4 );
	out[2] = ( ( b[1] & 0xf ) << 2 ) + ( ( b[2] & 0xc0 ) >> 6 );
	out[3] = b[2] & 0x3f;
}

/*
================
SixtetsForIntBig
TTimo: untested - that's the version from initial base64 encode
================
*/
ID_INLINE static void SixtetsForIntBig( byte *out, int src) {
	for( int i = 0 ; i < 4 ; i++ ) {
		out[i] = src & 0x3f;
		src >>= 6;
	}
}

/*
================
IntForSixtetsLittle
================
*/
ID_INLINE static int IntForSixtetsLittle( byte *in ) {
	int ret = 0;
	byte *b = (byte *)&ret;
	b[0] |= in[0] << 2;
	b[0] |= ( in[1] & 0x30 ) >> 4;
	b[1] |= ( in[1] & 0xf ) << 4;
	b[1] |= ( in[2] & 0x3c ) >> 2;
	b[2] |= ( in[2] & 0x3 ) << 6;
	b[2] |= in[3];
	return ret;
}

/*
================
IntForSixtetsBig
TTimo: untested - that's the version from initial base64 decode
================
*/
ID_INLINE static int IntForSixtetsBig( byte *in ) {
	int ret = 0;
	ret |= in[0];
	ret |= in[1] << 6;
	ret |= in[2] << 2*6;
	ret |= in[3] << 3*6;
	return ret;
}

/*
==========
Swap_IsBigEndian
==========
*/
bool Swap_IsBigEndian( void ) {
#if defined(ID_LITTLE_ENDIAN)
	return false;
#elif defined(ID_BIG_ENDIAN)
	return true;
#else
#error unknown endianness!
#endif
}

short	BigShort( short l ) {
#if defined(ID_LITTLE_ENDIAN)
	return ShortSwap(l);
#elif defined(ID_BIG_ENDIAN)
	return l;
#else
#error unknown endianness!
#endif
}

short	LittleShort( short l ) {
#if defined(ID_LITTLE_ENDIAN)
	return l;
#elif defined(ID_BIG_ENDIAN)
	return ShortSwap(l);
#else
#error unknown endianness!
#endif
}

int		BigLong( int l ) {
#if defined(ID_LITTLE_ENDIAN)
	return LongSwap(l);
#elif defined(ID_BIG_ENDIAN)
	return l;
#else
#error unknown endianness!
#endif
}

int		LittleLong( int l ) {
#if defined(ID_LITTLE_ENDIAN)
	return l;
#elif defined(ID_BIG_ENDIAN)
	return LongSwap(l);
#else
#error unknown endianness!
#endif
}

float	BigFloat( float l ) {
#if defined(ID_LITTLE_ENDIAN)
	return FloatSwap(l);
#elif defined(ID_BIG_ENDIAN)
	return l;
#else
#error unknown endianness!
#endif
}

float	LittleFloat( float l ) {
#if defined(ID_LITTLE_ENDIAN)
	return l;
#elif defined(ID_BIG_ENDIAN)
	return FloatSwap(l);
#else
#error unknown endianness!
#endif
}

void	BigRevBytes( void *bp, int elsize, int elcount ) {
#if defined(ID_LITTLE_ENDIAN)
	RevBytesSwap(bp, elsize, elcount);
#elif defined(ID_BIG_ENDIAN)
	return;
#else
#error unknown endianness!
#endif
}

void	LittleRevBytes( void *bp, int elsize, int elcount ){
#if defined(ID_LITTLE_ENDIAN)
	return;
#elif defined(ID_BIG_ENDIAN)
	RevBytesSwap(bp, elsize, elcount);
#else
#error unknown endianness!
#endif
}

void	LittleBitField( void *bp, int elsize ){
#if defined(ID_LITTLE_ENDIAN)
	return;
#elif defined(ID_BIG_ENDIAN)
	RevBitFieldSwap(bp, elsize);
#else
#error unknown endianness!
#endif
}

void	SixtetsForInt( byte *out, int src) {
#if defined(ID_LITTLE_ENDIAN)
	SixtetsForIntLittle(out, src);
#elif defined(ID_BIG_ENDIAN)
	SixtetsForIntBig(out, src);
#else
#error unknown endianness!
#endif
}

int		IntForSixtets( byte *in ) {
#if defined(ID_LITTLE_ENDIAN)
	return IntForSixtetsLittle(in);
#elif defined(ID_BIG_ENDIAN)
	return IntForSixtetsBig(in);
#else
#error unknown endianness!
#endif
}

/*
===============================================================================

	Assertion

===============================================================================
*/

void AssertFailed( const char *file, int line, const char *expression ) {
	idLib::sys->DebugPrintf( "\n\nASSERTION FAILED!\n%s(%d): '%s'\n", file, line, expression );
#ifdef _WIN32
	__asm int 0x03
#elif defined( __GNUC__ )
	__builtin_trap();
#else
#error dont know how to crash :P
#endif
}
