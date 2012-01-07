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

#ifndef __JPEG_MEMORY_SRC_H__
#define __JPEG_MEMORY_SRC_H__

#include <jpeglib.h>
#include <jerror.h>

void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char *inbuffer, unsigned long insize);

#endif
