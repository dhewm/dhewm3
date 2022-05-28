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


#include "SDL_endian.h"
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  #define STB_VORBIS_BIG_ENDIAN
#endif
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API // we're using the pulldata API
#include "stb_vorbis.h"
#undef L // the implementation part of stb_vorbis has these defines, they confuse other code..
#undef C
#undef R

#include "sys/platform.h"
#include "framework/FileSystem.h"

#include "sound/snd_local.h"

/*
===================================================================================

  Thread safe decoder memory allocator.

  Each OggVorbis decoder consumes about 150kB of memory.

===================================================================================
*/

idDynamicBlockAlloc<byte, 1<<20, 128>		decoderMemoryAllocator;

const int MIN_OGGVORBIS_MEMORY				= 768 * 1024;

// DG: this was only used with original Doom3's patched libvorbis
// TODO: could use it in stb_vorbis setup_malloc() etc
#if 0
extern "C" {
	void *_decoder_malloc( size_t size );
	void *_decoder_calloc( size_t num, size_t size );
	void *_decoder_realloc( void *memblock, size_t size );
	void _decoder_free( void *memblock );
}

void *_decoder_malloc( size_t size ) {
	void *ptr = decoderMemoryAllocator.Alloc( size );
	assert( size == 0 || ptr != NULL );
	return ptr;
}

void *_decoder_calloc( size_t num, size_t size ) {
	void *ptr = decoderMemoryAllocator.Alloc( num * size );
	assert( ( num * size ) == 0 || ptr != NULL );
	memset( ptr, 0, num * size );
	return ptr;
}

void *_decoder_realloc( void *memblock, size_t size ) {
	void *ptr = decoderMemoryAllocator.Resize( (byte *)memblock, size );
	assert( size == 0 || ptr != NULL );
	return ptr;
}

void _decoder_free( void *memblock ) {
	decoderMemoryAllocator.Free( (byte *)memblock );
}
#endif

static const char* my_stbv_strerror(int stbVorbisError)
{
	switch(stbVorbisError)
	{
		case VORBIS__no_error: return "No Error";
#define ERRCASE(X) \
		case VORBIS_ ## X : return #X;

		ERRCASE( need_more_data )    // not a real error

		ERRCASE( invalid_api_mixing )           // can't mix API modes
		ERRCASE( outofmem )                     // not enough memory
		ERRCASE( feature_not_supported )        // uses floor 0
		ERRCASE( too_many_channels )            // STB_VORBIS_MAX_CHANNELS is too small
		ERRCASE( file_open_failure )            // fopen() failed
		ERRCASE( seek_without_length )          // can't seek in unknown-length file

		ERRCASE( unexpected_eof )               // file is truncated?
		ERRCASE( seek_invalid )                 // seek past EOF

		// decoding errors (corrupt/invalid stream) -- you probably
		// don't care about the exact details of these

		// vorbis errors:
		ERRCASE( invalid_setup )
		ERRCASE( invalid_stream )

		// ogg errors:
		ERRCASE( missing_capture_pattern )
		ERRCASE( invalid_stream_structure_version )
		ERRCASE( continued_packet_flag_invalid )
		ERRCASE( incorrect_stream_serial_number )
		ERRCASE( invalid_first_page )
		ERRCASE( bad_packet_type )
		ERRCASE( cant_find_last_page )
		ERRCASE( seek_failed )
		ERRCASE( ogg_skeleton_not_supported )

#undef ERRCASE
	}
	assert(0 && "unknown stb_vorbis errorcode!");
	return "Unknown Error!";
}


/*
===================================================================================

  OggVorbis file loading/decoding.

===================================================================================
*/

/*
====================
idWaveFile::OpenOGG
====================
*/
int idWaveFile::OpenOGG( const char* strFileName, waveformatex_t *pwfx ) {

	memset( pwfx, 0, sizeof( waveformatex_t ) );

	mhmmio = fileSystem->OpenFileRead( strFileName );
	if ( !mhmmio ) {
		return -1;
	}

	Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );

	int fileSize = mhmmio->Length();
	byte* oggFileData = (byte*)Mem_Alloc( fileSize );

	mhmmio->Read( oggFileData, fileSize );

	int stbverr = 0;
	stb_vorbis *ov = stb_vorbis_open_memory( oggFileData, fileSize, &stbverr, NULL );
	if( ov == NULL ) {
		Mem_Free( oggFileData );
		Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );
		common->Warning( "Opening OGG file '%s' with stb_vorbis failed: %s\n", strFileName, my_stbv_strerror(stbverr) );
		fileSystem->CloseFile( mhmmio );
		mhmmio = NULL;
		return -1;
	}

	mfileTime = mhmmio->Timestamp();

	stb_vorbis_info stbvi = stb_vorbis_get_info( ov );
	int numSamples = stb_vorbis_stream_length_in_samples( ov );
	if(numSamples == 0) {
		stbverr = stb_vorbis_get_error( ov );
		common->Warning( "Couldn't get sound length of '%s' with stb_vorbis: %s\n", strFileName, my_stbv_strerror(stbverr) );
		// TODO:  return -1 etc?
	}

	mpwfx.Format.nSamplesPerSec = stbvi.sample_rate;
	mpwfx.Format.nChannels = stbvi.channels;
	mpwfx.Format.wBitsPerSample = sizeof(short) * 8;
	mdwSize = numSamples * stbvi.channels;	// pcm samples * num channels
	mbIsReadingFromMemory = false;

	if ( idSoundSystemLocal::s_realTimeDecoding.GetBool() ) {

		stb_vorbis_close( ov );
		fileSystem->CloseFile( mhmmio );
		mhmmio = NULL;
		Mem_Free( oggFileData );

		mpwfx.Format.wFormatTag = WAVE_FORMAT_TAG_OGG;
		mhmmio = fileSystem->OpenFileRead( strFileName );
		mMemSize = mhmmio->Length();

	} else {

		ogg = ov;
		oggData = oggFileData;

		mpwfx.Format.wFormatTag = WAVE_FORMAT_TAG_PCM;
		mMemSize = mdwSize * sizeof( short );
	}

	memcpy( pwfx, &mpwfx, sizeof( waveformatex_t ) );

	Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );

	isOgg = true;

	return 0;
}

/*
====================
idWaveFile::ReadOGG
====================
*/
int idWaveFile::ReadOGG( byte* pBuffer, int dwSizeToRead, int *pdwSizeRead ) {
	// DG: Note that stb_vorbis_get_samples_short_interleaved() operates on shorts,
	//     while VorbisFile's ov_read() operates on bytes, so some numbers are different
	int total = dwSizeToRead/sizeof(short);
	short *bufferPtr = (short *)pBuffer;
	stb_vorbis *ov = (stb_vorbis *) ogg;

	do {
		int numShorts = total; // total >= 2048 ? 2048 : total; - I think stb_vorbis doesn't mind decoding all of it
		int ret = stb_vorbis_get_samples_short_interleaved( ov, mpwfx.Format.nChannels, bufferPtr, numShorts );
		if ( ret == 0 ) {
			break;
		}
		if ( ret < 0 ) {
			int stbverr = stb_vorbis_get_error( ov );
			common->Warning( "idWaveFile::ReadOGG() stb_vorbis_get_samples_short_interleaved() %d shorts failed: %s\n", numShorts, my_stbv_strerror(stbverr) );
			return -1;
		}
		// for some reason, stb_vorbis_get_samples_short_interleaved() takes the absolute
		// number of shorts to read as a function argument, but returns the number of samples
		// that were read PER CHANNEL
		ret *= mpwfx.Format.nChannels;
		bufferPtr += ret;
		total -= ret;
	} while( total > 0 );

	dwSizeToRead = (byte *)bufferPtr - pBuffer;

	if ( pdwSizeRead != NULL ) {
		*pdwSizeRead = dwSizeToRead;
	}

	return dwSizeToRead;
}

/*
====================
idWaveFile::CloseOGG
====================
*/
int idWaveFile::CloseOGG( void ) {
	stb_vorbis* ov = (stb_vorbis *)ogg;
	if ( ov != NULL ) {
		Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );
		stb_vorbis_close( ov );
		Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );
		fileSystem->CloseFile( mhmmio );
		mhmmio = NULL;
		ogg = NULL;
		Mem_Free( oggData );
		oggData = NULL;
		return 0;
	}
	return -1;
}


/*
===================================================================================

  idSampleDecoderLocal

===================================================================================
*/

class idSampleDecoderLocal : public idSampleDecoder {
public:
	virtual void			Decode( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest );
	virtual void			ClearDecoder( void );
	virtual idSoundSample *	GetSample( void ) const;
	virtual int				GetLastDecodeTime( void ) const;

	void					Clear( void );
	int						DecodePCM( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest );
	int						DecodeOGG( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest );

private:
	bool					failed;				// set if decoding failed
	int						lastFormat;			// last format being decoded
	idSoundSample *			lastSample;			// last sample being decoded
	int						lastSampleOffset;	// last offset into the decoded sample
	int						lastDecodeTime;		// last time decoding sound

	stb_vorbis*				stbv;				// stb_vorbis (Ogg) handle, using lastSample->nonCacheData
};

idBlockAlloc<idSampleDecoderLocal, 64>		sampleDecoderAllocator;

/*
====================
idSampleDecoder::Init
====================
*/
void idSampleDecoder::Init( void ) {
	decoderMemoryAllocator.Init();
	decoderMemoryAllocator.SetLockMemory( true );
	decoderMemoryAllocator.SetFixedBlocks( idSoundSystemLocal::s_realTimeDecoding.GetBool() ? 10 : 1 );
}

/*
====================
idSampleDecoder::Shutdown
====================
*/
void idSampleDecoder::Shutdown( void ) {
	decoderMemoryAllocator.Shutdown();
	sampleDecoderAllocator.Shutdown();
}

/*
====================
idSampleDecoder::Alloc
====================
*/
idSampleDecoder *idSampleDecoder::Alloc( void ) {
	idSampleDecoderLocal *decoder = sampleDecoderAllocator.Alloc();
	decoder->Clear();
	return decoder;
}

/*
====================
idSampleDecoder::Free
====================
*/
void idSampleDecoder::Free( idSampleDecoder *decoder ) {
	idSampleDecoderLocal *localDecoder = static_cast<idSampleDecoderLocal *>( decoder );
	localDecoder->ClearDecoder();
	sampleDecoderAllocator.Free( localDecoder );
}

/*
====================
idSampleDecoder::GetNumUsedBlocks
====================
*/
int idSampleDecoder::GetNumUsedBlocks( void ) {
	return decoderMemoryAllocator.GetNumUsedBlocks();
}

/*
====================
idSampleDecoder::GetUsedBlockMemory
====================
*/
int idSampleDecoder::GetUsedBlockMemory( void ) {
	return decoderMemoryAllocator.GetUsedBlockMemory();
}

/*
====================
idSampleDecoderLocal::Clear
====================
*/
void idSampleDecoderLocal::Clear( void ) {
	failed = false;
	lastFormat = WAVE_FORMAT_TAG_PCM;
	lastSample = NULL;
	lastSampleOffset = 0;
	lastDecodeTime = 0;
	stbv = NULL;
}

/*
====================
idSampleDecoderLocal::ClearDecoder
====================
*/
void idSampleDecoderLocal::ClearDecoder( void ) {
	Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );

	switch( lastFormat ) {
		case WAVE_FORMAT_TAG_PCM: {
			break;
		}
		case WAVE_FORMAT_TAG_OGG: {
			stb_vorbis_close( stbv );
			stbv = NULL;
			break;
		}
	}

	Clear();

	Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );
}

/*
====================
idSampleDecoderLocal::GetSample
====================
*/
idSoundSample *idSampleDecoderLocal::GetSample( void ) const {
	return lastSample;
}

/*
====================
idSampleDecoderLocal::GetLastDecodeTime
====================
*/
int idSampleDecoderLocal::GetLastDecodeTime( void ) const {
	return lastDecodeTime;
}

/*
====================
idSampleDecoderLocal::Decode
====================
*/
void idSampleDecoderLocal::Decode( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest ) {
	int readSamples44k;

	if ( sample->objectInfo.wFormatTag != lastFormat || sample != lastSample ) {
		ClearDecoder();
	}

	lastDecodeTime = soundSystemLocal.CurrentSoundTime;

	if ( failed ) {
		memset( dest, 0, sampleCount44k * sizeof( dest[0] ) );
		return;
	}

	// samples can be decoded both from the sound thread and the main thread for shakes
	Sys_EnterCriticalSection( CRITICAL_SECTION_ONE );

	switch( sample->objectInfo.wFormatTag ) {
		case WAVE_FORMAT_TAG_PCM: {
			readSamples44k = DecodePCM( sample, sampleOffset44k, sampleCount44k, dest );
			break;
		}
		case WAVE_FORMAT_TAG_OGG: {
			readSamples44k = DecodeOGG( sample, sampleOffset44k, sampleCount44k, dest );
			break;
		}
		default: {
			readSamples44k = 0;
			break;
		}
	}

	Sys_LeaveCriticalSection( CRITICAL_SECTION_ONE );

	if ( readSamples44k < sampleCount44k ) {
		memset( dest + readSamples44k, 0, ( sampleCount44k - readSamples44k ) * sizeof( dest[0] ) );
	}
}

/*
====================
idSampleDecoderLocal::DecodePCM
====================
*/
int idSampleDecoderLocal::DecodePCM( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest ) {
	const byte *first;
	int pos, size, readSamples;

	lastFormat = WAVE_FORMAT_TAG_PCM;
	lastSample = sample;

	int shift = 22050 / sample->objectInfo.nSamplesPerSec;
	int sampleOffset = sampleOffset44k >> shift;
	int sampleCount = sampleCount44k >> shift;

	if ( sample->nonCacheData == NULL ) {
		//assert( false );	// this should never happen ( note: I've seen that happen with the main thread down in idGameLocal::MapClear clearing entities - TTimo )
		// DG: see comment in DecodeOGG()
		common->Warning( "Called idSampleDecoderLocal::DecodePCM() on idSoundSample '%s' without nonCacheData\n", sample->name.c_str() );
		failed = true;
		return 0;
	}

	if ( !sample->FetchFromCache( sampleOffset * sizeof( short ), &first, &pos, &size, false ) ) {
		failed = true;
		return 0;
	}

	if ( size - pos < sampleCount * sizeof( short ) ) {
		readSamples = ( size - pos ) / sizeof( short );
	} else {
		readSamples = sampleCount;
	}

	// duplicate samples for 44kHz output
	SIMDProcessor->UpSamplePCMTo44kHz( dest, (const short *)(first+pos), readSamples, sample->objectInfo.nSamplesPerSec, sample->objectInfo.nChannels );

	return ( readSamples << shift );
}

/*
====================
idSampleDecoderLocal::DecodeOGG
====================
*/
int idSampleDecoderLocal::DecodeOGG( idSoundSample *sample, int sampleOffset44k, int sampleCount44k, float *dest ) {
	int readSamples, totalSamples;

	int shift = 22050 / sample->objectInfo.nSamplesPerSec;
	int sampleOffset = sampleOffset44k >> shift;
	int sampleCount = sampleCount44k >> shift;

	// open OGG file if not yet opened
	if ( lastSample == NULL ) {
		// make sure there is enough space for another decoder
		if ( decoderMemoryAllocator.GetFreeBlockMemory() < MIN_OGGVORBIS_MEMORY ) {
			return 0;
		}
		if ( sample->nonCacheData == NULL ) {
			//assert( false );	// this should never happen
			/* DG: turned this assertion into a warning, because this can happen, at least with
			 * the Classic Doom3 mod (when starting a new game). There idSoundCache::EndLevelLoad()
			 * purges (with idSoundSample::PurgeSoundSample()) sound/music/cdoomtheme.ogg
			 * (the music running in the main menu), which free()s nonCacheData.
			 * But afterwards (still during loading) idSoundSystemLocal::currentSoundWorld
			 * is set back to menuSoundWorld, which still tries to play that sample,
			 * which brings us here. Shortly afterwards the sound world is set to
			 * the game soundworld (sw) and that sample is not referenced anymore
			 * (until opening the menu again, when that sample is apparently properly reloaded)
			 * see also https://github.com/dhewm/dhewm3/issues/461 */
			common->Warning( "Called idSampleDecoderLocal::DecodeOGG() on idSoundSample '%s' without nonCacheData\n", sample->name.c_str() );
			failed = true;
			return 0;
		}
		assert(stbv == NULL);
		int stbVorbErr = 0;
		stbv = stb_vorbis_open_memory( sample->nonCacheData, sample->objectMemSize, &stbVorbErr, NULL );
		if ( stbv == NULL ) {
			common->Warning( "idSampleDecoderLocal::DecodeOGG() stb_vorbis_open_memory() for %s failed: %s\n",
							 sample->name.c_str(), my_stbv_strerror(stbVorbErr) );
			failed = true;
			return 0;
		}
		lastFormat = WAVE_FORMAT_TAG_OGG;
		lastSample = sample;
	}

	if( sample->objectInfo.nChannels > 2 ) {
		assert( 0 && ">2 channels currently not supported (samplesBuf expects 1 or 2)" );
		common->Warning( "Ogg Vorbis files with >2 channels are not supported!\n" );
		// no idea if other parts of the engine support more than stereo;
		// pretty sure though the standard gamedata doesn't use it (positional sounds must be mono anyway)
		failed = true;
		return 0;
	}

	// seek to the right offset if necessary
	if ( sampleOffset != lastSampleOffset ) {
		if ( stb_vorbis_seek( stbv, sampleOffset / sample->objectInfo.nChannels ) == 0 ) {
			int stbVorbErr = stb_vorbis_get_error( stbv );
			int offset = sampleOffset / sample->objectInfo.nChannels;
			common->Warning( "idSampleDecoderLocal::DecodeOGG() stb_vorbis_seek(%d) for %s failed: %s\n",
			                 offset, sample->name.c_str(), my_stbv_strerror( stbVorbErr ) );
			failed = true;
			return 0;
		}
	}

	lastSampleOffset = sampleOffset;

	// decode OGG samples
	totalSamples = sampleCount;
	readSamples = 0;
	do {
		// DG: in contrast to libvorbisfile's ov_read_float(), stb_vorbis_get_samples_float() expects you to
		//     pass a buffer to store the decoded samples in, so limit it to 4096 samples/channel per iteration
		float samplesBuf[2][MIXBUFFER_SAMPLES];
		float* samples[2] = { samplesBuf[0], samplesBuf[1] };
		int reqSamples = Min( MIXBUFFER_SAMPLES, totalSamples / sample->objectInfo.nChannels );
		int ret = stb_vorbis_get_samples_float( stbv, sample->objectInfo.nChannels, samples, reqSamples );
		if ( reqSamples == 0 ) {
			// DG: it happened that sampleCount was an odd number in a *stereo* sound file
			//  and eventually totalSamples was 1 and thus reqSamples = totalSamples/2 was 0
			//  so this turned into an endless loop.. it shouldn't happen anymore due to changes
			//  in idSoundWorldLocal::ReadFromSaveGame(), but better safe than sorry..
			common->DPrintf( "idSampleDecoderLocal::DecodeOGG() reqSamples == 0\n  for %s ?!\n", sample->name.c_str() );
			readSamples += totalSamples;
			totalSamples = 0;
			break;
		}
		if ( ret == 0 ) {
			int stbVorbErr = stb_vorbis_get_error( stbv );
			if ( stbVorbErr == VORBIS__no_error && reqSamples < 5 ) {
				// DG: it sometimes happens that 0 is returned when reqSamples was 1 and there is no error.
				// don't really know why; I'll just (arbitrarily) accept up to 5 "dropped" samples
				ret = reqSamples; // pretend decoding went ok
				common->DPrintf( "idSampleDecoderLocal::DecodeOGG() IGNORING stb_vorbis_get_samples_float() dropping %d (%d) samples\n  for %s\n",
					reqSamples, totalSamples, sample->name.c_str() );
			} else {
				common->Warning( "idSampleDecoderLocal::DecodeOGG() stb_vorbis_get_samples_float() %d (%d) samples\n  for %s failed: %s\n",
					reqSamples, totalSamples, sample->name.c_str(), my_stbv_strerror( stbVorbErr ) );
				failed = true;
				break;
			}
		}
		if ( ret < 0 ) {
			failed = true;
			return 0;
		}
		ret *= sample->objectInfo.nChannels;

		SIMDProcessor->UpSampleOGGTo44kHz( dest + ( readSamples << shift ), samples, ret, sample->objectInfo.nSamplesPerSec, sample->objectInfo.nChannels );

		readSamples += ret;
		totalSamples -= ret;
	} while( totalSamples > 0 );

	lastSampleOffset += readSamples;

	return ( readSamples << shift );
}
