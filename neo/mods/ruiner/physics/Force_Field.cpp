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
#include "physics/Physics_Player.h"
#include "physics/Physics_Monster.h"
#include "WorldSpawn.h"

#include "physics/Force_Field.h"

CLASS_DECLARATION( idForce, idForce_Field )
END_CLASS

/*
================
idForce_Field::idForce_Field
================
*/
idForce_Field::idForce_Field( void ) {
	type			= FORCEFIELD_UNIFORM;
	applyType		= FORCEFIELD_APPLY_FORCE;
	//ivan start
	magnitudeType	= FORCEFIELD_MAGNITUDE_FIXED; 
	distance_radius	= 0.0f;
	distance_offset	= 0.0f;
	oldVelocityPct	= 0.0f;
	//ivan end
	magnitude		= 0.0f;
	dir.Set( 0, 0, 1 );
	randomTorque	= 0.0f;
	playerOnly		= false;
	monsterOnly		= false;
	clipModel		= NULL;
}

/*
================
idForce_Field::~idForce_Field
================
*/
idForce_Field::~idForce_Field( void ) {
	if ( this->clipModel ) {
		delete this->clipModel;
	}
}

/*
================
idForce_Field::Save
================
*/
void idForce_Field::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( type );
	savefile->WriteInt( applyType);
	//ivan start
	savefile->WriteInt( magnitudeType);
	savefile->WriteFloat( distance_radius );
	savefile->WriteFloat( distance_offset );
	savefile->WriteFloat( oldVelocityPct );
	//ivan end
	savefile->WriteFloat( magnitude );
	savefile->WriteVec3( dir );
	savefile->WriteFloat( randomTorque );
	savefile->WriteBool( playerOnly );
	savefile->WriteBool( monsterOnly );
	savefile->WriteClipModel( clipModel );
}

/*
================
idForce_Field::Restore
================
*/
void idForce_Field::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( (int &)type );
	savefile->ReadInt( (int &)applyType);
	//ivan start
	savefile->ReadInt( (int &)magnitudeType);
	savefile->ReadFloat( distance_radius );
	savefile->ReadFloat( distance_offset );
	savefile->ReadFloat( oldVelocityPct );
	//ivan end
	savefile->ReadFloat( magnitude );
	savefile->ReadVec3( dir );
	savefile->ReadFloat( randomTorque );
	savefile->ReadBool( playerOnly );
	savefile->ReadBool( monsterOnly );
	savefile->ReadClipModel( clipModel );
}

/*
================
idForce_Field::SetClipModel
================
*/
void idForce_Field::SetClipModel( idClipModel *clipModel ) {
	if ( this->clipModel && clipModel != this->clipModel ) {
		delete this->clipModel;
	}
	this->clipModel = clipModel;
}

/*
================
idForce_Field::Uniform
================
*/
void idForce_Field::Uniform( const idVec3 &force ) {
	dir = force;
	magnitude = dir.Normalize();
	type = FORCEFIELD_UNIFORM;
}

/*
================
idForce_Field::Explosion
================
*/
void idForce_Field::Explosion( float force ) {
	magnitude = force;
	type = FORCEFIELD_EXPLOSION;
}

/*
================
idForce_Field::Implosion
================
*/
void idForce_Field::Implosion( float force ) {
	magnitude = force;
	type = FORCEFIELD_IMPLOSION;
}

/*
================
idForce_Field::RandomTorque
================
*/
void idForce_Field::RandomTorque( float force ) {
	randomTorque = force;
}

//ivan start
/*
================
idForce_Field::SetDistanceBounds
================
*/
void idForce_Field::SetDistanceBounds( float offset, float radius ) {
	distance_offset = offset;
	distance_radius = radius;
}

/*
================
idForce_Field::GetDistancePct
================
*/
float idForce_Field::GetDistancePct( float distance, bool inverse ) { //inverse means 1 inside and 0 outside the hypersphere
	float resultPct;

	//distance = idMath::Sqrt( distance );  //use sqrt distance

	if( distance_radius <= 0.0f ){
		if( distance_offset <= 0.0f ){ 
			return 1.0f; //don't use pct
		}else{ //either 0 or 1 if smaller or greater than offset
			resultPct = ( (distance - distance_offset) > 0 ) ? 1.0f : 0.0f;
		}
	}else{
		resultPct = (distance - distance_offset)/distance_radius;
		if( resultPct < 0.0f ){ resultPct = 0.0f; } //less than the offset is zero
		if( resultPct > 1.0f ) { resultPct = 1.0f; } //greater than offset+radius
	}

	if( inverse ){
		resultPct = 1 - resultPct;
	}
	
	return resultPct;
}
//ivan end

/*
================
idForce_Field::Evaluate
================
*/
void idForce_Field::Evaluate( int time ) {
	int numClipModels, i;
	idBounds bounds;
	idVec3 force, torque, angularVelocity;
	idClipModel *cm, *clipModelList[ MAX_GENTITIES ];
	//ivan start
	idVec3 distanceVec; 
	float realMagnitude;
	//ivan end

	assert( clipModel );

	bounds.FromTransformedBounds( clipModel->GetBounds(), clipModel->GetOrigin(), clipModel->GetAxis() );
	numClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	torque.Zero();

	for ( i = 0; i < numClipModels; i++ ) {
		cm = clipModelList[ i ];

		if ( !cm->IsTraceModel() ) {
			continue;
		}

		idEntity *entity = cm->GetEntity();

		if ( !entity ) {
			continue;
		}

		idPhysics *physics = entity->GetPhysics();

		if ( playerOnly ) {
			if ( !physics->IsType( idPhysics_Player::Type ) ) {
				continue;
			}
		} else if ( monsterOnly ) {
			if ( !physics->IsType( idPhysics_Monster::Type ) ) {
				continue;
			}
		}

		if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
									clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) {
			continue;
		}

		//ivan start
		distanceVec = cm->GetOrigin() - clipModel->GetOrigin();
		//ivan end

		switch( type ) {
			case FORCEFIELD_UNIFORM: {
				force = dir;
				break;
			}
			case FORCEFIELD_EXPLOSION: {
				//force = cm->GetOrigin() - clipModel->GetOrigin();
				force = distanceVec;
				force.Normalize();
				break;
			}
			case FORCEFIELD_IMPLOSION: {
				//force = clipModel->GetOrigin() - cm->GetOrigin();
				force = -distanceVec;
				force.Normalize();
				break;
			}
			default: {
				gameLocal.Error( "idForce_Field: invalid type" );
				return;
			}
		}

		if ( randomTorque != 0.0f ) {
			torque[0] = gameLocal.random.CRandomFloat();
			torque[1] = gameLocal.random.CRandomFloat();
			torque[2] = gameLocal.random.CRandomFloat();
			if ( torque.Normalize() == 0.0f ) {
				torque[2] = 1.0f;
			}
		}

		//ivan start - magnitude
		switch( magnitudeType ) {
			case FORCEFIELD_MAGNITUDE_FIXED: {
				realMagnitude = magnitude; //do nothing, just use the default magnitude
				break;
			}
			case FORCEFIELD_MAGNITUDE_DISTANCE: {
				realMagnitude = magnitude * GetDistancePct( distanceVec.LengthFast(), false );
				break;
			}
			case FORCEFIELD_MAGNITUDE_DISTANCE_INV: {
				realMagnitude = magnitude * GetDistancePct( distanceVec.LengthFast(), true );
				break;
			}
			default: {
				gameLocal.Error( "idForce_Field: invalid magnitude type" );
				break;
			}
		}
		//ivan end		

		switch( applyType ) {
			case FORCEFIELD_APPLY_FORCE: {
				if ( randomTorque != 0.0f ) {
					entity->AddForce( gameLocal.world, cm->GetId(), cm->GetOrigin() + torque.Cross( dir ) * randomTorque, dir * realMagnitude );
				}
				else {
					entity->AddForce( gameLocal.world, cm->GetId(), cm->GetOrigin(), force * realMagnitude );
				}
				break;
			}
			case FORCEFIELD_APPLY_VELOCITY: {
				//ivan start
				idVec3 oldVelocity;
				if( oldVelocityPct > 0 ){
					oldVelocity = oldVelocityPct * physics->GetLinearVelocity();
				}else{
					oldVelocity.Zero();
				}

				physics->SetLinearVelocity( oldVelocity + force * realMagnitude, cm->GetId() );
				//was: physics->SetLinearVelocity( force * realMagnitude, cm->GetId() );
				//ivan end
				if ( randomTorque != 0.0f ) {
					angularVelocity = physics->GetAngularVelocity( cm->GetId() );
					physics->SetAngularVelocity( 0.5f * (angularVelocity + torque * randomTorque), cm->GetId() );
				}
				break;
			}
			case FORCEFIELD_APPLY_IMPULSE: {
				if ( randomTorque != 0.0f ) {
					entity->ApplyImpulse( gameLocal.world, cm->GetId(), cm->GetOrigin() + torque.Cross( dir ) * randomTorque, dir * realMagnitude );
				}
				else {
					entity->ApplyImpulse( gameLocal.world, cm->GetId(), cm->GetOrigin(), force * realMagnitude );
				}
				break;
			}
			default: {
				gameLocal.Error( "idForce_Field: invalid apply type" );
				return;
			}
		}
	}
}
