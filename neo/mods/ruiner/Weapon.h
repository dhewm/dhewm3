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

#ifndef __GAME_WEAPON_H__
#define __GAME_WEAPON_H__

#include "script/Script_Thread.h"
#include "Entity.h"
#include "Light.h"
#include "Actor.h"
#include "Grabber.h"


//ivan - uncomment this to create other test trails that are not removed and don't fade out
//#define TEST_TRAIL

//uncomment this to enable aim fix based on camera pos 
#define USE_AIM_DIR_FIX

/*
===============================================================================

	Player Weapon

===============================================================================
*/

typedef enum {
	WP_READY,
	WP_OUTOFAMMO,
	WP_RELOAD,
	WP_HOLSTERED,
	WP_RISING,
	WP_LOWERING
} weaponStatus_t;

//ivan start - values must match the ones in the defs!
enum {
	WP_SPREADMODE_DEFAULT		= 0,	//default: 3D spread
	WP_SPREADMODE_2D_STEP		= 1,	//turrican style spread
	WP_SPREADMODE_2D_PARALLEL	= 2,	//parallel projs. Spread is the offset
	WP_SPREADMODE_2D_RANDOM		= 3		//similar to default, but spread is 2D
};
//ivan end

typedef int ammo_t;
static const int AMMO_NUMTYPES = 16;
static const int TRAIL_NONE = -1; //ivan

class idPlayer;

static const int LIGHTID_WORLD_MUZZLE_FLASH = 1;
static const int LIGHTID_VIEW_MUZZLE_FLASH = 100;

class idMoveableItem;

//---------------------------------------- New Particle and light support

#ifdef _DENTONMOD

struct particleFlags_s {
	bool		isActive			: 1;		// Is the particle active	
	bool		isSmoke				: 1;		// Is this a smoke particle
	bool		isContinuous		: 1;		// Is the effect continuous
	bool		isOffset			: 1;		// Is new Offset needed
	bool		isDir				: 1;		// Is new Direction needed
	bool		isOnWorldModel		: 1;		// Is this effect intended for world model only
	bool		isUpdateJoint		: 1;
	bool		isViewDir			: 1;		//ivan - use player view dir even if it's using a joint. Thas can be still rotated by the usual "dir" key. 
};

typedef struct {
	char			name[64];
	char			particlename[128];
	int				startTime;
	idVec3			offset;			//Sometimes you cant find proper joint, then use offset along with muzzle bone
	idVec3			dir;
	jointHandle_t	joint;			//The joint on which to attach the particle

	particleFlags_s particleFlags;	// flags
    
	const idDeclParticle* particle;		//Used for smoke particles
	renderEntity_t renderEntity;
	qhandle_t modelDefHandle;
	//idFuncEmitter*  emitter;		//Used for non-smoke particles
} WeaponParticle_t;


struct lightFlags_s {
	bool		isActive		: 1;		// Is the particle active	
	bool		isAlwaysOn		: 1;		// Is this light always on
	bool		isOffset		: 1;		// Is new Offset needed
	bool		isDir			: 1;		// Is new Direction needed
	bool		isOnWorldModel	: 1;		// Is this light intended for world model only
};

typedef struct {
	char			name[64];
	int				startTime; 
	int				endTime;
	int				lightHandle;
	idVec3			offset;			//If weapons does not have bones in proper places for some effect use this
	idVec3			dir;			//If the desired bone is not pointing in proper direction use this to fix it.
									// Note that the dir should be vector representing X-axis of the bone.
	lightFlags_s	lightFlags;
	jointHandle_t	joint;
	renderLight_t	light;
} WeaponLight_t;
//----------------------------------------------
#endif

//
// events
//
//ivan start
extern const idEventDef EV_Weapon_StartAutoMelee; 
extern const idEventDef EV_Weapon_StopAutoMelee; 
extern const idEventDef EV_Weapon_StartMeleeBeam; 
extern const idEventDef EV_Weapon_StopMeleeBeam; 
extern const idEventDef EV_Weapon_StartParticle; 
extern const idEventDef EV_Weapon_StopParticle; 
//ivan end

//#define TRAIL_FX_CHAIN

class idWeapon : public idAnimatedEntity {
public:
	CLASS_PROTOTYPE( idWeapon );

							idWeapon();
	virtual					~idWeapon();

	// Init
	void					Spawn( void );
	void					SetOwner( idPlayer *owner );
	idPlayer*				GetOwner( void );
	virtual bool			ShouldConstructScriptObjectAtSpawn( void ) const;

	static void				CacheWeapon( const char *weaponName );

	// save games
	void					Save( idSaveGame *savefile ) const;					// archives object for save game file
	void					Restore( idRestoreGame *savefile );					// unarchives object from save game file

	// Weapon definition management
	void					Clear( void );
	void					GetWeaponDef( const char *objectname, int ammoinclip );
	bool					IsLinked( void );
	bool					IsWorldModelReady( void );

	// GUIs
	const char *			Icon( void ) const;
	void					UpdateGUI( void );

	virtual void			SetModel( const char *modelname );
	bool					GetGlobalJointTransform( const jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis ); //bool viewModel removed
	void					SetPushVelocity( const idVec3 &pushVelocity );
	bool					UpdateSkin( void );

	// State control/player interface
	void					Think( void );
	void					Raise( void );
	void					PutAway( void );
	void					Reload( void );
	void					LowerWeapon( void );
	void					RaiseWeapon( void );
	void					HideWeapon( void );
	void					ShowWeapon( void );
	void					HideWorldModel( void );
	void					ShowWorldModel( void );
	void					OwnerDied( void );
	void					BeginAttack( void );
	void					EndAttack( void );
	void					BeginSpecialFunction( bool );	// Added by Clone JCD 
	void					EndSpecialFunction( void );		// Added by Clone JCD 
	bool					IsReady( void ) const;
	bool					IsReloading( void ) const;
	bool					IsHolstered( void ) const;
	bool					ShowCrosshair( void ) const;
	idEntity *				DropItem( const idVec3 &velocity, int activateDelay, int removeDelay, bool died );
	bool					CanDrop( void ) const;
	void					WeaponStolen( void );

	// Script state management
	virtual idThread *		ConstructScriptObject( void );
	virtual void			DeconstructScriptObject( void );
	void					SetState( const char *statename, int blendFrames );
	void					UpdateScript( void );
	void					EnterCinematic( void );
	void					ExitCinematic( void );
	void					NetCatchup( void );

	// Visual presentation
	void					PresentWeapon( bool showViewModel );
	int						GetZoomFov( void );
	void					GetWeaponAngleOffsets( int *average, float *scale, float *max );
	void					GetWeaponTimeOffsets( float *time, float *scale );
	bool					BloodSplat( float size );

	// Ammo
	static ammo_t			GetAmmoNumForName( const char *ammoname );
	static const char		*GetAmmoNameForNum( ammo_t ammonum );
	static const char		*GetAmmoPickupNameForNum( ammo_t ammonum );
	ammo_t					GetAmmoType( void ) const;
	int						AmmoAvailable( void ) const;
	int						AmmoInClip( void ) const;
	int						GetMaxAmmo( void ) const;
	void					ResetAmmoClip( void );
	int						ClipSize( void ) const;
	int						LowAmmo( void ) const;
	int						AmmoRequired( void ) const;
//rev grab
	int						GetGrabberState( void ) const;
//rev grab
	int						AmmoCount( void ) const; //new

	//ivan start
	bool					HasToWalk( void ) const;
	float					GetWalkSpeedMult( void ) const;
	bool					GetIsFiring( void ) const;
	bool					GetIsSecFiring( void ) const;  
	void					StartWeaponParticle( const char* name );
	void					StopWeaponParticle( const char* name );
	void					StartAutoMelee( float dmgMult, int trailNum );
	void					StopAutoMelee( void );
	//void					StartMeleeBeam( int num );
	//void					StopMeleeBeam( void );

	float					GetDynamicSpread( void ) const;
	//ivan end


	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

	enum {
		EVENT_RELOAD = idEntity::EVENT_MAXEVENTS,
		EVENT_ENDRELOAD,
		EVENT_CHANGESKIN,
		EVENT_MAXEVENTS
	};
	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );

	virtual void			ClientPredictionThink( void );

private:
	// script control
	idScriptBool			WEAPON_ATTACK;	
	idScriptBool			WEAPON_RELOAD;
	idScriptBool			WEAPON_SPECIAL;  // For weapon special function, added by clone JCD
	idScriptBool			WEAPON_SPECIAL_HOLD;  // For weapon special function, added by clone JCD	idScriptBool			WEAPON_RELOAD;
	idScriptBool			WEAPON_NETRELOAD;
	idScriptBool			WEAPON_NETENDRELOAD;
	idScriptBool			WEAPON_NETFIRING;
	idScriptBool			WEAPON_RAISEWEAPON;
	idScriptBool			WEAPON_LOWERWEAPON;
	weaponStatus_t			status;
	idThread *				thread;
	idStr					state;
	idStr					idealState;
	int						animBlendFrames;
	int						animDoneTime;
	int						lastFiredTime; //ivan
	bool					isLinked;

	// precreated projectile
	idEntity				*projectileEnt;

	idPlayer *				owner;
	//idEntityPtr<idAnimatedEntity>	worldModel;

	// hiding (for GUIs and NPCs)
	int						hideTime;
	float					hideDistance;
	int						hideStartTime;
	float					hideStart;
	float					hideEnd;
	//float					hideOffset;
	bool					hide;
	bool					disabled;

	// berserk
	int						berserk;

	// these are the player render view parms, which include bobbing
	idVec3					playerViewOrigin;
	idMat3					playerViewAxis;

    /*
	// the view weapon render entity parms
	idVec3					viewWeaponOrigin;
	idMat3					viewWeaponAxis;
    */

	// the muzzle bone's position, used for launching projectiles and trailing smoke
	idVec3					muzzleOrigin;
	idMat3					muzzleAxis;

	//ivan start
	// the melee bone's position, used for launching projectiles and trailing smoke
	idVec3					meleeJointOrigin;
	idMat3					meleeJointAxis;
	idEntity				*lastMeleeEnt;  
	idBounds				meleebox;
	bool                    autoMeleeEnabled;
	bool					useMeleeBox;
	float					comboMultiplier;
	float					firingWalkSpeedMult;
	int						nextMeleeSnd; // used for autoMelee sound

	//spread
	float					dynamicSpreadValue; //current value
	float					spreadBaseValue;
	float					spreadVelocityFactor;
	float					spreadCrouchFactor;
	
#ifdef TRAIL_FX_CHAIN
	idEntityPtr<idBeam>		lastBeamInChain;
#else
	idTrailGenerator*		trailGen;
#ifdef TEST_TRAIL
	idTrailGenerator*		testGen;
#endif
#endif

	int						trailNumType;
	int						trailLowOffset;
	int						trailHighOffset;
	//ivan end

	idVec3					pushVelocity;

	// weapon definition
	// we maintain local copies of the projectile and brass dictionaries so they
	// do not have to be copied across the DLL boundary when entities are spawned
	const idDeclEntityDef *	weaponDef;
	const idDeclEntityDef *	meleeDef;
	idDict					projectileDict;
	float					meleeDistance;
	idStr					meleeDefName;
	idDict					brassDict;
	int						brassDelay;
	idStr					icon;

	/*
	// view weapon gui light
	renderLight_t			guiLight;
	int						guiLightHandle;
	*/

	/*
	// muzzle flash
	renderLight_t			muzzleFlash;		//ivan: now it's positioned on world weapon bone!
	int						muzzleFlashHandle;
	*/

	renderLight_t			worldMuzzleFlash;	// positioned on world weapon bone
	int						worldMuzzleFlashHandle;

	idVec3					flashColor;
	int						muzzleFlashEnd;
	int						flashTime;
	bool					lightOn;
	bool					silent_fire;
	bool					allowDrop;

	// effects
	bool					hasBloodSplat;

	/*
	// weapon kick
	int						kick_endtime;
	int						muzzle_kick_time;
	int						muzzle_kick_maxtime;
	idAngles				muzzle_kick_angles;
	idVec3					muzzle_kick_offset;
	*/

	// ammo management
	ammo_t					ammoType;
	int						ammoRequired;		// amount of ammo to use each shot.  0 means weapon doesn't need ammo.
	int						clipSize;			// 0 means no reload
	int						ammoClip;
	int						lowAmmo;			// if ammo in clip hits this threshold, snd_
	bool					powerAmmo;			// true if the clip reduction is a factor of the power setting when
												// a projectile is launched
	// mp client //ivan: also used for dynamic walkspeed
	bool					isFiring; 
	bool					isSecFiring; //ivan

	// zoom
	int						zoomFov;			// variable zoom fov per weapon

	// joints from models
    /*
	jointHandle_t			barrelJointView;
	jointHandle_t			flashJointView;
	jointHandle_t			ejectJointView;
	jointHandle_t			guiLightJointView;
	jointHandle_t			ventLightJointView;
    */

	jointHandle_t			flashJointWorld;
	jointHandle_t			barrelJointWorld;
	jointHandle_t			ejectJointWorld;
	jointHandle_t           meleeJointWorld; //ivan

	//ivan start
	int						maxAmmo;	
	int						spreadMode;
	//ivan end


#ifdef _DENTONMOD

/*	typedef struct {
		char name[64];
		jointHandle_t joint;
	}WeaponJoint_t;

	idHashTable<WeaponJoint_t>		weaponJoints;		
*/
	idHashTable<WeaponParticle_t>	weaponParticles;
	idHashTable<WeaponLight_t>		weaponLights;

#endif //_DENTONMOD

	// sound
	const idSoundShader *	sndHum;

	// new style muzzle smokes
	const idDeclParticle *	weaponSmoke;			// null if it doesn't smoke
	int						weaponSmokeStartTime;	// set to gameLocal.time every weapon fire
	bool					continuousSmoke;		// if smoke is continuous ( chainsaw )
	const idDeclParticle *  strikeSmoke;			// striking something in melee
	int						strikeSmokeStartTime;	// timing
	idVec3					strikePos;				// position of last melee strike
	idMat3					strikeAxis;				// axis of last melee strike
	int						nextStrikeFx;			// used for sound and decal ( may use for strike smoke too )
	
	//ivan note: all nozzleGlow stuff is currently not used because ventLightJointView no longer exists + was only for first person.
	//TODO: remove nozzleGlow stuff or enable it for 3th person.
	// nozzle effects
	bool					nozzleFx;			// does this use nozzle effects ( parm5 at rest, parm6 firing )
										// this also assumes a nozzle light atm
	int						nozzleFxFade;		// time it takes to fade between the effects
	int						lastAttack;			// last time an attack occured
	renderLight_t			nozzleGlow;			// nozzle light
	int						nozzleGlowHandle;	// handle for nozzle light

	idVec3					nozzleGlowColor;	// color of the nozzle glow
	const idMaterial *		nozzleGlowShader;	// shader for glow light
	float					nozzleGlowRadius;	// radius of glow light

	// weighting for viewmodel angles
	int						weaponAngleOffsetAverages;
	float					weaponAngleOffsetScale;
	float					weaponAngleOffsetMax;
	float					weaponOffsetTime;
	float					weaponOffsetScale;

	// flashlight
	void					AlertMonsters( void );

	// Visual presentation
	void					InitWorldModel( const idDeclEntityDef *def );
	void					MuzzleFlashLight( void );
	//void					MuzzleRise( idVec3 &origin, idMat3 &axis );
	void					UpdateNozzleFx( void );
#ifdef _DENTONMOD
	void					InitWeaponFx( void );		
	void					StopWeaponFx( void );		
	void					UpdateWeaponFx( void );
	
	bool					ChangeProjectileDef( int number );// New
#endif 

	void					UpdateFlashPosition( void );
	
	//ivan start
	bool					EvaluateMelee( void ); 
#ifdef TRAIL_FX_CHAIN
	idBeam *				SpawnMeleeBeam( const idVec3 &pos );
#else
	void					UpdateTrailVerts( void );
#endif

	void					UpdSpreadSettings( void );
	void					CalculateDynamicSpread( void );
	//ivan end

	// script events
	void					Event_Clear( void );
	void					Event_GetOwner( void );
	void					Event_WeaponState( const char *statename, int blendFrames );
	void					Event_WeaponReady( void );
	void					Event_WeaponOutOfAmmo( void );
	void					Event_WeaponReloading( void );
	void					Event_WeaponHolstered( void );
	void					Event_WeaponRising( void );
	void					Event_WeaponLowering( void );
	void					Event_UseAmmo( int amount );
	void					Event_AddToClip( int amount );
	void					Event_AmmoInClip( void );
	void					Event_AmmoAvailable( void );
	void					Event_TotalAmmoCount( void );
	void					Event_ClipSize( void );
	void					Event_PlayAnim( int channel, const char *animname );
	void					Event_PlayCycle( int channel, const char *animname );
	void					Event_AnimDone( int channel, int blendFrames );
	void					Event_SetBlendFrames( int channel, int blendFrames );
	void					Event_GetBlendFrames( int channel );
	void					Event_Next( void );
	void					Event_SetSkin( const char *skinname );
	void					Event_Flashlight( int enable );
	void					Event_GetLightParm( int parmnum );
	void					Event_SetLightParm( int parmnum, float value );
	void					Event_SetLightParms( float parm0, float parm1, float parm2, float parm3 );
	void					Event_LaunchProjectiles( int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower , int projtype, int useBarrelDir ); //ivan: projtype, useBarrelDir added
	void					Event_CreateProjectile( int projtype ); //ivan: projtype added
	void					Event_EjectBrass( void );
	void					Event_Melee( void );
	void					Event_GetWorldModel( void );
	void					Event_AllowDrop( int allow );
	void					Event_AutoReload( void );
	void					Event_NetReload( void );
	void					Event_IsInvisible( void );
	void					Event_NetEndReload( void );
//rev grab
	idGrabber				grabber;
	int						grabberState;

	void					Event_Grabber( int enable );
	void					Event_GrabberHasTarget( void );
	void					Event_GrabberSetGrabDistance( float dist );
//rev grab
	void					Event_SetZoom( int status );	//New

	void					Event_GetProjectileType( void );// New
	void					Event_ChangeProjectileDef( int number );// New
	void					Event_StartWeaponParticle( const char* name ); // New
	void					Event_StopWeaponParticle( const char* name );// New

	void					Event_StartWeaponLight( const char* name );// New
	void					Event_StopWeaponLight( const char* name );// New
	
	//Ivan start
	void					Event_StartAutoMelee( float dmgMult, int trailNum );
	void					Event_StopAutoMelee( void );
	//void					Event_StartMeleeBeam( int num );
	//void					Event_StopMeleeBeam( void );
	void					Event_SetWeaponMode( int value );
	void					Event_GetWeaponMode( void );
	//Ivan end
};

ID_INLINE bool idWeapon::IsLinked( void ) {
	return isLinked;
}

ID_INLINE bool idWeapon::IsWorldModelReady( void ) {
	return true; //ivan - was: ( worldModel.GetEntity() != NULL );
}

ID_INLINE idPlayer* idWeapon::GetOwner( void ) {
	return owner;
}

//ivan start
ID_INLINE bool idWeapon::GetIsFiring( void ) const {
	return isFiring;
}

ID_INLINE float idWeapon::GetDynamicSpread( void ) const {
	return dynamicSpreadValue;
}

ID_INLINE bool idWeapon::GetIsSecFiring( void ) const {
	return isSecFiring;
}
//ivan end

#endif /* !__GAME_WEAPON_H__ */
