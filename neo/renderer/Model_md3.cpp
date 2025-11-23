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
#include "renderer/tr_local.h"
#include "renderer/Model_local.h"

#include "renderer/Model_md3.h"

/***********************************************************************

	idMD3Mesh

***********************************************************************/

// DG: added constructor to make sure all members are initialized
idRenderModelMD3::idRenderModelMD3() : index(-1), dataSize(0), md3(NULL), numLods(0),
	lastFrame(-1), lastOldFrame(0), lastBackLerp(0.0f)
{}

// DG: added destructor to clean up the newly added silInfos (and md3 which used to leak)
idRenderModelMD3::~idRenderModelMD3() {
	srfTriangles_t tri;
	memset( &tri, 0, sizeof( tri ) );

	for ( int i=0, n=silInfos.Num() ; i < n ; ++i ) {
		silInfo_t& silInfo = silInfos[i];
		// Note: Because tri.sil* was allocated by specialized allocators only available
		//       in tr.trisurf.cpp, they need to be free'd with them as well.
		//       The easiest way to do this is by putting them into a srfTriangles_t first
		tri.silIndexes = silInfo.silIndexes;
		tri.silEdges = silInfo.silEdges;

		R_FreeStaticTriSurfSilIndexes( &tri );
		R_FreeStaticTriSurfSilEdges( &tri );

		if ( sizeof(md3Triangle_t::indexes[0]) != sizeof(silInfo.indexes[0]) ) {
			tri.indexes = silInfo.indexes;
			R_FreeStaticTriSurfIndexes( &tri );
		} // else silInfos[i].indexes points directly into the corresponding data in this->md3
	}
	silInfos.Clear();

	Mem_Free(md3);
}

#define	LL(x) x=LittleInt(x)

/*
=================
idRenderModelMD3::InitFromFile
=================
*/
void idRenderModelMD3::InitFromFile( const char *fileName ) {
	int					i, j;
	md3Header_t			*pinmodel;
	md3Frame_t			*frame;
	md3Surface_t		*surf;
	md3Shader_t			*shader;
	md3Triangle_t		*tri;
	md3St_t				*st;
	md3XyzNormal_t		*xyz;
	md3Tag_t			*tag;
	void				*buffer = NULL;
	int					version;
	int					size;


	name = fileName;

	size = fileSystem->ReadFile( fileName, &buffer, NULL );
	// DG: without the int cast, an unsigned comparison is made which fails for size = -1
	if ( size <= (int)sizeof(md3Header_t) ) {
		if (buffer != NULL) {
			fileSystem->FreeFile( buffer );
		}
		// DG: must make it a default model so failure can be detected elsewhere
		MakeDefaultModel();
		return;
	}

	pinmodel = (md3Header_t *)buffer;

	version = LittleInt (pinmodel->version);
	if (version != MD3_VERSION) {
		fileSystem->FreeFile( buffer );
		common->Warning( "InitFromFile: %s has wrong version (%i should be %i)",
				 fileName, version, MD3_VERSION);
		MakeDefaultModel();
		return;
	}

	size = LittleInt(pinmodel->ofsEnd);
	dataSize += size;
	md3 = (md3Header_t *)Mem_Alloc( size );

	memcpy (md3, buffer, LittleInt(pinmodel->ofsEnd) );

	LL(md3->ident);
	LL(md3->version);
	LL(md3->numFrames);
	LL(md3->numTags);
	LL(md3->numSurfaces);
	LL(md3->ofsFrames);
	LL(md3->ofsTags);
	LL(md3->ofsSurfaces);
	LL(md3->ofsEnd);

	if ( md3->numFrames < 1 ) {
		common->Warning( "InitFromFile: %s has no frames", fileName );
		fileSystem->FreeFile( buffer );
		MakeDefaultModel();
		return;
	}

	// swap all the frames
	frame = (md3Frame_t *) ( (byte *)md3 + md3->ofsFrames );
	for ( i = 0 ; i < md3->numFrames ; i++, frame++) {
		frame->radius = LittleFloat( frame->radius );
		for ( j = 0 ; j < 3 ; j++ ) {
			frame->bounds[0][j] = LittleFloat( frame->bounds[0][j] );
			frame->bounds[1][j] = LittleFloat( frame->bounds[1][j] );
			frame->localOrigin[j] = LittleFloat( frame->localOrigin[j] );
		}
	}

	// swap all the tags
	tag = (md3Tag_t *) ( (byte *)md3 + md3->ofsTags );
	for ( i = 0 ; i < md3->numTags * md3->numFrames ; i++, tag++) {
		for ( j = 0 ; j < 3 ; j++ ) {
			tag->origin[j] = LittleFloat( tag->origin[j] );
			tag->axis[0][j] = LittleFloat( tag->axis[0][j] );
			tag->axis[1][j] = LittleFloat( tag->axis[1][j] );
			tag->axis[2][j] = LittleFloat( tag->axis[2][j] );
		}
	}

	// swap all the surfaces
	surf = (md3Surface_t *) ( (byte *)md3 + md3->ofsSurfaces );
	for ( i = 0 ; i < md3->numSurfaces ; i++) {

		LL(surf->ident);
		LL(surf->flags);
		LL(surf->numFrames);
		LL(surf->numShaders);
		LL(surf->numTriangles);
		LL(surf->ofsTriangles);
		LL(surf->numVerts);
		LL(surf->ofsShaders);
		LL(surf->ofsSt);
		LL(surf->ofsXyzNormals);
		LL(surf->ofsEnd);

		if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
			common->Error( "InitFromFile: %s has more than %i verts on a surface (%i)",
				fileName, SHADER_MAX_VERTEXES, surf->numVerts );
		}
		if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) {
			common->Error( "InitFromFile: %s has more than %i triangles on a surface (%i)",
				fileName, SHADER_MAX_INDEXES / 3, surf->numTriangles );
		}

		// change to surface identifier
		surf->ident = 0;	//SF_MD3;

		// lowercase the surface name so skin compares are faster
		int slen = (int)strlen( surf->name );
		for( j = 0; j < slen; j++ ) {
			surf->name[j] = tolower( surf->name[j] );
		}

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen( surf->name );
		if ( j > 2 && surf->name[j-2] == '_' ) {
			surf->name[j-2] = 0;
		}

		// register the shaders
		shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
		for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
			const idMaterial *sh = declManager->FindMaterial( shader->name );
			// DG: md3Shader_t must use an index to the material instead of a pointer,
			//     otherwise the sizes are wrong on 64bit and we get data corruption
			shader->shaderIndex = (sh != NULL) ? shaders.AddUnique( sh ) : -1;
		}

		// swap all the triangles
		tri = (md3Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
		for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
			LL(tri->indexes[0]);
			LL(tri->indexes[1]);
			LL(tri->indexes[2]);
		}

		// swap all the ST
		st = (md3St_t *) ( (byte *)surf + surf->ofsSt );
		for ( j = 0 ; j < surf->numVerts ; j++, st++ ) {
			st->st[0] = LittleFloat( st->st[0] );
			st->st[1] = LittleFloat( st->st[1] );
		}

		// swap all the XyzNormals
		xyz = (md3XyzNormal_t *) ( (byte *)surf + surf->ofsXyzNormals );
		for ( j = 0 ; j < surf->numVerts * surf->numFrames ; j++, xyz++ )
		{
			xyz->xyz[0] = LittleShort( xyz->xyz[0] );
			xyz->xyz[1] = LittleShort( xyz->xyz[1] );
			xyz->xyz[2] = LittleShort( xyz->xyz[2] );

			xyz->normal = LittleShort( xyz->normal );
		}

		silInfos.Append( BuildSilInfo( surf ) );

		// find the next surface
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}

	fileSystem->FreeFile( buffer );
}

/*
=================
idRenderModelMD3::IsDynamicModel
=================
*/
dynamicModel_t idRenderModelMD3::IsDynamicModel() const {
	return DM_CACHED;
}

/*
=================
idRenderModelMD3::LerpMeshVertexes
=================
*/
void idRenderModelMD3::LerpMeshVertexes ( srfTriangles_t *tri, const struct md3Surface_s *surf, const float backlerp, const int frame, const int oldframe ) const {
	short	*oldXyz, *newXyz;
	float	oldXyzScale, newXyzScale;
	int		vertNum;
	int		numVerts;

	newXyz = (short *)((byte *)surf + surf->ofsXyzNormals) + (frame * surf->numVerts * 4);

	newXyzScale = MD3_XYZ_SCALE * (1.0 - backlerp);

	numVerts = surf->numVerts;

	if ( backlerp == 0 ) {
		//
		// just copy the vertexes
		//
		for (vertNum=0 ; vertNum < numVerts ; vertNum++, newXyz += 4 ) {

			idDrawVert *outvert = &tri->verts[tri->numVerts];

			outvert->xyz.x = newXyz[0] * newXyzScale;
			outvert->xyz.y = newXyz[1] * newXyzScale;
			outvert->xyz.z = newXyz[2] * newXyzScale;

			tri->numVerts++;
		}
	} else {
		//
		// interpolate and copy the vertexes
		//
		oldXyz = (short *)((byte *)surf + surf->ofsXyzNormals) + (oldframe * surf->numVerts * 4);

		oldXyzScale = MD3_XYZ_SCALE * backlerp;

		for (vertNum=0 ; vertNum < numVerts ; vertNum++, oldXyz += 4, newXyz += 4 ) {

			idDrawVert *outvert = &tri->verts[tri->numVerts];

			// interpolate the xyz
			outvert->xyz.x = oldXyz[0] * oldXyzScale + newXyz[0] * newXyzScale;
			outvert->xyz.y = oldXyz[1] * oldXyzScale + newXyz[1] * newXyzScale;
			outvert->xyz.z = oldXyz[2] * oldXyzScale + newXyz[2] * newXyzScale;

			tri->numVerts++;
		}
	}
}

/*
=============
idRenderModelMD3::InstantiateDynamicModel
=============
*/
idRenderModel *idRenderModelMD3::InstantiateDynamicModel( const struct renderEntity_s *ent, const struct viewDef_s *view, idRenderModel *cachedModel ) {
	int				i, j;
	float			backlerp;
	float *			texCoords;
	int				numVerts;
	md3Surface_t *	surface;
	int				frame, oldframe;
	idRenderModelStatic	*staticModel;

	int numFrames = md3->numFrames;

	// TODO: these need set by an entity
	frame = idMath::ClampInt(0, numFrames-1, ent->shaderParms[SHADERPARM_MD3_FRAME]);	// probably want to keep frames < 1000 or so
	oldframe = idMath::ClampInt(0, numFrames-1,ent->shaderParms[SHADERPARM_MD3_LASTFRAME]);
	backlerp = ent->shaderParms[SHADERPARM_MD3_BACKLERP];

	if ( cachedModel ) {
		if ( !r_useCachedDynamicModels.GetBool() || frame != lastFrame
		     || oldframe != lastOldFrame || backlerp != lastBackLerp ) {
			delete cachedModel;
			cachedModel = NULL;
		} else {
			// if we're still in the same animation frame etc, just use the existing model
			return cachedModel;
		}
	}

	// DG: remember (old)frame and backlerp for the check above if the cached model can be reused
	lastFrame = frame;
	lastOldFrame = oldframe;
	lastBackLerp = backlerp;

	staticModel = new idRenderModelStatic;
	staticModel->InitEmpty("_MD3_Snapshot_");
	staticModel->bounds.Clear();

	surface = (md3Surface_t *) ((byte *)md3 + md3->ofsSurfaces);

	for( i = 0; i < md3->numSurfaces; i++ ) {

		srfTriangles_t *tri = R_AllocStaticTriSurf();
		R_AllocStaticTriSurfVerts( tri, surface->numVerts );

		// DG: set sil edges for shadows
		silInfo_t& silInfo = silInfos[i];
		tri->numSilEdges = silInfo.numSilEdges;
		tri->silEdges = silInfo.silEdges;
		tri->silIndexes = silInfo.silIndexes;

		tri->indexes = silInfo.indexes;
		tri->numIndexes = silInfo.numIndexes;
		assert(surface->numTriangles * 3 == silInfo.numIndexes);

		// deformedSurface prevents silEdges, silIndexes, indexes (and more not used here)
		// from being freed (they belong to silInfo which is freed in ~idRenderModelMD3())
		tri->deformedSurface = true;

		tri->bounds.Clear();

		modelSurface_t	surf;

		surf.geometry = tri;

		md3Shader_t* shaders = (md3Shader_t *) ((byte *)surface + surface->ofsShaders);
		// FIXME: theoretically there can be multiple shaders?
		// DG: turned md3Shader_t::shader (pointer) into an int (index)
		int shaderIdx = shaders->shaderIndex;
		surf.shader = (shaderIdx >= 0) ? this->shaders[shaderIdx] : NULL;

		LerpMeshVertexes( tri, surface, backlerp, frame, oldframe );

		texCoords = (float *) ((byte *)surface + surface->ofsSt);

		numVerts = surface->numVerts;
		for ( j = 0; j < numVerts; j++ ) {
			idDrawVert *stri = &tri->verts[j];
			stri->st[0] = texCoords[j*2+0];
			stri->st[1] = texCoords[j*2+1];
		}

		R_BoundTriSurf( tri );

		surf.id = staticModel->NumSurfaces(); // DG: make sure to initialize id; FIXME: or just set id to 0?
		staticModel->AddSurface( surf );
		staticModel->bounds.AddPoint( surf.geometry->bounds[0] );
		staticModel->bounds.AddPoint( surf.geometry->bounds[1] );

		// find the next surface
		surface = (md3Surface_t *)( (byte *)surface + surface->ofsEnd );
	}

	return staticModel;
}

/*
=====================
idRenderModelMD3::Bounds
=====================
*/

idBounds idRenderModelMD3::Bounds(const struct renderEntity_s *ent) const {
	idBounds		ret;

	ret.Clear();

	if (!ent || !md3) {
		// just give it the editor bounds
		ret.AddPoint(idVec3(-10,-10,-10));
		ret.AddPoint(idVec3( 10, 10, 10));
		return ret;
	}

	md3Frame_t	*frame = (md3Frame_t *)( (byte *)md3 + md3->ofsFrames );
	int frameNum = Max(0, Min(md3->numFrames-1, (int)ent->shaderParms[SHADERPARM_MD3_FRAME]));
	frame += frameNum; // DG: use bounds of current frame

	ret.AddPoint( frame->bounds[0] );
	ret.AddPoint( frame->bounds[1] );

	return ret;
}

/*
=====================
idRenderModelMD3::BuildSilInfo
=====================
*/

silInfo_t idRenderModelMD3::BuildSilInfo( md3Surface_t *surf ) {
	//
	// build the information that will be common to all animations of this surface:
	// silhouette edge connectivity and normal / tangent generation information
	//
	int numVerts = surf->numVerts;
	silInfo_t ret = {0};

	md3XyzNormal_t *xyzs = (md3XyzNormal_t *) ( (byte *)surf + surf->ofsXyzNormals );

	bool onStack;
	idDrawVert *verts = (idDrawVert*)Mem_MallocA( numVerts*sizeof(idDrawVert), onStack );

	for ( int i=0; i < numVerts; ++i ) {
		idDrawVert& v = verts[i];
		v.Clear();
		// Note: MD3 defines these for all frames - using frame 0 here
		short* xyz = xyzs[i].xyz;
		v.xyz.x = xyz[0] * float(MD3_XYZ_SCALE);
		v.xyz.y = xyz[1] * float(MD3_XYZ_SCALE);
		v.xyz.z = xyz[2] * float(MD3_XYZ_SCALE);
		// Note: idDrawVert has more fields, but R_CreateSilIndexes() and R_IdentifySilEdges()
		//       only use .xyz
	}

	int numIndexes = surf->numTriangles * 3;
	md3Triangle_t *md3TriIdx = (md3Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
	int *triIndexes = (int*)md3TriIdx->indexes;

	srfTriangles_t tri;
	memset( &tri, 0, sizeof( tri ) );

	tri.verts = verts;
	tri.numVerts = numVerts;
	tri.numIndexes = numIndexes;
	if ( sizeof(triIndexes[0]) == sizeof(tri.indexes[0]) ) {
		tri.indexes = triIndexes;
	} else {
		R_AllocStaticTriSurfIndexes( &tri, numIndexes );
		// don't memcpy, so we can change the index type from int to short without changing the interface
		for ( int i = 0 ; i < numIndexes ; i++ ) {
			tri.indexes[i] = triIndexes[i];
		}
	}
	R_CreateSilIndexes(&tri); // allocates and sets tri.silIndexes with triSilIndexAllocator
	// if there's only one frame (no animation), the omitCoplanarEdges optimization can be used
	// (otherwise it can't because edges that are coplanar in frame 0 may not be in another frame)
	bool omitCoplanarEdges = (surf->numFrames == 1);
	R_IdentifySilEdges(&tri, omitCoplanarEdges); // allocates and sets tri.silEdges with triSilEdgeAllocator

	ret.numIndexes = tri.numIndexes;
	ret.numSilEdges = tri.numSilEdges;
	ret.indexes = tri.indexes;
	ret.silIndexes = tri.silIndexes;
	ret.silEdges = tri.silEdges;

	Mem_FreeA( verts, onStack );

	return ret;
}
