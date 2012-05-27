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

#ifndef __PHYSICS_PLAYER_H__
#define __PHYSICS_PLAYER_H__

#include "physics/Physics_Actor.h"
#include "physics/Physics_Monster.h"

/*
===================================================================================

	Player physics

	Simulates the motion of a player through the environment. Input from the
	player is used to allow a certain degree of control over the motion.

===================================================================================
*/

// movementType
typedef enum {
	PM_NORMAL,				// normal physics
	PM_DEAD,				// no acceleration or turning, but free falling
	PM_SPECTATOR,			// flying without gravity but with collision detection
	PM_FREEZE,				// stuck in place without control
	PM_NOCLIP,				// flying without collision detection nor gravity
	PM_ANIM_CROUCH					//ivan - animation and gravity 
} pmtype_t;

#ifdef _WATER_PHYSICS
//removed
#else
typedef enum {
	WATERLEVEL_NONE,
	WATERLEVEL_FEET,
	WATERLEVEL_WAIST,
	WATERLEVEL_HEAD
} waterLevel_t;
#endif


//ivan start
typedef enum {
	WALLJUMP_NONE,
	WALLJUMP_L1_HOLD,
	//WALLJUMP_L1_LEFT,
	WALLJUMP_R1_HOLD,
	//WALLJUMP_R1_LEFT,
	/*
	WALLJUMP_L2_HOLD,
	WALLJUMP_L2_LEFT,
	WALLJUMP_R2_HOLD,
	WALLJUMP_R2_LEFT,
	*/
	WALLJUMP_R_READY,
	WALLJUMP_L_READY
} wallJumpState_t;

//ivan end

#define	MAXTOUCH					32

typedef struct playerPState_s {
	idVec3					origin;
	idVec3					velocity;
	idVec3					localOrigin;
	idVec3					pushVelocity;
	float					stepUp;
	int						movementType;
	int						movementFlags;
	int						movementTime;
} playerPState_t;

class idPhysics_Player : public idPhysics_Actor {

public:
	CLASS_PROTOTYPE( idPhysics_Player );

							idPhysics_Player( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

							// initialisation
	void					SetSpeed( const float newWalkSpeed, const float newCrouchSpeed );
	void					SetMaxStepHeight( const float newMaxStepHeight );
	float					GetMaxStepHeight( void ) const;
	void					SetMaxJumpHeight( const float newMaxJumpHeight );
	void					SetMovementType( const pmtype_t type );
	void					SetPlayerInput( const usercmd_t &cmd, const idAngles &newViewAngles );
	void					SetKnockBack( const int knockBackTime );
	void					SetDebugLevel( bool set );
							// feed back from last physics frame
#ifdef _WATER_PHYSICS
	//removed
#else
	waterLevel_t			GetWaterLevel( void ) const;
	int						GetWaterType( void ) const;
#endif	
	
	bool					HasJumped( void ) const;
	bool					HasSteppedUp( void ) const;
	float					GetStepUp( void ) const;
	bool					IsCrouching( void ) const;
	bool					OnLadder( void ) const;
	const idVec3 &			PlayerGetOrigin( void ) const;	// != GetOrigin

        //ivan start
	void					SetFwInverted( bool inv ){ fw_inverted = inv; };
	int					GetHintForForceFields( void );
	//ivan end

public:	// common physics interface
	bool					Evaluate( int timeStepMSec, int endTimeMSec );
	void					UpdateTime( int endTimeMSec );
	int						GetTime( void ) const;

	void					GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const;
	void					ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse );
	bool					IsAtRest( void ) const;
	int						GetRestStartTime( void ) const;

	void					SaveState( void );
	void					RestoreState( void );

	void					SetOrigin( const idVec3 &newOrigin, int id = -1 );
	void					SetAxis( const idMat3 &newAxis, int id = -1 );

	void					Translate( const idVec3 &translation, int id = -1 );
	void					Rotate( const idRotation &rotation, int id = -1 );

	void					SetLinearVelocity( const idVec3 &newLinearVelocity, int id = 0 );

	const idVec3 &			GetLinearVelocity( int id = 0 ) const;

	void					SetPushed( int deltaTime );
	const idVec3 &			GetPushedLinearVelocity( const int id = 0 ) const;
	void					ClearPushedVelocity( void );

	void					SetMaster( idEntity *master, const bool orientated = true );

	void					WriteToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadFromSnapshot( const idBitMsgDelta &msg );

	//ivan start
	void					SetDoubleJumpEnabled( bool on ){ doubleJumpEnabled = on; };
	bool					IsDoubleJumpEnabled( void ){ return doubleJumpEnabled; }; 

	void					SetWallJumpEnabled( bool on ){ wallJumpEnabled = on; };
	bool					IsWallJumpEnabled( void ){ return wallJumpEnabled; };

	void					SetDelta( const idVec3 &d ); // ivan - set delta for next move
	//ivan end

private:
	// player physics state
	playerPState_t			current;
	playerPState_t			saved;

	// properties
	float					walkSpeed;
	float					crouchSpeed;
	float					maxStepHeight;
	float					maxJumpHeight;
	int						debugLevel;				// if set, diagnostic output will be printed

	// player input
	usercmd_t				command;
	idAngles				viewAngles;

	// run-time variables
	int						framemsec;
	float					frametime;
	float					playerSpeed;
	idVec3					viewForward;
	idVec3					viewRight;

	// walk movement
	bool					walking;
	bool					groundPlane;
	trace_t					groundTrace;
	const idMaterial *		groundMaterial;

	// ladder movement
	bool					ladder;
	idVec3					ladderNormal;

#ifdef _WATER_PHYSICS
	//removed
#else
	// results of last evaluate
	waterLevel_t			waterLevel;
	int						waterType;
#endif

	//ivan start
	idEntity *				blockingEntity;		//ivan 
	idVec3					delta;				//ivan - delta for next move

	bool					doubleJumpDone;
	bool					doubleJumpEnabled; 
	int						nextDoubleJump;

	bool					fw_inverted;

	bool					wallJumpDone;
	bool					wallJumpEnabled; 
	int						nextWallJump;
	int						wallJumpTimeout;
	wallJumpState_t			wallJumpKeyState;
	//ivan end

private:
	float					CmdScale( const usercmd_t &cmd ) const;
	void					Accelerate( const idVec3 &wishdir, const float wishspeed, const float accel );
	bool					SlideMove( bool gravity, bool stepUp, bool stepDown, bool push );
	void					Friction( void );
	void					WaterJumpMove( void );
	void					WaterMove( void );
	void					FlyMove( void );
	void					AirMove( void );
	void					WalkMove( void );
	void					DeadMove( void );
	void					NoclipMove( void );
	void					SpectatorMove( void );
	void					LadderMove( void );
	void					CorrectAllSolid( trace_t &trace, int contents );
	void					CheckGround( void );
	void					CheckDuck( void );
	void					CheckLadder( void );
	bool					CheckJump( void );
	bool					CheckWaterJump( void );
#ifdef _WATER_PHYSICS
	//removed
#else
	void					SetWaterLevel( void );
#endif

	void					DropTimers( void );
	void					MovePlayer( int msec );

	//ivan start
	bool					CheckDoubleJump( void );
	bool					CheckWallJump( void );
	bool					CanWallJump( bool left );

	void					AnimMove( idVec3 &start, idVec3 &velocity, const idVec3 &delta ); //ivan - taken from idPhysics_Monster::StepMove
	monsterMoveResult_t		AnimSlideMove( idVec3 &start, idVec3 &velocity, const idVec3 &delta ); //ivan - from idPhysics_Monster::SlideMove
	//ivan end
};

#endif /* !__PHYSICS_PLAYER_H__ */
