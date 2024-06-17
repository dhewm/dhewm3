// this source file includes the implementations of stb_image and stb_image_write
// having it in a separate source file allows optimizing it in debug builds (for faster load times)
// without hurting the debugability of the source files stb_image(_write) is used in

// include this first, otherwise build breaks because of  use_idStr_* #defines in Str.h
#if defined(__APPLE__) && !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 5
  // Extra-Hack for ancient GCC 4.2-based Apple compilers that don't support __thread
  #define STBI_NO_THREAD_LOCALS
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_JPEG // at least for now, only use it for JPEG
#define STBI_NO_STDIO  // images are passed as buffers
#include "stb_image.h"


#include "framework/miniz/miniz.h"

static unsigned char* compress_for_stbiw(unsigned char* data, int data_len, int* out_len, int quality)
{
	uLongf bufSize = compressBound(data_len);
	// note that buf will be free'd by stb_image_write.h
	// with STBIW_FREE() (plain free() by default)
	unsigned char* buf = (unsigned char*)malloc(bufSize);
	if (buf == NULL)  return NULL;
	if (compress2(buf, &bufSize, data, data_len, quality) != Z_OK)
	{
		free(buf);
		return NULL;
	}
	*out_len = bufSize;

	return buf;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_ZLIB_COMPRESS compress_for_stbiw
#include "stb_image_write.h"

