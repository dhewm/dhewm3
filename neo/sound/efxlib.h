/*
*/

#ifndef __EFXLIBH
#define __EFXLIBH

#include "idlib/containers/List.h"
#include "idlib/Str.h"
#include "idlib/Lexer.h"
#include "idlib/Heap.h"
#include "sound/sound.h"

struct idSoundEffect {
	idSoundEffect();
	~idSoundEffect();

	bool alloc();

	idStr name;
	ALuint effect;
};

class idEFXFile {
public:
	idEFXFile();
	~idEFXFile();

	bool FindEffect( idStr &name, ALuint *effect );
	bool LoadFile( const char *filename, bool OSPath = false );
	void Clear( void );

private:
	bool ReadEffect( idLexer &lexer, idSoundEffect *effect );

	idList<idSoundEffect *>effects;
};

#endif // __EFXLIBH
