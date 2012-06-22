// sikk---> Object Manipulation

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "Entity.h"
#include "../Game_local.h"
#include "Force.h"
#include "Force_Grab.h"

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
	if ( !physics )
		return;

	idVec3	forceDir, v, objectCenter;
	float	forceAmt;
	float	mass = physics->GetMass( id );

	objectCenter = physics->GetAbsBounds( id ).GetCenter();

	forceDir = goalPosition - objectCenter;
	distanceToGoal = forceDir.Normalize();

	float temp = distanceToGoal;
	if ( temp > 12.0f && temp < 32.0f ) {
		temp = 32.0f;
	}
	forceAmt = ( 1000.0f * mass ) + ( 500.0f * temp * mass );

	if ( forceAmt / mass > 120000.0f ) {
		forceAmt = 120000.0f * mass;
	}
	physics->AddForce( id, objectCenter, forceDir * forceAmt );

	if ( distanceToGoal < 128.0f ) {
		v = physics->GetLinearVelocity( id );
		if ( distanceToGoal <= 1.0f )
			physics->SetLinearVelocity( vec3_origin, id );
		else
			physics->SetLinearVelocity( v * damping, id );
	}

	physics->SetAngularVelocity( vec3_origin, id );
	//if ( distanceToGoal < 16.0f ) {
	//	v = physics->GetAngularVelocity( id );
	//	if ( v.LengthSqr() > Square( 8 ) ) {
	//		physics->SetAngularVelocity( v * 0.99999f, id );
	//	}
	//}
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

// <---sikk
