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
#include "renderer/ModelManager.h"

#include "Fx.h"

#include "Moveable.h"

/*
===============================================================================

  idMoveable

===============================================================================
*/

const idEventDef EV_BecomeNonSolid( "becomeNonSolid" );
const idEventDef EV_SetOwnerFromSpawnArgs( "<setOwnerFromSpawnArgs>" );
const idEventDef EV_IsAtRest( "isAtRest", NULL, 'd' );
const idEventDef EV_EnableDamage( "enableDamage", "f" );
//ivan start
#ifdef _WATER_PHYSICS
const idEventDef EV_ActivateIfInWater( "<activateIfInWater>" ); 
#endif
const idEventDef EV_ActivatePhysics( "<activatePhysics>" ); 
//ivan end


CLASS_DECLARATION( idEntity, idMoveable )
	EVENT( EV_Activate,					idMoveable::Event_Activate )
	EVENT( EV_BecomeNonSolid,			idMoveable::Event_BecomeNonSolid )
	EVENT( EV_SetOwnerFromSpawnArgs,	idMoveable::Event_SetOwnerFromSpawnArgs )
	EVENT( EV_IsAtRest,					idMoveable::Event_IsAtRest )
	EVENT( EV_EnableDamage,				idMoveable::Event_EnableDamage )
#ifdef _WATER_PHYSICS
	//ivan
	EVENT( EV_ActivateIfInWater,		idMoveable::Event_ActivateIfInWater )
	EVENT( EV_ActivatePhysics,			idMoveable::Event_ActivatePhysics ) 
#endif
END_CLASS


static const float BOUNCE_SOUND_MIN_VELOCITY	= 80.0f; // 80
static const float BOUNCE_SOUND_MAX_VELOCITY	= 200.0f; // 200

/*
================
idMoveable::idMoveable
================
*/
idMoveable::idMoveable( void ) {
	minDamageVelocity	= 100.0f;
	maxDamageVelocity	= 200.0f;
	nextCollideFxTime	= 0;
	nextDamageTime		= 0;
	nextSoundTime		= 0;
	initialSpline		= NULL;
	initialSplineDir	= vec3_zero;
	explode				= false;
	unbindOnDeath		= false;
	allowStep			= false;
	canDamage			= false;

/*#ifdef _DENTONMOD
	entDamageEffects	= NULL;
#endif*/

	//ivan start
	isXlocked			= false; 
	lockedXpos			= 0.0f;
	//ivan end
}

/*
================
idMoveable::~idMoveable
================
*/
idMoveable::~idMoveable( void ) {
	delete initialSpline;
	initialSpline = NULL;
}

/*
================
idMoveable::Spawn
================
*/
void idMoveable::Spawn( void ) {
	idTraceModel trm;
	float density, friction, bouncyness, mass;
	int clipShrink;
	idStr clipModelName;

	// check if a clip model is set
	spawnArgs.GetString( "clipmodel", "", clipModelName );
	if ( !clipModelName[0] ) {
		clipModelName = spawnArgs.GetString( "model" );		// use the visual model
	}

	if ( !collisionModelManager->TrmFromModel( clipModelName, trm ) ) {
		gameLocal.Error( "idMoveable '%s': cannot load collision model %s", name.c_str(), clipModelName.c_str() );
		return;
	}

	// if the model should be shrinked
	clipShrink = spawnArgs.GetInt( "clipshrink" );
	if ( clipShrink != 0 ) {
		trm.Shrink( clipShrink * CM_CLIP_EPSILON );
	}

	// get rigid body properties
	spawnArgs.GetFloat( "density", "0.5", density );
	density = idMath::ClampFloat( 0.001f, 1000.0f, density );
	spawnArgs.GetFloat( "friction", "0.05", friction );
	friction = idMath::ClampFloat( 0.0f, 1.0f, friction );
	spawnArgs.GetFloat( "bouncyness", "0.6", bouncyness );
	bouncyness = idMath::ClampFloat( 0.0f, 1.0f, bouncyness );
	explode = spawnArgs.GetBool( "explode" );
	unbindOnDeath = spawnArgs.GetBool( "unbindondeath" );

	fxCollide = spawnArgs.GetString( "fx_collide" );
	nextCollideFxTime = 0;

	fl.takedamage = true;
	damage = spawnArgs.GetString( "def_damage", "" );
	canDamage = spawnArgs.GetBool( "damageWhenActive" ) ? false : true;
	minDamageVelocity = spawnArgs.GetFloat( "minDamageVelocity", "100" );
	maxDamageVelocity = spawnArgs.GetFloat( "maxDamageVelocity", "200" );
	nextDamageTime = 0;
	nextSoundTime = 0;

	health = spawnArgs.GetInt( "health", "0" );
	spawnArgs.GetString( "broken", "", brokenModel );

	if ( health ) {
		if ( brokenModel != "" && !renderModelManager->CheckModel( brokenModel ) ) {
			gameLocal.Error( "idMoveable '%s' at (%s): cannot load broken model '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), brokenModel.c_str() );
		}
	}

	// setup the physics
	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( trm ), density );
	physicsObj.GetClipModel()->SetMaterial( GetRenderModelMaterial() );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetBouncyness( bouncyness );
	physicsObj.SetFriction( 0.6f, 0.6f, friction );
	physicsObj.SetGravity( gameLocal.GetGravity() );
	physicsObj.SetContents( CONTENTS_SOLID );
	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_MOVEABLECLIP );
	SetPhysics( &physicsObj );

	if ( spawnArgs.GetFloat( "mass", "10", mass ) ) {
		physicsObj.SetMass( mass );
	}
	
	if ( spawnArgs.GetBool( "nodrop" ) ) {
		physicsObj.PutToRest();
		//ivan start
		int actDelay = spawnArgs.GetInt( "nodrop_delayms", "0" );
		if ( actDelay > 0 ) {
			physicsObj.DelayedActivationOnImpact( actDelay );
		}
		//ivan end
	} else {
		physicsObj.DropToFloor();
	}

	if ( spawnArgs.GetBool( "noimpact" ) || spawnArgs.GetBool( "notPushable" ) ) {
		physicsObj.DisableImpact();
	}
#ifdef _WATER_PHYSICS
	else if ( !spawnArgs.GetBool( "startDisabledInWater", "0" ) ) { //start active in water by default, unless 'noimpact' or 'notPushable' or 'startDisabledInWater' are set
		PostEventMS( &EV_ActivateIfInWater, 0 ); //wait for all entities to spawn before checking collisions
	}
#endif

	if ( spawnArgs.GetBool( "nonsolid" ) ) {
		BecomeNonSolid();
	}

	allowStep = spawnArgs.GetBool( "allowStep", "1" );

	//ivan start
	lockedXpos = GetPhysics()->GetOrigin().x; //remember initial pos.
	isXlocked = spawnArgs.GetBool( "isXlocked", "0" );
#ifdef _WATER_PHYSICS
	physicsObj.SetWaterGravityMult( spawnArgs.GetFloat( "waterGravityMult", "1" )); //1 = no extra mult by default
#endif
	//ivan end

	PostEventMS( &EV_SetOwnerFromSpawnArgs, 0 );
}

/*
================
idMoveable::Save
================
*/
void idMoveable::Save( idSaveGame *savefile ) const {

	savefile->WriteString( brokenModel );
	savefile->WriteString( damage );
	savefile->WriteString( fxCollide );
	savefile->WriteInt( nextCollideFxTime );
	savefile->WriteFloat( minDamageVelocity );
	savefile->WriteFloat( maxDamageVelocity );
	savefile->WriteBool( explode );
	savefile->WriteBool( unbindOnDeath );
	savefile->WriteBool( allowStep );
	savefile->WriteBool( canDamage );
	savefile->WriteInt( nextDamageTime );
	savefile->WriteInt( nextSoundTime );
	savefile->WriteInt( initialSpline != NULL ? initialSpline->GetTime( 0 ) : -1 );
	savefile->WriteVec3( initialSplineDir );

	savefile->WriteStaticObject( physicsObj );

	//ivan start
	savefile->WriteBool( isXlocked ); 
	savefile->WriteFloat( lockedXpos ); 
	//ivan end
}

/*
================
idMoveable::Restore
================
*/
void idMoveable::Restore( idRestoreGame *savefile ) {
	int initialSplineTime;

	savefile->ReadString( brokenModel );
	savefile->ReadString( damage );
	savefile->ReadString( fxCollide );
	savefile->ReadInt( nextCollideFxTime );
	savefile->ReadFloat( minDamageVelocity );
	savefile->ReadFloat( maxDamageVelocity );
	savefile->ReadBool( explode );
	savefile->ReadBool( unbindOnDeath );
	savefile->ReadBool( allowStep );
	savefile->ReadBool( canDamage );
	savefile->ReadInt( nextDamageTime );
	savefile->ReadInt( nextSoundTime );
	savefile->ReadInt( initialSplineTime );
	savefile->ReadVec3( initialSplineDir );

	if ( initialSplineTime != -1 ) {
		InitInitialSpline( initialSplineTime );
	} else {
		initialSpline = NULL;
	}

	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );

	//ivan start
	savefile->ReadBool( isXlocked ); 
	savefile->ReadFloat( lockedXpos ); 
	//ivan end
}

/*
================
idMoveable::Hide
================
*/
void idMoveable::Hide( void ) {
	idEntity::Hide();
	physicsObj.SetContents( 0 );
}

/*
================
idMoveable::Show
================
*/
void idMoveable::Show( void ) {
	idEntity::Show();
	if ( !spawnArgs.GetBool( "nonsolid" ) ) {
		physicsObj.SetContents( CONTENTS_SOLID );
	}
}

/*
=================
idMoveable::Collide
=================
*/
bool idMoveable::Collide( const trace_t &collision, const idVec3 &velocity ) {
	float v, f;
	idVec3 dir;
	idEntity *ent;

	//gameLocal.Printf("collision\n");

	v = -( velocity * collision.c.normal );
	if ( v > BOUNCE_SOUND_MIN_VELOCITY && gameLocal.time > nextSoundTime ) {
		f = v > BOUNCE_SOUND_MAX_VELOCITY ? 1.0f : idMath::Sqrt( v - BOUNCE_SOUND_MIN_VELOCITY ) * ( 1.0f / idMath::Sqrt( BOUNCE_SOUND_MAX_VELOCITY - BOUNCE_SOUND_MIN_VELOCITY ) );
		if ( StartSound( "snd_bounce", SND_CHANNEL_BODY, 0, false, NULL ) ) {
			// don't set the volume unless there is a bounce sound as it overrides the entire channel
			// which causes footsteps on ai's to not honor their shader parms
			SetSoundVolume( f );
		}
		nextSoundTime = gameLocal.time + 500;
	}
	

	if ( canDamage && damage.Length() && gameLocal.time > nextDamageTime ) {
		ent = gameLocal.entities[ collision.c.entityNum ];
		if ( ent && v > minDamageVelocity ) {
			f = v > maxDamageVelocity ? 1.0f : idMath::Sqrt( v - minDamageVelocity ) * ( 1.0f / idMath::Sqrt( maxDamageVelocity - minDamageVelocity ) );
			dir = velocity;
			dir.NormalizeFast();
			ent->Damage( this, GetPhysics()->GetClipModel()->GetOwner(), dir, damage, f, INVALID_JOINT );
			nextDamageTime = gameLocal.time + 1000;
		}
	}

	if ( fxCollide.Length() && gameLocal.time > nextCollideFxTime ) {
		idEntityFx::StartFx( fxCollide, &collision.c.point, NULL, this, false );
		nextCollideFxTime = gameLocal.time + 3500;
	}

	return false;
}

/*
============
idMoveable::Killed
============
*/
void idMoveable::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	if ( unbindOnDeath ) {
		Unbind();
	}

	if ( brokenModel != "" ) {
		SetModel( brokenModel );
	}

	if ( explode ) {
		if ( brokenModel == "" ) {
			PostEventMS( &EV_Remove, 1000 );
		}
	}

	if ( renderEntity.gui[ 0 ] ) {
		renderEntity.gui[ 0 ] = NULL;
	}

	ActivateTargets( this );

	fl.takedamage = false;
}

/*
================
idMoveable::AllowStep
================
*/
bool idMoveable::AllowStep( void ) const {
	return allowStep;
}

/*
================
idMoveable::BecomeNonSolid
================
*/
void idMoveable::BecomeNonSolid( void ) {
	// set CONTENTS_RENDERMODEL so bullets still collide with the moveable
	physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_RENDERMODEL );
	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_CORPSE | CONTENTS_MOVEABLECLIP );
}

/*
================
idMoveable::EnableDamage
================
*/
void idMoveable::EnableDamage( bool enable, float duration ) {
	canDamage = enable;
	if ( duration ) {
		PostEventSec( &EV_EnableDamage, duration, ( !enable ) ? 0.0f : 1.0f );
	}
}

/*
================
idMoveable::InitInitialSpline
================
*/
void idMoveable::InitInitialSpline( int startTime ) {
	int initialSplineTime;

	initialSpline = GetSpline();
	initialSplineTime = spawnArgs.GetInt( "initialSplineTime", "300" );

	if ( initialSpline != NULL ) {
		initialSpline->MakeUniform( initialSplineTime );
		initialSpline->ShiftTime( startTime - initialSpline->GetTime( 0 ) );
		initialSplineDir = initialSpline->GetCurrentFirstDerivative( startTime );
		initialSplineDir *= physicsObj.GetAxis().Transpose();
		initialSplineDir.Normalize();
		BecomeActive( TH_THINK );
	}
}

/*
================
idMoveable::FollowInitialSplinePath
================
*/
bool idMoveable::FollowInitialSplinePath( void ) {
	if ( initialSpline != NULL ) {
		if ( gameLocal.time < initialSpline->GetTime( initialSpline->GetNumValues() - 1 ) ) {
			idVec3 splinePos = initialSpline->GetCurrentValue( gameLocal.time );
			idVec3 linearVelocity = ( splinePos - physicsObj.GetOrigin() ) * USERCMD_HZ;
			physicsObj.SetLinearVelocity( linearVelocity );

			idVec3 splineDir = initialSpline->GetCurrentFirstDerivative( gameLocal.time );
			idVec3 dir = initialSplineDir * physicsObj.GetAxis();
			idVec3 angularVelocity = dir.Cross( splineDir );
			angularVelocity.Normalize();
			angularVelocity *= idMath::ACos16( dir * splineDir / splineDir.Length() ) * USERCMD_HZ;
			physicsObj.SetAngularVelocity( angularVelocity );
			return true;
		} else {
			delete initialSpline;
			initialSpline = NULL;
		}
	}
	return false;
}

/*
================
idMoveable::Think
================
*/
void idMoveable::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		if ( !FollowInitialSplinePath() ) {
			BecomeInactive( TH_THINK );
		}
	}
	idEntity::Think();

	//ivan start - lock the X position 
	//if ( thinkFlags & TH_PHYSICS ) {			
		if( isXlocked ){
			physicsObj.SetOrigin( idVec3( lockedXpos, physicsObj.GetOrigin().y, physicsObj.GetOrigin().z ) );
		}
	//}
	//ivan end
}

/*
================
idMoveable::GetRenderModelMaterial
================
*/
const idMaterial *idMoveable::GetRenderModelMaterial( void ) const {
	if ( renderEntity.customShader ) {
		return renderEntity.customShader;
	}
	if ( renderEntity.hModel && renderEntity.hModel->NumSurfaces() ) {
		 return renderEntity.hModel->Surface( 0 )->shader;
	}
	return NULL;
}

/*
================
idMoveable::WriteToSnapshot
================
*/
void idMoveable::WriteToSnapshot( idBitMsgDelta &msg ) const {
	physicsObj.WriteToSnapshot( msg );
}

/*
================
idMoveable::ReadFromSnapshot
================
*/
void idMoveable::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	physicsObj.ReadFromSnapshot( msg );
	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}

/*
================
idMoveable::Event_BecomeNonSolid
================
*/
void idMoveable::Event_BecomeNonSolid( void ) {
	BecomeNonSolid();
}

/*
================
idMoveable::Event_Activate
================
*/
void idMoveable::Event_Activate( idEntity *activator ) {
	float delay;
	idVec3 init_velocity, init_avelocity;

	Show();

	if ( !spawnArgs.GetInt( "notPushable" ) ) {
		physicsObj.EnableImpact();
	}

	physicsObj.Activate();

	spawnArgs.GetVector( "init_velocity", "0 0 0", init_velocity );
	spawnArgs.GetVector( "init_avelocity", "0 0 0", init_avelocity );

	delay = spawnArgs.GetFloat( "init_velocityDelay", "0" );
	if ( delay == 0.0f ) {
		physicsObj.SetLinearVelocity( init_velocity );
	} else {
		PostEventSec( &EV_SetLinearVelocity, delay, init_velocity );
	}

	delay = spawnArgs.GetFloat( "init_avelocityDelay", "0" );
	if ( delay == 0.0f ) {
		physicsObj.SetAngularVelocity( init_avelocity );
	} else {
		PostEventSec( &EV_SetAngularVelocity, delay, init_avelocity );
	}

	InitInitialSpline( gameLocal.time );
}

/*
================
idMoveable::Event_SetOwnerFromSpawnArgs
================
*/
void idMoveable::Event_SetOwnerFromSpawnArgs( void ) {
	idStr owner;

	if ( spawnArgs.GetString( "owner", "", owner ) ) {
		ProcessEvent( &EV_SetOwner, gameLocal.FindEntity( owner ) );
	}
}

//ivan start
#ifdef _WATER_PHYSICS

/*
================
idMoveable::Event_ActivateIfInWater
================
*/
void idMoveable::Event_ActivateIfInWater( void ) {
	if( physicsObj.SetWaterLevelf() == 1.0f ) {
		//gameLocal.Printf("'%s' activated by water\n", GetName() );
		//ProcessEvent( &EV_Activate, this ); //this would kill exploding barrels and start splines!
		Show();
		if ( !spawnArgs.GetInt( "notPushable" ) ) {
			physicsObj.EnableImpact();
		}
		physicsObj.AbortDropToFloor(); //this stops the drop-to-floor thing, which could take a bit of time to complete at map start
		physicsObj.Activate();
	}
}
#endif

/*
================
idMoveable::Event_ActivatePhysics
================
*/
void idMoveable::Event_ActivatePhysics( void ) {
	gameLocal.Printf("'%s': Event_ActivatePhysics\n", GetName() );
	Show();
	if ( !spawnArgs.GetInt( "notPushable" ) ) {
		physicsObj.EnableImpact();
	}
	physicsObj.Activate();
}
//ivan end

/*
================
idMoveable::Event_IsAtRest
================
*/
void idMoveable::Event_IsAtRest( void ) {
	idThread::ReturnInt( physicsObj.IsAtRest() );
}

/*
================
idMoveable::Event_EnableDamage
================
*/
void idMoveable::Event_EnableDamage( float enable ) {
	canDamage = ( enable != 0.0f );
}


/*
===============================================================================

  idBarrel

===============================================================================
*/

CLASS_DECLARATION( idMoveable, idBarrel )
END_CLASS

/*
================
idBarrel::idBarrel
================
*/
idBarrel::idBarrel() {
	radius = 1.0f;
	barrelAxis = 0;
	lastOrigin.Zero();
	lastAxis.Identity();
	additionalRotation = 0.0f;
	additionalAxis.Identity();
	fl.networkSync = true;
/*#ifdef _DENTONMOD
	entDamageEffects	= NULL;
#endif*/
}

/*
================
idBarrel::Save
================
*/
void idBarrel::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( radius );
	savefile->WriteInt( barrelAxis );
	savefile->WriteVec3( lastOrigin );
	savefile->WriteMat3( lastAxis );
	savefile->WriteFloat( additionalRotation );
	savefile->WriteMat3( additionalAxis );
}

/*
================
idBarrel::Restore
================
*/
void idBarrel::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( radius );
	savefile->ReadInt( barrelAxis );
	savefile->ReadVec3( lastOrigin );
	savefile->ReadMat3( lastAxis );
	savefile->ReadFloat( additionalRotation );
	savefile->ReadMat3( additionalAxis );
}

/*
================
idBarrel::BarrelThink
================
*/
void idBarrel::BarrelThink( void ) {
	bool wasAtRest, onGround;
	float movedDistance, rotatedDistance, angle;
	idVec3 curOrigin, gravityNormal, dir;
	idMat3 curAxis, axis;

	wasAtRest = IsAtRest();

	// run physics
	RunPhysics();

	//ivan start - lock the X position 
	if( isXlocked ){
		physicsObj.SetOrigin( idVec3( lockedXpos, physicsObj.GetOrigin().y, physicsObj.GetOrigin().z ) );
	}
	//ivan end

	// only need to give the visual model an additional rotation if the physics were run
	if ( !wasAtRest ) {

		// current physics state
		onGround = GetPhysics()->HasGroundContacts();
		curOrigin = GetPhysics()->GetOrigin();
		curAxis = GetPhysics()->GetAxis();

		// if the barrel is on the ground
		if ( onGround ) {
			gravityNormal = GetPhysics()->GetGravityNormal();

			dir = curOrigin - lastOrigin;
			dir -= gravityNormal * dir * gravityNormal;
			movedDistance = dir.LengthSqr();

			// if the barrel moved and the barrel is not aligned with the gravity direction
			if ( movedDistance > 0.0f && idMath::Fabs( gravityNormal * curAxis[barrelAxis] ) < 0.7f ) {

				// barrel movement since last think frame orthogonal to the barrel axis
				movedDistance = idMath::Sqrt( movedDistance );
				dir *= 1.0f / movedDistance;
				movedDistance = ( 1.0f - idMath::Fabs( dir * curAxis[barrelAxis] ) ) * movedDistance;

				// get rotation about barrel axis since last think frame
				angle = lastAxis[(barrelAxis+1)%3] * curAxis[(barrelAxis+1)%3];
				angle = idMath::ACos( angle );
				// distance along cylinder hull
				rotatedDistance = angle * radius;

				// if the barrel moved further than it rotated about it's axis
				if ( movedDistance > rotatedDistance ) {

					// additional rotation of the visual model to make it look
					// like the barrel rolls instead of slides
					angle = 180.0f * (movedDistance - rotatedDistance) / (radius * idMath::PI);
					if ( gravityNormal.Cross( curAxis[barrelAxis] ) * dir < 0.0f ) {
						additionalRotation += angle;
					} else {
						additionalRotation -= angle;
					}
					dir = vec3_origin;
					dir[barrelAxis] = 1.0f;
					additionalAxis = idRotation( vec3_origin, dir, additionalRotation ).ToMat3();
				}
			}
		}

		// save state for next think
		lastOrigin = curOrigin;
		lastAxis = curAxis;
	}

	Present();
}

/*
================
idBarrel::Think
================
*/
void idBarrel::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		if ( !FollowInitialSplinePath() ) {
			BecomeInactive( TH_THINK );
		}
	}

	BarrelThink();

#ifdef _DENTONMOD
	if ( thinkFlags & TH_UPDATEWOUNDPARTICLES )
		UpdateParticles();
#endif
}

/*
================
idBarrel::GetPhysicsToVisualTransform
================
*/
bool idBarrel::GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis ) {
	origin = vec3_origin;
	axis = additionalAxis;
	return true;
}

/*
================
idBarrel::Spawn
================
*/
void idBarrel::Spawn( void ) {
	const idBounds &bounds = GetPhysics()->GetBounds();

	// radius of the barrel cylinder
	radius = ( bounds[1][0] - bounds[0][0] ) * 0.5f;

	// always a vertical barrel with cylinder axis parallel to the z-axis
	barrelAxis = 2;

	lastOrigin = GetPhysics()->GetOrigin();
	lastAxis = GetPhysics()->GetAxis();

	additionalRotation = 0.0f;
	additionalAxis.Identity();
}

/*
================
idBarrel::ClientPredictionThink
================
*/
void idBarrel::ClientPredictionThink( void ) {
	Think();
}


/*
===============================================================================

idExplodingBarrel

===============================================================================
*/
const idEventDef EV_Respawn( "<respawn>" );
const idEventDef EV_TriggerTargets( "<triggertargets>" );

CLASS_DECLARATION( idBarrel, idExplodingBarrel )
	EVENT( EV_Activate,					idExplodingBarrel::Event_Activate )
	EVENT( EV_Respawn,					idExplodingBarrel::Event_Respawn )
	EVENT( EV_Explode,					idExplodingBarrel::Event_Explode )
	EVENT( EV_TriggerTargets,			idExplodingBarrel::Event_TriggerTargets )
END_CLASS

/*
================
idExplodingBarrel::idExplodingBarrel
================
*/
idExplodingBarrel::idExplodingBarrel() {
	spawnOrigin.Zero();
	spawnAxis.Zero();
	state = NORMAL;
	particleModelDefHandle = -1;
	lightDefHandle = -1;
	memset( &particleRenderEntity, 0, sizeof( particleRenderEntity ) );
	memset( &light, 0, sizeof( light ) );
	particleTime = 0;
	lightTime = 0;
	time = 0.0f;
}

/*
================
idExplodingBarrel::~idExplodingBarrel
================
*/
idExplodingBarrel::~idExplodingBarrel() {
	if ( particleModelDefHandle >= 0 ){
		gameRenderWorld->FreeEntityDef( particleModelDefHandle );
	}
	if ( lightDefHandle >= 0 ) {
		gameRenderWorld->FreeLightDef( lightDefHandle );
	}
}

/*
================
idExplodingBarrel::Save
================
*/
void idExplodingBarrel::Save( idSaveGame *savefile ) const {
	savefile->WriteVec3( spawnOrigin );
	savefile->WriteMat3( spawnAxis );

	savefile->WriteInt( state );
	savefile->WriteInt( particleModelDefHandle );
	savefile->WriteInt( lightDefHandle );

	savefile->WriteRenderEntity( particleRenderEntity );
	savefile->WriteRenderLight( light );

	savefile->WriteInt( particleTime );
	savefile->WriteInt( lightTime );
	savefile->WriteFloat( time );
}

/*
================
idExplodingBarrel::Restore
================
*/
void idExplodingBarrel::Restore( idRestoreGame *savefile ) {
	savefile->ReadVec3( spawnOrigin );
	savefile->ReadMat3( spawnAxis );

	savefile->ReadInt( (int &)state );
	savefile->ReadInt( (int &)particleModelDefHandle );
	savefile->ReadInt( (int &)lightDefHandle );

	savefile->ReadRenderEntity( particleRenderEntity );
	savefile->ReadRenderLight( light );

	savefile->ReadInt( particleTime );
	savefile->ReadInt( lightTime );
	savefile->ReadFloat( time );
}

/*
================
idExplodingBarrel::Spawn
================
*/
void idExplodingBarrel::Spawn( void ) {
	health = spawnArgs.GetInt( "health", "5" );
	fl.takedamage = true;
	spawnOrigin = GetPhysics()->GetOrigin();
	spawnAxis = GetPhysics()->GetAxis();
	state = NORMAL;
	particleModelDefHandle = -1;
	lightDefHandle = -1;
	lightTime = 0;
	particleTime = 0;
	time = spawnArgs.GetFloat( "time" );
	memset( &particleRenderEntity, 0, sizeof( particleRenderEntity ) );
	memset( &light, 0, sizeof( light ) );
}

/*
================
idExplodingBarrel::Think
================
*/
void idExplodingBarrel::Think( void ) {
	idBarrel::BarrelThink();

	#ifdef _DENTONMOD
		if ( thinkFlags & TH_UPDATEWOUNDPARTICLES )
			UpdateParticles();
	#endif

	if ( lightDefHandle >= 0 ){
		if ( state == BURNING ) {
			// ramp the color up over 250 ms
			float pct = (gameLocal.time - lightTime) / 250.f;
			if ( pct > 1.0f ) {
				pct = 1.0f;
			}
			light.origin = physicsObj.GetAbsBounds().GetCenter();
			light.axis = mat3_identity;
			light.shaderParms[ SHADERPARM_RED ] = pct;
			light.shaderParms[ SHADERPARM_GREEN ] = pct;
			light.shaderParms[ SHADERPARM_BLUE ] = pct;
			light.shaderParms[ SHADERPARM_ALPHA ] = pct;
			gameRenderWorld->UpdateLightDef( lightDefHandle, &light );
		} else {
			if ( gameLocal.time - lightTime > 250 ) {
				gameRenderWorld->FreeLightDef( lightDefHandle );
				lightDefHandle = -1;
			}
			return;
		}
	}

	if ( !gameLocal.isClient && state != BURNING && state != EXPLODING ) {
		BecomeInactive( TH_THINK );
		return;
	}

	// This condition fixes the problem where particleRenderEntity is used for explosion effect 
	// and it still tries to track the physics origin even after physics is put to rest.
#ifdef _DENTONMOD 
	if ( particleModelDefHandle >= 0 && state == BURNING ){
#else
	if ( particleModelDefHandle >= 0 ){
#endif
		particleRenderEntity.origin = physicsObj.GetAbsBounds().GetCenter();
		particleRenderEntity.axis = mat3_identity;
		gameRenderWorld->UpdateEntityDef( particleModelDefHandle, &particleRenderEntity );

	}
}

/*
================
idExplodingBarrel::AddParticles
================
*/
void idExplodingBarrel::AddParticles( const char *name, bool burn ) {
	if ( name && *name ) {
		if ( particleModelDefHandle >= 0 ){
			gameRenderWorld->FreeEntityDef( particleModelDefHandle );
		}
		memset( &particleRenderEntity, 0, sizeof ( particleRenderEntity ) );
		const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, name ) );
		if ( modelDef ) {
			particleRenderEntity.origin = physicsObj.GetAbsBounds().GetCenter();
			particleRenderEntity.axis = mat3_identity;
			particleRenderEntity.hModel = modelDef->ModelHandle();
			float rgb = ( burn ) ? 0.0f : 1.0f;
			particleRenderEntity.shaderParms[ SHADERPARM_RED ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_GREEN ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_BLUE ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_ALPHA ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.realClientTime );
			particleRenderEntity.shaderParms[ SHADERPARM_DIVERSITY ] = ( burn ) ? 1.0f : gameLocal.random.RandomInt( 90 );
			if ( !particleRenderEntity.hModel ) {
				particleRenderEntity.hModel = renderModelManager->FindModel( name );
			}
			particleModelDefHandle = gameRenderWorld->AddEntityDef( &particleRenderEntity );
			if ( burn ) {
				BecomeActive( TH_THINK );
			}
			particleTime = gameLocal.realClientTime;
		}
	}
}

/*
================
idExplodingBarrel::AddLight
================
*/
void idExplodingBarrel::AddLight( const char *name, bool burn ) {
	if ( lightDefHandle >= 0 ){
		gameRenderWorld->FreeLightDef( lightDefHandle );
	}
	memset( &light, 0, sizeof ( light ) );
	light.axis = mat3_identity;
	light.lightRadius.x = spawnArgs.GetFloat( "light_radius" );
	light.lightRadius.y = light.lightRadius.z = light.lightRadius.x;
	light.origin = physicsObj.GetOrigin();
	light.origin.z += 128;
	light.pointLight = true;
	light.shader = declManager->FindMaterial( name );
	light.shaderParms[ SHADERPARM_RED ] = 2.0f;
	light.shaderParms[ SHADERPARM_GREEN ] = 2.0f;
	light.shaderParms[ SHADERPARM_BLUE ] = 2.0f;
	light.shaderParms[ SHADERPARM_ALPHA ] = 2.0f;
	lightDefHandle = gameRenderWorld->AddLightDef( &light );
	lightTime = gameLocal.realClientTime;
	BecomeActive( TH_THINK );
}

/*
================
idExplodingBarrel::ExplodingEffects
================
*/
void idExplodingBarrel::ExplodingEffects( void ) {
	const char *temp;

	StartSound( "snd_explode", SND_CHANNEL_ANY, 0, false, NULL );

	temp = spawnArgs.GetString( "model_damage" );
	if ( *temp != '\0' ) {
		SetModel( temp );
		Show();
	}

	temp = spawnArgs.GetString( "mtr_lightexplode" );
	if ( *temp != '\0' ) {
		AddLight( temp, false );
	}

	temp = spawnArgs.GetString( "mtr_burnmark" );
	if ( *temp != '\0' ) {
		gameLocal.ProjectDecal( GetPhysics()->GetOrigin(), GetPhysics()->GetGravity(), 128.0f, true, 96.0f, temp );
	}
	// put the explosion particle effect to the end -- By Clone JCD
	temp = spawnArgs.GetString( "model_detonate" );
	if ( *temp != '\0' ) {
		AddParticles( temp, false );
	}
}

//ivan start
/*
================
idExplodingBarrel::SpawnDrops
================
*/
void idExplodingBarrel::SpawnDrops( void ){
	idVec3 offset;
	idStr offsetKey;
	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_dropItem", NULL );
	while( kv ) {

		//get the offset
		offsetKey = kv->GetKey().c_str() + 4;
		offsetKey += "Offset";
		offset = spawnArgs.GetVector( offsetKey.c_str(), "0 0 0" );

		idMoveableItem::DropItem( kv->GetValue().c_str(), physicsObj.GetAbsBounds().GetCenter() + offset,physicsObj.GetAxis(), vec3_origin, 0, 0, false ); //don't drop to floor so offset is respected
		kv = spawnArgs.MatchPrefix( "def_dropItem", kv );
	}
}
//ivan end

/*
================
idExplodingBarrel::Killed
================
*/
void idExplodingBarrel::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {

	// This simple condition causes a barrel to explode when shot while burning
#ifdef _DENTONMOD
	if ( IsHidden() || state == EXPLODING ) {
#else
	if ( IsHidden() || state == EXPLODING || state == BURNING ) {
#endif
		return;
	}

	float f = spawnArgs.GetFloat( "burn" );

#ifdef _DENTONMOD
	int explodeHealth = spawnArgs.GetInt( "explode_health" );

	if ( f > 0.0f && state == NORMAL && health > explodeHealth ) {
#else
	if ( f > 0.0f && state == NORMAL ) {
#endif
		state = BURNING;
		PostEventSec( &EV_Explode, f );
		StartSound( "snd_burn", SND_CHANNEL_ANY, 0, false, NULL );
		AddParticles( spawnArgs.GetString ( "model_burn", "" ), true );
		return;
	} else {
		
#ifdef _DENTONMOD	
		if( state == BURNING && health > explodeHealth ) { 
			return;
		}
#endif

		state = EXPLODING;
		if ( gameLocal.isServer ) {
			idBitMsg	msg;
			byte		msgBuf[MAX_EVENT_PARAM_SIZE];

			msg.Init( msgBuf, sizeof( msgBuf ) );
			msg.WriteLong( gameLocal.time );
			ServerSendEvent( EVENT_EXPLODE, &msg, false, -1 );
		}
	}

	// do this before applying radius damage so the ent can trace to any damagable ents nearby
	Hide();
#ifdef _DENTONMOD
	BecomeInactive(TH_PHYSICS); // This causes the physics not to update after explosion
#else					
	physicsObj.SetContents( 0 ); // Set physics content 0 after spawining debris.
#endif
	
	const char *splash = spawnArgs.GetString( "def_splash_damage", "damage_explosion" );
	if ( splash && *splash ) {
		gameLocal.RadiusDamage( GetPhysics()->GetOrigin(), this, attacker, this, this, splash );
	}

	ExplodingEffects( );

	//FIXME: need to precache all the debris stuff here and in the projectiles
	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_debris" );
	// bool first = true;
	while ( kv ) {
		const idDict *debris_args = gameLocal.FindEntityDefDict( kv->GetValue(), false );
		if ( debris_args ) {
			idEntity *ent;
			idVec3 dir;
			idDebris *debris;
			//if ( first ) {
				dir = physicsObj.GetAxis()[1];
			//	first = false;
			//} else {
				dir.x += gameLocal.random.CRandomFloat() * 4.0f;
				dir.y += gameLocal.random.CRandomFloat() * 4.0f;
				//dir.z = gameLocal.random.RandomFloat() * 8.0f;
			//}
			dir.Normalize();

			gameLocal.SpawnEntityDef( *debris_args, &ent, false );
			if ( !ent || !ent->IsType( idDebris::Type ) ) {
				gameLocal.Error( "'projectile_debris' is not an idDebris" );
			}

			debris = static_cast<idDebris *>(ent);
			debris->Create( this, physicsObj.GetOrigin(), dir.ToMat3() );
			debris->Launch();
			debris->GetRenderEntity()->shaderParms[ SHADERPARM_TIME_OF_DEATH ] = ( gameLocal.time + 1500 ) * 0.001f;
			debris->UpdateVisuals();

		}
		kv = spawnArgs.MatchPrefix( "def_debris", kv );
	}
	
	SpawnDrops(); //ivan

#ifdef _DENTONMOD
	physicsObj.SetContents( 0 );
#endif

	physicsObj.PutToRest();

	CancelEvents( &EV_Explode );
	CancelEvents( &EV_Activate );

	f = spawnArgs.GetFloat( "respawn" );
	if ( f > 0.0f ) {
		PostEventSec( &EV_Respawn, f );
	} else {
		PostEventMS( &EV_Remove, 5000 );
	}

	if ( spawnArgs.GetBool( "triggerTargets" ) ) {
		ActivateTargets( this );
	}

	//ivan start - add score only if player is the killer
	if ( attacker && attacker->IsType( idPlayer::Type ) ) {
		static_cast< idPlayer* >( attacker )->AddScore( spawnArgs.GetInt( "score", "100" ) ); 
	}
	//ivan end
}

/*
================
idExplodingBarrel::Damage
================
*/
void idExplodingBarrel::Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir,
					  const char *damageDefName, const float damageScale, const int location ) {

	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefName );
	if ( !damageDef ) {
		gameLocal.Error( "Unknown damageDef '%s'\n", damageDefName );
	}
#ifdef _DENTONMOD		// Following condition means, if inflictor's got a radius damage then explode immediately, 
						// which could cause explosions when barrel's health is greater than 0 so I am disabling it.
#else
	if ( damageDef->FindKey( "radius" ) && GetPhysics()->GetContents() != 0 && GetBindMaster() == NULL ) {
		PostEventMS( &EV_Explode, 400 );
	} else {
#endif
		idEntity::Damage( inflictor, attacker, dir, damageDefName, damageScale, location );
#ifdef _DENTONMOD	
#else
	}
#endif
}

/*
================
idExplodingBarrel::Event_TriggerTargets
================
*/
void idExplodingBarrel::Event_TriggerTargets() {
	ActivateTargets( this );
}

/*
================
idExplodingBarrel::Event_Explode
================
*/
void idExplodingBarrel::Event_Explode() {
	if ( state == NORMAL || state == BURNING ) {
		state = BURNEXPIRED;
		Killed( NULL, NULL, 0, vec3_zero, 0 );
	}
}

/*
================
idExplodingBarrel::Event_Respawn
================
*/
void idExplodingBarrel::Event_Respawn() {
	int i;
	int minRespawnDist = spawnArgs.GetInt( "respawn_range", "256" );
	if ( minRespawnDist ) {
		float minDist = -1;
		for ( i = 0; i < gameLocal.numClients; i++ ) {
			if ( !gameLocal.entities[ i ] || !gameLocal.entities[ i ]->IsType( idPlayer::Type ) ) {
				continue;
			}
			idVec3 v = gameLocal.entities[ i ]->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
			float dist = v.Length();
			if ( minDist < 0 || dist < minDist ) {
				minDist = dist;
			}
		}
		if ( minDist < minRespawnDist ) {
			PostEventSec( &EV_Respawn, spawnArgs.GetInt( "respawn_again", "10" ) );
			return;
		}
	}
	const char *temp = spawnArgs.GetString( "model" );
	if ( temp && *temp ) {
		SetModel( temp );
	}
	health = spawnArgs.GetInt( "health", "5" );
	fl.takedamage = true;
	physicsObj.SetOrigin( spawnOrigin );
	physicsObj.SetAxis( spawnAxis );
	physicsObj.SetContents( CONTENTS_SOLID );
	physicsObj.DropToFloor();
	state = NORMAL;
	Show();
	UpdateVisuals();
}

/*
================
idMoveable::Event_Activate
================
*/
void idExplodingBarrel::Event_Activate( idEntity *activator ) {
	Killed( activator, activator, 0, vec3_origin, 0 );
}

/*
================
idMoveable::WriteToSnapshot
================
*/
void idExplodingBarrel::WriteToSnapshot( idBitMsgDelta &msg ) const {
	idMoveable::WriteToSnapshot( msg );
	msg.WriteBits( IsHidden(), 1 );
}

/*
================
idMoveable::ReadFromSnapshot
================
*/
void idExplodingBarrel::ReadFromSnapshot( const idBitMsgDelta &msg ) {

	idMoveable::ReadFromSnapshot( msg );
	if ( msg.ReadBits( 1 ) ) {
		Hide();
	} else {
		Show();
	}
}

/*
================
idExplodingBarrel::ClientReceiveEvent
================
*/
bool idExplodingBarrel::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {

	switch( event ) {
		case EVENT_EXPLODE:
			if ( gameLocal.realClientTime - msg.ReadLong() < spawnArgs.GetInt( "explode_lapse", "1000" ) ) {
				ExplodingEffects( );
			}
			return true;
		default:
			break;
	}

	return idBarrel::ClientReceiveEvent( event, time, msg );
}
