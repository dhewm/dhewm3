/*
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * Modified 2009-2010 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from memory or from a file (or any stdio stream).
 * While these routines are sufficient for most applications,
 * some will want to use a different source manager.
 * IMPORTANT: we assume that fread() will correctly transcribe an array of
 * JOCTETs from 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

#include "sys/platform.h"

#include "renderer/jpeg_memory_src.h"

#ifdef HAVE_JPEG_MEM_SRC
void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char *inbuffer, unsigned long insize) {
	jpeg_mem_src(cinfo, inbuffer, insize);
}
#else
static void init_mem_source(j_decompress_ptr cinfo) {
	/* no work necessary here */
}

static boolean fill_mem_input_buffer(j_decompress_ptr cinfo) {
	static JOCTET mybuffer[4];

	/* The whole JPEG data is expected to reside in the supplied memory
	 * buffer, so any request for more data beyond the given buffer size
	 * is treated as an error.
	 */
	WARNMS(cinfo, JWRN_JPEG_EOF);
	/* Insert a fake EOI marker */
	mybuffer[0] = (JOCTET) 0xFF;
	mybuffer[1] = (JOCTET) JPEG_EOI;

	cinfo->src->next_input_byte = mybuffer;
	cinfo->src->bytes_in_buffer = 2;

	return TRUE;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
	struct jpeg_source_mgr *src = cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long)src->bytes_in_buffer) {
			num_bytes -= (long)src->bytes_in_buffer;
			(void)(*src->fill_input_buffer) (cinfo);
			/* note we assume that fill_input_buffer will never return FALSE,
			 * so suspension need not be handled.
			 */
		}
		src->next_input_byte += (size_t)num_bytes;
		src->bytes_in_buffer -= (size_t)num_bytes;
	}
}

static void term_source(j_decompress_ptr cinfo)
{
	/* no work necessary here */
}

void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char *inbuffer, unsigned long insize) {
	struct jpeg_source_mgr *src;

	if (inbuffer == NULL || insize == 0)	/* Treat empty input as fatal error */
		ERREXIT(cinfo, JERR_INPUT_EMPTY);

	/* The source object is made permanent so that a series of JPEG images
	 * can be read from the same buffer by calling jpeg_mem_src only before
	 * the first one.
	 */
	if (cinfo->src == NULL) {	/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
			(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT,
										sizeof(struct jpeg_source_mgr));
	}

	src = cinfo->src;
	src->init_source = init_mem_source;
	src->fill_input_buffer = fill_mem_input_buffer;
	src->skip_input_data = skip_input_data;
	src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->term_source = term_source;
	src->bytes_in_buffer = (size_t)insize;
	src->next_input_byte = (JOCTET *)inbuffer;
}
#endif
