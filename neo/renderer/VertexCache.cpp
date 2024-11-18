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

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of
these additional terms immediately following the terms and conditions of the GNU General Public License which
accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address
below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id
Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/platform.h"
#include "framework/Common.h"
#include "renderer/tr_local.h"
#include "renderer/VertexCache.h"

static const int FRAME_MEMORY_BYTES = 0x400000;
static const int EXPAND_HEADERS = 1024;

idCVar idVertexCache::r_showVertexCache( "r_showVertexCache", "0", CVAR_INTEGER | CVAR_RENDERER,
		"show vertex cache, 0 = off, 1 = vbo debug, 2 = vbo mem",
		idCmdSystem::ArgCompletion_Integer<0, 2> );
idCVar idVertexCache::r_useArbBufferRange( "r_useArbBufferRange", "1", CVAR_BOOL | CVAR_RENDERER,
		"use ARB_map_buffer_range for optimization, 0 = off, 1 = on",
		idCmdSystem::ArgCompletion_Integer<0, 1> );
idCVar
idVertexCache::r_reuseVertexCacheSooner( "r_reuseVertexCacheSooner", "1", CVAR_BOOL | CVAR_RENDERER,
		"reuse vertex buffers as soon as possible after freeing, 0 = off, 1 = on",
		idCmdSystem::ArgCompletion_Integer<0, 1> );

idVertexCache vertexCache;

/*
==============
R_ShowVBOMem_f
==============
*/
void R_ShowVBOMem_f( const idCmdArgs &args ) {
	vertexCache.Show();
}

/*
==============
R_ListVBOMem_f
==============
*/
void R_ListVBOMem_f( const idCmdArgs &args ) {
	vertexCache.List();
}

/*
==============
idVertexCache::ActuallyFree
==============
*/
void idVertexCache::ActuallyFree( vertCache_t *block ) {
	if ( !block ) {
		common->Error( "idVertexCache Free: NULL pointer" );
	}

	if ( block->user ) {
		// let the owner know we have purged it
		*block->user = NULL;
		block->user = NULL;
	}

	// temp blocks are in a shared space that won't be freed
	if ( block->tag != TAG_TEMP ) {
		staticAllocTotal -= block->size;
		staticCountTotal--;
	}
	block->tag = TAG_FREE; // mark as free

	// unlink stick it back on the free list
	block->next->prev = block->prev;
	block->prev->next = block->next;

	if ( r_reuseVertexCacheSooner.GetBool() ) {
		// stick it on the front of the free list so it will be reused immediately
		block->next = freeStaticHeaders.next;
		block->prev = &freeStaticHeaders;
	} else {
		// stick it on the back of the free list so it won't be reused soon (just for debugging)
		block->next = &freeStaticHeaders;
		block->prev = freeStaticHeaders.prev;
	}
	block->next->prev = block;
	block->prev->next = block;
}

/*
==============
idVertexCache::Position

this will be a real pointer with virtual memory,
but it will be an int offset cast to a pointer with
ARB_vertex_buffer_object

The ARB_vertex_buffer_object will be bound
==============
*/
const void *idVertexCache::Position( vertCache_t *buffer ) {
	if ( !buffer || buffer->tag == TAG_FREE ) {
		common->FatalError( "idVertexCache::Position: bad vertCache_t" );
	}

	// the ARB vertex object just uses an offset
	if ( buffer->vbo ) {
		if ( r_showVertexCache.GetInteger() == 2 ) {
			if ( buffer->tag == TAG_TEMP ) {
				common->Printf( "GL_ARRAY_BUFFER_ARB = %i + %i (%i bytes)\n", buffer->vbo, buffer->offset, buffer->size );
			} else {
				common->Printf( "GL_ARRAY_BUFFER_ARB = %i (%i bytes)\n", buffer->vbo, buffer->size );
			}
		}
		BindIndex( ( buffer->indexBuffer ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER ), buffer->vbo );
		return reinterpret_cast<const void *>( buffer->offset );
	}
	return NULL;
}

//================================================================================

/*
===========
idVertexCache::BindIndex

Makes sure it only allocates the right buffers once.
===========
*/
void idVertexCache::BindIndex( GLenum target, GLuint vbo ) {
	switch ( target ) {
		case GL_ARRAY_BUFFER:
			if ( vertexBuffer != vbo ) {
				// this happens more often than you might think :(
				qglBindBufferARB( target, vbo );
				vertexBuffer = vbo;
				return;
			}
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			if ( indexBuffer != vbo ) {
				// this happens more often than you might think :(
				qglBindBufferARB( target, vbo );
				indexBuffer = vbo;
				return;
			}
			break;
		default:
			common->FatalError( "BindIndex : unknown buffer target : %i\n", static_cast<int>( target ) );
			break;
	}
}

/*
===========
idVertexCache::UnbindIndex

Pass 0 to vbo to invalidate it.
===========
*/
void idVertexCache::UnbindIndex( GLenum target ) {
	BindIndex( target, 0 );
}

//================================================================================

/*
===========
idVertexCache::Init
===========
*/
void idVertexCache::Init() {
	// well this would suck...
	if ( !glConfig.ARBVertexBufferObjectAvailable ) {
		common->FatalError( "idVertexCache::Init: Your card is to old\n" );
	}
	cmdSystem->AddCommand( "showVBOMem", R_ShowVBOMem_f, CMD_FL_RENDERER, "Shows Allocated Vertex Buffer Memory" );
	cmdSystem->AddCommand( "ListVBOMem", R_ListVBOMem_f, CMD_FL_RENDERER, "lists Objects Allocated in Vertex Cache" );

	// initialize the buffers
	vertexBuffer = 0;
	indexBuffer = 0;

	// initialize the cache memory blocks
	freeStaticHeaders.next = freeStaticHeaders.prev = &freeStaticHeaders;
	staticHeaders.next = staticHeaders.prev = &staticHeaders;
	freeDynamicHeaders.next = freeDynamicHeaders.prev = &freeDynamicHeaders;
	dynamicHeaders.next = dynamicHeaders.prev = &dynamicHeaders;
	deferredFreeList.next = deferredFreeList.prev = &deferredFreeList;

	// set up the dynamic frame memory
	frameBytes = FRAME_MEMORY_BYTES;
	staticAllocTotal = 0;

	// allocate a dummy buffer
	byte *frameBuffer = new byte[frameBytes];

	for ( int i = 0; i < NUM_VERTEX_FRAMES; i++ ) {
		// force the alloc to use GL_STREAM_DRAW_ARB
		allocatingTempBuffer = true;
		Alloc( frameBuffer, frameBytes, &tempBuffers[i] );
		allocatingTempBuffer = false;
		tempBuffers[i]->tag = TAG_FIXED;

		// unlink these from the static list, so they won't ever get purged
		tempBuffers[i]->next->prev = tempBuffers[i]->prev;
		tempBuffers[i]->prev->next = tempBuffers[i]->next;
	}

	// use C++ allocation
	delete[] frameBuffer;
	frameBuffer = NULL;
	EndFrame();
}

/*
===========
idVertexCache::PurgeAll

Used when toggling vertex programs on or off, because
the cached data isn't valid
===========
*/
void idVertexCache::PurgeAll() {
	while ( staticHeaders.next != &staticHeaders ) {
		ActuallyFree( staticHeaders.next );
	}
}

/*
===========
idVertexCache::Shutdown
===========
*/
void idVertexCache::Shutdown() {
	headerAllocator.Shutdown();
}

/*
===========
idVertexCache::Alloc
===========
*/
void idVertexCache::Alloc( void *data, int size, vertCache_t **buffer, bool doIndex ) {
	vertCache_t *block = NULL;

	if ( size <= 0 ) {
		common->Error( "idVertexCache::Alloc: size = %i\n", size );
	}

	// if we can't find anything, it will be NULL
	*buffer = NULL;

	// if we don't have any remaining unused headers, allocate some more
	if ( freeStaticHeaders.next == &freeStaticHeaders ) {
		for ( int i = 0; i < EXPAND_HEADERS; i++ ) {
			block = headerAllocator.Alloc();
			qglGenBuffersARB( 1, &block->vbo );
			block->size = 0;
			block->next = freeStaticHeaders.next;
			block->prev = &freeStaticHeaders;
			block->next->prev = block;
			block->prev->next = block;
		}
	}
	GLenum target = ( doIndex ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER );
	GLenum usage = ( allocatingTempBuffer ? GL_STREAM_DRAW : GL_STATIC_DRAW );

	// try to find a matching block to replace so that we're not continually respecifying vbo data each frame
	for ( vertCache_t *findblock = freeStaticHeaders.next; /**/; findblock = findblock->next ) {
		if ( findblock == &freeStaticHeaders ) {
			block = freeStaticHeaders.next;
			break;
		}

		if ( findblock->target != target ) {
			continue;
		}

		if ( findblock->usage != usage ) {
			continue;
		}

		if ( findblock->size != size ) {
			continue;
		}
		block = findblock;
		break;
	}

	// move it from the freeStaticHeaders list to the staticHeaders list
	block->target = target;
	block->usage = usage;

	if ( block->vbo ) {
		// orphan the buffer in case it needs respecifying (it usually will)
		BindIndex( target, block->vbo );
		qglBufferDataARB( target, static_cast<GLsizeiptr>( size ), NULL, usage );
		qglBufferDataARB( target, static_cast<GLsizeiptr>( size ), data, usage );
	}
	block->next->prev = block->prev;
	block->prev->next = block->next;
	block->next = staticHeaders.next;
	block->prev = &staticHeaders;
	block->next->prev = block;
	block->prev->next = block;
	block->size = size;
	block->offset = 0;
	block->tag = TAG_USED;

	// save data for debugging
	staticAllocThisFrame += block->size;
	staticCountThisFrame++;
	staticCountTotal++;
	staticAllocTotal += block->size;

	// this will be set to zero when it is purged
	block->user = buffer;
	*buffer = block;

	// allocation doesn't imply used-for-drawing, because at level
	// load time lots of things may be created, but they aren't
	// referenced by the GPU yet, and can be purged if needed.
	block->frameUsed = currentFrame - NUM_VERTEX_FRAMES;
	block->indexBuffer = doIndex;
}

/*
===========
idVertexCache::Touch
===========
*/
void idVertexCache::Touch( vertCache_t *block ) {
	if ( !block ) {
		common->Error( "idVertexCache Touch: NULL pointer" );
	}

	if ( block->tag == TAG_FREE ) {
		common->FatalError( "idVertexCache Touch: freed pointer" );
	}

	if ( block->tag == TAG_TEMP ) {
		common->FatalError( "idVertexCache Touch: temporary pointer" );
	}
	block->frameUsed = currentFrame;

	// move to the head of the LRU list
	block->next->prev = block->prev;
	block->prev->next = block->next;
	block->next = staticHeaders.next;
	block->prev = &staticHeaders;

	staticHeaders.next->prev = block;
	staticHeaders.next = block;
}

/*
===========
idVertexCache::Free
===========
*/
void idVertexCache::Free( vertCache_t *block ) {
	if ( !block ) {
		return;
	}

	if ( block->tag == TAG_FREE ) {
		common->FatalError( "idVertexCache Free: freed pointer" );
	}

	if ( block->tag == TAG_TEMP ) {
		common->FatalError( "idVertexCache Free: temporary pointer" );
	}

	// this block still can't be purged until the frame count has expired,
	// but it won't need to clear a user pointer when it is
	block->user = NULL;
	block->next->prev = block->prev;
	block->prev->next = block->next;
	block->next = deferredFreeList.next;
	block->prev = &deferredFreeList;

	deferredFreeList.next->prev = block;
	deferredFreeList.next = block;
}

/*
===========
idVertexCache::AllocFrameTemp

A frame temp allocation must never be allowed to fail due to overflow.
We can't simply sync with the GPU and overwrite what we have, because
there may still be future references to dynamically created surfaces.
===========
*/
vertCache_t *idVertexCache::AllocFrameTemp( void *data, int size ) {
	vertCache_t *block = NULL;

	if ( size <= 0 ) {
		common->Error( "idVertexCache::AllocFrameTemp: size = %i\n", size );
	}

	if ( dynamicAllocThisFrame + size > frameBytes ) {
		// if we don't have enough room in the temp block, allocate a static block,
		// but immediately free it so it will get freed at the next frame
		tempOverflow = true;
		Alloc( data, size, &block );
		Free( block );
		return block;
	}

	// this data is just going on the shared dynamic list
	// if we don't have any remaining unused headers, allocate some more
	if ( freeDynamicHeaders.next == &freeDynamicHeaders ) {
		for ( int i = 0; i < EXPAND_HEADERS; i++ ) {
			block = headerAllocator.Alloc();
			block->next = freeDynamicHeaders.next;
			block->prev = &freeDynamicHeaders;
			block->next->prev = block;
			block->prev->next = block;
		}
	}

	// move it from the freeDynamicHeaders list to the dynamicHeaders list
	block = freeDynamicHeaders.next;
	block->next->prev = block->prev;
	block->prev->next = block->next;
	block->next = dynamicHeaders.next;
	block->prev = &dynamicHeaders;
	block->next->prev = block;
	block->prev->next = block;
	block->size = size;
	block->tag = TAG_TEMP;
	block->indexBuffer = false;
	block->offset = dynamicAllocThisFrame;

	dynamicAllocThisFrame += block->size;
	dynamicCountThisFrame++;

	block->user = NULL;
	block->frameUsed = 0;

	// copy the data
	block->vbo = tempBuffers[listNum]->vbo;

	// mh code start
	if ( block->vbo ) {
		BindIndex( GL_ARRAY_BUFFER, block->vbo );

		// try to get an unsynchronized map if at all possible
		if ( r_useArbBufferRange.GetBool() && glConfig.ARBMapBufferRangeAvailable ) {
			GLbitfield access = ( GL_MAP_WRITE_BIT | ( ( block->offset == 0 ) ? GL_MAP_INVALIDATE_BUFFER_BIT : GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT ) );
			GLvoid *dst = qglMapBufferRange( GL_ARRAY_BUFFER, block->offset, static_cast<GLsizeiptr>( size ), access );

			// if the buffer has wrapped then we orphan it
			if ( dst ) {
				SIMDProcessor->Memcpy( reinterpret_cast<byte *>( dst ), data, size );
				qglUnmapBufferARB( GL_ARRAY_BUFFER );
				return block;
			} else {
				qglBufferSubDataARB( GL_ARRAY_BUFFER, block->offset, static_cast<GLsizeiptr>( size ), data );
			}
		} else {
			qglBufferSubDataARB( GL_ARRAY_BUFFER, block->offset, static_cast<GLsizeiptr>( size ), data );
		}
	}
	return block;
}

/*
===========
idVertexCache::EndFrame
===========
*/
void idVertexCache::EndFrame() {
	// display debug information
	if ( r_showVertexCache.GetBool() == 1 ) {
		int staticUseCount = 0;
		int staticUseSize = 0;

		for ( vertCache_t *block = staticHeaders.next; block != &staticHeaders; block = block->next ) {
			if ( block->frameUsed == currentFrame ) {
				staticUseCount++;
				staticUseSize += block->size;
			}
		}
		const char *frameOverflow = tempOverflow ? "(OVERFLOW)" : "";
		common->Printf( "vertex dynamic:%i=%ik%s, static alloc:%i=%ik used:%i=%ik total:%i=%ik\n", dynamicCountThisFrame,
						dynamicAllocThisFrame / 1024, frameOverflow, staticCountThisFrame, staticAllocThisFrame / 1024,
						staticUseCount, staticUseSize / 1024, staticCountTotal, staticAllocTotal / 1024 );
	}

	// unbind vertex buffers
	UnbindIndex( GL_ARRAY_BUFFER_ARB );
	UnbindIndex( GL_ELEMENT_ARRAY_BUFFER_ARB );

	currentFrame = tr.frameCount;
	listNum = currentFrame % NUM_VERTEX_FRAMES;
	staticAllocThisFrame = 0;
	staticCountThisFrame = 0;
	dynamicAllocThisFrame = 0;
	dynamicCountThisFrame = 0;
	tempOverflow = false;

	// free all the deferred free headers
	while ( deferredFreeList.next != &deferredFreeList ) {
		ActuallyFree( deferredFreeList.next );
	}

	// free all the frame temp headers
	vertCache_t *block = dynamicHeaders.next;

	if ( block != &dynamicHeaders ) {
		block->prev = &freeDynamicHeaders;
		dynamicHeaders.prev->next = freeDynamicHeaders.next;
		freeDynamicHeaders.next->prev = dynamicHeaders.prev;
		freeDynamicHeaders.next = block;
		dynamicHeaders.next = dynamicHeaders.prev = &dynamicHeaders;
	}
}

/*
=============
idVertexCache::List
=============
*/
void idVertexCache::List( void ) {
	int numActive = 0;
	int frameStatic = 0;
	int totalStatic = 0;
	vertCache_t *block = NULL;

	for ( block = staticHeaders.next; block != &staticHeaders; block = block->next ) {
		numActive++;
		totalStatic += block->size;

		if ( block->frameUsed == currentFrame ) {
			frameStatic += block->size;
		}
	}
	int numFreeStaticHeaders = 0;

	for ( block = freeStaticHeaders.next; block != &freeStaticHeaders; block = block->next ) {
		numFreeStaticHeaders++;
	}
	int numFreeDynamicHeaders = 0;

	for ( block = freeDynamicHeaders.next; block != &freeDynamicHeaders; block = block->next ) {
		numFreeDynamicHeaders++;
	}
	common->Printf( "%i dynamic temp buffers of %ik\n", NUM_VERTEX_FRAMES, frameBytes / 1024 );
	common->Printf( "%5i active static headers\n", numActive );
	common->Printf( "%5i free static headers\n", numFreeStaticHeaders );
	common->Printf( "%5i free dynamic headers\n", numFreeDynamicHeaders );
}

/*
=============
idVertexCache::Show

Barnes,
replaces the broken glconfig string version.
Real performance killer here, use with care.
=============
*/
void idVertexCache::Show( void ) {
	GLint mem[4];

	if ( glConfig.vendor == glvAMD ) {
		common->Printf( "\nATI/AMD specific memory info:\n" );
		common->Printf( "\n" );
		qglGetIntegerv( GL_VBO_FREE_MEMORY_ATI, mem );
		common->Printf( "VBO: total memory free in the pool %i MB\n", mem[0] >> 10 );
		common->Printf( "VBO: largest available free block in the pool %i MB\n", mem[1] >> 10 );
		common->Printf( "VBO: total auxiliary memory free %i MB\n", mem[2] >> 10 );
		common->Printf( "VBO: largest auxiliary free block %i MB\n", mem[3] >> 10 );
		qglGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, mem );
		common->Printf( "Texture: total memory free in the pool %i MB\n", mem[0] >> 10 );
		common->Printf( "Texture: largest available free block in the pool %i MB\n", mem[1] >> 10 );
		common->Printf( "Texture: total auxiliary memory free %i MB\n", mem[2] >> 10 );
		common->Printf( "Texture: largest auxiliary free block %i MB\n", mem[3] >> 10 );
		qglGetIntegerv( GL_RENDERBUFFER_FREE_MEMORY_ATI, mem );
		common->Printf( "RenderBuffer: total memory free in the pool %i MB\n", mem[0] >> 10 );
		common->Printf( "RenderBuffer: largest available free block in the pool %i MB\n", mem[1] >> 10 );
		common->Printf( "RenderBuffer: total auxiliary memory free %i MB\n", mem[2] >> 10 );
		common->Printf( "RenderBuffer: largest auxiliary free block %i MB\n", mem[3] >> 10 );
	} else if ( glConfig.vendor == glvNVIDIA ) {
		common->Printf( "\nNvidia specific memory info:\n" );
		common->Printf( "\n" );
		qglGetIntegerv( GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, mem );
		common->Printf( "dedicated video memory %i MB\n", mem[0] >> 10 );
		qglGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, mem );
		common->Printf( "total available memory %i MB\n", mem[0] >> 10 );
		qglGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, mem );
		common->Printf( "currently unused GPU memory %i MB\n", mem[0] >> 10 );
		qglGetIntegerv( GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, mem );
		common->Printf( "count of total evictions seen by system %i MB\n", mem[0] >> 10 );
		qglGetIntegerv( GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, mem );
		common->Printf( "total video memory evicted %i MB\n", mem[0] >> 10 );
	} else if ( glConfig.vendor == glvIntel ) {
		common->Printf( "\nIntel cannot do memory info, sorry:\n" );
	} else {
		common->Printf( "\nUnsupported card cannot do memory info, sorry:\n" );
	}
}
