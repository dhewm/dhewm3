
#include "../../idlib/precompiled.h"
#pragma hdrstop

//#ifdef _D3XP

#include "../Game_local.h"
#include "Physics.h"
#include "Force_Grab.h"
#include "gamesys/SysCvar.h"

CLASS_DECLARATION( idForce, idForce_Grab )
END_CLASS

/*
================
idForce_Grab::Save
================
*/
void idForce_Grab::Save( idSaveGame *savefile ) const {
	
	savefile->WriteFloat( damping );
	savefile->WriteVec3( goalPosition );
	savefile->WriteFloat( distanceToGoal );
	savefile->WriteInt( id );
}

/*
================
idForce_Grab::Restore
================
*/
void idForce_Grab::Restore( idRestoreGame *savefile ) {

	//Note: Owner needs to call set physics
	savefile->ReadFloat( damping );
	savefile->ReadVec3( goalPosition );
	savefile->ReadFloat( distanceToGoal );
	savefile->ReadInt( id );
}

/*
================
idForce_Grab::idForce_Grab
================
*/
idForce_Grab::idForce_Grab( void ) {
	damping			= 0.5f;
	physics			= NULL;
	id				= 0;
}

/*
================
idForce_Grab::~idForce_Grab
================
*/
idForce_Grab::~idForce_Grab( void ) {
}

/*
================
idForce_Grab::Init
================
*/
void idForce_Grab::Init( float damping ) {
	if ( damping >= 0.0f && damping < 1.0f ) {
		this->damping = damping;
	}
}

/*
================
idForce_Grab::SetPhysics
================
*/
void idForce_Grab::SetPhysics( idPhysics *phys, int id, const idVec3 &goal ) {
	this->physics = phys;
	this->id = id;
	this->goalPosition = goal;
}

/*
================
idForce_Grab::SetGoalPosition
================
*/
void idForce_Grab::SetGoalPosition( const idVec3 &goal ) {
	this->goalPosition = goal;
}

/*
=================
idForce_Grab::GetDistanceToGoal
=================
*/
float idForce_Grab::GetDistanceToGoal( void ) {
	return distanceToGoal;
}

/*
================
idForce_Grab::Evaluate
================
*/
void idForce_Grab::Evaluate( int time ) {
	if ( !physics ) {
		return;
	}
	idVec3			forceDir, v, objectCenter;
	float			forceAmt;
	float			mass = physics->GetMass(id);

	objectCenter = physics->GetAbsBounds(id).GetCenter();

	if ( g_grabberRandomMotion.GetBool() && !gameLocal.isMultiplayer ) {
		// Jitter the objectCenter around so it doesn't remain stationary
		float SinOffset = idMath::Sin( (float)(gameLocal.time)/66.f );
		float randScale1 = gameLocal.random.RandomFloat();
		float randScale2 = gameLocal.random.CRandomFloat();
		objectCenter.x += ( SinOffset * 3.5f * randScale1 ) + ( randScale2 * 1.2f );
		objectCenter.y += ( SinOffset * -3.5f * randScale1 ) + ( randScale2 * 1.4f );
		objectCenter.z += ( SinOffset * 2.4f * randScale1 ) + ( randScale2 * 1.6f );
	}

	forceDir = goalPosition - objectCenter;
	distanceToGoal = forceDir.Normalize();

	float temp = distanceToGoal;
	if ( temp > 12.f && temp < 32.f ) {
		temp = 32.f;
	}
	forceAmt = (1000.f * mass) + (500.f * temp * mass);

	if ( forceAmt/mass > 120000.f ) {
		forceAmt = 120000.f * mass;
	}
	physics->AddForce( id, objectCenter, forceDir * forceAmt );

	if ( distanceToGoal < 196.f ) {
		v = physics->GetLinearVelocity( id );
		physics->SetLinearVelocity( v * damping, id );
	}
	if ( distanceToGoal < 16.f ) {
		v = physics->GetAngularVelocity(id);
		if ( v.LengthSqr() > Square(8) ) {
			physics->SetAngularVelocity( v * 0.99999f, id );
		}
	}
}

/*
================
idForce_Grab::RemovePhysics
================
*/
void idForce_Grab::RemovePhysics( const idPhysics *phys ) {
	if ( physics == phys ) {
		physics = NULL;
	}
}

//#endif	// _D3XP
