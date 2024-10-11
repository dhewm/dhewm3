// this source file includes the implementation of stb_vorbis
// having it in a separate source file allows optimizing it in debug builds (for faster load times)
// without hurting the debugability of the source files it's used in

// (I'm doing this instead of renaming stb_vorbis.h to stb_vorbis.c so the configuration
//  like STB_VORBIS_BIG_ENDIAN etc can be done here in code)

#if D3_IS_BIG_ENDIAN
  #define STB_VORBIS_BIG_ENDIAN
#endif
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API // we're using the pulldata API
#include "stb_vorbis.h"
