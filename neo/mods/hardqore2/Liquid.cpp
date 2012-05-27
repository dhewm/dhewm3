#include "../idlib/precompiled.h"
#pragma hdrstop

#ifdef _WATER_PHYSICS

#include "Game_local.h"
#include "Entity.h"
#include "physics/Physics.h"
#include "physics/Physics_AF.h"
#include "physics/Physics_Actor.h"
#include "physics/Physics_Liquid.h"
#include "SmokeParticles.h"
#include "Liquid.h"

// We do these splashes if the mass of the colliding object is less than these values.
// Anything large than MEDIUM_SPLASH does a large splash. (get it?)
const int SMALL_SPLASH		= 25;
const int MEDIUM_SPLASH		= 1000;

/*
===============================================================================

	idLiquid

===============================================================================
*/

CLASS_DECLARATION( idEntity, idLiquid )
	EVENT( EV_Touch,			idLiquid::Event_Touch )
END_CLASS

/*
================
idLiquid::Save
================
*/
void idLiquid::Save( idSaveGame *savefile ) const {

	int i;

   	savefile->WriteStaticObject( this->physicsObj );
	
	//savefile->WriteString(smokeName.c_str());
	//savefile->WriteString(soundName.c_str());
	//bool skipSound //ivan - don't waste space to save this

	for( i = 0; i < 3; i++ ){
		savefile->WriteParticle(this->splash[i]);
	}
	savefile->WriteParticle(this->waves);
}

/*
================
idLiquid::Restore
================
*/
void idLiquid::Restore( idRestoreGame *savefile ) {
	
	int i;

	savefile->ReadStaticObject( this->physicsObj );
	RestorePhysics( &this->physicsObj );

	//savefile->ReadString(this->smokeName);
	//savefile->ReadString(this->soundName);
	//bool skipSound //ivan - don't waste space to save this
	skipSound = false;

	for( i = 0; i < 3; i++ ){
		savefile->ReadParticle(this->splash[i]);
	}
	savefile->ReadParticle(this->waves);
}

/*
================
idLiquid::Spawn
================
*/
void idLiquid::Spawn() {
/*
	model = dynamic_cast<idRenderModelLiquid *>( renderEntity.hModel );
	if ( !model ) {
		gameLocal.Error( "Entity '%s' must have liquid model", name.c_str() );
	}
	model->Reset();
*/
	float liquidDensity;
	float liquidViscosity;
	//float liquidFriction;
	idVec3 minSplash;
	idVec3 minWave;
	idStr temp;
	const char *splashName;
	idStr tempName; //ivan

	// getters
	spawnArgs.GetFloat("density","0.01043",liquidDensity); //changes the gravity (>density --> > upward push)
	spawnArgs.GetFloat("viscosity","3.0",liquidViscosity); //changes the friction (>viscosity --> > friction)
	//spawnArgs.GetFloat("friction","3.0",liquidFriction);
	spawnArgs.GetString("liquid_name","water",temp);
	spawnArgs.GetVector("minSplashVelocity","100 100 100",minSplash);
	spawnArgs.GetVector("minWaveVelocity","60 60 60",minWave);

	// setters
	/*
	this->smokeName = "smoke_";
	this->smokeName.Append(temp);
	*/

	/*
	this->soundName = "snd_";
	this->soundName.Append(temp);
	*/

	
	//sndshader = declManager->FindSound( spawnArgs.GetString( "snd_water", "water/water_splash" ) ); //ivan

	//ivan start
	skipSound = false; //init

	sprintf( tempName, "%s_splash_tiny", temp.c_str() );
	splashName = spawnArgs.GetString("smoke_small", tempName.c_str() );
	this->splash[0] = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));

	sprintf( tempName, "%s_splash", temp.c_str() );
	splashName = spawnArgs.GetString("smoke_medium", tempName.c_str() );
	this->splash[1] = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));

	sprintf( tempName, "%s_splash_large", temp.c_str() );
	splashName = spawnArgs.GetString("smoke_large", tempName.c_str() );
	this->splash[2] = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));

	sprintf( tempName, "%s_waves", temp.c_str() );
	splashName = spawnArgs.GetString("smoke_waves", tempName.c_str() );
	this->waves = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));

	/*
	splashName = spawnArgs.GetString("smoke_small","water_splash_tiny");
	this->splash[0] = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));

	splashName = spawnArgs.GetString("smoke_medium","water_splash");
	this->splash[1] = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));

	splashName = spawnArgs.GetString("smoke_large","water_splash_large");
	this->splash[2] = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));

	splashName = spawnArgs.GetString("smoke_waves","water_waves");
	this->waves = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,splashName));
	*/
	//ivan end

	// setup physics
	this->physicsObj.SetSelf(this);
	this->physicsObj.SetClipModel( new idClipModel(this->GetPhysics()->GetClipModel()), liquidDensity );
	this->physicsObj.SetOrigin(this->GetPhysics()->GetOrigin());
	this->physicsObj.SetAxis(this->GetPhysics()->GetAxis());	
	this->physicsObj.SetGravity( gameLocal.GetGravity() );
	this->physicsObj.SetContents( CONTENTS_WATER | CONTENTS_TRIGGER );

	this->physicsObj.SetDensity(liquidDensity);
	this->physicsObj.SetViscosity(liquidViscosity);
	this->physicsObj.SetMinSplashVelocity(minSplash);
	this->physicsObj.SetMinWaveVelocity(minWave);

	this->SetPhysics( &this->physicsObj );

	BecomeActive( TH_THINK );
}

/*
================
idLiquid::Event_Touch

	This is mainly used for actors who touch the liquid, it spawns a splash
	near they're feet if they're moving fast enough.
================
*/
void idLiquid::Event_Touch( idEntity *other, trace_t *trace ) {
	//idPhysics_Liquid *liquid;
	idPhysics_Actor *phys;

	//only for actors!
	if( !other->GetPhysics()->IsType(idPhysics_Actor::Type) ){
		return;
	}

	//only if enough in the water
	phys = static_cast<idPhysics_Actor *>(other->GetPhysics());
	if( phys->GetWaterLevel() != WATERLEVEL_FEET ){
		return;
	}

	//ivan start - stop actors from constantly splashing when they're in the water
	
	//nextWaterSplash can be reset by the player if its walking in order to force this random behaviour
	if ( phys->nextWaterSplash == 0 ) { //someone reset this (or it's the first time) 
		if(	gameLocal.random.RandomFloat() > 0.5f ){ return; }
	}

	//default behaviour
	if ( phys->nextWaterSplash < gameLocal.time ) {
		phys->nextWaterSplash = gameLocal.time + 50 + gameLocal.random.RandomInt(50);
		if( phys->nextWaterSnd < gameLocal.time ){
			phys->nextWaterSnd = gameLocal.time + 300 + gameLocal.random.RandomInt(100);
		}else{
			skipSound = true;
		}
	}else{
		return;
	}
	//ivan end

	impactInfo_t info;
	other->GetImpactInfo(this,trace->c.id,trace->c.point,&info);
	//liquid = &this->physicsObj;

	trace->c.point = info.position + other->GetPhysics()->GetOrigin();
	trace->c.entityNum = other->entityNumber;

	this->Collide(*trace,info.velocity);
}

/*
================
idLiquid::Collide
	Spawns a splash particle and attaches a sound to the colliding entity.
================
*/
bool idLiquid::Collide( const trace_t &collision, const idVec3 &velocity ) {
	idEntity *e = gameLocal.entities[collision.c.entityNum];
	const idDeclParticle *splash;
	float eMass;
	idVec3 splashSpot;
	float velSquare = velocity.LengthSqr();
	//ivan start - why did he do this? We can directly use our physicsObj :|
	//was: idPhysics_Liquid *phys = static_cast<idPhysics_Liquid *>( this->GetPhysics() );
	//was: const char *sName; 
	int customType; 

	//avoid too many spashes for AFs!
	if( e->GetPhysics()->IsType(idPhysics_AF::Type) ){
		idPhysics_AF *phys = static_cast<idPhysics_AF *>(e->GetPhysics());

		if ( phys->nextWaterSplash < gameLocal.time ) {
			phys->nextWaterSplash = gameLocal.time + 30 + gameLocal.random.RandomInt(50);
		}else{
			return true;
		}
	}
	//ivan end

	eMass = e->GetPhysics()->GetMass();
	splashSpot = collision.c.point;
		
	if( velSquare > physicsObj.GetMinSplashVelocity().LengthSqr() ) { //ivan - was: phys
		//ivan start
		/*
		// pick which splash particle to spawn
		// first we check the entity, if it's not defined we use
		// one defined for this liquid.
		sName = e->spawnArgs.GetString(this->smokeName.c_str());
		if( *sName != '\0' ) {
			// load entity particle
			splash = static_cast<const idDeclParticle *>(declManager->FindType(DECL_PARTICLE,sName));
		}
		else {
		*/
		
		customType = e->spawnArgs.GetInt( "water_size" , "-1"); //default is invalid
		if( customType >= 0 && customType < 3 ){
			splash = this->splash[customType]; 
		}else{
		//ivan end
			// load a liquid particle based on the mass of the splashing entity
			if( eMass < SMALL_SPLASH )
				splash = this->splash[0];
			else if( eMass < MEDIUM_SPLASH )
				splash = this->splash[1];
			else
				splash = this->splash[2];
		}
	
		//ivan start
		// only play the sound for a splash
		//was: e->StartSound( this->soundName.c_str(), SND_CHANNEL_ANY, 0, false, NULL);
		if( skipSound ){ 
			skipSound = false;
		}else{
			e->StartSound( "snd_water", SND_CHANNEL_ANY, 0, false, NULL);
		}
		//ivan end
	}
	else if( velSquare > physicsObj.GetMinWaveVelocity().LengthSqr() ) { //ivan - was: phys
		splash = this->waves;
	}
	else {
		// the object is moving too slow so we abort
		return true;
	}

	// spawn the particle
	gameLocal.smokeParticles->EmitSmoke(splash,gameLocal.time,gameLocal.random.RandomFloat(),splashSpot,mat3_identity);
	return true;
}

#endif // _WATER_PHYSICS
