
#include "../idlib/precompiled.h"
#pragma hdrstop

#ifdef _DENTONMOD

#include "tracer.h"
#include "Entity.h"
#include "SmokeParticles.h"
#include "Game_local.h"

/*
===============================================================================
dnTracer:
A kind of tracer effect that starts from weapon's barrel and ends at collision point, 
while the actual projectile starts from the view origin. This tracer travels in parallel 
with the projectile.
===============================================================================
*/

static const char *TRACER_WIDTH	  	 =   "1.7f";
static const char *TRACER_LENGTH	 = "170.0f";

/*
================
dnTracer::dnTracer
================ 
*/
dnTracer::dnTracer( idEntity *owner, const float distanceRatio, const idVec3 &viewOrigin, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis ) : dnTracerEffect( owner ) {

	SetType( TR_TYPE_TRACER );// We must set type in the beginning of every constructor.
	const char *modelName = owner->spawnArgs.GetString( "model_tracer" );

	if ( modelName[0] == '\0' ) {
		return; 	
	}
	this->distanceRatio			= distanceRatio;
	this->weaponMuzzleOrigin	= muzzleOrigin;
	this->playerViewOrigin		= viewOrigin;

	// align z-axis of model with the direction 
	this->axis[0] = tracerAxis[2];
	this->axis[2] = tracerAxis[0];
	this->axis[1] = tracerAxis[1];


	owner->SetModel( modelName );
}

/*
================
dnTracer::Think
================ 
*/
void dnTracer::Think( void )  {
	const float projectileDist = ( owner->GetPhysics()->GetOrigin() - playerViewOrigin ).LengthSqr() * distanceRatio;

	owner->GetRenderEntity()->axis		= axis;
	//owner->GetRenderEntity()->origin	= weaponMuzzleOrigin + idMath::Sqrt(projectileDist) * owner->GetRenderEntity()->.axis[2];
	owner->GetRenderEntity()->origin	= weaponMuzzleOrigin + projectileDist * idMath::RSqrt(projectileDist) * owner->GetRenderEntity()->axis[2]; // this is faster than commented line above
}


/*
=======================================================================================
dnBeamTracer:

Same effect as that of dnTracer. The only difference is, dnTracer uses a model or particle 
effect to represent the tracer, where as, this one uses _beam model. This tracer is pretty 
fast to draw than dnTracer.
=======================================================================================
*/

/*
================
dnBeamTracer::dnBeamTracer
================ 
*/
dnBeamTracer::dnBeamTracer( idEntity *owner, const float distanceRatio, const idVec3 &viewOrigin, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis ) : dnTracer( owner ) {

	SetType( TR_TYPE_BEAMTRACER );// We must set type in the beginning of every constructor.

	const char *skinName = owner->spawnArgs.GetString( "beam_skin" );

	if ( skinName[0] == '\0' ) {
		return; 	
	}
	this->distanceRatio			= distanceRatio;
	this->weaponMuzzleOrigin	= muzzleOrigin;
	this->playerViewOrigin		= viewOrigin;

	// align z-axis of model with the direction 
	this->axis[0] = tracerAxis[2];
	this->axis[2] = tracerAxis[0];
	this->axis[1] = tracerAxis[1];

	length	= owner->spawnArgs.GetFloat( "beam_length", TRACER_LENGTH);

	renderEntity_t *renderEntity = owner->GetRenderEntity();

	idVec4 tracerColor = owner->spawnArgs.GetVec4( "beam_color", "1.0 1.0 1.0 1.0" );

	renderEntity->origin = muzzleOrigin;

	renderEntity->shaderParms[ SHADERPARM_RED ]		= tracerColor[0];
	renderEntity->shaderParms[ SHADERPARM_GREEN ]	= tracerColor[1];
	renderEntity->shaderParms[ SHADERPARM_BLUE ]	= tracerColor[2];
	renderEntity->shaderParms[ SHADERPARM_ALPHA ]	= tracerColor[3];

	renderEntity->shaderParms[ SHADERPARM_BEAM_WIDTH ] = owner->spawnArgs.GetFloat( "beam_width", TRACER_WIDTH ); 

	idVec3 endPos;
	if( length == 0.0f ) {				//if the length is 0 then keep endPos to muzzleorigin constantly.
		endPos = muzzleOrigin;
	}
	else {
		endPos = renderEntity->origin - renderEntity->axis[2] * length;
	}
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_X ] = endPos[0];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Y ] = endPos[1];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Z ] = endPos[2];
	
	owner->SetModel( "_beam" );
	owner->SetSkin( declManager->FindSkin( skinName ) );	
}

/*
================
dnBeamTracer::Think
================ 
*/
void dnBeamTracer::Think( void )  {
	
	dnTracer::Think();

	renderEntity_t *renderEntity = owner->GetRenderEntity();

	idVec3 endPos;
	if( length == 0.0f ) {
		endPos = weaponMuzzleOrigin;
	}
	else {
		endPos = renderEntity->origin - renderEntity->axis[2] * length;

		if( (renderEntity->origin - endPos).LengthSqr() > (renderEntity->origin - weaponMuzzleOrigin).LengthSqr() ) {
			endPos = weaponMuzzleOrigin;
		}
	}	
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_X ] = endPos[0];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Y ] = endPos[1];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Z ] = endPos[2];
}

/*
=======================================================================================
dnBarrelLaunchedBeamTracer:

Same effect as that of dnTracer. The only difference is, dnTracer uses a model or particle 
effect to represent the tracer, where as, this one uses _beam model. This tracer is pretty 
fast to draw than dnTracer.
=======================================================================================
*/

/*
================
dnBarrelLaunchedBeamTracer::dnBarrelLaunchedBeamTracer
================ 
*/
dnBarrelLaunchedBeamTracer::dnBarrelLaunchedBeamTracer( idEntity *owner ) : dnTracerEffect( owner ) {

	SetType( TR_TYPE_BEAMTRACER );// We must set type in the beginning of every constructor.

	const char *skinName = owner->spawnArgs.GetString( "beam_skin" );

	if ( skinName[0] == '\0' ) {
		return; 	
	}

	length	= owner->spawnArgs.GetFloat( "beam_length", TRACER_LENGTH);

	renderEntity_t *renderEntity = owner->GetRenderEntity();
	
	weaponMuzzleOrigin = renderEntity->origin;
	
	idVec4 tracerColor = owner->spawnArgs.GetVec4( "beam_color", "1.0 1.0 1.0 1.0" );

	renderEntity->shaderParms[ SHADERPARM_RED ]		= tracerColor[0];
	renderEntity->shaderParms[ SHADERPARM_GREEN ]	= tracerColor[1];
	renderEntity->shaderParms[ SHADERPARM_BLUE ]	= tracerColor[2];
	renderEntity->shaderParms[ SHADERPARM_ALPHA ]	= tracerColor[3];

	renderEntity->shaderParms[ SHADERPARM_BEAM_WIDTH ] = owner->spawnArgs.GetFloat( "beam_width", TRACER_WIDTH ); 

	idVec3 endPos = renderEntity->origin - renderEntity->axis[2] * length;
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_X ] = endPos[0];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Y ] = endPos[1];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Z ] = endPos[2];

	owner->SetModel( "_beam" );
	owner->SetSkin( declManager->FindSkin( skinName ) );	
}

/*
================
dnBarrelLaunchedBeamTracer::Think
================ 
*/
void dnBarrelLaunchedBeamTracer::Think( void )  {
	
	renderEntity_t *renderEntity = owner->GetRenderEntity();
	
	idVec3 endPos = renderEntity->origin - renderEntity->axis[2] * length;
		
	if( (renderEntity->origin - endPos).LengthSqr() > (renderEntity->origin - weaponMuzzleOrigin).LengthSqr() ) {
		endPos = weaponMuzzleOrigin;
	}
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_X ] = endPos[0];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Y ] = endPos[1];
	renderEntity->shaderParms[ SHADERPARM_BEAM_END_Z ] = endPos[2];
}

/*
===============================================================================
dnSpeedTracer:

Same effect as that of dnTracer only the difference is, this effect travels with user 
defined speed (speed taken from projectile.def). It involves less calculations than 
dnTracer and dnBeamTracer.
===============================================================================
*/

/*
================
dnSpeedTracer::dnSpeedTracer
================ 
*/
dnSpeedTracer::dnSpeedTracer( idEntity *owner, const float speed, const float distance, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis ) : dnTracerEffect( owner ) {

	SetType( TR_TYPE_SPEEDTRACER ); // We must set type info in the beginning of every constructor.

	const char *modelName = owner->spawnArgs.GetString( "model_tracer" );

	if ( modelName[0] == '\0' ) {
		modelDefHandle = -1;	
		return; 	
	}

	const float offset = owner->spawnArgs.GetFloat( "tracer_offset","0.0f" );

	owner->SetModel( "" );
	
	this->distance			= distance - offset;
	this->lastPos			= muzzleOrigin + tracerAxis[0] * offset;
	this->flyTime			= gameLocal.time;
	this->speed				= speed/1000.0f;

	memset( &renderEntity, 0, sizeof ( renderEntity ) );

	// align z-axis of model with the direction 
	renderEntity.axis[0] = tracerAxis[2];
	renderEntity.axis[2] = tracerAxis[0];
	renderEntity.axis[1] = tracerAxis[1];

	renderEntity.origin = lastPos;

//	SetModel( modelName );

	const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, modelName ) );

	if( modelDef ) {
		renderEntity.hModel = modelDef->ModelHandle();
		renderEntity.shaderParms[ SHADERPARM_RED ]		= 1.0f;
		renderEntity.shaderParms[ SHADERPARM_GREEN ]	= 1.0f;
		renderEntity.shaderParms[ SHADERPARM_BLUE ]		= 1.0f;
		renderEntity.shaderParms[ SHADERPARM_ALPHA ]	= 1.0f;

		renderEntity.hModel = renderModelManager->FindModel( modelName );

		modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	}
	else {
		modelDefHandle = -1;
	}
}

/*
================
dnSpeedTracer::UpdatePosition
================ 
*/
bool dnSpeedTracer::UpdatePosition( void ) {
	const float tracerTravelDist = (gameLocal.time - flyTime )* speed ;

	if ( tracerTravelDist > distance ) { // if travelled all the distance, stop doing tracers.

		if ( owner->thinkFlags & TH_UPDATEPARTICLES ) {			
			owner->BecomeInactive( TH_UPDATEPARTICLES );
		}
		gameRenderWorld->FreeEntityDef( modelDefHandle );
		modelDefHandle = -1;
		return false;
	}
	renderEntity.origin = lastPos + renderEntity.axis[2] * tracerTravelDist;

	lastPos = renderEntity.origin;
	flyTime = gameLocal.time;
	distance -= tracerTravelDist;
	return true;
}

/*
================
dnSpeedTracer::Think
================ 
*/
void dnSpeedTracer::Think( void )  {
	if ( modelDefHandle < 0 ){
		return;
	}
	if( UpdatePosition() ){
		gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
	}
}

/*
===============================================================================
dnBeamSpeedTracer:

Same effect as that of dnTracer only the difference is, this effect travels with user 
defined speed (found in projectile.def). The only difference is, dnTracer uses a model or
particle effect to represent the tracer, where as, this one uses _beam model. This tracer is
pretty fast to draw than dnTracer. 
===============================================================================
*/

dnBeamSpeedTracer::dnBeamSpeedTracer( idEntity *owner, const float speed, const float distance, const idVec3 &muzzleOrigin, const idMat3 &tracerAxis ) 
: dnSpeedTracer( owner ) {

	SetType( TR_TYPE_SPEEDTRACER ); // We must set type in the beginning of every constructor.

	const char *skinName = owner->spawnArgs.GetString( "beam_skin" );

	if ( skinName[0] == '\0' ) {
		modelDefHandle = -1;	
		return; 	
	}

	const float offset = owner->spawnArgs.GetFloat( "tracer_offset","0.0f" );

	owner->SetModel( "" );
	
	this->distance			= distance - offset;
	this->lastPos			= muzzleOrigin + tracerAxis[0] * offset;
	this->flyTime			= gameLocal.time;
	this->speed				= speed/1000.0f;

	memset( &renderEntity, 0, sizeof ( renderEntity ) );

	// align z-axis of model with the direction 
	renderEntity.axis[0] = tracerAxis[2];
	renderEntity.axis[2] = tracerAxis[0];
	renderEntity.axis[1] = tracerAxis[1];

	length			= owner->spawnArgs.GetFloat( "beam_length", TRACER_LENGTH );
	this->muzzleOrigin	= lastPos;

	idVec4 tracerColor = owner->spawnArgs.GetVec4( "beam_color", "1.0 1.0 1.0 1.0" );

	renderEntity.origin = lastPos;

	renderEntity.shaderParms[ SHADERPARM_RED ]		= tracerColor[0];
	renderEntity.shaderParms[ SHADERPARM_GREEN ]	= tracerColor[1];
	renderEntity.shaderParms[ SHADERPARM_BLUE ]		= tracerColor[2];
	renderEntity.shaderParms[ SHADERPARM_ALPHA ]	= tracerColor[3];

	renderEntity.shaderParms[ SHADERPARM_BEAM_WIDTH ] = owner->spawnArgs.GetFloat( "beam_width", TRACER_WIDTH ); 

	idVec3 endPos = renderEntity.origin - renderEntity.axis[2] * length;
	renderEntity.shaderParms[ SHADERPARM_BEAM_END_X ] = endPos[0];
	renderEntity.shaderParms[ SHADERPARM_BEAM_END_Y ] = endPos[1];
	renderEntity.shaderParms[ SHADERPARM_BEAM_END_Z ] = endPos[2];

	renderEntity.hModel = renderModelManager->FindModel( "_beam" );
	renderEntity.customSkin = declManager->FindSkin( skinName );	

	modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );
}


/*
================
dnBeamSpeedTracer::Think
================ 
*/
void dnBeamSpeedTracer::Think( void )  {
	if ( modelDefHandle < 0 ){
		return;
	}
	if( UpdatePosition() ){
		idVec3 endPos = renderEntity.origin - renderEntity.axis[2] * length;
		
		if( (renderEntity.origin - endPos).LengthSqr() > (renderEntity.origin - muzzleOrigin).LengthSqr() ) {
			endPos = muzzleOrigin;
		}
		renderEntity.shaderParms[ SHADERPARM_BEAM_END_X ] = endPos[0];
		renderEntity.shaderParms[ SHADERPARM_BEAM_END_Y ] = endPos[1];
		renderEntity.shaderParms[ SHADERPARM_BEAM_END_Z ] = endPos[2];

		gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
	}
}

/*
===============================================================================
	dnRailBeam

===============================================================================
*/

dnRailBeam::dnRailBeam( idEntity *owner, const idVec3 &beamStart ) 
: dnTracerEffect( owner ) {

	SetType( TR_TYPE_RAILBEAM ); // We must set type in the beginning of every constructor.

	const char *skinName = owner->spawnArgs.GetString( "beam_skin" );

	if ( skinName[0] == '\0' ) {
		modelDefHandle = -1;	
		return; 	
	}

	offset			= owner->spawnArgs.GetFloat( "beam_offset","0.0f" );
	smokeOffset[0]	= owner->spawnArgs.GetFloat( "beam_smoke_offset","0.0f" );

	owner->SetModel( "" );
	
	this->time			= SEC2MS( owner->spawnArgs.GetFloat( "beam_time", "2.0f") );
	this->fadeOut		= owner->spawnArgs.GetFloat( "beam_fadeout", "0.0f") * time; // percentage of time the beam fades out

	idVec4 beamColor	= owner->spawnArgs.GetVec4( "beam_color",		"1.0 1.0 1.0 1.0" );
	this->fadeColor		= owner->spawnArgs.GetVec4( "beamfade_color",	"0.0 0.0 0.0 0.0" );

	if( fadeOut > 0 ) {
		this->fadeOutIntervals = (beamColor - fadeColor) / fadeOut; 
	}

	memset( &renderEntity, 0, sizeof ( renderEntity ) );

	renderEntity.origin = beamStart;

	renderEntity.shaderParms[ SHADERPARM_RED ]		= beamColor[0];
	renderEntity.shaderParms[ SHADERPARM_GREEN ]	= beamColor[1];
	renderEntity.shaderParms[ SHADERPARM_BLUE ]		= beamColor[2];
	renderEntity.shaderParms[ SHADERPARM_ALPHA ]	= beamColor[3];

	renderEntity.shaderParms[ SHADERPARM_BEAM_WIDTH ] = owner->spawnArgs.GetFloat( "beam_width", TRACER_WIDTH ); 

	if( owner->spawnArgs.GetFloat( "beam_end_width", TRACER_WIDTH, beamEndWidth ) && time > 0 ){
		deltaWidthPerMsec = (beamEndWidth - renderEntity.shaderParms[ SHADERPARM_BEAM_WIDTH ]) / time;
	}
	else {
		deltaWidthPerMsec = 0.0f;
	}

	renderEntity.hModel = renderModelManager->FindModel( "_beam" );
	renderEntity.customSkin = declManager->FindSkin( skinName );

	modelDefHandle = -1;

	smokeParticle = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, owner->spawnArgs.GetString("beam_smoke"), false ) );
	smokeLength	  = owner->spawnArgs.GetFloat( "beam_smoke_length", "10" );
}

/*
================
dnBeamSpeedTracer::Think
================ 
*/
void dnRailBeam::Create( const idVec3 &beamEnd ) {

	time += gameLocal.time;
	
	previousTime	= gameLocal.time; 
	fadeOut			= time - fadeOut; //use same fadeOut var as a fadeOut time Offset

	renderEntity.shaderParms[ SHADERPARM_BEAM_END_X ] = beamEnd[0];
	renderEntity.shaderParms[ SHADERPARM_BEAM_END_Y ] = beamEnd[1];
	renderEntity.shaderParms[ SHADERPARM_BEAM_END_Z ] = beamEnd[2];

	if( offset != 0.0f ) {
		idVec3 dir = beamEnd - renderEntity.origin;

		dir.Normalize();
		renderEntity.origin	+= offset * dir;
		smokeOffset			 = renderEntity.origin + smokeOffset[0] * dir;
	}

	modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );

	if( smokeParticle != NULL ) {
//		idVec3 length = beamEnd - renderEntity.origin;
		idVec3 length = beamEnd - smokeOffset;
		
		smokeLength = smokeLength > 0.0f ? smokeLength : 10.0f; // make sure smokeLength is non-zero

		nSmokeParticles = length.LengthFast()/smokeLength;		 
        
		length.Normalize();
		renderEntity.axis = length.ToMat3();				//For creating proper non random particle effect, we need to set the axis right
		
		//test 1:swap(x-z)
		idVec3 tmp = renderEntity.axis[0];
		renderEntity.axis[0] = renderEntity.axis[2];
		renderEntity.axis[2] = -tmp;
		//-----------------

		smokeStartTime = gameLocal.time;
	}
	else {
		nSmokeParticles = 0;
		renderEntity.axis = owner->GetPhysics()->GetAxis(); // we just need to set this to something.
	}
	owner->BecomeActive( TH_UPDATEPARTICLES );

}

/*
================
dnBeamSpeedTracer::Think
================ 
*/
void dnRailBeam::Think( void )  {

	if ( modelDefHandle < 0 ){
		return;
	}
	int deadSmokeCount = 0;

	if( nSmokeParticles > 0 ){ 
		for( int i=0; i < nSmokeParticles; i++ ) {
	//		if( !gameLocal.smokeParticles->EmitSmoke( smokeParticle, smokeStartTime, gameLocal.random.RandomFloat(), renderEntity.origin + smokeLength * (i+1) * renderEntity.axis[0], renderEntity.axis ) ) {
			if( !gameLocal.smokeParticles->EmitSmoke( smokeParticle, smokeStartTime, gameLocal.random.RandomFloat(), smokeOffset - smokeLength * (i+1) * renderEntity.axis[2], renderEntity.axis ) ) {
				deadSmokeCount++;
			}	
		}
	}

	if( gameLocal.time >= time ) {
		renderEntity.shaderParms[ SHADERPARM_RED ]		= fadeColor[0];
		renderEntity.shaderParms[ SHADERPARM_GREEN ]	= fadeColor[1];
		renderEntity.shaderParms[ SHADERPARM_BLUE ]		= fadeColor[2];
		renderEntity.shaderParms[ SHADERPARM_ALPHA ]	= fadeColor[3];

		if( deltaWidthPerMsec != 0.0f ) {
			renderEntity.shaderParms[ SHADERPARM_BEAM_WIDTH ] = beamEndWidth;
		}

		if( deadSmokeCount == nSmokeParticles ) {
			owner->BecomeInactive( TH_UPDATEPARTICLES );
		}
	}
	else {
		int delta = gameLocal.time - previousTime;

		if( deltaWidthPerMsec != 0.0f ) {
			renderEntity.shaderParms[ SHADERPARM_BEAM_WIDTH ] += deltaWidthPerMsec * delta;
		}

		if( fadeOut > 0 && gameLocal.time >= fadeOut ) {

			idVec4 intervals = fadeOutIntervals * delta;

			renderEntity.shaderParms[ SHADERPARM_RED ]		-= intervals[0];
			renderEntity.shaderParms[ SHADERPARM_GREEN ]	-= intervals[1];
			renderEntity.shaderParms[ SHADERPARM_BLUE ]		-= intervals[2];
			renderEntity.shaderParms[ SHADERPARM_ALPHA ]	-= intervals[3];
		}

		previousTime = gameLocal.time;
	}

	gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
}


#endif /* !_DENTONMOD */
