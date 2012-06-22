// Created by ivan_the_B
//

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "TrailGenerator.h"
#include "Game_local.h"

//uncomment those to enable debug
//#define _DEBUG_CUSTOM_GEOM
//#define _DEBUG_TRAIL 

static const dword trailGenerator_vertexColor = PackColor( idVec4( 1,1,1,1 ) ); 
static const char *trailGenerator_SnapshotName = "_TrailGenerator_Snapshot_"; //Note: all the trail models use this name. It's not used so it's ok.

static const int TRAIL_ID_INVALID_UNIQUE = 0;

/*
===============================================================================

idTrailManager

===============================================================================
*/

/*
================
idTrailManager::idTrailManager
================
*/
idTrailManager::idTrailManager( void ) {
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailManager::idTrailManager\n");
#endif
	initialized = false;
}

/*
================
idTrailManager::Init
================
*/
void idTrailManager::Init( void ) {

#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailManager::Init\n");
#endif
		
	Shutdown();

	memset( trails, 0, sizeof( trails ) );
	memset( uniqueIds, TRAIL_ID_INVALID_UNIQUE, sizeof( uniqueIds ) );

	lastUniqueId = TRAIL_ID_INVALID_UNIQUE;

	minFreePos = 0; //first slot will be empty here
	maxAllocPos = -1; //no one

	initialized = true;
}

/*
================
idTrailManager::~idTrailManager
================
*/
idTrailManager::~idTrailManager( void ) {
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailManager::~idTrailManager\n");
#endif

	if(!initialized) return;

	for ( int i = 0; i < MAX_TRAILS; i++ ) {
		if( trails[ i ] ){
			delete trails[ i ];
			trails[ i ] = NULL;
		}
	}
}


/*
================
idTrailManager::Shutdown
================
*/
void idTrailManager::Shutdown( void ) {
	if( !initialized ) return;

#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailManager::Shutdown\n");
#endif

	//deallocate all the trails that are still in memory
	for ( int i = 0; i < MAX_TRAILS; i++ ) {
		if( trails[ i ] ){
			gameLocal.Warning("Someone created a trail (uid: %d), but never removed it!\TrailManager will now take care to free the memory...but check your code!", trails[ i ]->GetUniqueId() );
			delete trails[ i ];
			trails[ i ] = NULL;
		}
	}

	initialized = false;
}

/*
================
idTrailManager::Save
================
*/
void idTrailManager::Save( idSaveGame *savefile ) const {
#ifdef _DEBUG_TRAIL 	
	gameLocal.Printf("idTrailManager::Save\n");
#endif
	savefile->WriteBool( initialized );
	savefile->WriteInt( lastUniqueId ); 

	savefile->WriteInt( minFreePos );
	savefile->WriteInt( maxAllocPos );
	
	for ( int i = 0; i <= maxAllocPos; i++ ) { //only stuff before 'maxAllocPos'
#ifdef _DEBUG_TRAIL 
		gameLocal.Printf("saving int %d\n", uniqueIds[ i ]);
#endif
		savefile->WriteInt( uniqueIds[ i ] ); 
		if( uniqueIds[ i ] != TRAIL_ID_INVALID_UNIQUE ){
			trails[ i ]->Save( savefile );
		}
	}
}

/*
================
idTrailManager::Restore
================
*/
void idTrailManager::Restore( idRestoreGame *savefile ) {	

#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailManager::Restore\n");
#endif

	savefile->ReadBool( initialized );
	savefile->ReadInt( lastUniqueId );

	savefile->ReadInt( minFreePos );
	savefile->ReadInt( maxAllocPos );

	//load the ones before 'maxAllocPos'
	for( int i = 0; i <= maxAllocPos; i++ ) { //only stuff before 'maxAllocPos'
		savefile->ReadInt( uniqueIds[ i ] );
		if( uniqueIds[ i ] != TRAIL_ID_INVALID_UNIQUE ){ //it was there. NOTE: same local pos!
			trails[i] = new idTrailGenerator;
			trails[i]->Restore( savefile );

#ifdef _DEBUG_TRAIL 
			//test only
			if( trails[ i ]->GetLocalPos() != i ){
				gameLocal.Error("localPos != array position! (%d)", i);
			}
			gameLocal.Printf("restored: - localPos: %d, uniId: %d\n", trails[ i ]->GetLocalPos(), trails[ i ]->GetUniqueId() ); 
#endif
		}else{

#ifdef _DEBUG_TRAIL 
			gameLocal.Printf("restored: NULL at pos %d\n", i); 
#endif
			trails[i] = NULL;
		}
	}

	//null the others
	for( int i = maxAllocPos+1; i < MAX_TRAILS; i++ ) { 
		uniqueIds[ i ] = TRAIL_ID_INVALID_UNIQUE;
		trails[i] = NULL;
	}

}


/*
================
idTrailManager::FindTrailByUniqueId
================
*/
int idTrailManager::GetSafeUniqueId( idTrailGenerator* trailGen ){
	return ( trailGen ? trailGen->GetUniqueId() : TRAIL_ID_INVALID_UNIQUE );
}

/*
================
idTrailManager::FindTrailByUniqueId
================
*/
idTrailGenerator* idTrailManager::FindTrailByLocalPos( int pos ) { //fast
	if( pos < 0 || pos >= MAX_TRAILS ){
		return NULL;
	}
	return trails[ pos ];
}


/*
================
idTrailManager::FindTrailByUniqueId
================
*/
idTrailGenerator* idTrailManager::FindTrailByUniqueId( int id ) { //slow, but ensure it's not another trail at the same pos
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailManager::FindTrailById...\n"); 
#endif
	
	if( id == TRAIL_ID_INVALID_UNIQUE ){ return NULL; }

	for ( int i = 0; i < MAX_TRAILS; i++ ) {
		if( trails[ i ] && trails[ i ]->GetUniqueId() == id ){
#ifdef _DEBUG_TRAIL 
			gameLocal.Printf("...found at pos: %d\n", i); 
#endif
			return trails[ i ];
		}
	}
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("...not found\n"); 
#endif
	return NULL;
}

/*
================
idTrailManager::Think
================
*/
void idTrailManager::Think( void ) {	
	for ( int i = 0; i <= maxAllocPos; i++ ) { //don't even check beyond 'maxAllocPos' because they all are null
		if( trails[ i ] && trails[ i ]->fading ){ //->IsFading()
			trails[ i ]->Fade();
		}
	}
}

/*
================
idTrailManager::GetTrailGen
================
*/
idTrailGenerator* idTrailManager::NewTrailGen( void ) {	
	int i;

#ifdef _DEBUG_TRAIL
	gameLocal.Printf("idTrailManager::NewTrailGen\n");
#endif

	if( minFreePos == MAX_TRAILS ){
		gameLocal.Error("No free slots for another trail!");
		return NULL;
	}

	idTrailGenerator* myGen = new idTrailGenerator;

	//set an unique id
	lastUniqueId++;
	myGen->Init( lastUniqueId, minFreePos );


	//add it to list
	trails[ minFreePos ] = myGen;
	uniqueIds[ minFreePos ] = lastUniqueId;

#ifdef _DEBUG_TRAIL
	gameLocal.Printf("Added trail 'uid: %d' at pos: %d\n", lastUniqueId, minFreePos );
#endif

	//-- upd max pos allocated -- 
	if( minFreePos > maxAllocPos ){ //note: minFreePos is were it was placed
		maxAllocPos = minFreePos;

#ifdef _DEBUG_TRAIL
		gameLocal.Printf("new maxAllocPos: %d\n", maxAllocPos);
#endif
	}

	//-- upd min pos free -- (we are sure no one is free before 'minFreePos')
	for ( i = minFreePos+1; i < MAX_TRAILS; i++ ) { //from here (well, next one) to max
		if( !trails[ i ] ){ break; } //null
	}
	minFreePos = i; // MAX_TRAILS if full array

#ifdef _DEBUG_TRAIL
	gameLocal.Printf("new minFreePos: %d\n", minFreePos);

	if( minFreePos == MAX_TRAILS ){
		gameLocal.Warning("This was the last free slot for a trail!");
	}
#endif
	
	return myGen;
}

/*
================
idTrailManager::RemoveTrailGen
================
*/
void idTrailManager::RemoveTrailGen( idTrailGenerator* trailGen ) {	
	int i;
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailManager::RemoveTrailGen\n");
#endif
	if( !trailGen ){ return; }

	//delete
	int localPos = trailGen->GetLocalPos();
	delete trails[ localPos ];
	trails[ localPos ] = NULL;

	//clear id
	uniqueIds[ localPos ] = TRAIL_ID_INVALID_UNIQUE;

	//-- upd min pos free -- 
	if( localPos < minFreePos ){ //if this one is now the first NULL
		minFreePos = localPos; 

#ifdef _DEBUG_TRAIL
		gameLocal.Printf("new minFreePos: %d\n", minFreePos);
#endif

	}

	//-- upd max pos allocated -- 
	if( localPos == maxAllocPos ){ //if this one was the last allocated... go back until we find the first used slot
		for ( i = maxAllocPos-1; i >= 0; i-- ) { //from prev one to 0
			if( trails[ i ] ){ break; } //used
		}
		maxAllocPos = i; // -1 if empty array

#ifdef _DEBUG_TRAIL
		gameLocal.Printf("new maxAllocPos: %d\n", maxAllocPos);
#endif

	}

#ifdef _DEBUG_TRAIL
	gameLocal.Printf("Removed trail at pos: %d\n", minFreePos );
#endif
}



/*
===============================================================================

idTrailGenerator

===============================================================================
*/


/*
================
idTrailGenerator::idTrailGenerator
================
*/
idTrailGenerator::idTrailGenerator( void ) {
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailGenerator::idTrailGenerator\n");
#endif

	initialized = false;

	//new
	uniqueId = TRAIL_ID_INVALID_UNIQUE; 
	localPos = MAX_TRAILS; 	
	//thinkFlags = 0;
}

/*
================
idTrailGenerator::Init
================
*/
void idTrailGenerator::Init( int uniId, int locPos ) {
	//gameLocal.Printf("idTrailGenerator::Init\n");
	
	if( initialized ){
		gameLocal.Error("Cannot initialize the same trail twice");
	}

	uniqueId = uniId; //used for restoring references!
	localPos = locPos; //used for callback! It's faster but cannot identify it univocally 
	//thinkFlags = 0;

	//settings
	enabled = false;
	fading = false;
	fadingTime = 0;
	fadingEndTime = 0;
	fadingColorStep.Zero();
	maxPoints = 0;
	material = NULL;
	trailDef = NULL;
	trailDefName = "";
	points.SetGranularity( 2 ); //use 2 because we always add/remove couple of points.


	//rendering
	memset( &renderEntity, 0, sizeof( renderEntity ) );

	renderEntity.entityNum = localPos; 
	renderEntity.bounds.Clear();
	renderEntity.axis = mat3_identity;
	renderEntity.shaderParms[ SHADERPARM_RED ]		= 1;
	renderEntity.shaderParms[ SHADERPARM_GREEN ]	= 1;
	renderEntity.shaderParms[ SHADERPARM_BLUE ]		= 1;
	renderEntity.shaderParms[3] = 1;
	renderEntity.hModel = renderModelManager->AllocModel();
	renderEntity.hModel->InitEmpty( trailGenerator_SnapshotName );
	renderEntity.noShadow = 1;
	renderEntity.callback = idTrailGenerator::ModelCallback;
	renderEntity.bounds.AddPoint( idVec3(-100000, -100000, -100000) ); // huge bounds, so it will be present in every world area
	renderEntity.bounds.AddPoint( idVec3( 100000,  100000,  100000) );

	// add to renderer list
	//renderEntityHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	renderEntityHandle = -1; //don't add it to renderworld until it's disabled

	lastRenderEntityUpdate = -1;
	modelChanged = false;
	
	initialized = true;
}

//always call this after color/flags change on renderEntity
void idTrailGenerator::UpdateVisuals( void ) {
	// add to refresh list
	if ( renderEntityHandle == -1 ) {
		renderEntityHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	} else {
		gameRenderWorld->UpdateEntityDef( renderEntityHandle, &renderEntity ); //save colors and other flags on renderEntity
	}
}

//remove it from RenderWorld when disabled
void idTrailGenerator::FreeEntityDef( void ) {
	if ( renderEntityHandle != -1 ) {
		gameRenderWorld->FreeEntityDef( renderEntityHandle );
		renderEntityHandle = -1;
	}
}


/*
================
idTrailGenerator::~idTrailGenerator
================
*/
idTrailGenerator::~idTrailGenerator( void ) {
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailGenerator::~idTrailGenerator\n");
#endif

	if(!initialized){ return; }

	// make sure the render entity is freed before the model is freed
	if ( renderEntityHandle != -1 ) {
		gameRenderWorld->FreeEntityDef( renderEntityHandle );
		renderEntityHandle = -1;
	}
	if ( renderEntity.hModel != NULL ) {
		renderModelManager->FreeModel( renderEntity.hModel );
		renderEntity.hModel = NULL;
	}
}

/*
================
idTrailGenerator::Save
================
*/
void idTrailGenerator::Save( idSaveGame *savefile ) const {
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailGenerator::Save\n");
#endif
	//global
	savefile->WriteInt( uniqueId );
	savefile->WriteInt( localPos );
	savefile->WriteBool( initialized );

	// settings
	savefile->WriteBool( enabled );
	savefile->WriteBool( fading );
	savefile->WriteVec3( fadingColorStep );
	savefile->WriteInt( fadingTime );
	savefile->WriteInt( fadingEndTime );
	savefile->WriteInt( maxPoints );
	savefile->WriteMaterial( material );

	//trail def
	//trailDef is NOT saved, but can be retrieved from trailDefName
	savefile->WriteString( trailDefName );

	//points are NOT saved!! The trail will be be empty.
	//idList<idVec3>	points;

	//rendering
	savefile->WriteRenderEntity( renderEntity );
	savefile->WriteInt( renderEntityHandle );
	savefile->WriteInt( lastRenderEntityUpdate );
	savefile->WriteBool( modelChanged );
}

/*
================
idTrailGenerator::Restore
================
*/
void idTrailGenerator::Restore( idRestoreGame *savefile ) {	
#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("idTrailGenerator::Restore\n");
#endif
	//global
	savefile->ReadInt( uniqueId ); //used for restoring references!
	savefile->ReadInt( localPos );
	savefile->ReadBool( initialized );

	// settings
	savefile->ReadBool( enabled );
	savefile->ReadBool( fading );
	savefile->ReadVec3( fadingColorStep );
	savefile->ReadInt( fadingTime );
	savefile->ReadInt( fadingEndTime );
	savefile->ReadInt( maxPoints );
	savefile->ReadMaterial( material );

	//trail def
	savefile->ReadString( trailDefName );
	if ( trailDefName.Length() ) {
		trailDef = gameLocal.FindEntityDefDict( trailDefName );
	} else {
		trailDef = NULL;
		//gameLocal.Warning("trailDef not found!");
	}

	//points are NOT saved!!
	points.Clear();
	points.SetGranularity( 2 );

	//rendering
	savefile->ReadRenderEntity( renderEntity );
	savefile->ReadInt( renderEntityHandle );
	savefile->ReadInt( lastRenderEntityUpdate );
	savefile->ReadBool( modelChanged );

	//fix - reallocate the model ( this was created on init )
	renderEntity.entityNum = localPos; //used for callback!
	renderEntity.hModel = renderModelManager->AllocModel();
	renderEntity.hModel->InitEmpty( trailGenerator_SnapshotName );
	renderEntity.callback = idTrailGenerator::ModelCallback;
	renderEntity.noShadow = true;
	renderEntity.noSelfShadow = true;
	renderEntity.noDynamicInteractions = false;

	/*
	// restore must retrieve renderEntityHandle from the renderer
	if ( renderEntityHandle != -1 ) {
		renderEntityHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	}
	*/
	if ( renderEntityHandle != -1 ) {
			renderEntityHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	}
}

/*
================
idTrailGenerator::RestartTrail
================
*/
void idTrailGenerator::RestartTrail( void ){
	idVec3 color;

#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("RestartTrail %s\n", trailDefName.c_str() );
#endif

	if( trailDef ){ 
		//restore the old color
		trailDef->GetVector( "color", "1 1 1", color );	
		renderEntity.shaderParms[ SHADERPARM_RED ]		= color[ 0 ];
		renderEntity.shaderParms[ SHADERPARM_GREEN ]	= color[ 1 ];
		renderEntity.shaderParms[ SHADERPARM_BLUE ]		= color[ 2 ];
	}else{
		gameLocal.Warning("Trying to restart a trail without a def!\n");
		return;
	}

	//state
	enabled = true; //weapon will upd the position
	fading = false;
	modelChanged = true; //upd renderer
	UpdateVisuals(); //make sure it's in the world and updated
}


/*
================
idTrailGenerator::StartTrail
================
*/
void idTrailGenerator::StartTrail( const char *newTrailDefName ){	
	const char *materialName;
	idVec3 color;
	const idDict *newTrailDef;

#ifdef _DEBUG_TRAIL 
	gameLocal.Printf("StartTrail %s\n", newTrailDefName );
#endif

	newTrailDef = gameLocal.FindEntityDefDict( newTrailDefName, false );
	if ( !newTrailDef ) {
		gameLocal.Error( "Unknown def '%s'", newTrailDefName );
	}
	
	//NOTE: this works, but we assume that if it's the same def, RestartTrail() will be called instead.
	if( trailDef && newTrailDef && (newTrailDef == trailDef) ){
		//gameLocal.Warning("Started a new Trail with the same def! use RestartTrail() instead to save performance!");
		RestartTrail();
		return;
	}

	//remove the points
	//points.Clear(); //we can keep the old ones so, if we already are active the trail goes on. 

	//fadingTime
	newTrailDef->GetInt( "fadeTime", "200", fadingTime );
	if( fadingTime < 0 ) fadingTime = 0;

	//maxPoints
	newTrailDef->GetInt( "numFrames", "10", maxPoints );
	maxPoints = ( maxPoints < 1 ) ? 4 : ( maxPoints*2+2 ); //1frame -> 4points, 2frame -> 6points, ... 

	//color
	newTrailDef->GetVector( "color", "1 1 1", color );	
	renderEntity.shaderParms[ SHADERPARM_RED ]		= color[ 0 ];
	renderEntity.shaderParms[ SHADERPARM_GREEN ]	= color[ 1 ];
	renderEntity.shaderParms[ SHADERPARM_BLUE ]		= color[ 2 ];

	//material
	materialName = newTrailDef->GetString( "mtr_surface", "" );
	material = declManager->FindMaterial( materialName );		

	//remember the new trail def
	trailDef = newTrailDef;
	trailDefName = newTrailDefName;

	//state
	enabled = true; //weapon will upd the position
	fading = false;
	modelChanged = true; //upd renderer
	UpdateVisuals(); //make sure it's in the world and updated
}

/*
================
idTrailGenerator::FadeTrail
================
*/
void idTrailGenerator::FadeTrail( void ){
	idVec3 colorNow;

	if( fadingTime <= 0 ){
		StopTrail();
		return;
	}

	//color
	colorNow[ 0 ] = renderEntity.shaderParms[ SHADERPARM_RED ];
	colorNow[ 1 ] = renderEntity.shaderParms[ SHADERPARM_GREEN ];
	colorNow[ 2 ] = renderEntity.shaderParms[ SHADERPARM_BLUE ];

	//color step for each frame
	fadingColorStep = ( vec3_zero - colorNow)*USERCMD_MSEC/fadingTime; 

	//time
	fadingEndTime = gameLocal.time + fadingTime;

	//state
	enabled = true; //weapon will upd the position
	fading = true;
	UpdateVisuals(); //make sure it's in the world and updated

}

/*
================
idTrailGenerator::StopTrail
================
*/
void idTrailGenerator::StopTrail( void ){
	if(!enabled) return;

	//remove points
	points.Clear();

	//state
	enabled = false;
	fading = false;
	modelChanged = true; //upd renderer

	FreeEntityDef(); //remove from world
}

/*
================
idTrailGenerator::AddNewPoints
================
*/
void idTrailGenerator::AddNewPoints( const idVec3 &newPosL, const idVec3 &newPosH ) {

	if( !enabled ){
		gameLocal.Warning("Cannot add points to a disabled trail!");
		return;
	}

	/*
	//NOTE: this is commented out because we assume that sword is moving while trail is active.
	if( points.Num() >= 2 ){
		if( points[0].Compare( newPosL ) && points[1].Compare( newPosH ) ){ 
			return; //if points did not change don't upd anything 
		} 
	}
	*/

	//remove oldest points
	while( points.Num() >= maxPoints ){ //example: maxPoints 6 and we have 6 -> remove 2 so we can add the new ones
		points.RemoveIndex(0);
		points.RemoveIndex(0);
	}

	points.Append( newPosL );
	points.Append( newPosH );

	modelChanged = true; //upd renderer
}

/*
================
idTrailGenerator::RemoveOldestPoints
================
*/
void idTrailGenerator::RemoveOldestPoints( void ) {

	if( !enabled ){
		gameLocal.Warning("Cannot remove points to a disabled trail!");
		return;
	}

	/*
	//NOTE: this is commented out because we assume that sword is moving while trail is active.
	if( points.Num() >= 2 ){
		if( points[0].Compare( newPosL ) && points[1].Compare( newPosH ) ){ 
			return; //if points did not change don't upd anything 
		} 
	}
	*/

	//remove oldest points
	if( points.Num() >= 2 ){ 
		points.RemoveIndex(0);
		points.RemoveIndex(0);
		modelChanged = true; //upd renderer
	}
}

/*
================
idTrailGenerator::UpdateRenderEntity
================
*/
bool idTrailGenerator::UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) const {
	int i, j, numTris, numPoints;
	float fitpct, fitoffset;
	srfTriangles_t *tris;
	modelSurface_t surface; 
	idDrawVert *v;
	idPlane plane;
	idMat3 tangents;
	idFixedWinding winding; //TODO: this is big...promote it to class member to save perf?

	// this may be triggered by a model trace or other non-view related source,
	// to which we should look like an empty model
	if ( !renderView ) {
		return false;
	}

	// regenerate only if modelChanged. Only one per frame.
	//was: if ( !modelChanged || lastRenderEntityUpdate == gameLocal.time ) {
	if ( lastRenderEntityUpdate == gameLocal.time ) { //moved the "modelChanged" check in callback to save perf.
		return false;
	}

	lastRenderEntityUpdate = gameLocal.time;
	modelChanged = false;
	//gameLocal.Printf("UpdateRenderEntity at time: %d\n", lastRenderEntityUpdate);

	//we need at least 4 points + even number
	numPoints = points.Num();
	if( numPoints < 4 ){ //|| (numPoints % 2 != 0) //number is always even thanks to creation policy.
		//clean the model and return.
		renderEntity->hModel->InitEmpty( trailGenerator_SnapshotName );
		return true;
	}

	//calculate number of tris and materialOffset-per-coupleOfTris pct 
	numTris = numPoints - 2; //examples: 4p -> 2t, 6p -> 4t, 8p ->6t
	fitpct = 1.0f / ( numTris /2.0f ); // numTris/2 is the number of steps. 4p -> 2t -> 1step, 6p -> 4t -> 2steps, 8p ->6t -> 3steps 

	// FIXME: re-use model surfaces? <-- we cannot do that: we would need to remove the oldest piece and shift the texture.
	renderEntity->hModel->InitEmpty( trailGenerator_SnapshotName );

	#ifdef _DEBUG_CUSTOM_GEOM
	//debug
	idBounds bbox;
	bbox.Zero();
	bbox.ExpandSelf(1);
	idVec4 color;
	color[3] = 1; //alpha
	for( i = 0; i < numPoints; i++ ){
		if( i % 2 == 0){
			color[0] = gameLocal.random.RandomFloat();
			color[1] = gameLocal.random.RandomFloat();
			color[2] = gameLocal.random.RandomFloat();
		}
		gameRenderWorld->DebugBounds( color, bbox, points[i], 20000);
	}
	#endif

	// allocate triangle surfaces for the fractures and decals
	tris = renderEntity->hModel->AllocSurfaceTriangles( numTris * 3, material->ShouldCreateBackSides() ? numTris * 6 : numTris * 3 );

	//packedColor = PackColor( idVec4( 1,1,1,1 ) ); 

	/*
	gameLocal.Printf("color found: %f, %f, %f\n", renderEntity->shaderParms[ SHADERPARM_RED ], renderEntity->shaderParms[ SHADERPARM_GREEN ], renderEntity->shaderParms[ SHADERPARM_BLUE ] );				
	if( &(this->renderEntity) == renderEntity ){
		gameLocal.Printf("Stesso renderEntity!\n");
	}
	*/

	//gameLocal.Printf("iteration starts...\n ");
	fitoffset = 1.0f;
	for( i = 0; i < (numPoints - 3); i += 2 ){
		//gameLocal.Printf(",%d",i);
		
		//re-use the same winding every time
		winding.Clear();
		 
		//points closer to the sword
		winding.AddPoint( idVec5( points[i], idVec2(0, fitoffset )) );	//Lower point
		winding.AddPoint( idVec5( points[i+1], idVec2(1, fitoffset)) );	//Higher point  

		//gameLocal.Printf("fitoffset: %f, fitoffset-fitpct: %f\n", fitoffset, (fitoffset-fitpct) );
		fitoffset -= fitpct; //decrease the texture offset so that the texture is correctly fit
		if( fitoffset < 0.0f ){ fitoffset = 0.0f; } //solve approximation issues

		//previous points
		winding.AddPoint( idVec5( points[i+3], idVec2(1, fitoffset)) );	//Higher point 
		winding.AddPoint( idVec5( points[i+2], idVec2(0, fitoffset)) );	//Lower point 

		//get info from windings
		winding.GetPlane( plane );
		tangents = ( plane.Normal() ).ToMat3(); //was: ( plane.Normal() * axis ).ToMat3();

		//create tris given points in winding. Usually we have 4 points -> 2tris
		for ( j = 2; j < winding.GetNumPoints(); j++ ) {

			//first vertex is always placed at the first point
			v = &tris->verts[tris->numVerts++];
			v->Clear();
			v->xyz = winding[0].ToVec3();
			v->st[0] = winding[0].s;
			v->st[1] = winding[0].t;
			v->normal = tangents[0];
			v->tangents[0] = tangents[1];
			v->tangents[1] = tangents[2];
			v->SetColor( trailGenerator_vertexColor );

			//second vertex
			v = &tris->verts[tris->numVerts++];
			v->Clear();
			v->xyz = winding[j-1].ToVec3();
			v->st[0] = winding[j-1].s;
			v->st[1] = winding[j-1].t;
			v->normal = tangents[0];
			v->tangents[0] = tangents[1];
			v->tangents[1] = tangents[2];
			v->SetColor( trailGenerator_vertexColor );

			//third vertex
			v = &tris->verts[tris->numVerts++];
			v->Clear();
			v->xyz = winding[j].ToVec3();
			v->st[0] = winding[j].s;
			v->st[1] = winding[j].t;
			v->normal = tangents[0];
			v->tangents[0] = tangents[1];
			v->tangents[1] = tangents[2];
			v->SetColor( trailGenerator_vertexColor );

			//set the index for each tris
			tris->indexes[tris->numIndexes++] = tris->numVerts - 3;
			tris->indexes[tris->numIndexes++] = tris->numVerts - 2;
			tris->indexes[tris->numIndexes++] = tris->numVerts - 1;

			if ( material->ShouldCreateBackSides() ) {

				tris->indexes[tris->numIndexes++] = tris->numVerts - 2;
				tris->indexes[tris->numIndexes++] = tris->numVerts - 3;
				tris->indexes[tris->numIndexes++] = tris->numVerts - 1;
			}
		}
	}

	tris->tangentsCalculated = true;

	SIMDProcessor->MinMax( tris->bounds[0], tris->bounds[1], tris->verts, tris->numVerts );

	memset( &surface, 0, sizeof( surface ) );
	surface.shader = material;
	surface.id = 0;
	surface.geometry = tris;
	renderEntity->hModel->AddSurface( surface );

	return true;
}

/*
================
idTrailGenerator::ModelCallback
================
*/
bool idTrailGenerator::ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView ) {
	//gameLocal.Printf("idTrailGenerator::ModelCallback\n");

	const idTrailGenerator *trailGen;

	trailGen = gameLocal.trailsManager->trails[ renderEntity->entityNum ];
	if ( !trailGen ) {
		gameLocal.Error( "idTrailGenerator::ModelCallback: callback with NULL trailGen" );
	}

	if ( !trailGen->modelChanged ){
		return false;
	}

	return trailGen->UpdateRenderEntity( renderEntity, renderView );
}


/*
================
idTrailGenerator::Think
================
*/
void idTrailGenerator::Fade( void ) { //TrailGenerator thinks before all the other entities.
	//gameLocal.Printf("idTrailGenerator::Fade\n"); 

	//if ( fading ) {
		if( fadingEndTime > gameLocal.time ){
			//gameLocal.Printf("fading...\n"); 
			renderEntity.shaderParms[ SHADERPARM_RED ]		+= fadingColorStep[ 0 ];
			renderEntity.shaderParms[ SHADERPARM_GREEN ]	+= fadingColorStep[ 1 ];
			renderEntity.shaderParms[ SHADERPARM_BLUE ]		+= fadingColorStep[ 2 ];
			UpdateVisuals(); //make sure it's in the world and updated
		}else{
			StopTrail();
		}	
	//} 
}
