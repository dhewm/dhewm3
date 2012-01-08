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
#include "idlib/math/Math.h"

#include "sound/snd_local.h"

static inline ALdouble mB_to_gain(ALdouble millibels) {
	return idMath::Pow(10.0, millibels / 2000.0);
}

idSoundEffect::idSoundEffect() :
	effect(0) {
}

idSoundEffect::~idSoundEffect() {
	if (effect)
	    soundSystemLocal.alDeleteEffects(1, &effect);
}

bool idSoundEffect::alloc() {
	alGetError();

	ALenum e;

	soundSystemLocal.alGenEffects(1, &effect);
	if ((e = alGetError()) != AL_NO_ERROR) {
		return false;
	}

	soundSystemLocal.alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	if ((e = alGetError()) != AL_NO_ERROR) {
		return false;
	}

	return true;
}

/*
===============
idEFXFile::idEFXFile
===============
*/
idEFXFile::idEFXFile( void ) {
}

/*
===============
idEFXFile::Clear
===============
*/
void idEFXFile::Clear( void ) {
	effects.DeleteContents( true );
}

/*
===============
idEFXFile::~idEFXFile
===============
*/
idEFXFile::~idEFXFile( void ) {
	Clear();
}

/*
===============
idEFXFile::FindEffect
===============
*/
bool idEFXFile::FindEffect( idStr &name, ALuint *effect ) {
	int i;

	for ( i = 0; i < effects.Num(); i++ ) {
		if ( effects[i]->name == name ) {
			*effect = effects[i]->effect;
			return true;
		}
	}

	return false;
}

/*
===============
idEFXFile::ReadEffect
===============
*/
bool idEFXFile::ReadEffect( idLexer &src, idSoundEffect *effect ) {
	idToken name, token;

	if ( !src.ReadToken( &token ) )
		return false;

	// reverb effect
	if ( token != "reverb" ) {
		// other effect (not supported at the moment)
		src.Error( "idEFXFile::ReadEffect: Unknown effect definition" );

		return false;
	}

	src.ReadTokenOnLine( &token );
	name = token;

	if ( !src.ReadToken( &token ) )
		return false;

	if ( token != "{" ) {
		src.Error( "idEFXFile::ReadEffect: { not found, found %s", token.c_str() );
		return false;
	}

	do {
		if ( !src.ReadToken( &token ) ) {
			src.Error( "idEFXFile::ReadEffect: EOF without closing brace" );
			return false;
		}

		if ( token == "}" ) {
			effect->name = name;
			break;
		}

		ALuint e = effect->effect;

		// mappings taken from
		// http://repo.or.cz/w/dsound-openal.git/blob/HEAD:/primary.c#l1795
		// which are marked with a FIXME, so this is maybe not 100% correct
		if ( token == "environment" ) {
			// <+KittyCat> the "environment" token should be ignored (efx has nothing equatable to it)
			src.ParseInt();
		} else if ( token == "environment size" ) {
			float size = src.ParseFloat();
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_DENSITY, ((size < 2.0f) ? (size - 1.0f) : 1.0f));
		} else if ( token == "environment diffusion" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_DIFFUSION, src.ParseFloat());
		} else if ( token == "room" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_GAIN, mB_to_gain(src.ParseInt()));
		} else if ( token == "room hf" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_GAINHF, mB_to_gain(src.ParseInt()));
		} else if ( token == "room lf" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_GAINLF, mB_to_gain(src.ParseInt()));
		} else if ( token == "decay time" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_DECAY_TIME, src.ParseFloat());
		} else if ( token == "decay hf ratio" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_DECAY_HFRATIO, src.ParseFloat());
		} else if ( token == "decay lf ratio" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_DECAY_LFRATIO, src.ParseFloat());
		} else if ( token == "reflections" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_REFLECTIONS_GAIN, mB_to_gain(src.ParseInt()));
		} else if ( token == "reflections delay" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_REFLECTIONS_DELAY, src.ParseFloat());
		} else if ( token == "reflections pan" ) {
			float fv[3];
			fv[0] = src.ParseFloat();
			fv[1] = src.ParseFloat();
			fv[2] = src.ParseFloat();
			soundSystemLocal.alEffectfv(e, AL_EAXREVERB_REFLECTIONS_PAN, fv);
		} else if ( token == "reverb" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_LATE_REVERB_GAIN, mB_to_gain(src.ParseInt()));
		} else if ( token == "reverb delay" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_LATE_REVERB_DELAY, src.ParseFloat());
		} else if ( token == "reverb pan" ) {
			float fv[3];
			fv[0] = src.ParseFloat();
			fv[1] = src.ParseFloat();
			fv[2] = src.ParseFloat();
			soundSystemLocal.alEffectfv(e, AL_EAXREVERB_LATE_REVERB_PAN, fv);
		} else if ( token == "echo time" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_ECHO_TIME, src.ParseFloat());
		} else if ( token == "echo depth" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_ECHO_DEPTH, src.ParseFloat());
		} else if ( token == "modulation time" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_MODULATION_TIME, src.ParseFloat());
		} else if ( token == "modulation depth" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_MODULATION_DEPTH, src.ParseFloat());
		} else if ( token == "air absorption hf" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, mB_to_gain(src.ParseFloat()));
		} else if ( token == "hf reference" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_HFREFERENCE, src.ParseFloat());
		} else if ( token == "lf reference" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_LFREFERENCE, src.ParseFloat());
		} else if ( token == "room rolloff factor" ) {
			soundSystemLocal.alEffectf(e, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, src.ParseFloat());
		} else if ( token == "flags" ) {
			src.ReadTokenOnLine( &token );
			unsigned int flags = token.GetUnsignedLongValue();

			soundSystemLocal.alEffecti(e, AL_EAXREVERB_DECAY_HFLIMIT, (flags & 0x20) ? AL_TRUE : AL_FALSE);
			// the other SCALE flags have no equivalent in efx
		} else {
			src.ReadTokenOnLine( &token );
			src.Error( "idEFXFile::ReadEffect: Invalid parameter in reverb definition" );
		}
	} while ( 1 );

	return true;
}

/*
===============
idEFXFile::LoadFile
===============
*/
bool idEFXFile::LoadFile( const char *filename, bool OSPath ) {
	idLexer src( LEXFL_NOSTRINGCONCAT );
	idToken token;

	src.LoadFile( filename, OSPath );
	if ( !src.IsLoaded() ) {
		return false;
	}

	if ( !src.ExpectTokenString( "Version" ) ) {
		return NULL;
	}

	if ( src.ParseInt() != 1 ) {
		src.Error( "idEFXFile::LoadFile: Unknown file version" );
		return false;
	}

	while ( !src.EndOfFile() ) {
		idSoundEffect *effect = new idSoundEffect;

		if (!effect->alloc()) {
			delete effect;
			Clear();
			return false;
		}

		if (ReadEffect(src, effect))
			effects.Append(effect);
		else
			delete effect;
	};

	return true;
}
