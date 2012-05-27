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
#include "framework/DeclEntityDef.h"
#include "framework/DeclSkin.h"
#include "renderer/ModelManager.h"

#include "gamesys/SysCvar.h"
#include "ai/AI.h"
#include "Player.h"
#include "Trigger.h"
#include "SmokeParticles.h"
#include "WorldSpawn.h"

#include "Weapon.h"

/***********************************************************************

idWeapon  

***********************************************************************/

//
// event defs
//
const idEventDef EV_Weapon_Clear( "<clear>" );
const idEventDef EV_Weapon_GetOwner( "getOwner", NULL, 'e' );
const idEventDef EV_Weapon_Next( "nextWeapon" );
const idEventDef EV_Weapon_State( "weaponState", "sd" );
const idEventDef EV_Weapon_UseAmmo( "useAmmo", "d" );
const idEventDef EV_Weapon_AddToClip( "addToClip", "d" );
const idEventDef EV_Weapon_AmmoInClip( "ammoInClip", NULL, 'f' );
const idEventDef EV_Weapon_AmmoAvailable( "ammoAvailable", NULL, 'f' );
const idEventDef EV_Weapon_TotalAmmoCount( "totalAmmoCount", NULL, 'f' );
const idEventDef EV_Weapon_ClipSize( "clipSize", NULL, 'f' );
const idEventDef EV_Weapon_WeaponOutOfAmmo( "weaponOutOfAmmo" );
const idEventDef EV_Weapon_WeaponReady( "weaponReady" );
const idEventDef EV_Weapon_WeaponReloading( "weaponReloading" );
const idEventDef EV_Weapon_WeaponHolstered( "weaponHolstered" );
const idEventDef EV_Weapon_WeaponRising( "weaponRising" );
const idEventDef EV_Weapon_WeaponLowering( "weaponLowering" );
const idEventDef EV_Weapon_Flashlight( "flashlight", "d" );
const idEventDef EV_Weapon_LaunchProjectiles( "launchProjectiles", "dffff" );
const idEventDef EV_Weapon_CreateProjectile( "createProjectile", NULL, 'e' );
const idEventDef EV_Weapon_EjectBrass( "ejectBrass" );
const idEventDef EV_Weapon_Melee( "melee", NULL, 'd' );
const idEventDef EV_Weapon_GetWorldModel( "getWorldModel", NULL, 'e' );
const idEventDef EV_Weapon_AllowDrop( "allowDrop", "d" );
const idEventDef EV_Weapon_AutoReload( "autoReload", NULL, 'f' );
const idEventDef EV_Weapon_NetReload( "netReload" );
const idEventDef EV_Weapon_IsInvisible( "isInvisible", NULL, 'f' );
const idEventDef EV_Weapon_NetEndReload( "netEndReload" );

// New------------------
#ifdef _DENTONMOD 

const idEventDef EV_Weapon_ChangeProjectileDef( "changeProjectileDef" , "d", 'f' ); // New
const idEventDef EV_Weapon_GetProjectileType( "getProjectileType", NULL, 'f' );

const idEventDef EV_Weapon_SetZoom( "setZoom" , "d"); // New
const idEventDef EV_Weapon_StartWeaponParticle( "startWeaponParticle", "s" );
const idEventDef EV_Weapon_StopWeaponParticle( "stopWeaponParticle", "s" );
const idEventDef EV_Weapon_StartWeaponLight( "startWeaponLight", "s" );
const idEventDef EV_Weapon_StopWeaponLight( "stopWeaponLight", "s" );

#endif //_DENTONMOD

//
// class def
//
CLASS_DECLARATION( idAnimatedEntity, idWeapon )

#ifdef _DENTONMOD
EVENT( EV_Weapon_SetZoom,					idWeapon::Event_SetZoom )		// New
#endif //_DENTONMOD
EVENT( EV_Weapon_Clear,						idWeapon::Event_Clear )
EVENT( EV_Weapon_GetOwner,					idWeapon::Event_GetOwner )
EVENT( EV_Weapon_State,						idWeapon::Event_WeaponState )
EVENT( EV_Weapon_WeaponReady,				idWeapon::Event_WeaponReady )
EVENT( EV_Weapon_WeaponOutOfAmmo,			idWeapon::Event_WeaponOutOfAmmo )
EVENT( EV_Weapon_WeaponReloading,			idWeapon::Event_WeaponReloading )
EVENT( EV_Weapon_WeaponHolstered,			idWeapon::Event_WeaponHolstered )
EVENT( EV_Weapon_WeaponRising,				idWeapon::Event_WeaponRising )
EVENT( EV_Weapon_WeaponLowering,			idWeapon::Event_WeaponLowering )
EVENT( EV_Weapon_UseAmmo,					idWeapon::Event_UseAmmo )
EVENT( EV_Weapon_AddToClip,					idWeapon::Event_AddToClip )
EVENT( EV_Weapon_AmmoInClip,				idWeapon::Event_AmmoInClip )
EVENT( EV_Weapon_AmmoAvailable,				idWeapon::Event_AmmoAvailable )
EVENT( EV_Weapon_TotalAmmoCount,			idWeapon::Event_TotalAmmoCount )
EVENT( EV_Weapon_ClipSize,					idWeapon::Event_ClipSize )
EVENT( AI_PlayAnim,							idWeapon::Event_PlayAnim )
EVENT( AI_PlayCycle,						idWeapon::Event_PlayCycle )
EVENT( AI_SetBlendFrames,					idWeapon::Event_SetBlendFrames )
EVENT( AI_GetBlendFrames,					idWeapon::Event_GetBlendFrames )
EVENT( AI_AnimDone,							idWeapon::Event_AnimDone )
EVENT( EV_Weapon_Next,						idWeapon::Event_Next )
EVENT( EV_SetSkin,							idWeapon::Event_SetSkin )
EVENT( EV_Weapon_Flashlight,				idWeapon::Event_Flashlight )
EVENT( EV_Light_GetLightParm,				idWeapon::Event_GetLightParm )
EVENT( EV_Light_SetLightParm,				idWeapon::Event_SetLightParm )
EVENT( EV_Light_SetLightParms,				idWeapon::Event_SetLightParms )
EVENT( EV_Weapon_LaunchProjectiles,			idWeapon::Event_LaunchProjectiles )
EVENT( EV_Weapon_CreateProjectile,			idWeapon::Event_CreateProjectile )
EVENT( EV_Weapon_EjectBrass,				idWeapon::Event_EjectBrass )
EVENT( EV_Weapon_Melee,						idWeapon::Event_Melee )
EVENT( EV_Weapon_GetWorldModel,				idWeapon::Event_GetWorldModel )
EVENT( EV_Weapon_AllowDrop,					idWeapon::Event_AllowDrop )
EVENT( EV_Weapon_AutoReload,				idWeapon::Event_AutoReload )
EVENT( EV_Weapon_NetReload,					idWeapon::Event_NetReload )
EVENT( EV_Weapon_IsInvisible,				idWeapon::Event_IsInvisible )
EVENT( EV_Weapon_NetEndReload,				idWeapon::Event_NetEndReload )

#ifdef _DENTONMOD

EVENT( EV_Weapon_ChangeProjectileDef,		idWeapon::Event_ChangeProjectileDef ) //  New
EVENT( EV_Weapon_GetProjectileType,			idWeapon::Event_GetProjectileType ) //  New

EVENT( EV_Weapon_StartWeaponParticle,		idWeapon::Event_StartWeaponParticle ) // All Four New
EVENT( EV_Weapon_StopWeaponParticle,		idWeapon::Event_StopWeaponParticle )
EVENT( EV_Weapon_StartWeaponLight,			idWeapon::Event_StartWeaponLight )
EVENT( EV_Weapon_StopWeaponLight,			idWeapon::Event_StopWeaponLight )

#endif // _DENTONMOD

END_CLASS

/***********************************************************************

init

***********************************************************************/

/*
================
idWeapon::idWeapon()
================
*/
idWeapon::idWeapon() {
	owner					= NULL;
	weaponDef				= NULL;
	thread					= NULL;

	memset( &worldMuzzleFlash, 0, sizeof( worldMuzzleFlash ) );
	memset( &nozzleGlow, 0, sizeof( nozzleGlow ) );

	muzzleFlashEnd			= 0;
	flashColor				= vec3_origin;
	worldMuzzleFlashHandle	= -1;
	nozzleGlowHandle		= -1;
	modelDefHandle			= -1;

	berserk					= 2;
	brassDelay				= 0;

	allowDrop				= true;

	Clear();

	fl.networkSync = true;
}

/*
================
idWeapon::~idWeapon()
================
*/
idWeapon::~idWeapon() {
	Clear();
}


/*
================
idWeapon::Spawn
================
*/
void idWeapon::Spawn( void ) {
	thread = new idThread();
	thread->ManualDelete();
	thread->ManualControl();
}

/*
================
idWeapon::SetOwner

Only called at player spawn time, not each weapon switch
================
*/
void idWeapon::SetOwner( idPlayer *_owner ) {
	assert( !owner );
	owner = _owner;
	SetName( va( "%s_weapon", owner->name.c_str() ) );
}

/*
================
idWeapon::ShouldConstructScriptObjectAtSpawn

Called during idEntity::Spawn to see if it should construct the script object or not.
Overridden by subclasses that need to spawn the script object themselves.
================
*/
bool idWeapon::ShouldConstructScriptObjectAtSpawn( void ) const {
	return false;
}

/*
================
idWeapon::CacheWeapon
================
*/
void idWeapon::CacheWeapon( const char *weaponName ) {
	const idDeclEntityDef *weaponDef;
	const char *brassDefName;
	const char *clipModelName;
	idTraceModel trm;
	const char *guiName;

	weaponDef = gameLocal.FindEntityDef( weaponName, false );
	if ( !weaponDef ) {
		return;
	}

	// precache the brass collision model
	brassDefName = weaponDef->dict.GetString( "def_ejectBrass" );
	if ( brassDefName[0] ) {
		const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( brassDefName, false );
		if ( brassDef ) {
			brassDef->dict.GetString( "clipmodel", "", &clipModelName );
			if ( !clipModelName[0] ) {
				clipModelName = brassDef->dict.GetString( "model" );		// use the visual model
			}
			// load the trace model
			collisionModelManager->TrmFromModel( clipModelName, trm );
		}
	}

	guiName = weaponDef->dict.GetString( "gui" );
	if ( guiName[0] ) {
		uiManager->FindGui( guiName, true, false, true );
	}
}

/*
================
idWeapon::Save
================
*/
void idWeapon::Save( idSaveGame *savefile ) const {

	savefile->WriteInt( status );
	savefile->WriteObject( thread );
	savefile->WriteString( state );
	savefile->WriteString( idealState );
	savefile->WriteInt( animBlendFrames );
	savefile->WriteInt( animDoneTime );
	savefile->WriteBool( isLinked );

	savefile->WriteObject( owner );

	savefile->WriteInt( hideTime );
	savefile->WriteFloat( hideDistance );
	savefile->WriteInt( hideStartTime );
	savefile->WriteFloat( hideStart );
	savefile->WriteFloat( hideEnd );
	savefile->WriteBool( hide );
	savefile->WriteBool( disabled );

	savefile->WriteInt( berserk );

	savefile->WriteVec3( playerViewOrigin );
	savefile->WriteMat3( playerViewAxis );

	savefile->WriteVec3( muzzleOrigin );
	savefile->WriteMat3( muzzleAxis );

	savefile->WriteVec3( pushVelocity );

	savefile->WriteString( weaponDef->GetName() );
	savefile->WriteFloat( meleeDistance );
	savefile->WriteString( meleeDefName );
	savefile->WriteInt( brassDelay );
	savefile->WriteString( icon );

	savefile->WriteInt( worldMuzzleFlashHandle );
	savefile->WriteRenderLight( worldMuzzleFlash );

	savefile->WriteVec3( flashColor );
	savefile->WriteInt( muzzleFlashEnd );
	savefile->WriteInt( flashTime );

	savefile->WriteBool( lightOn );
	savefile->WriteBool( silent_fire );

	savefile->WriteInt( ammoType );
	savefile->WriteInt( ammoRequired );
	savefile->WriteInt( clipSize );
	savefile->WriteInt( ammoClip );
	savefile->WriteInt( lowAmmo );
	savefile->WriteBool( powerAmmo );

	// savegames <= 17
	savefile->WriteInt( 0 );

	savefile->WriteInt( zoomFov );

	savefile->WriteJoint( flashJointWorld );
	savefile->WriteJoint( barrelJointWorld );
	savefile->WriteJoint( ejectJointWorld );

	savefile->WriteBool( hasBloodSplat );

	savefile->WriteSoundShader( sndHum );

	savefile->WriteParticle( weaponSmoke );
	savefile->WriteInt( weaponSmokeStartTime );
	savefile->WriteBool( continuousSmoke );
	savefile->WriteParticle( strikeSmoke );
	savefile->WriteInt( strikeSmokeStartTime );
	savefile->WriteVec3( strikePos );
	savefile->WriteMat3( strikeAxis );
	savefile->WriteInt( nextStrikeFx );

	savefile->WriteBool( nozzleFx );
	savefile->WriteInt( nozzleFxFade );

	savefile->WriteInt( lastAttack );

	savefile->WriteInt( nozzleGlowHandle );
	savefile->WriteRenderLight( nozzleGlow );

	savefile->WriteVec3( nozzleGlowColor );
	savefile->WriteMaterial( nozzleGlowShader );
	savefile->WriteFloat( nozzleGlowRadius );

	savefile->WriteInt( weaponAngleOffsetAverages );
	savefile->WriteFloat( weaponAngleOffsetScale );
	savefile->WriteFloat( weaponAngleOffsetMax );
	savefile->WriteFloat( weaponOffsetTime );
	savefile->WriteFloat( weaponOffsetScale );

	savefile->WriteBool( allowDrop );
	savefile->WriteObject( projectileEnt );

	// New----------------------------------------------
#ifdef _DENTONMOD

	savefile->WriteInt(weaponParticles.Num());

	for(int i = 0; i < weaponParticles.Num(); i++) {
		WeaponParticle_t* part = weaponParticles.GetIndex(i);

		particleFlags_s flags = part->particleFlags;

		LittleBitField( &flags, sizeof( flags ) );
		savefile->Write( &flags, sizeof( flags ) );

		savefile->WriteString( part->name );
		savefile->WriteString( part->particlename );
		savefile->WriteInt( part->startTime );
		savefile->WriteJoint( part->joint );

		if( !part->particleFlags.isSmoke ) {
			savefile->WriteRenderEntity( part->renderEntity );
		}
		if( part->particleFlags.isDir ) {
			savefile->WriteVec3(part->dir);
		}
		if( part->particleFlags.isOffset ) {
			savefile->WriteVec3(part->offset);
		}
	}
	savefile->WriteInt(weaponLights.Num());
	for(int i = 0; i < weaponLights.Num(); i++) {

		WeaponLight_t* light = weaponLights.GetIndex(i);

		lightFlags_s flags = light->lightFlags;

		LittleBitField( &flags, sizeof( flags ) );
		savefile->Write( &flags, sizeof( flags ) );

		savefile->WriteString( light->name );
		savefile->WriteInt( light->startTime );
		savefile->WriteJoint( light->joint );
		savefile->WriteInt( light->lightHandle );
		savefile->WriteRenderLight( light->light );

		if( !light->lightFlags.isAlwaysOn ) {
			savefile->WriteInt( light->endTime );
		}
		if( light->lightFlags.isDir ) {
			savefile->WriteVec3(light->dir);
		}
		if( light->lightFlags.isOffset ) {
			savefile->WriteVec3(light->offset);
		}
	}
#endif// _DENTONMOD
	
	//ivan start
	savefile->WriteInt( maxAmmo ); 
	savefile->WriteInt( fireMode ); 
	savefile->WriteFloat( mouseAimOffsetY );
	//ivan end
}

/*
================
idWeapon::Restore
================
*/
void idWeapon::Restore( idRestoreGame *savefile ) {

	savefile->ReadInt( (int &)status );
	savefile->ReadObject( reinterpret_cast<idClass *&>( thread ) );
	savefile->ReadString( state );
	savefile->ReadString( idealState );
	savefile->ReadInt( animBlendFrames );
	savefile->ReadInt( animDoneTime );
	savefile->ReadBool( isLinked );

	// Re-link script fields
	WEAPON_ATTACK.LinkTo(			scriptObject, "WEAPON_ATTACK" );
	WEAPON_SPECIAL.LinkTo(			scriptObject, "WEAPON_SPECIAL" );		// New
	WEAPON_SPECIAL_HOLD.LinkTo(		scriptObject, "WEAPON_SPECIAL_HOLD" );	//
	WEAPON_RELOAD.LinkTo(			scriptObject, "WEAPON_RELOAD" );
	WEAPON_NETRELOAD.LinkTo(		scriptObject, "WEAPON_NETRELOAD" );
	WEAPON_NETENDRELOAD.LinkTo(		scriptObject, "WEAPON_NETENDRELOAD" );
	WEAPON_NETFIRING.LinkTo(		scriptObject, "WEAPON_NETFIRING" );
	WEAPON_RAISEWEAPON.LinkTo(		scriptObject, "WEAPON_RAISEWEAPON" );
	WEAPON_LOWERWEAPON.LinkTo(		scriptObject, "WEAPON_LOWERWEAPON" );

	savefile->ReadObject( reinterpret_cast<idClass *&>( owner ) );

	savefile->ReadInt( hideTime );
	savefile->ReadFloat( hideDistance );
	savefile->ReadInt( hideStartTime );
	savefile->ReadFloat( hideStart );
	savefile->ReadFloat( hideEnd );
	savefile->ReadBool( hide );
	savefile->ReadBool( disabled );

	savefile->ReadInt( berserk );

	savefile->ReadVec3( playerViewOrigin );
	savefile->ReadMat3( playerViewAxis );

	savefile->ReadVec3( muzzleOrigin );
	savefile->ReadMat3( muzzleAxis );

	savefile->ReadVec3( pushVelocity );

	idStr objectname;
	savefile->ReadString( objectname );
	weaponDef = gameLocal.FindEntityDef( objectname );
	meleeDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_melee" ), false );

/*	const idDeclEntityDef *projectileDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_projectile" ), false );
	if ( projectileDef ) {
		projectileDict = projectileDef->dict;
	} else {
		projectileDict.Clear();
	}*/

	const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_ejectBrass" ), false );
	if ( brassDef ) {
		brassDict = brassDef->dict;
	} else {
		brassDict.Clear();
	}

	savefile->ReadFloat( meleeDistance );
	savefile->ReadString( meleeDefName );
	savefile->ReadInt( brassDelay );
	savefile->ReadString( icon );

	savefile->ReadInt( worldMuzzleFlashHandle );
	savefile->ReadRenderLight( worldMuzzleFlash );

	savefile->ReadVec3( flashColor );
	savefile->ReadInt( muzzleFlashEnd );
	savefile->ReadInt( flashTime );

	savefile->ReadBool( lightOn );
	savefile->ReadBool( silent_fire );

	savefile->ReadInt( (int &)ammoType );
	savefile->ReadInt( ammoRequired );
	savefile->ReadInt( clipSize );
	savefile->ReadInt( ammoClip );
	savefile->ReadInt( lowAmmo );
	savefile->ReadBool( powerAmmo );

	// savegame versions <= 17
	int foo;
	savefile->ReadInt( foo );

	savefile->ReadInt( zoomFov );

	savefile->ReadJoint( flashJointWorld );
	savefile->ReadJoint( barrelJointWorld );
	savefile->ReadJoint( ejectJointWorld );

	savefile->ReadBool( hasBloodSplat );

	savefile->ReadSoundShader( sndHum );

	savefile->ReadParticle( weaponSmoke );
	savefile->ReadInt( weaponSmokeStartTime );
	savefile->ReadBool( continuousSmoke );
	savefile->ReadParticle( strikeSmoke );
	savefile->ReadInt( strikeSmokeStartTime );
	savefile->ReadVec3( strikePos );
	savefile->ReadMat3( strikeAxis );
	savefile->ReadInt( nextStrikeFx );

	savefile->ReadBool( nozzleFx );
	savefile->ReadInt( nozzleFxFade );

	savefile->ReadInt( lastAttack );

	savefile->ReadInt( nozzleGlowHandle );
	savefile->ReadRenderLight( nozzleGlow );

	savefile->ReadVec3( nozzleGlowColor );
	savefile->ReadMaterial( nozzleGlowShader );
	savefile->ReadFloat( nozzleGlowRadius );

	savefile->ReadInt( weaponAngleOffsetAverages );
	savefile->ReadFloat( weaponAngleOffsetScale );
	savefile->ReadFloat( weaponAngleOffsetMax );
	savefile->ReadFloat( weaponOffsetTime );
	savefile->ReadFloat( weaponOffsetScale );

	savefile->ReadBool( allowDrop );
	savefile->ReadObject( reinterpret_cast<idClass *&>( projectileEnt ) );

#ifdef _DENTONMOD

	if( !ChangeProjectileDef( owner->GetProjectileType() ) ) { // if restore fails we must clear the dict
		projectileDict.Clear();
	}

	//All rest New
	int particleCount;
	savefile->ReadInt( particleCount );
	for(int i = 0; i < particleCount; i++) {
		WeaponParticle_t newParticle;
		memset(&newParticle, 0, sizeof(newParticle));

		savefile->Read( &newParticle.particleFlags, sizeof( newParticle.particleFlags ) );
		LittleBitField( &newParticle.particleFlags, sizeof( newParticle.particleFlags ) );

		idStr prtName, particlename;
		savefile->ReadString( prtName );
		savefile->ReadString( particlename );

		strcpy( newParticle.name, prtName.c_str() );
		strcpy( newParticle.particlename, particlename.c_str() );

		savefile->ReadInt( newParticle.startTime );
		savefile->ReadJoint( newParticle.joint );

		if(newParticle.particleFlags.isSmoke) {
			newParticle.particle = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, particlename, false ) );
		} 
		else {
			savefile->ReadRenderEntity( newParticle.renderEntity );
			newParticle.modelDefHandle = -1;
		}
		if( newParticle.particleFlags.isDir ) {
			savefile->ReadVec3( newParticle.dir );
		}
		if( newParticle.particleFlags.isOffset ) {
			savefile->ReadVec3( newParticle.offset );
		}

		weaponParticles.Set(newParticle.name, newParticle);
	}

	int lightCount;
	savefile->ReadInt( lightCount );
	for(int i = 0; i < lightCount; i++) {
		WeaponLight_t newLight;
		memset(&newLight, 0, sizeof(newLight));

		savefile->Read( &newLight.lightFlags, sizeof( newLight.lightFlags ) );
		LittleBitField( &newLight.lightFlags, sizeof( newLight.lightFlags ) );

		idStr lightName;
		savefile->ReadString( lightName );
		strcpy( newLight.name, lightName.c_str() );

		savefile->ReadInt( newLight.startTime );
		savefile->ReadJoint( newLight.joint );
		savefile->ReadInt( newLight.lightHandle );
		savefile->ReadRenderLight( newLight.light );
		if ( newLight.lightHandle >= 0 ) {
			newLight.lightHandle = gameRenderWorld->AddLightDef( &newLight.light );
		}

		if( !newLight.lightFlags.isAlwaysOn ) {
			savefile->ReadInt( newLight.endTime );
		}
		if( newLight.lightFlags.isDir ) {
			savefile->ReadVec3(newLight.dir);
		}
		if( newLight.lightFlags.isOffset ) {
			savefile->ReadVec3(newLight.offset);
		}
		weaponLights.Set(newLight.name, newLight);
	}
#endif //_DENTONMOD

	//ivan start
	savefile->ReadInt( maxAmmo ); 
	savefile->ReadInt( fireMode ); 
	savefile->ReadFloat( mouseAimOffsetY );
	//ivan end
}

/***********************************************************************

Weapon definition management

***********************************************************************/

/*
================
idWeapon::Clear
================
*/
void idWeapon::Clear( void ) {
	CancelEvents( &EV_Weapon_Clear );

	DeconstructScriptObject();
	scriptObject.Free();

	WEAPON_ATTACK.Unlink();
	WEAPON_SPECIAL.Unlink();			//new
	WEAPON_SPECIAL_HOLD.Unlink();		//new
	WEAPON_RELOAD.Unlink();
	WEAPON_NETRELOAD.Unlink();
	WEAPON_NETENDRELOAD.Unlink();
	WEAPON_NETFIRING.Unlink();
	WEAPON_RAISEWEAPON.Unlink();
	WEAPON_LOWERWEAPON.Unlink();

	if ( worldMuzzleFlashHandle != -1 ) {
		gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
		worldMuzzleFlashHandle = -1;
	}

	if ( nozzleGlowHandle != -1 ) {
		gameRenderWorld->FreeLightDef( nozzleGlowHandle );
		nozzleGlowHandle = -1;
	}

	memset( &renderEntity, 0, sizeof( renderEntity ) );
	renderEntity.entityNum	= entityNumber;

	renderEntity.noShadow		= true;
	renderEntity.noSelfShadow	= true;
	renderEntity.customSkin		= NULL;

	// set default shader parms
	renderEntity.shaderParms[ SHADERPARM_RED ]	= 1.0f;
	renderEntity.shaderParms[ SHADERPARM_GREEN ]= 1.0f;
	renderEntity.shaderParms[ SHADERPARM_BLUE ]	= 1.0f;
	renderEntity.shaderParms[3] = 1.0f;
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = 0.0f;
	renderEntity.shaderParms[5] = 0.0f;
	renderEntity.shaderParms[6] = 0.0f;
	renderEntity.shaderParms[7] = 0.0f;

	if ( refSound.referenceSound ) {
		refSound.referenceSound->Free( true );
	}
	memset( &refSound, 0, sizeof( refSound_t ) );

	// setting diversity to 0 results in no random sound.  -1 indicates random.
	refSound.diversity = -1.0f;

	if ( owner ) {
		// don't spatialize the weapon sounds
		refSound.listenerId = owner->GetListenerId();
	}

	// clear out the sounds from our spawnargs since we'll copy them from the weapon def
	const idKeyValue *kv = spawnArgs.MatchPrefix( "snd_" );
	while( kv ) {
		spawnArgs.Delete( kv->GetKey() );
		kv = spawnArgs.MatchPrefix( "snd_" );
	}

	hideTime		= 300;
	hideDistance	= -15.0f;
	hideStartTime	= gameLocal.time - hideTime;
	hideStart		= 0.0f;
	hideEnd			= 0.0f;
	//hideOffset		= 0.0f;
	hide			= false;
	disabled		= false;

	weaponSmoke		= NULL;
	weaponSmokeStartTime = 0;
	continuousSmoke = false;
	strikeSmoke		= NULL;
	strikeSmokeStartTime = 0;
	strikePos.Zero();
	strikeAxis = mat3_identity;
	nextStrikeFx = 0;

	icon			= "";

	playerViewAxis.Identity();
	playerViewOrigin.Zero();
	muzzleAxis.Identity();
	muzzleOrigin.Zero();
	pushVelocity.Zero();

	status			= WP_HOLSTERED;
	state			= "";
	idealState		= "";
	animBlendFrames	= 0;
	animDoneTime	= 0;

	projectileDict.Clear();
	meleeDef		= NULL;
	meleeDefName	= "";
	meleeDistance	= 0.0f;
	brassDict.Clear();

	flashTime		= 250;
	lightOn			= false;
	silent_fire		= false;

	ammoType		= 0;
	ammoRequired	= 0;
	ammoClip		= 0;
	clipSize		= 0;
	lowAmmo			= 0;
	powerAmmo		= false;

	zoomFov = 90;

	barrelJointWorld	= INVALID_JOINT;
	flashJointWorld		= INVALID_JOINT;
	ejectJointWorld		= INVALID_JOINT;

	// ------------------- New Start
#ifdef _DENTONMOD

	//Clean up the weapon particles
	for(int i = 0; i < weaponParticles.Num(); i++) {
		WeaponParticle_t* part = weaponParticles.GetIndex(i);
		if(!part->particleFlags.isSmoke) {
			if ( part->modelDefHandle >= 0 )
				gameRenderWorld->FreeEntityDef( part->modelDefHandle );
		}
	}
	weaponParticles.Clear();

	//Clean up the weapon lights
	for(int i = 0; i < weaponLights.Num(); i++) {
		WeaponLight_t* light = weaponLights.GetIndex(i);
		if ( light->lightHandle != -1 ) {
			gameRenderWorld->FreeLightDef( light->lightHandle );
		}
	}
	weaponLights.Clear();
	//------------------------------------ New End
#endif //_DENTONMOD

	hasBloodSplat		= false;
	nozzleFx			= false;
	nozzleFxFade		= 1500;
	lastAttack			= 0;
	//	lastSpecialFunc		= 0; // new 
	nozzleGlowHandle	= -1;
	nozzleGlowShader	= NULL;
	nozzleGlowRadius	= 10;
	nozzleGlowColor.Zero();

	weaponAngleOffsetAverages	= 0;
	weaponAngleOffsetScale		= 0.0f;
	weaponAngleOffsetMax		= 0.0f;
	weaponOffsetTime			= 0.0f;
	weaponOffsetScale			= 0.0f;

	allowDrop			= true;

	animator.ClearAllAnims( gameLocal.time, 0 );
	FreeModelDef();

	sndHum				= NULL;

	isLinked			= false;
	projectileEnt		= NULL;

	isFiring			= false;

	//ivan start
	mouseAimOffsetY		= 0.0f;
	//ivan end
}

/*
================
idWeapon::InitWorldModel
================
*/
void idWeapon::InitWorldModel( const idDeclEntityDef *def ) {
	assert( def );

	const char *model = def->dict.GetString( "model_world" );
	const char *attach = def->dict.GetString( "joint_attach" );

	//ivan start
	if ( model[0] && attach[0] ) {
		SetModel( model );

		GetPhysics()->SetContents( 0 );
		GetPhysics()->SetClipModel( NULL, 1.0f );
		BindToJoint( owner, attach, true );
		GetPhysics()->SetOrigin( vec3_origin );
		GetPhysics()->SetAxis( mat3_identity );

		// supress model in player views, but allow it in mirrors and remote views
		renderEntity.suppressSurfaceInViewID = owner->entityNumber+1;
		renderEntity.suppressShadowInViewID = owner->entityNumber+1;
		renderEntity.suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + owner->entityNumber;
	} else {
		SetModel( "" );
	}

	// find some joints in the model for locating effects
	barrelJointWorld = animator.GetJointHandle( def->dict.GetString( "world_barrel_joint", "barrel" ) );
	flashJointWorld = animator.GetJointHandle( "flash" );
	ejectJointWorld = animator.GetJointHandle( "eject" );
	//ivan end
}

/*
================
idWeapon::GetWeaponDef
================
*/
void idWeapon::GetWeaponDef( const char *objectname, int ammoinclip ) {
	const char *shader;
	const char *objectType;
	const char *guiName;
	const char *brassDefName;
	const char *smokeName;
	int			ammoAvail;

	Clear();

	if ( !objectname || !objectname[ 0 ] ) {
		return;
	}

	assert( owner );

	weaponDef			= gameLocal.FindEntityDef( objectname );

	ammoType			= GetAmmoNumForName( weaponDef->dict.GetString( "ammoType" ) );
	ammoRequired		= weaponDef->dict.GetInt( "ammoRequired" );
	clipSize			= weaponDef->dict.GetInt( "clipSize" );
	lowAmmo				= weaponDef->dict.GetInt( "lowAmmo" );
	
	//ivan start
	maxAmmo				= owner->inventory.MaxAmmoForAmmoClass( owner, weaponDef->dict.GetString( "ammoType" ) );
	fireMode			= weaponDef->dict.GetInt( "fireMode", "0" );
	//ivan end

	icon				= weaponDef->dict.GetString( "icon" );
	silent_fire			= weaponDef->dict.GetBool( "silent_fire" );
	powerAmmo			= weaponDef->dict.GetBool( "powerAmmo" );

	hideTime			= SEC2MS( weaponDef->dict.GetFloat( "hide_time", "0.3" ) );
	hideDistance		= weaponDef->dict.GetFloat( "hide_distance", "-15" );

	// muzzle smoke
	smokeName = weaponDef->dict.GetString( "smoke_muzzle" );
	if ( *smokeName != '\0' ) {
		weaponSmoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	} else {
		weaponSmoke = NULL;
	}
	continuousSmoke = weaponDef->dict.GetBool( "continuousSmoke" );
	weaponSmokeStartTime = ( continuousSmoke ) ? gameLocal.time : 0;

	smokeName = weaponDef->dict.GetString( "smoke_strike" );
	if ( *smokeName != '\0' ) {
		strikeSmoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	} else {
		strikeSmoke = NULL;
	}
	strikeSmokeStartTime = 0;
	strikePos.Zero();
	strikeAxis = mat3_identity;
	nextStrikeFx = 0;

	// setup the world model
	InitWorldModel( weaponDef );

	// copy the sounds from the weapon model def into our spawnargs
	const idKeyValue *kv = weaponDef->dict.MatchPrefix( "snd_" );
	while( kv ) {
		spawnArgs.Set( kv->GetKey(), kv->GetValue() );
		kv = weaponDef->dict.MatchPrefix( "snd_", kv );
	}

#ifdef _DENTONMOD
#else
	// get the projectile
	projectileDict.Clear();

	const char *projectileName = weaponDef->dict.GetString( "def_projectile" );
	if ( projectileName[0] != '\0' ) {
		const idDeclEntityDef *projectileDef = gameLocal.FindEntityDef( projectileName, false );
		if ( !projectileDef ) {
			gameLocal.Warning( "Unknown projectile '%s' in weapon '%s'", projectileName, objectname );
		} else {
			const char *spawnclass = projectileDef->dict.GetString( "spawnclass" );
			idTypeInfo *cls = idClass::GetClass( spawnclass );
			if ( !cls || !cls->IsType( idProjectile::Type ) ) {
				gameLocal.Warning( "Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')", spawnclass, projectileName, objectname );
			} else {
				projectileDict = projectileDef->dict;
			}
		}
	}
#endif

	// set up muzzleflash render light
	const idMaterial*flashShader;
	idVec3			flashTarget;
	idVec3			flashUp;
	idVec3			flashRight;
	float			flashRadius;
	bool			flashPointLight;

	weaponDef->dict.GetString( "mtr_flashShader", "", &shader );
	flashShader = declManager->FindMaterial( shader, false );
	flashPointLight = weaponDef->dict.GetBool( "flashPointLight", "1" );
	weaponDef->dict.GetVector( "flashColor", "0 0 0", flashColor );
	flashRadius		= (float)weaponDef->dict.GetInt( "flashRadius" );	// if 0, no light will spawn
	flashTime		= SEC2MS( weaponDef->dict.GetFloat( "flashTime", "0.25" ) );
	flashTarget		= weaponDef->dict.GetVector( "flashTarget" );
	flashUp			= weaponDef->dict.GetVector( "flashUp" );
	flashRight		= weaponDef->dict.GetVector( "flashRight" );

	memset( &worldMuzzleFlash, 0, sizeof( worldMuzzleFlash ) );
	
	//nozzleGlow will only be in first person //TODO: use nozzle stuff in 3th person?
	nozzleGlow.allowLightInViewID = owner->entityNumber+1;

	worldMuzzleFlash.suppressLightInViewID = owner->entityNumber+1;
	worldMuzzleFlash.allowLightInViewID = 0;
	worldMuzzleFlash.lightId = LIGHTID_WORLD_MUZZLE_FLASH + owner->entityNumber;

	worldMuzzleFlash.pointLight								= flashPointLight;
	worldMuzzleFlash.shader									= flashShader;
	worldMuzzleFlash.shaderParms[ SHADERPARM_RED ]			= flashColor[0];
	worldMuzzleFlash.shaderParms[ SHADERPARM_GREEN ]			= flashColor[1];
	worldMuzzleFlash.shaderParms[ SHADERPARM_BLUE ]			= flashColor[2];
	worldMuzzleFlash.shaderParms[ SHADERPARM_TIMESCALE ]		= 1.0f;

	worldMuzzleFlash.lightRadius[0]							= flashRadius;
	worldMuzzleFlash.lightRadius[1]							= flashRadius;
	worldMuzzleFlash.lightRadius[2]							= flashRadius;

	if ( !flashPointLight ) {
		worldMuzzleFlash.target								= flashTarget;
		worldMuzzleFlash.up									= flashUp;
		worldMuzzleFlash.right								= flashRight;
		worldMuzzleFlash.end									= flashTarget;
	}

	//-----------------------------------

	nozzleFx			= weaponDef->dict.GetBool("nozzleFx");
	nozzleFxFade		= weaponDef->dict.GetInt("nozzleFxFade", "1500");
	nozzleGlowColor		= weaponDef->dict.GetVector("nozzleGlowColor", "1 1 1");
	nozzleGlowRadius	= weaponDef->dict.GetFloat("nozzleGlowRadius", "10");
	weaponDef->dict.GetString( "mtr_nozzleGlowShader", "", &shader );
	nozzleGlowShader = declManager->FindMaterial( shader, false );

	// get the melee damage def
	meleeDistance = weaponDef->dict.GetFloat( "melee_distance" );
	meleeDefName = weaponDef->dict.GetString( "def_melee" );
	if ( meleeDefName.Length() ) {
		meleeDef = gameLocal.FindEntityDef( meleeDefName, false );
		if ( !meleeDef ) {
			gameLocal.Error( "Unknown melee '%s'", meleeDefName.c_str() );
		}
	}

	// get the brass def
	brassDict.Clear();
	brassDelay = weaponDef->dict.GetInt( "ejectBrassDelay", "0" );
	brassDefName = weaponDef->dict.GetString( "def_ejectBrass" );

	if ( brassDefName[0] ) {
		const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( brassDefName, false );
		if ( !brassDef ) {
			gameLocal.Warning( "Unknown brass '%s'", brassDefName );
		} else {
			brassDict = brassDef->dict;
		}
	}

	if ( ( ammoType < 0 ) || ( ammoType >= AMMO_NUMTYPES ) ) {
		gameLocal.Warning( "Unknown ammotype in object '%s'", objectname );
	}

	ammoClip = ammoinclip;
	if ( ( ammoClip < 0 ) || ( ammoClip > clipSize ) ) {
		// first time using this weapon so have it fully loaded to start
		ammoClip = clipSize;
		ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
		if ( ammoClip > ammoAvail ) {
			ammoClip = ammoAvail;
		}
		//In D3XP we use ammo as soon as it is moved into the clip. This allows for weapons that share ammo
		//owner->inventory.UseAmmo(ammoType, ammoClip); //ivan - commented out!
	}

	renderEntity.gui[ 0 ] = NULL;
	guiName = weaponDef->dict.GetString( "gui" );
	if ( guiName[0] ) {
		renderEntity.gui[ 0 ] = uiManager->FindGui( guiName, true, false, true );
	}

	zoomFov = weaponDef->dict.GetInt( "zoomFov", "70" );
	berserk = weaponDef->dict.GetInt( "berserk", "2" );

	//ivan start
	mouseAimOffsetY	= weaponDef->dict.GetFloat( "mouseAimOffsetY", "0" );
	//ivan end

	weaponAngleOffsetAverages = weaponDef->dict.GetInt( "weaponAngleOffsetAverages", "10" );
	weaponAngleOffsetScale = weaponDef->dict.GetFloat( "weaponAngleOffsetScale", "0.25" );
	weaponAngleOffsetMax = weaponDef->dict.GetFloat( "weaponAngleOffsetMax", "10" );

	weaponOffsetTime = weaponDef->dict.GetFloat( "weaponOffsetTime", "400" );
	weaponOffsetScale = weaponDef->dict.GetFloat( "weaponOffsetScale", "0.005" );

	if ( !weaponDef->dict.GetString( "weapon_scriptobject", NULL, &objectType ) ) {
		gameLocal.Error( "No 'weapon_scriptobject' set on '%s'.", objectname );
	}

	// setup script object
	if ( !scriptObject.SetType( objectType ) ) {
		gameLocal.Error( "Script object '%s' not found on weapon '%s'.", objectType, objectname );
	}

	WEAPON_ATTACK.LinkTo(			scriptObject, "WEAPON_ATTACK" );
	WEAPON_SPECIAL.LinkTo(			scriptObject, "WEAPON_SPECIAL" );		//new
	WEAPON_SPECIAL_HOLD.LinkTo(		scriptObject, "WEAPON_SPECIAL_HOLD" );	//
	WEAPON_RELOAD.LinkTo(			scriptObject, "WEAPON_RELOAD" );
	WEAPON_NETRELOAD.LinkTo(		scriptObject, "WEAPON_NETRELOAD" );
	WEAPON_NETENDRELOAD.LinkTo(		scriptObject, "WEAPON_NETENDRELOAD" );
	WEAPON_NETFIRING.LinkTo(		scriptObject, "WEAPON_NETFIRING" );
	WEAPON_RAISEWEAPON.LinkTo(		scriptObject, "WEAPON_RAISEWEAPON" );
	WEAPON_LOWERWEAPON.LinkTo(		scriptObject, "WEAPON_LOWERWEAPON" );

	spawnArgs = weaponDef->dict;

#ifdef _DENTONMOD
	projectileDict.Clear();
	ChangeProjectileDef( owner->GetProjectileType() );
#endif

	shader = spawnArgs.GetString( "snd_hum" );
	if ( shader && *shader ) {
		sndHum = declManager->FindSound( shader );
		StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
	}

	isLinked = true;

	// call script object's constructor
	ConstructScriptObject();

	// make sure we have the correct skin
	UpdateSkin();

#ifdef _DENTONMOD
	InitWeaponFx();
#endif
}
/*********************
idWeapon::initWeaponFx
**********************/

#ifdef _DENTONMOD

void idWeapon::InitWeaponFx( void )
{
	const idKeyValue *pkv;
	/*
	// Create new joints

	const idKeyValue *pkv = weaponDef->dict.MatchPrefix( "weapon_createjoint", NULL );

	while( pkv ) {
		WeaponJoint_t newJoint;
		jointHandle_t joint;

		idStr jointName = pkv->GetValue();
		
		bool isOnWorldModel = weaponDef->dict.GetBool(  va( "%s_onWorldModel", jointName.c_str() ), "0"); 

		idStr referenceJoint = weaponDef->dict.GetString(va("%s_referencejoint", jointName.c_str()));

		idAnimator *weapAnimator;
		if ( !isOnWorldModel ) {	
			joint = animator.GetJointHandle( referenceJoint.c_str() );
			weapAnimator = &animator;
		} 
		else {
			idEntity *ent = worldModel.GetEntity();
			weapAnimator = ent->GetAnimator();
			joint = weapAnimator->GetJointHandle( referenceJoint.c_str() );
		} 

		if( joint != INVALID_JOINT ) {	// To create a new joint we need a reference to an existing joint

			idVec3 offset, origin, dir;
			idMat3 axis;

			GetGlobalJointTransform( !isOnWorldModel, joint, origin, axis );  

			if( weaponDef->dict.GetVector(  va( "%s_offset", jointName.c_str() ), "0 0 0", offset) ) {
				origin += axis * offset;
			}
			if ( weaponDef->dict.GetVector(  va( "%s_dir", jointName.c_str() ), "1 0 0", dir) ) {
				idVec3 &xAxis = axis[0];

				xAxis = axis * dir;
				xAxis.Normalize();

				axis = xAxis.ToMat3 ();					
			}

			strcpy( newJoint.name, jointName.c_str() ); 

			newJoint.joint = (jointHandle_t)weapAnimator->NumJoints();

			weapAnimator->SetJointPos( newJoint.joint, JOINTMOD_WORLD_OVERRIDE, origin );
			weapAnimator->SetJointAxis( newJoint.joint, JOINTMOD_WORLD_OVERRIDE, axis );

			weaponJoints.Set( jointName.c_str(), newJoint );
		}
		pkv = weaponDef->dict.MatchPrefix( "weapon_createjoint", pkv );
	}

	// Update existing joints
	pkv = weaponDef->dict.MatchPrefix( "weapon_updatejoint", NULL );

	while( pkv ) {
		jointHandle_t refJoint, joint;

		idStr jointName = pkv->GetValue();

		bool isOnWorldModel = weaponDef->dict.GetBool(  va( "%s_onWorldModel", jointName.c_str() ), "0"); 

		idStr refJointName = weaponDef->dict.GetString(va("%s_referencejoint", jointName.c_str()));

		idAnimator *weapAnimator;
		
		if ( !isOnWorldModel ) {	
			refJoint	= animator.GetJointHandle( refJointName.c_str() );
			joint		= animator.GetJointHandle( jointName.c_str() );
			weapAnimator = &animator;
		} 
		else {
			idEntity *ent = worldModel.GetEntity();
			weapAnimator = ent->GetAnimator();
			refJoint	= weapAnimator->GetJointHandle( refJointName.c_str() );
			joint		= weapAnimator->GetJointHandle( jointName.c_str() );

		} 

		if( joint != INVALID_JOINT && refJoint != INVALID_JOINT ) {

			idVec3 offset, origin, dir;
			idMat3 axis;

			GetGlobalJointTransform( !isOnWorldModel, refJoint, origin, axis );  

			if( weaponDef->dict.GetVector(  va( "%s_offset", jointName.c_str() ), "0 0 0", offset) ) {
				origin += axis * offset;
			}
			if ( weaponDef->dict.GetVector(  va( "%s_dir", jointName.c_str() ), "1 0 0", dir) ) {
				idVec3 &xAxis = axis[0];

				xAxis = axis * dir;
				xAxis.Normalize();

				axis = xAxis.ToMat3 ();					
			}

			weapAnimator->SetJointAxis( joint, JOINTMOD_WORLD_OVERRIDE, axis );
			weapAnimator->SetJointPos( joint, JOINTMOD_WORLD_OVERRIDE, origin );
		}
		pkv = weaponDef->dict.MatchPrefix( "weapon_updatejoint", pkv );
	}
*/
	//Initialize the particles

	pkv = weaponDef->dict.MatchPrefix( "weapon_particle", NULL );
	while( pkv ) {
		WeaponParticle_t newParticle;
		memset( &newParticle, 0, sizeof( newParticle ) );

		idStr prtName = pkv->GetValue();

		strcpy(newParticle.name, prtName.c_str());

		newParticle.particleFlags.isOffset = weaponDef->dict.GetVector(  va( "%s_offset", prtName.c_str() ), "0 0 0", newParticle.offset ); // this offset will be added to the origin
		newParticle.particleFlags.isDir = weaponDef->dict.GetVector(  va( "%s_dir", prtName.c_str() ), "1 0 0", newParticle.dir ); // adjust the dir
		newParticle.particleFlags.isViewDir = weaponDef->dict.GetBool(  va( "%s_useViewDir", prtName.c_str() ), "0"); //ivan
		newParticle.particleFlags.isOnWorldModel = weaponDef->dict.GetBool(  va( "%s_onWorldModel", prtName.c_str() ), "0"); 

		idStr jointName = weaponDef->dict.GetString(va("%s_joint", prtName.c_str()));
		
//		WeaponJoint_t *customJoint;

		//ivan start		
		newParticle.particleFlags.isSmoke = weaponDef->dict.GetBool(va("%s_smoke", prtName.c_str()), "0");
		
		if ( newParticle.particleFlags.isOnWorldModel ) { //now worldmodel is the weapon model itself	
			newParticle.joint = animator.GetJointHandle( jointName.c_str() );
		} 
		else { //not supported
			newParticle.joint = INVALID_JOINT;
		}

		/*
		if( newParticle.joint == INVALID_JOINT ){
			gameLocal.Printf( "joint %s for %s is INVALID\n", jointName.c_str(), prtName.c_str());
		}else{
			gameLocal.Printf( "joint %s for %s is OK\n", jointName.c_str(), prtName.c_str());
		}
		*/
		
		//ivan end

		newParticle.particleFlags.isActive = false;
		newParticle.startTime = 0;

		idStr particle = weaponDef->dict.GetString(va("%s_particle", prtName.c_str()));
		strcpy(newParticle.particlename, particle.c_str());
		newParticle.particleFlags.isContinuous = weaponDef->dict.GetBool(va("%s_continuous", prtName.c_str()), "1");

		if(newParticle.particleFlags.isSmoke) {
			//				newParticle.particleFlags.isContinuous = weaponDef->dict.GetBool(va("%s_continuous", prtName.c_str()), "1");
			newParticle.particle = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, particle.c_str(), false ) );
		} 
		else {

			newParticle.renderEntity.shaderParms[ SHADERPARM_RED ]		= 1.0;
			newParticle.renderEntity.shaderParms[ SHADERPARM_GREEN ]	= 1.0;
			newParticle.renderEntity.shaderParms[ SHADERPARM_BLUE ]		= 1.0;
			newParticle.renderEntity.shaderParms[ SHADERPARM_ALPHA ]	= 1.0;
		
			if ( newParticle.particleFlags.isOnWorldModel ) {
				newParticle.renderEntity.suppressSurfaceInViewID = owner->entityNumber+1; // Make sure this is third person effect only.
			} 
			else {					
				newParticle.renderEntity.weaponDepthHack = weaponDef->dict.GetBool(va("%s_weaponDepthHack", prtName.c_str()), "0");
				newParticle.renderEntity.allowSurfaceInViewID = owner->entityNumber+1; // Make sure this is first person effect only.
			}

			const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, newParticle.particlename ) );
			if ( modelDef ) {
				newParticle.renderEntity.hModel = renderModelManager->FindModel( newParticle.particlename );
			}
			newParticle.modelDefHandle = -1;
		}
		weaponParticles.Set(prtName.c_str(), newParticle);

		pkv = weaponDef->dict.MatchPrefix( "weapon_particle", pkv );
	}

	const idKeyValue *lkv = weaponDef->dict.MatchPrefix( "weapon_light", NULL );
	while( lkv ) {
		WeaponLight_t newLight;
		memset( &newLight, 0, sizeof( newLight ) );

		newLight.lightHandle = -1;
		newLight.lightFlags.isActive = false;
		newLight.startTime = 0;

		idStr lightName = lkv->GetValue(), debug;


		strcpy(newLight.name, lightName.c_str());

		newLight.lightFlags.isOffset = weaponDef->dict.GetVector(  va( "%s_offset", lightName.c_str() ), "0 0 0", newLight.offset ); // this offset will be added to the origin
		newLight.lightFlags.isDir = weaponDef->dict.GetVector(  va( "%s_dir", lightName.c_str() ), "1 0 0", newLight.dir ); // Direction can be adjusted with this

		idStr lightShader = weaponDef->dict.GetString(va("%s_shader", lightName.c_str()));
		newLight.light.shader = declManager->FindMaterial( lightShader.c_str(), false );

		float radius = weaponDef->dict.GetFloat(va("%s_radius", lightName.c_str()));

		newLight.light.lightRadius[0] = radius;
		newLight.light.lightRadius[1] = radius;
		newLight.light.lightRadius[2] = radius;

		newLight.lightFlags.isAlwaysOn = weaponDef->dict.GetBool( va( "%s_alwaysOn", lightName.c_str () ), "1" );
		newLight.light.pointLight = weaponDef->dict.GetBool( va( "%s_pointLight", lightName.c_str() ), "0" );
		newLight.light.noShadows = weaponDef->dict.GetBool( va( "%s_noShadows", lightName.c_str() ), "0" );

		if (!newLight.light.pointLight) { 
			newLight.light.target	= weaponDef->dict.GetVector(  va( "%s_target", lightName.c_str() ) );
			newLight.light.up		= weaponDef->dict.GetVector(  va( "%s_up", lightName.c_str() ) );
			newLight.light.right	= weaponDef->dict.GetVector(  va( "%s_right", lightName.c_str() ) );
			newLight.light.end		= newLight.light.target;
		}

		if (!newLight.lightFlags.isAlwaysOn) {
			newLight.endTime = SEC2MS( weaponDef->dict.GetFloat( va( "%s_endTime", lightName.c_str() ), "0.25" ) );
		}

		idVec3 lightColor = weaponDef->dict.GetVector( va( "%s_color", lightName.c_str() ), "1 1 1");

		newLight.light.shaderParms[ SHADERPARM_RED ]		= lightColor[0];
		newLight.light.shaderParms[ SHADERPARM_GREEN ]		= lightColor[1];
		newLight.light.shaderParms[ SHADERPARM_BLUE ]		= lightColor[2];
		newLight.light.shaderParms[ SHADERPARM_TIMESCALE ]	= 1.0f;

		newLight.lightFlags.isOnWorldModel = weaponDef->dict.GetBool(  va( "%s_onWorldModel", lightName.c_str() ), "0"); 

		idStr jointName = weaponDef->dict.GetString(va("%s_joint", lightName.c_str()));

		if ( newLight.lightFlags.isOnWorldModel ) { // third person only light
			newLight.joint = animator.GetJointHandle( jointName.c_str() ); //ivan
			newLight.light.suppressLightInViewID = owner->entityNumber+1;
		}
		else { //not supported
			newLight.joint = INVALID_JOINT; //ivan
			newLight.light.allowLightInViewID = owner->entityNumber+1;
		}

		weaponLights.Set(lightName.c_str(), newLight);

		lkv = weaponDef->dict.MatchPrefix( "weapon_light", lkv );
	}
}
#endif //_DENTONMOD
/***********************************************************************

GUIs

***********************************************************************/

/*
================
idWeapon::Icon
================
*/
const char *idWeapon::Icon( void ) const {
	return icon;
}

/*
================
idWeapon::UpdateGUI
================
*/
void idWeapon::UpdateGUI( void ) {
	if ( !renderEntity.gui[ 0 ] ) {
		return;
	}

	if ( status == WP_HOLSTERED ) {
		return;
	}

	if ( owner->weaponGone ) {
		// dropping weapons was implemented wierd, so we have to not update the gui when it happens or we'll get a negative ammo count
		return;
	}

	if ( gameLocal.localClientNum != owner->entityNumber ) {
		// if updating the hud for a followed client
		if ( gameLocal.localClientNum >= 0 && gameLocal.entities[ gameLocal.localClientNum ] && gameLocal.entities[ gameLocal.localClientNum ]->IsType( idPlayer::Type ) ) {
			idPlayer *p = static_cast< idPlayer * >( gameLocal.entities[ gameLocal.localClientNum ] );
			if ( !p->spectating || p->spectator != owner->entityNumber ) {
				return;
			}
		} else {
			return;
		}
	}

	int inclip = AmmoInClip();
	int ammoamount = AmmoAvailable();

	if ( ammoamount < 0 ) {
		// show infinite ammo
		renderEntity.gui[ 0 ]->SetStateString( "player_ammo", "" );
	} else {
		// show remaining ammo
		renderEntity.gui[ 0 ]->SetStateString( "player_totalammo", va( "%i", ammoamount) );//new
		//		renderEntity.gui[ 0 ]->SetStateString( "player_totalammo", va( "%i", ammoamount - inclip) );
		renderEntity.gui[ 0 ]->SetStateString( "player_ammo", ClipSize() ? va( "%i", inclip ) : "--" );
		renderEntity.gui[ 0 ]->SetStateString( "player_clips", ClipSize() ? va("%i", ammoamount / ClipSize()) : "--" );
		//		renderEntity.gui[ 0 ]->SetStateString( "player_allammo", va( "%i/%i", inclip, ammoamount - inclip ) );
		renderEntity.gui[ 0 ]->SetStateString( "player_allammo", va( "%i/%i", inclip, ammoamount ) );//new
	}
	renderEntity.gui[ 0 ]->SetStateBool( "player_ammo_empty", ( ammoamount == 0 ) );
	renderEntity.gui[ 0 ]->SetStateBool( "player_clip_empty", ( inclip == 0 ) );
	renderEntity.gui[ 0 ]->SetStateBool( "player_clip_low", ( inclip <= lowAmmo ) );

	//Let the HUD know the total amount of ammo regardless of the ammo required value
	renderEntity.gui[ 0 ]->SetStateString( "player_ammo_count", va("%i", AmmoCount())); //new


}

/***********************************************************************

Model and muzzleflash

***********************************************************************/


/********************************************************************
idWeapon::UpdateWeaponFx( void )

Updates custom weapon particles and lights- By Clone JC Denton
********************************************************************/

#ifdef _DENTONMOD

void idWeapon::UpdateWeaponFx( void )
{
	for( int i = 0; i < weaponParticles.Num(); i++ ) {
		WeaponParticle_t* part = weaponParticles.GetIndex(i);

		//ivan - skip view particles! 
		if( !part->particleFlags.isOnWorldModel ) continue;

		if( part->particleFlags.isActive ) {
			if(part->particleFlags.isSmoke) {
				if(part->joint != INVALID_JOINT) {
					//ivan start
					/*
					//was
					GetGlobalJointTransform( true, part->joint, muzzleOrigin, muzzleAxis );
					*/
		
					GetGlobalJointTransform( part->joint, muzzleOrigin, muzzleAxis ); 
					
					if( part->particleFlags.isViewDir ){ //use view dir awyway
						muzzleAxis = playerViewAxis;
					}else{
						//direction fix!
						idVec3 tmp =  muzzleAxis[2];
						muzzleAxis[2] =  muzzleAxis[0];
						muzzleAxis[0] = -tmp;
					}
					//ivan end

				} else {
					// default to going straight out the view
					muzzleOrigin = playerViewOrigin;
					muzzleAxis = playerViewAxis;
				}

				if ( part->particleFlags.isOffset ) {
					muzzleOrigin += muzzleAxis * part->offset;
				}
				if ( part->particleFlags.isDir ) {
					idVec3 &dir = muzzleAxis[ 0 ];

					dir = muzzleAxis * part->dir;
					dir.Normalize();

					muzzleAxis = dir.ToMat3 ();					
				}

				if ( !gameLocal.smokeParticles->EmitSmoke( part->particle, part->startTime, gameLocal.random.RandomFloat(), muzzleOrigin, muzzleAxis ) ) {
					if ( !part->particleFlags.isContinuous ) {
						part->particleFlags.isActive = false;	// all done
						part->startTime = 0; 
					}
					else {
						part->startTime = gameLocal.time; // for continuous effect
					}
				}
			} 
			else {

				if( part->renderEntity.hModel ) {

					//ivan start

					/*
					//was: 
					GetGlobalJointTransform( !part->particleFlags.isOnWorldModel, part->joint, part->renderEntity.origin, part->renderEntity.axis );

					if ( part->particleFlags.isOffset ) {
						part->renderEntity.origin += part->renderEntity.axis * part->offset;
					}
					if ( part->particleFlags.isDir ) {

						idVec3 &dir = part->renderEntity.axis[ 0 ];

						dir = part->renderEntity.axis * part->dir;
						dir.Normalize();
						part->renderEntity.axis = dir.ToMat3();
					}
					*/

					GetGlobalJointTransform( part->joint, muzzleOrigin, muzzleAxis );

					if( part->particleFlags.isViewDir ){ //use view dir awyway
						muzzleAxis = playerViewAxis;
					}

					//direction fix!
					//do this also is isViewDir, so the "up" of the particle means "forward" also in that case
					idVec3 tmp =  muzzleAxis[2];
					muzzleAxis[2] =  muzzleAxis[0];
					muzzleAxis[0] = -tmp;					

					if ( part->particleFlags.isDir ) {

						idVec3 &dir = muzzleAxis[ 0 ];

						dir = muzzleAxis * part->dir;
						dir.Normalize();
						muzzleAxis = dir.ToMat3();
					}

					if ( part->particleFlags.isOffset ) {
						muzzleOrigin += muzzleAxis * part->offset;
					}

					part->renderEntity.origin = muzzleOrigin;
					part->renderEntity.axis = muzzleAxis;
					//ivan end

					if ( part->modelDefHandle != -1 ) {
						gameRenderWorld->UpdateEntityDef( part->modelDefHandle, &part->renderEntity );
					}
					else {
						part->renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
						part->renderEntity.shaderParms[ SHADERPARM_DIVERSITY ]	= gameLocal.random.RandomFloat(); // For effects like muzzleflashes etc.
						part->modelDefHandle = gameRenderWorld->AddEntityDef( &part->renderEntity );
					}
				}
			}
		}
	}

	for(int i = 0; i < weaponLights.Num(); i++) {
		WeaponLight_t* light = weaponLights.GetIndex(i);

		//ivan - skip view lights! 
		if( !light->lightFlags.isOnWorldModel ) continue;

		if(light->lightFlags.isActive) {
			if (GetGlobalJointTransform( light->joint, light->light.origin, light->light.axis )) {
				if ( light->lightFlags.isOffset ) {					
					light->light.origin += light->light.axis[ 0 ] * light->offset[ 0 ] + light->light.axis[ 1 ] * light->offset[ 1 ] + light->light.axis[ 2 ] * light->offset[ 2 ];
				} 
				if ( light->lightFlags.isDir ) {		
					idVec3 &dir = light->light.axis[ 0 ];

					dir = light->light.axis * light->dir;			
					dir.Normalize();
					light->light.axis = dir.ToMat3();
				}	
				if ( ( light->lightHandle != -1 ) ) {
					gameRenderWorld->UpdateLightDef( light->lightHandle, &light->light );
				} 
				else {
					light->light.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
					light->light.shaderParms[ SHADERPARM_DIVERSITY ]	= renderEntity.shaderParms[ SHADERPARM_DIVERSITY ]; // For muzzleflashes.
					light->lightHandle = gameRenderWorld->AddLightDef( &light->light );
				}

				if ( !light->lightFlags.isAlwaysOn && ( gameLocal.time >= light->endTime ) ) {
					light->endTime -= light->startTime; // readjust the value.
					Event_StopWeaponLight( light->name );
				}
			}
		}
	}
}

/********************************************************************
idWeapon::StopWeaponFx( void )

Stops custom weapon particles and lights temporarily- By Clone JC Denton
********************************************************************/

void idWeapon::StopWeaponFx( void )
{
	for( int i = 0; i < weaponParticles.Num(); i++ ) {
		WeaponParticle_t* part = weaponParticles.GetIndex(i);

		if(part->particleFlags.isActive) {
			//Free the particles
			if( !part->particleFlags.isSmoke && part->modelDefHandle > -1 ) {
				gameRenderWorld->FreeEntityDef( part->modelDefHandle );
				part->modelDefHandle = -1;
			}
		}
	}

	for(int i = 0; i < weaponLights.Num(); i++) {
		WeaponLight_t* light = weaponLights.GetIndex(i);
		if( light->lightFlags.isActive ) {
			if(light->lightHandle != -1) {
				gameRenderWorld->FreeLightDef( light->lightHandle );
				light->lightHandle = -1;
			}
		}
	}
}
#endif

/*
================
idWeapon::UpdateFlashPosition
================
*/
void idWeapon::UpdateFlashPosition( void ) {


/* REMOVED BY REVILITY FOR IVAN'S LAUNCH FROM BARREL FIX WHICH IS THE CODE BELOW IT
	// the flash has an explicit joint for locating it
	GetGlobalJointTransform( true, flashJointView, muzzleFlash.origin, muzzleFlash.axis );

	// if the desired point is inside or very close to a wall, back it up until it is clear
	idVec3	start = muzzleFlash.origin - playerViewAxis[0] * 16;
	idVec3	end = muzzleFlash.origin + playerViewAxis[0] * 8;
	trace_t	tr;
	gameLocal.clip.TracePoint( tr, start, end, MASK_SHOT_RENDERMODEL, owner );
	// be at least 8 units away from a solid
	muzzleFlash.origin = tr.endpos - playerViewAxis[0] * 8;

	// put the world muzzle flash on the end of the joint, no matter what
	GetGlobalJointTransform( false, flashJointWorld, worldMuzzleFlash.origin, worldMuzzleFlash.axis );
}
*/

	/*
	if ( flashJointView != INVALID_JOINT ) { //ivan
		// the flash has an explicit joint for locating it
		GetGlobalJointTransform( true, flashJointView, muzzleFlash.origin, muzzleFlash.axis ); 
	}
	*/

	/* ivan - commented out - don't waste time for useless things
	// if the desired point is inside or very close to a wall, back it up until it is clear
	idVec3	start = muzzleFlash.origin - playerViewAxis[0] * 16;
	idVec3	end = muzzleFlash.origin + playerViewAxis[0] * 8;
	trace_t	tr;
	gameLocal.clip.TracePoint( tr, start, end, MASK_SHOT_RENDERMODEL, owner );
	// be at least 8 units away from a solid
	muzzleFlash.origin = tr.endpos - playerViewAxis[0] * 8;
	*/

	// put the world muzzle flash on the end of the joint, no matter what
	GetGlobalJointTransform( flashJointWorld, worldMuzzleFlash.origin, worldMuzzleFlash.axis );
}

/*
================
idWeapon::MuzzleFlashLight
================
*/
void idWeapon::MuzzleFlashLight( void ) {

	if ( !lightOn && ( !g_muzzleFlash.GetBool() || !worldMuzzleFlash.lightRadius[0] ) ) {
		return;
	}

	//REVILITY START LAUNCH FROM BARREL FIX
	if ( flashJointWorld == INVALID_JOINT ) {
		return;
	}
	//REVILITY END LAUNCH FROM BARREL FIX
	
	UpdateFlashPosition();

	// these will be different each fire
	worldMuzzleFlash.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
	worldMuzzleFlash.shaderParms[ SHADERPARM_DIVERSITY ]	= renderEntity.shaderParms[ SHADERPARM_DIVERSITY ];

	// the light will be removed at this time
	muzzleFlashEnd = gameLocal.time + flashTime;

	if ( worldMuzzleFlashHandle != -1 ) {
		gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );
	} else {
		worldMuzzleFlashHandle = gameRenderWorld->AddLightDef( &worldMuzzleFlash );
	}
}

/*
================
idWeapon::UpdateSkin
================
*/
bool idWeapon::UpdateSkin( void ) {
	const function_t *func;

	if ( !isLinked ) {
		return false;
	}

	func = scriptObject.GetFunction( "UpdateSkin" );
	if ( !func ) {
		common->Warning( "Can't find function 'UpdateSkin' in object '%s'", scriptObject.GetTypeName() );
		return false;
	}

	// use the frameCommandThread since it's safe to use outside of framecommands
	gameLocal.frameCommandThread->CallFunction( this, func, true );
	gameLocal.frameCommandThread->Execute();

	return true;
}

/*
================
idWeapon::SetModel
================
*/
void idWeapon::SetModel( const char *modelname ) {
	assert( modelname );

	if ( modelDefHandle >= 0 ) {
		gameRenderWorld->RemoveDecals( modelDefHandle );
	}

	renderEntity.hModel = animator.SetModel( modelname );
	if ( renderEntity.hModel ) {
		renderEntity.customSkin = animator.ModelDef()->GetDefaultSkin();
		animator.GetJoints( &renderEntity.numJoints, &renderEntity.joints );
	} else {
		renderEntity.customSkin = NULL;
		renderEntity.callback = NULL;
		renderEntity.numJoints = 0;
		renderEntity.joints = NULL;
	}

	// hide the model until an animation is played
	Hide();
}

/*
================
idWeapon::GetGlobalJointTransform

This returns the offset and axis of a weapon bone in world space, suitable for attaching models or lights
================
*/
bool idWeapon::GetGlobalJointTransform( const jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis ) {
	//ivan - world model is now the model itself
	if ( animator.GetJointTransform( jointHandle, gameLocal.time, offset, axis ) ) {
		offset = GetPhysics()->GetOrigin() + offset * GetPhysics()->GetAxis();
		axis = axis * GetPhysics()->GetAxis();
		return true;
	}
	offset = GetPhysics()->GetOrigin(); //viewWeaponOrigin;
	axis = GetPhysics()->GetAxis(); //viewWeaponAxis;
	return false;
}

/*
================
idWeapon::SetPushVelocity
================
*/
void idWeapon::SetPushVelocity( const idVec3 &pushVelocity ) {
	this->pushVelocity = pushVelocity;
}


/***********************************************************************

State control/player interface

***********************************************************************/

/*
================
idWeapon::Think
================
*/
void idWeapon::Think( void ) {
	// do nothing because the present is called from the player through PresentWeapon
}

/*
================
idWeapon::Raise
================
*/
void idWeapon::Raise( void ) {
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = true;
	}
}

/*
================
idWeapon::PutAway
================
*/
void idWeapon::PutAway( void ) {
	hasBloodSplat = false;
	if ( isLinked ) {
		WEAPON_LOWERWEAPON = true;
	}
}

/*
================
idWeapon::Reload
NOTE: this is only for impulse-triggered reload, auto reload is scripted
================
*/
void idWeapon::Reload( void ) {
	if ( isLinked ) {
		WEAPON_RELOAD = true;
	}
}

/*
================
idWeapon::LowerWeapon
================
*/
void idWeapon::LowerWeapon( void ) {
	if ( !hide ) {
		hideStart	= 0.0f;
		hideEnd		= hideDistance;
		if ( gameLocal.time - hideStartTime < hideTime ) {
			hideStartTime = gameLocal.time - ( hideTime - ( gameLocal.time - hideStartTime ) );
		} else {
			hideStartTime = gameLocal.time;
		}
		hide = true;
	}
}

/*
================
idWeapon::RaiseWeapon
================
*/
void idWeapon::RaiseWeapon( void ) {
	Show();

	if ( hide ) {
		hideStart	= hideDistance;
		hideEnd		= 0.0f;
		if ( gameLocal.time - hideStartTime < hideTime ) {
			hideStartTime = gameLocal.time - ( hideTime - ( gameLocal.time - hideStartTime ) );
		} else {
			hideStartTime = gameLocal.time;
		}
		hide = false;
	}
}

/*
================
idWeapon::HideWeapon
================
*/
void idWeapon::HideWeapon( void ) {
	Hide();
	muzzleFlashEnd = 0;
}

/*
================
idWeapon::ShowWeapon
================
*/
void idWeapon::ShowWeapon( void ) {
	Show();
	if ( lightOn ) {
		MuzzleFlashLight();
	}
}

/*
================
idWeapon::HideWorldModel
================
*/
void idWeapon::HideWorldModel( void ) {
	Hide();
}

/*
================
idWeapon::ShowWorldModel
================
*/
void idWeapon::ShowWorldModel( void ) {
	Show();
}

/*
================
idWeapon::OwnerDied
================
*/
void idWeapon::OwnerDied( void ) {
	if ( isLinked ) {
		SetState( "OwnerDied", 0 );
		thread->Execute();
	}

	Hide();

	// don't clear the weapon immediately since the owner might have killed himself by firing the weapon
	// within the current stack frame
	PostEventMS( &EV_Weapon_Clear, 0 );
}

/*
================
idWeapon::BeginSpecialFunction
================
*/
void idWeapon::BeginSpecialFunction( bool keyTapped ) {	// new
	if ( isLinked ) {
		if (keyTapped){
			WEAPON_SPECIAL = true;
		}
		WEAPON_SPECIAL_HOLD = true;
	}

}

/*
================
idWeapon::EndSpecialFunc
================
*/
void idWeapon::EndSpecialFunction( void ) {		//New
	if ( !WEAPON_SPECIAL_HOLD.IsLinked() ) {
		return;
	}
	if ( WEAPON_SPECIAL_HOLD ) {
		WEAPON_SPECIAL_HOLD = false;
	}
}

/*
================
idWeapon::BeginAttack
================
*/
void idWeapon::BeginAttack( void ) {
	if ( status != WP_OUTOFAMMO ) {
		lastAttack = gameLocal.time;
	}

	if ( !isLinked ) {
		return;
	}

	if ( !WEAPON_ATTACK ) {
		if ( sndHum ) {
			StopSound( SND_CHANNEL_BODY, false );
		}
	}
	WEAPON_ATTACK = true;
}

/*
================
idWeapon::EndAttack
================
*/
void idWeapon::EndAttack( void ) {
	if ( !WEAPON_ATTACK.IsLinked() ) {
		return;
	}
	if ( WEAPON_ATTACK ) {
		WEAPON_ATTACK = false;
		if ( sndHum ) {
			StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
		}
	}
}

/*
================
idWeapon::isReady
================
*/
bool idWeapon::IsReady( void ) const {
	return !hide && !IsHidden() && ( ( status == WP_RELOAD ) || ( status == WP_READY ) || ( status == WP_OUTOFAMMO ) );
}

/*
================
idWeapon::IsReloading
================
*/
bool idWeapon::IsReloading( void ) const {
	return ( status == WP_RELOAD );
}

/*
================
idWeapon::IsHolstered
================
*/
bool idWeapon::IsHolstered( void ) const {
	return ( status == WP_HOLSTERED );
}

/*
================
idWeapon::ShowCrosshair
================
*/
bool idWeapon::ShowCrosshair( void ) const {
	return !( state == idStr( WP_RISING ) || state == idStr( WP_LOWERING ) || state == idStr( WP_HOLSTERED ) );
}

/*
=====================
idWeapon::CanDrop
=====================
*/
bool idWeapon::CanDrop( void ) const {
	if ( !weaponDef ) {
		return false;
	}
	const char *classname = weaponDef->dict.GetString( "def_dropItem" );
	if ( !classname[ 0 ] ) {
		return false;
	}
	return true;
}

/*
================
idWeapon::WeaponStolen
================
*/
void idWeapon::WeaponStolen( void ) {
	assert( !gameLocal.isClient );
	if ( projectileEnt ) {
		if ( isLinked ) {
			SetState( "WeaponStolen", 0 );
			thread->Execute();
		}
		projectileEnt = NULL;
	}

	// set to holstered so we can switch weapons right away
	status = WP_HOLSTERED;

	HideWeapon();
}

/*
=====================
idWeapon::DropItem
=====================
*/
idEntity * idWeapon::DropItem( const idVec3 &velocity, int activateDelay, int removeDelay, bool died ) {
	if ( !weaponDef ) {
		return NULL;
	}
	if ( !allowDrop ) {
		return NULL;
	}
	const char *classname = weaponDef->dict.GetString( "def_dropItem" );
	if ( !classname[0] ) {
		return NULL;
	}
	StopSound( SND_CHANNEL_BODY, true );
	StopSound( SND_CHANNEL_BODY3, true );

	//ivan start
	//was:
	//idMoveableItem::DropItem( classname, worldModel.GetEntity()->GetPhysics()->GetOrigin(), worldModel.GetEntity()->GetPhysics()->GetAxis(), velocity, activateDelay, removeDelay );
	return idMoveableItem::DropItem( classname, GetPhysics()->GetOrigin(), GetPhysics()->GetAxis(), velocity, activateDelay, removeDelay );
	//ivan end
}

/***********************************************************************

Script state management

***********************************************************************/

/*
=====================
idWeapon::SetState
=====================
*/
void idWeapon::SetState( const char *statename, int blendFrames ) {
	const function_t *func;

	if ( !isLinked ) {
		return;
	}

	func = scriptObject.GetFunction( statename );
	if ( !func ) {
		assert( 0 );
		gameLocal.Error( "Can't find function '%s' in object '%s'", statename, scriptObject.GetTypeName() );
	}

	thread->CallFunction( this, func, true );
	state = statename;

	animBlendFrames = blendFrames;
	if ( g_debugWeapon.GetBool() ) {
		gameLocal.Printf( "%d: weapon state : %s\n", gameLocal.time, statename );
	}

	idealState = "";
}


/***********************************************************************

Particles/Effects

***********************************************************************/

/*
================
idWeapon::UpdateNozzelFx
================
*/
void idWeapon::UpdateNozzleFx( void ) {
	if ( !nozzleFx ) {
		return;
	}

	//
	// shader parms
	//
	int la = gameLocal.time - lastAttack + 1;
	float s = 1.0f;
	float l = 0.0f;
	if ( la < nozzleFxFade ) {
		s = ((float)la / nozzleFxFade);
		l = 1.0f - s;
	}
	renderEntity.shaderParms[5] = s;
	renderEntity.shaderParms[6] = l;

	/*
	if ( ventLightJointView == INVALID_JOINT ) {
		return;
	}

	//
	// vent light
	//
	if ( nozzleGlowHandle == -1 ) {
		memset(&nozzleGlow, 0, sizeof(nozzleGlow));
		if ( owner ) {
			nozzleGlow.allowLightInViewID = owner->entityNumber+1;
		}
		nozzleGlow.pointLight = true;
		nozzleGlow.noShadows = true;
		nozzleGlow.lightRadius.x = nozzleGlowRadius;
		nozzleGlow.lightRadius.y = nozzleGlowRadius;
		nozzleGlow.lightRadius.z = nozzleGlowRadius;
		nozzleGlow.shader = nozzleGlowShader;
		nozzleGlow.shaderParms[ SHADERPARM_TIMESCALE ]	= 1.0f;
		nozzleGlow.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
		GetGlobalJointTransform( true, ventLightJointView, nozzleGlow.origin, nozzleGlow.axis );
		nozzleGlowHandle = gameRenderWorld->AddLightDef(&nozzleGlow);
	}

	GetGlobalJointTransform( true, ventLightJointView, nozzleGlow.origin, nozzleGlow.axis );

	nozzleGlow.shaderParms[ SHADERPARM_RED ] = nozzleGlowColor.x * s;
	nozzleGlow.shaderParms[ SHADERPARM_GREEN ] = nozzleGlowColor.y * s;
	nozzleGlow.shaderParms[ SHADERPARM_BLUE ] = nozzleGlowColor.z * s;
	gameRenderWorld->UpdateLightDef(nozzleGlowHandle, &nozzleGlow);
	*/
}


/*
================
idWeapon::BloodSplat
================
*/
bool idWeapon::BloodSplat( float size ) {
	float s, c;
	idMat3 localAxis, axistemp;
	idVec3 localOrigin, normal;

	if ( hasBloodSplat ) {
		return true;
	}

	hasBloodSplat = true;

	if ( modelDefHandle < 0 ) {
		return false;
	}

	if ( !GetGlobalJointTransform( ejectJointWorld, localOrigin, localAxis ) ) { //ivan 
	//was: if ( !GetGlobalJointTransform( true, ejectJointView, localOrigin, localAxis ) ) {
		return false;
	}

	localOrigin[0] += gameLocal.random.RandomFloat() * -10.0f;
	localOrigin[1] += gameLocal.random.RandomFloat() * 1.0f;
	localOrigin[2] += gameLocal.random.RandomFloat() * -2.0f;

	normal = idVec3( gameLocal.random.CRandomFloat(), -gameLocal.random.RandomFloat(), -1 );
	normal.Normalize();

	idMath::SinCos16( gameLocal.random.RandomFloat() * idMath::TWO_PI, s, c );

	localAxis[2] = -normal;
	localAxis[2].NormalVectors( axistemp[0], axistemp[1] );
	localAxis[0] = axistemp[ 0 ] * c + axistemp[ 1 ] * -s;
	localAxis[1] = axistemp[ 0 ] * -s + axistemp[ 1 ] * -c;

	localAxis[0] *= 1.0f / size;
	localAxis[1] *= 1.0f / size;

	idPlane		localPlane[2];

	localPlane[0] = localAxis[0];
	localPlane[0][3] = -(localOrigin * localAxis[0]) + 0.5f;

	localPlane[1] = localAxis[1];
	localPlane[1][3] = -(localOrigin * localAxis[1]) + 0.5f;

	const idMaterial *mtr = declManager->FindMaterial( "textures/decals/duffysplatgun" );

	gameRenderWorld->ProjectOverlay( modelDefHandle, localPlane, mtr );

	return true;
}


/***********************************************************************

Visual presentation

***********************************************************************/

/*
================
idWeapon::MuzzleRise

The machinegun and chaingun will incrementally back up as they are being fired
================

void idWeapon::MuzzleRise( idVec3 &origin, idMat3 &axis ) {
	int			time;
	float		amount;
	idAngles	ang;
	idVec3		offset;

	time = kick_endtime - gameLocal.time;
	if ( time <= 0 ) {
		return;
	}

	if ( muzzle_kick_maxtime <= 0 ) {
		return;
	}

	if ( time > muzzle_kick_maxtime ) {
		time = muzzle_kick_maxtime;
	}

	amount = ( float )time / ( float )muzzle_kick_maxtime;
	ang		= muzzle_kick_angles * amount;
	offset	= muzzle_kick_offset * amount;

	origin = origin - axis * offset;
	axis = ang.ToMat3() * axis;
}
*/

/*
================
idWeapon::ConstructScriptObject

Called during idEntity::Spawn.  Calls the constructor on the script object.
Can be overridden by subclasses when a thread doesn't need to be allocated.
================
*/
idThread *idWeapon::ConstructScriptObject( void ) {
	const function_t *constructor;

	thread->EndThread();

	// call script object's constructor
	constructor = scriptObject.GetConstructor();
	if ( !constructor ) {
		gameLocal.Error( "Missing constructor on '%s' for weapon", scriptObject.GetTypeName() );
	}

	// init the script object's data
	scriptObject.ClearObject();
	thread->CallFunction( this, constructor, true );
	thread->Execute();

	return thread;
}

/*
================
idWeapon::DeconstructScriptObject

Called during idEntity::~idEntity.  Calls the destructor on the script object.
Can be overridden by subclasses when a thread doesn't need to be allocated.
Not called during idGameLocal::MapShutdown.
================
*/
void idWeapon::DeconstructScriptObject( void ) {
	const function_t *destructor;

	if ( !thread ) {
		return;
	}

	// don't bother calling the script object's destructor on map shutdown
	if ( gameLocal.GameState() == GAMESTATE_SHUTDOWN ) {
		return;
	}

	thread->EndThread();

	// call script object's destructor
	destructor = scriptObject.GetDestructor();
	if ( destructor ) {
		// start a thread that will run immediately and end
		thread->CallFunction( this, destructor, true );
		thread->Execute();
		thread->EndThread();
	}

	// clear out the object's memory
	scriptObject.ClearObject();
}

/*
================
idWeapon::UpdateScript
================
*/
void idWeapon::UpdateScript( void ) {
	int	count;

	if ( !isLinked ) {
		return;
	}

	// only update the script on new frames
	if ( !gameLocal.isNewFrame ) {
		return;
	}

	if ( idealState.Length() ) {
		SetState( idealState, animBlendFrames );
	}

	// update script state, which may call Event_LaunchProjectiles, among other things
	count = 10;
	while( ( thread->Execute() || idealState.Length() ) && count-- ) {
		// happens for weapons with no clip (like grenades)
		if ( idealState.Length() ) {
			SetState( idealState, animBlendFrames );
		}
	}

	WEAPON_RELOAD  = false;
	WEAPON_SPECIAL = false;
}

/*
================
idWeapon::AlertMonsters
================
*/
void idWeapon::AlertMonsters( void ) {
	trace_t	tr;
	idEntity *ent;
	idVec3 end = worldMuzzleFlash.origin + worldMuzzleFlash.axis * worldMuzzleFlash.target;

	gameLocal.clip.TracePoint( tr, worldMuzzleFlash.origin, end, CONTENTS_OPAQUE | MASK_SHOT_RENDERMODEL | CONTENTS_FLASHLIGHT_TRIGGER, owner );
	if ( g_debugWeapon.GetBool() ) {
		gameRenderWorld->DebugLine( colorYellow, worldMuzzleFlash.origin, end, 0 );
		gameRenderWorld->DebugArrow( colorGreen, worldMuzzleFlash.origin, tr.endpos, 2, 0 );
	}

	if ( tr.fraction < 1.0f ) {
		ent = gameLocal.GetTraceEntity( tr );
		if ( ent->IsType( idAI::Type ) ) {
			static_cast<idAI *>( ent )->TouchedByFlashlight( owner );
		} else if ( ent->IsType( idTrigger::Type ) ) {
			ent->Signal( SIG_TOUCH );
			ent->ProcessEvent( &EV_Touch, owner, &tr );
		}
	}

	// jitter the trace to try to catch cases where a trace down the center doesn't hit the monster
	end += worldMuzzleFlash.axis * worldMuzzleFlash.right * idMath::Sin16( MS2SEC( gameLocal.time ) * 31.34f );
	end += worldMuzzleFlash.axis * worldMuzzleFlash.up * idMath::Sin16( MS2SEC( gameLocal.time ) * 12.17f );
	gameLocal.clip.TracePoint( tr, worldMuzzleFlash.origin, end, CONTENTS_OPAQUE | MASK_SHOT_RENDERMODEL | CONTENTS_FLASHLIGHT_TRIGGER, owner );
	if ( g_debugWeapon.GetBool() ) {
		gameRenderWorld->DebugLine( colorYellow, worldMuzzleFlash.origin, end, 0 );
		gameRenderWorld->DebugArrow( colorGreen, worldMuzzleFlash.origin, tr.endpos, 2, 0 );
	}

	if ( tr.fraction < 1.0f ) {
		ent = gameLocal.GetTraceEntity( tr );
		if ( ent->IsType( idAI::Type ) ) {
			static_cast<idAI *>( ent )->TouchedByFlashlight( owner );
		} else if ( ent->IsType( idTrigger::Type ) ) {
			ent->Signal( SIG_TOUCH );
			ent->ProcessEvent( &EV_Touch, owner, &tr );
		}
	}
}

/*
================
idWeapon::PresentWeapon
================
*/
void idWeapon::PresentWeapon( bool showViewModel ) { //showViewModel is "ui_showGun". We want to make everything to work in 3th person even if this is false
	playerViewOrigin = owner->firstPersonViewOrigin;
	playerViewAxis = owner->firstPersonViewAxis;

	/*
	// calculate weapon position based on player movement bobbing
	owner->CalculateViewWeaponPos( viewWeaponOrigin, viewWeaponAxis ); //no: now weapon is bound to joint

	// hide offset is for dropping the gun when approaching a GUI or NPC //no: now weapon is bound to joint
	// This is simpler to manage than doing the weapon put-away animation
	if ( gameLocal.time - hideStartTime < hideTime ) {
		float frac = ( float )( gameLocal.time - hideStartTime ) / ( float )hideTime;
		if ( hideStart < hideEnd ) {
			frac = 1.0f - frac;
			frac = 1.0f - frac * frac;
		} else {
			frac = frac * frac;
		}
		hideOffset = hideStart + ( hideEnd - hideStart ) * frac;
	} else {
		hideOffset = hideEnd;
		if ( hide && disabled ) {
			Hide();
		}
	}
	viewWeaponOrigin += hideOffset * viewWeaponAxis[ 2 ];
	*/

	// kick up based on repeat firing //no: now weapon is bound to joint
	//MuzzleRise( viewWeaponOrigin, viewWeaponAxis );

	/*
	// set the physics position and orientation //no: now weapon is bound to joint
	GetPhysics()->SetOrigin( viewWeaponOrigin );
	GetPhysics()->SetAxis( viewWeaponAxis );
	*/
	//UpdateVisuals(); //not needed now?

	// update the weapon script
	UpdateScript();

	UpdateGUI();

	// update animation
	UpdateAnimation();

	Present(); //always show the model

	//ivan - allow shadows
	renderEntity.suppressShadowInViewID	= 0;

	if ( nozzleFx ) {
		UpdateNozzleFx();
	}

	// muzzle smoke
	if ( !disabled && weaponSmoke && ( weaponSmokeStartTime != 0 ) ) { //ivan - showViewModel commented out
		// use the barrel joint if available

		if ( barrelJointWorld != INVALID_JOINT ) { //ivan - compare with INVALID_JOINT !
			GetGlobalJointTransform( barrelJointWorld, muzzleOrigin, muzzleAxis ); 
		} else {
			// default to going straight out the view
			muzzleOrigin = playerViewOrigin;
			muzzleAxis = playerViewAxis;
		}
		// spit out a particle
		if ( !gameLocal.smokeParticles->EmitSmoke( weaponSmoke, weaponSmokeStartTime, gameLocal.random.RandomFloat(), muzzleOrigin, muzzleAxis ) ) {
			weaponSmokeStartTime = ( continuousSmoke ) ? gameLocal.time : 0;
		}
	}

	if ( strikeSmoke && strikeSmokeStartTime != 0 ) { //ivan - showViewModel commented out
		// spit out a particle
		if ( !gameLocal.smokeParticles->EmitSmoke( strikeSmoke, strikeSmokeStartTime, gameLocal.random.RandomFloat(), strikePos, strikeAxis ) ) {
			strikeSmokeStartTime = 0;
		}
	}

#ifdef _DENTONMOD

	//ivan start
	//disabled flag ensures that fx wont be played when entering cinematic
	if ( !disabled ) { 
		UpdateWeaponFx();
	}
	//ivan end

#endif //_DENTONMOD

	// remove the muzzle flash light when it's done
	if ( ( !lightOn && ( gameLocal.time >= muzzleFlashEnd ) ) || IsHidden() ) {
		if ( worldMuzzleFlashHandle != -1 ) {
			gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
			worldMuzzleFlashHandle = -1;
		}
	}

	// update the muzzle flash light, so it moves with the gun
	if ( worldMuzzleFlashHandle != -1 ) {
		UpdateFlashPosition();
		gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );

		// wake up monsters with the flashlight
		if ( !gameLocal.isMultiplayer && lightOn && !owner->fl.notarget ) {
			AlertMonsters();
		}
	}

	if ( status != WP_READY && sndHum ) {
		StopSound( SND_CHANNEL_BODY, false );
	}

	UpdateSound();
}

/*
================
idWeapon::EnterCinematic
================
*/
void idWeapon::EnterCinematic( void ) {
	StopSound( SND_CHANNEL_ANY, false );
#ifdef _DENTONMOD
	StopWeaponFx();
#endif

	if ( isLinked ) {
		SetState( "EnterCinematic", 0 );
		thread->Execute();

		WEAPON_ATTACK		= false;
		WEAPON_SPECIAL		= false;	//	new
		WEAPON_SPECIAL_HOLD	= false;	//	new
		WEAPON_RELOAD		= false;
		WEAPON_NETRELOAD	= false;
		WEAPON_NETENDRELOAD	= false;
		WEAPON_NETFIRING	= false;
		WEAPON_RAISEWEAPON	= false;
		WEAPON_LOWERWEAPON	= false;
	}

	disabled = true;

	LowerWeapon();
}

/*
================
idWeapon::ExitCinematic
================
*/
void idWeapon::ExitCinematic( void ) {
	disabled = false;

	if ( isLinked ) {
		SetState( "ExitCinematic", 0 );
		thread->Execute();
	}

	RaiseWeapon();
}

/*
================
idWeapon::NetCatchup
================
*/
void idWeapon::NetCatchup( void ) {
	if ( isLinked ) {
		SetState( "NetCatchup", 0 );
		thread->Execute();
	}
}

/*
================
idWeapon::GetZoomFov
================
*/
int	idWeapon::GetZoomFov( void ) {
	return zoomFov;
}

/*
================
idWeapon::GetWeaponAngleOffsets
================
*/
void idWeapon::GetWeaponAngleOffsets( int *average, float *scale, float *max ) {
	*average = weaponAngleOffsetAverages;
	*scale = weaponAngleOffsetScale;
	*max = weaponAngleOffsetMax;
}

/*
================
idWeapon::GetWeaponTimeOffsets
================
*/
void idWeapon::GetWeaponTimeOffsets( float *time, float *scale ) {
	*time = weaponOffsetTime;
	*scale = weaponOffsetScale;
}


/***********************************************************************

Ammo

***********************************************************************/

/*
================
idWeapon::GetAmmoNumForName
================
*/
ammo_t idWeapon::GetAmmoNumForName( const char *ammoname ) {
	int num;
	const idDict *ammoDict;

	assert( ammoname );

	ammoDict = gameLocal.FindEntityDefDict( "ammo_types", false );
	if ( !ammoDict ) {
		gameLocal.Error( "Could not find entity definition for 'ammo_types'\n" );
	}

	if ( !ammoname[ 0 ] ) {
		return 0;
	}

	if ( !ammoDict->GetInt( ammoname, "-1", num ) ) {
		gameLocal.Error( "Unknown ammo type '%s'", ammoname );
	}

	if ( ( num < 0 ) || ( num >= AMMO_NUMTYPES ) ) {
		gameLocal.Error( "Ammo type '%s' value out of range.  Maximum ammo types is %d.\n", ammoname, AMMO_NUMTYPES );
	}

	return ( ammo_t )num;
}

/*
================
idWeapon::GetAmmoNameForNum
================
*/
const char *idWeapon::GetAmmoNameForNum( ammo_t ammonum ) {
	int i;
	int num;
	const idDict *ammoDict;
	const idKeyValue *kv;
	char text[ 32 ];

	ammoDict = gameLocal.FindEntityDefDict( "ammo_types", false );
	if ( !ammoDict ) {
		gameLocal.Error( "Could not find entity definition for 'ammo_types'\n" );
	}

	sprintf( text, "%d", ammonum );

	num = ammoDict->GetNumKeyVals();
	for( i = 0; i < num; i++ ) {
		kv = ammoDict->GetKeyVal( i );
		if ( kv->GetValue() == text ) {
			return kv->GetKey();
		}
	}

	return NULL;
}

/*
================
idWeapon::GetAmmoPickupNameForNum
================
*/
const char *idWeapon::GetAmmoPickupNameForNum( ammo_t ammonum ) {
	int i;
	int num;
	const idDict *ammoDict;
	const idKeyValue *kv;

	ammoDict = gameLocal.FindEntityDefDict( "ammo_names", false );
	if ( !ammoDict ) {
		gameLocal.Error( "Could not find entity definition for 'ammo_names'\n" );
	}

	const char *name = GetAmmoNameForNum( ammonum );

	if ( name && *name ) {
		num = ammoDict->GetNumKeyVals();
		for( i = 0; i < num; i++ ) {
			kv = ammoDict->GetKeyVal( i );
			if ( idStr::Icmp( kv->GetKey(), name) == 0 ) {
				return kv->GetValue();
			}
		}
	}

	return "";
}

/*
================
idWeapon::AmmoAvailable
================
*/
int idWeapon::AmmoAvailable( void ) const {
	if ( owner ) {
		//ivan - this was wrong! everyone expects this being the real amount of ammo avalable, including clip!
		return owner->inventory.HasAmmo( ammoType, ammoRequired );
		//return owner->inventory.HasAmmo( ammoType, ammoRequired ) + ( ammoClip > 0 ? ammoClip : 0 );
	} else {
		return 0;
	}
}

/*
================
idWeapon::AmmoInClip
================
*/
int idWeapon::AmmoInClip( void ) const {
	return ammoClip;
}

//ivan start
/*
================
idWeapon::GetMaxAmmo
================
*/
int idWeapon::GetMaxAmmo( void ) const {
	return maxAmmo;
}
//ivan end

/*
================
idWeapon::ResetAmmoClip
================
*/
void idWeapon::ResetAmmoClip( void ) {
	ammoClip = -1;
}

/*
================
idWeapon::GetAmmoType
================
*/
ammo_t idWeapon::GetAmmoType( void ) const {
	return ammoType;
}

/*
================
idWeapon::ClipSize
================
*/
int	idWeapon::ClipSize( void ) const {
	return clipSize;
}

/*
================
idWeapon::LowAmmo
================
*/
int	idWeapon::LowAmmo() const {
	return lowAmmo;
}

/*
================
idWeapon::AmmoRequired
================
*/
int	idWeapon::AmmoRequired( void ) const {
	return ammoRequired;
}

/*
================
idWeapon::WriteToSnapshot
================
*/
void idWeapon::WriteToSnapshot( idBitMsgDelta &msg ) const {
	msg.WriteBits( ammoClip, ASYNC_PLAYER_INV_CLIP_BITS );
	//msg.WriteBits( worldModel.GetSpawnId(), 32 );
	msg.WriteBits( lightOn, 1 );
	msg.WriteBits( isFiring ? 1 : 0, 1 );
}

/*
================
idWeapon::ReadFromSnapshot
================
*/
void idWeapon::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	ammoClip = msg.ReadBits( ASYNC_PLAYER_INV_CLIP_BITS );
	//worldModel.SetSpawnId( msg.ReadBits( 32 ) );
	bool snapLight = msg.ReadBits( 1 ) != 0;
	isFiring = msg.ReadBits( 1 ) != 0;

	// WEAPON_NETFIRING is only turned on for other clients we're predicting. not for local client
	if ( owner && gameLocal.localClientNum != owner->entityNumber && WEAPON_NETFIRING.IsLinked() ) {

		// immediately go to the firing state so we don't skip fire animations
		if ( !WEAPON_NETFIRING && isFiring ) {
			idealState = "Fire";
		}

		// immediately switch back to idle
		if ( WEAPON_NETFIRING && !isFiring ) {
			idealState = "Idle";
		}

		WEAPON_NETFIRING = isFiring;
	}

	if ( snapLight != lightOn ) {
		Reload();
	}
}

/*
================
idWeapon::ClientReceiveEvent
================
*/
bool idWeapon::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {

	switch( event ) {
		case EVENT_RELOAD: {
			if ( gameLocal.time - time < 1000 ) {
				if ( WEAPON_NETRELOAD.IsLinked() ) {
					WEAPON_NETRELOAD = true;
					WEAPON_NETENDRELOAD = false;
				}
			}
			return true;
		}
		case EVENT_ENDRELOAD: {
			if ( WEAPON_NETENDRELOAD.IsLinked() ) {
				WEAPON_NETENDRELOAD = true;
			}
			return true;
		}
		case EVENT_CHANGESKIN: {
			int index = gameLocal.ClientRemapDecl( DECL_SKIN, msg.ReadLong() );
			renderEntity.customSkin = ( index != -1 ) ? static_cast<const idDeclSkin *>( declManager->DeclByIndex( DECL_SKIN, index ) ) : NULL;
			UpdateVisuals();
            /*
			if ( worldModel.GetEntity() ) {
				worldModel.GetEntity()->SetSkin( renderEntity.customSkin );
			}
            */
			return true;
		}
		default:
			break;
	}

	return idEntity::ClientReceiveEvent( event, time, msg );
}

/***********************************************************************

	Script events

***********************************************************************/

/*
===============
idWeapon::Event_Clear
===============
*/
void idWeapon::Event_Clear( void ) {
	Clear();
}

/*
===============
idWeapon::Event_GetOwner
===============
*/
void idWeapon::Event_GetOwner( void ) {
	idThread::ReturnEntity( owner );
}

/*
===============
idWeapon::Event_WeaponState
===============
*/
void idWeapon::Event_WeaponState( const char *statename, int blendFrames ) {
	const function_t *func;

	func = scriptObject.GetFunction( statename );
	if ( !func ) {
		assert( 0 );
		gameLocal.Error( "Can't find function '%s' in object '%s'", statename, scriptObject.GetTypeName() );
	}

	idealState = statename;

	if ( !idealState.Icmp( "Fire" ) ) {
		isFiring = true;
	} else {
		isFiring = false;
	}

	animBlendFrames = blendFrames;
	thread->DoneProcessing();
}

/*
===============
idWeapon::Event_WeaponReady
===============
*/
void idWeapon::Event_WeaponReady( void ) {
	status = WP_READY;
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = false;
	}
	if ( sndHum ) {
		StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
	}

}

/*
===============
idWeapon::Event_WeaponOutOfAmmo
===============
*/
void idWeapon::Event_WeaponOutOfAmmo( void ) {
	status = WP_OUTOFAMMO;
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = false;
	}
}

/*
===============
idWeapon::Event_WeaponReloading
===============
*/
void idWeapon::Event_WeaponReloading( void ) {
	status = WP_RELOAD;
}

/*
===============
idWeapon::Event_WeaponHolstered
===============
*/
void idWeapon::Event_WeaponHolstered( void ) {
	status = WP_HOLSTERED;
	if ( isLinked ) {
		WEAPON_LOWERWEAPON = false;
	}
}

/*
===============
idWeapon::Event_WeaponRising
===============
*/
void idWeapon::Event_WeaponRising( void ) {
	status = WP_RISING;
	if ( isLinked ) {
		WEAPON_LOWERWEAPON = false;
	}
	owner->WeaponRisingCallback();
}

/*
===============
idWeapon::Event_WeaponLowering
===============
*/
void idWeapon::Event_WeaponLowering( void ) {
	status = WP_LOWERING;
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = false;
	}
	owner->WeaponLoweringCallback();
}

/*
===============
idWeapon::Event_UseAmmo
===============
*/
void idWeapon::Event_UseAmmo( int amount ) {
	if ( gameLocal.isClient ) {
		return;
	}

	owner->inventory.UseAmmo( ammoType, ( powerAmmo ) ? amount : ( amount * ammoRequired ) );
	if ( clipSize && ammoRequired ) {
		ammoClip -= powerAmmo ? amount : ( amount * ammoRequired );
		if ( ammoClip < 0 ) {
			ammoClip = 0;
		}
	}
}

/*
===============
idWeapon::Event_AddToClip
===============
*/
void idWeapon::Event_AddToClip( int amount ) {
	int ammoAvail;

	if ( gameLocal.isClient ) {
		return;
	}

	/*
	//ivan - commented out
	int oldAmmo = ammoClip;

	ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
	ammoClip += amount;
	if ( ammoClip > clipSize ) {
		ammoClip = clipSize;
	}

	if ( ammoClip > ammoAvail ) {
		ammoClip = ammoAvail;
	}

	// for shared ammo we need to use the ammo when it is moved into the clip
	int usedAmmo = ammoClip - oldAmmo;
	owner->inventory.UseAmmo(ammoType, usedAmmo);
	*/

	ammoClip += amount;
	if ( ammoClip > clipSize ) {
		ammoClip = clipSize;
	}

	ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
	if ( ammoClip > ammoAvail ) {
		ammoClip = ammoAvail;
	}

	//ivan end
}

/*
===============
idWeapon::Event_AmmoInClip
===============
*/
void idWeapon::Event_AmmoInClip( void ) {
	int ammo = AmmoInClip();
	idThread::ReturnFloat( ammo );
}


/*
===============
idWeapon::Event_AmmoAvailable
===============
*/
void idWeapon::Event_AmmoAvailable( void ) {

	int ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
	//ivan - out: ammoAvail += AmmoInClip();

	idThread::ReturnFloat( ammoAvail );
}

/*
===============
idWeapon::Event_TotalAmmoCount
===============
*/
void idWeapon::Event_TotalAmmoCount( void ) {
	int ammoAvail = owner->inventory.HasAmmo( ammoType, 1 );
	idThread::ReturnFloat( ammoAvail );
}

/*
===============
idWeapon::Event_ClipSize
===============
*/
void idWeapon::Event_ClipSize( void ) {
	idThread::ReturnFloat( clipSize );
}

/*
===============
idWeapon::Event_AutoReload
===============
*/
void idWeapon::Event_AutoReload( void ) {
	assert( owner );
	if ( gameLocal.isClient ) {
		idThread::ReturnFloat( 0.0f );
		return;
	}
	idThread::ReturnFloat( gameLocal.userInfo[ owner->entityNumber ].GetBool( "ui_autoReload" ) );
}

/*
===============
idWeapon::Event_NetReload
===============
*/
void idWeapon::Event_NetReload( void ) {
	assert( owner );
	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_RELOAD, NULL, false, -1 );
	}
}

/*
===============
idWeapon::Event_NetEndReload
===============
*/
void idWeapon::Event_NetEndReload( void ) {
	assert( owner );
	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_ENDRELOAD, NULL, false, -1 );
	}
}

/*
===============
idWeapon::Event_PlayAnim
===============
*/
void idWeapon::Event_PlayAnim( int channel, const char *animname ) {
	int anim;

	anim = animator.GetAnim( animname );
	if ( !anim ) {
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName() );
		animator.Clear( channel, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = 0;
	} else {
		if ( !( owner && owner->GetInfluenceLevel() ) ) {
			Show();
		}
		animator.PlayAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = animator.CurrentAnim( channel )->GetEndTime();
	}
	animBlendFrames = 0;
	idThread::ReturnInt( 0 );
}

/*
===============
idWeapon::Event_PlayCycle
===============
*/
void idWeapon::Event_PlayCycle( int channel, const char *animname ) {
	int anim;

	anim = animator.GetAnim( animname );
	if ( !anim ) {
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName() );
		animator.Clear( channel, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = 0;
	} else {
		if ( !( owner && owner->GetInfluenceLevel() ) ) {
			Show();
		}
		animator.CycleAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = animator.CurrentAnim( channel )->GetEndTime();
	}
	animBlendFrames = 0;
	idThread::ReturnInt( 0 );
}

/*
===============
idWeapon::Event_AnimDone
===============
*/
void idWeapon::Event_AnimDone( int channel, int blendFrames ) {
	if ( animDoneTime - FRAME2MS( blendFrames ) <= gameLocal.time ) {
		idThread::ReturnInt( true );
	} else {
		idThread::ReturnInt( false );
	}
}

/*
===============
idWeapon::Event_SetBlendFrames
===============
*/
void idWeapon::Event_SetBlendFrames( int channel, int blendFrames ) {
	animBlendFrames = blendFrames;
}

/*
===============
idWeapon::Event_GetBlendFrames
===============
*/
void idWeapon::Event_GetBlendFrames( int channel ) {
	idThread::ReturnInt( animBlendFrames );
}

/*
================
idWeapon::Event_Next
================
*/
void idWeapon::Event_Next( void ) {
	// change to another weapon if possible
	owner->NextBestWeapon();
}

/*
================
idWeapon::Event_SetSkin
================
*/
void idWeapon::Event_SetSkin( const char *skinname ) {
	const idDeclSkin *skinDecl;

	if ( !skinname || !skinname[ 0 ] ) {
		skinDecl = NULL;
	} else {
		skinDecl = declManager->FindSkin( skinname );
	}

	renderEntity.customSkin = skinDecl;
	UpdateVisuals();

	if ( gameLocal.isServer ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteLong( ( skinDecl != NULL ) ? gameLocal.ServerRemapDecl( -1, DECL_SKIN, skinDecl->Index() ) : -1 );
		ServerSendEvent( EVENT_CHANGESKIN, &msg, false, -1 );
	}
}

/*
================
idWeapon::Event_Flashlight
================
*/
void idWeapon::Event_Flashlight( int enable ) {
	if ( enable ) {
		lightOn = true;
		MuzzleFlashLight();
	} else {
		lightOn = false;
		muzzleFlashEnd = 0;
	}
}

/*
================
idWeapon::Event_GetLightParm
================
*/
void idWeapon::Event_GetLightParm( int parmnum ) {
	if ( ( parmnum < 0 ) || ( parmnum >= MAX_ENTITY_SHADER_PARMS ) ) {
		gameLocal.Error( "shader parm index (%d) out of range", parmnum );
	}

	idThread::ReturnFloat( worldMuzzleFlash.shaderParms[ parmnum ] );
}

/*
================
idWeapon::Event_SetLightParm
================
*/
void idWeapon::Event_SetLightParm( int parmnum, float value ) {
	if ( ( parmnum < 0 ) || ( parmnum >= MAX_ENTITY_SHADER_PARMS ) ) {
		gameLocal.Error( "shader parm index (%d) out of range", parmnum );
	}

	worldMuzzleFlash.shaderParms[ parmnum ]	= value;
	UpdateVisuals();
}

/*
================
idWeapon::Event_SetLightParms
================
*/
void idWeapon::Event_SetLightParms( float parm0, float parm1, float parm2, float parm3 ) {
	worldMuzzleFlash.shaderParms[ SHADERPARM_RED ]		= parm0;
	worldMuzzleFlash.shaderParms[ SHADERPARM_GREEN ]	= parm1;
	worldMuzzleFlash.shaderParms[ SHADERPARM_BLUE ]		= parm2;
	worldMuzzleFlash.shaderParms[ SHADERPARM_ALPHA ]	= parm3;

	UpdateVisuals();
}

/*
================
idWeapon::Event_CreateProjectile
================
*/
void idWeapon::Event_CreateProjectile( void ) {
	if ( !gameLocal.isClient ) {
		projectileEnt = NULL;
		gameLocal.SpawnEntityDef( projectileDict, &projectileEnt, false );
		if ( projectileEnt ) {
			projectileEnt->SetOrigin( GetPhysics()->GetOrigin() );
			projectileEnt->Bind( owner, false );
			projectileEnt->Hide();
		}
		idThread::ReturnEntity( projectileEnt );
	} else {
		idThread::ReturnEntity( NULL );
	}
}

/*
================
idWeapon::Event_LaunchProjectiles

Modified & rearranged by Clone JCD for barrel launched Projectiles to hit the target accurately.
================
*/
void idWeapon::Event_LaunchProjectiles( int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower ) {
	idProjectile	*proj;
	idEntity		*ent;
	int				i;
	idVec3			dir;
	float			ang;
	float			spin;
	trace_t			tr;
	bool			barrelLaunch;
	bool			tracer, beam;

	if ( IsHidden() ) {
		return;
	}

	if ( !projectileDict.GetNumKeyVals() ) {
		const char *classname = weaponDef->dict.GetString( "classname" );
		gameLocal.Warning( "No projectile defined on '%s'", classname );
		return;
	}

	// avoid all ammo considerations on an MP client
	if ( !gameLocal.isClient ) {

		//ivan start

		/*
		int ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
		if ( ( clipSize != 0 ) && ( ammoClip <= 0 ) ) {
			return;
		}
		*/
		// check if we're out of ammo or the clip is empty
		int ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
		if ( !ammoAvail || ( ( clipSize != 0 ) && ( ammoClip <= 0 ) ) ) {
			return;
		}

		//ivan end

		// if this is a power ammo weapon ( currently only the bfg ) then make sure
		// we only fire as much power as available in each clip
		if ( powerAmmo ) {
			// power comes in as a float from zero to max
			// if we use this on more than the bfg will need to define the max
			// in the .def as opposed to just in the script so proper calcs
			// can be done here.
			dmgPower = ( int )dmgPower + 1;
			if ( dmgPower > ammoClip ) {
				dmgPower = ammoClip;
			}
		}

		if(clipSize == 0) {
			//Weapons with a clip size of 0 launch straight from inventory without moving to a clip
			//Now we used the ammo when the ammo was moved into the clip so we don't want to
			//use it now.
			owner->inventory.UseAmmo( ammoType, ( powerAmmo ) ? dmgPower : ammoRequired );

		}

		if ( clipSize && ammoRequired ) {
			ammoClip -= powerAmmo ? dmgPower : ammoRequired;
		}

	}

	if ( !silent_fire ) {
		// wake up nearby monsters
		gameLocal.AlertAI( owner );
	}

	// set the shader parm to the time of last projectile firing,
	// which the gun material shaders can reference for single shot barrel glows, etc
	renderEntity.shaderParms[ SHADERPARM_DIVERSITY ]	= gameLocal.random.CRandomFloat();
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.realClientTime );

	// predict instant hit projectiles
	const bool isPrediction = gameLocal.isClient && projectileDict.GetBool( "net_instanthit" );

	const float spreadRad = DEG2RAD( spread );

	idVec3 view_pos = playerViewOrigin + playerViewAxis[ 0 ] * 2.0f; // Muzzle pos for translation clip model only-- For barrel Launched projectiles
	idVec3 muzzle_pos;

	float muzzleDistFromView;
	float traceDist, muzzleToTargetDist;
	idVec3 muzzleDir;

	beam			= projectileDict.GetFloat( "fuse" ) <= 0 || projectileDict.GetBool( "rail_beam");
	tracer			= !beam && projectileDict.GetBool( "tracers" ) && (projectileDict.GetFloat("tracer_probability", "1.0") > gameLocal.random.RandomFloat()); 
	barrelLaunch	= projectileDict.GetBool( "launchFromBarrel" );

	if ( barrelLaunch || tracer || beam ) {
		// calculate the muzzle position
		if ( barrelJointWorld != INVALID_JOINT ) { //ivan
			// there is an explicit joint for the muzzle
			GetGlobalJointTransform( barrelJointWorld, muzzleOrigin, muzzleAxis ); 
			if ( barrelLaunch ) {
				tracer			= false;
			}
			muzzle_pos = muzzleOrigin; // + playerViewAxis[ 0 ] * 2.0f;	// muzzle_pos for multiplayer prediction
																		// as well as for launching the projectiles		
			// muzzleDistFromView = (muzzle_pos - view_pos).Length( ) * 3.5f;
			muzzleDistFromView = (muzzle_pos - view_pos).LengthSqr( ) * 3.5f; // This is faster
		} else { // if we dont find a proper bone then cancel all the effects.
			barrelLaunch = false;
			tracer = false;
			beam = false;
		}
	}
	
	idVec3 &launch_pos = view_pos;
	const float tracer_speed = projectileDict.GetFloat( "tracer_speed", "0.0f" ); 

	//ivan start - fire modes - setup
	int firemodeCounter = 0;
	int firemodeCounterPos = 0;
	bool odd_proj_num = (num_projectiles%2 != 0); //dispari
	idVec3 updown_offset;
	updown_offset.Zero();

	if( fireMode == WP_FIREMODE_2D_STEP_SPREAD){ 
		if(num_projectiles > 1){				//Example: spread = 90, num projs = 5
			ang = 2*spread/(num_projectiles-1); //Spread step: 2* 90 /(5-1) = 45 degrees
			ang = ang/180.0f;					//normalized from 0 to 1: 45/180 = 0.5 --> % of 180 degrees to use: 0, 0.25, -0.25, 0.5, -0.5
		}
	} 
	//ivan end

	for( i = 0; i < num_projectiles; i++ ) {
		
		//ivan start - fire modes - direction
		if( fireMode == WP_FIREMODE_DEFAULT){
			ang = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
			spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
			dir = playerViewAxis[ 0 ] + playerViewAxis[ 2 ] * ( ang * idMath::Sin( spin ) ) - playerViewAxis[ 1 ] * ( ang * idMath::Cos( spin ) );
		
		}else if( fireMode == WP_FIREMODE_2D_STEP_SPREAD){
			dir = (playerViewAxis[ 0 ] * (1-ang*firemodeCounterPos) ) + ( playerViewAxis[ 2 ] * ang*firemodeCounter );
			//upd counter
			if(firemodeCounter >= 0){
				firemodeCounter++;
				firemodeCounterPos = firemodeCounter;
			}
			firemodeCounter = - firemodeCounter;
		}else if( fireMode == WP_FIREMODE_2D_PARALLEL_SPREAD){ 
			dir = playerViewAxis[ 0 ];

			//upd counter before if even number of projs
			if( !odd_proj_num && firemodeCounter >= 0){ firemodeCounter++; }

			updown_offset = playerViewAxis[ 2 ]*spread*firemodeCounter; 
			
			//upd counter after if odd number of projs
			if( odd_proj_num && firemodeCounter >= 0){ firemodeCounter++; }

			firemodeCounter = - firemodeCounter;
		}else if( fireMode == WP_FIREMODE_2D_RANDOM_SPREAD){ 
			ang = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
			spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
			dir = playerViewAxis[ 0 ] + playerViewAxis[ 2 ] * ( ang * idMath::Sin( spin ) );
		}else{
			gameLocal.Error("Unknown WEAPON fire mode: %d", fireMode );
		}
		//ivan test end


		dir.Normalize();
		
		if ( barrelLaunch || tracer || beam ) { // Do not execute this part unless projectile is barrel launched or has a tracer effect.

			//ivan start
			gameLocal.clip.Translation( tr, view_pos, view_pos + dir * 4096.0f, NULL, mat3_identity, MASK_SHOT_RENDERMODEL, owner );
			traceDist = (tr.endpos - view_pos).LengthSqr(); 
			//gameRenderWorld->DebugLine( colorRed, view_pos, tr.endpos, 1000 );

			//fix - tracer still need muzzleToTargetDist
			if ( tracer ) { 
				muzzleDir = tr.endpos - muzzle_pos;
				muzzleToTargetDist = muzzleDir.LengthSqr(); 
				muzzleDir.Normalize();
			}
			//fix end
			
			//ivan end
		}

		if ( isPrediction ) {
			if ( tr.fraction < 1.0f ) {
				if ( barrelLaunch ) {	//a new trace should be made for multiplayer prediction of barrel launched projectiles
					gameLocal.clip.Translation( tr, muzzle_pos, muzzle_pos + dir * 4096.0f, NULL, mat3_identity, MASK_SHOT_RENDERMODEL, owner ); 
				}
				else
				{
					gameLocal.clip.Translation( tr, view_pos, view_pos + dir * 4096.0f, NULL, mat3_identity, MASK_SHOT_RENDERMODEL, owner );
				}
				idProjectile::ClientPredictionCollide( this, projectileDict, tr, vec3_origin, true );
			}
		}
		else {

			if ( projectileEnt ) {
				ent = projectileEnt;
				ent->Show();
				ent->Unbind();
				projectileEnt = NULL;
			} else {
				gameLocal.SpawnEntityDef( projectileDict, &ent, false );
			}

			if ( !ent || !ent->IsType( idProjectile::Type ) ) {
				const char *projectileName = weaponDef->dict.GetString( "def_projectile" );
				gameLocal.Error( "'%s' is not an idProjectile", projectileName );
			}

			if ( projectileDict.GetBool( "net_instanthit" ) ) {
				// don't synchronize this on top of the already predicted effect
				ent->fl.networkSync = false;
			}
			
			if ( barrelLaunch ){
				launch_pos = muzzle_pos;		
			}
			proj = static_cast<idProjectile *>(ent);
			proj->Create( owner, launch_pos, dir );

#ifdef _DENTONMOD
			if( tracer ) { 
		/*		if ( traceDist <= muzzleToTargetDist ) // Ideally, this should never happen
					gameLocal.Printf ( " Unpredicted traceDistance in idWeapon::Event_LaunchProjectiles " );
		*/
				bool beamTracer = (projectileDict.GetString( "beam_skin", NULL ) != NULL);

				if ( tracer_speed != 0.0f ) {
					if( beamTracer ) { // Check whether it's a beamTracer
						proj->setTracerEffect( new dnBeamSpeedTracer(proj, tracer_speed, muzzleToTargetDist * idMath::RSqrt( muzzleToTargetDist ), muzzle_pos, muzzleDir.ToMat3()) );
					}
					else {
						proj->setTracerEffect( new dnSpeedTracer(proj, tracer_speed, muzzleToTargetDist * idMath::RSqrt( muzzleToTargetDist ), muzzle_pos, muzzleDir.ToMat3()) );
					}
				} 
				else {
					if( beamTracer ) {
						proj->setTracerEffect( new dnBeamTracer(proj, traceDist/muzzleToTargetDist, view_pos, muzzle_pos, muzzleDir.ToMat3()) );
					}
					else {
						proj->setTracerEffect( new dnTracer(proj, traceDist/muzzleToTargetDist, view_pos, muzzle_pos, muzzleDir.ToMat3()) );
					}
				}
			}
			else if( beam ) {
				proj->setTracerEffect( new dnRailBeam(proj, muzzleOrigin) );
			}
#endif
			//ivan start - fire modes - position offset
			//proj->Launch( launch_pos, dir, pushVelocity, fuseOffset, launchPower, dmgPower );
			proj->Launch( launch_pos + updown_offset, dir, pushVelocity, fuseOffset, launchPower, dmgPower );
			//ivan test end
		}
	}


	if ( !gameLocal.isClient ) {
		// toss the brass
		if (brassDelay >= 0)  // eject brass behaviour can be disabled by simply setting the delay to 0
			PostEventMS( &EV_Weapon_EjectBrass, brassDelay );
	}

	// add the light for the muzzleflash
	if ( !lightOn ) {
		MuzzleFlashLight();
	}

	owner->WeaponFireFeedback( &weaponDef->dict );

	// reset muzzle smoke
	weaponSmokeStartTime = gameLocal.realClientTime;
}

//----------------New Start
#ifdef _DENTONMOD

bool idWeapon::ChangeProjectileDef( int number ) {

	if( projectileEnt != NULL ) {
		gameLocal.Printf("Projectile Entity exists \n ");
		return false;
	}
	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_projectile", NULL);

	for (int i=0; kv && i < number; i++ ) {
		kv = spawnArgs.MatchPrefix( "def_projectile", kv);
	}

	if (kv == NULL) {
		return false;
	}

	const idDeclEntityDef *projectileDef = gameLocal.FindEntityDef( kv->GetValue(), false );
	if ( projectileDef ) {
		const char *spawnclass = projectileDef->dict.GetString( "spawnclass" );
		idTypeInfo *cls = idClass::GetClass( spawnclass );
		if ( !cls || !cls->IsType( idProjectile::Type ) ) {
			gameLocal.Warning( "Invalid spawnclass in Event_ChangeProjectileDef.\n" );
		} else {
			projectileDict = projectileDef->dict;
			owner->SetProjectileType( number );
			return true;
		}
	}
	return false;
}

void idWeapon::Event_ChangeProjectileDef( int number ) {
	
	if( number == owner->GetProjectileType() ) {
		idThread::ReturnFloat( 1 );
	}

	idThread::ReturnFloat( ChangeProjectileDef(number) ? 1 : 0 );	
}

void idWeapon::Event_GetProjectileType( void ) {
	idThread::ReturnFloat( owner->GetProjectileType() );	
}

void idWeapon::Event_StartWeaponParticle( const char* prtName) {
	WeaponParticle_t* part;
	weaponParticles.Get(prtName, &part);

	if(part) {

		part->particleFlags.isActive = true;
		if( part->particleFlags.isSmoke ) {
			part->startTime = gameLocal.time;
		} else {
			if( part->modelDefHandle > -1 ) {
				gameRenderWorld->FreeEntityDef( part->modelDefHandle );
				part->modelDefHandle = -1;
			}
			//part->renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
			//part->renderEntity.shaderParms[ SHADERPARM_DIVERSITY ]	= gameLocal.random.RandomFloat(); // For effects like muzzleflashes etc.
		}
	}
}
void idWeapon::Event_StopWeaponParticle( const char* prtName) {
	WeaponParticle_t* part;
	weaponParticles.Get(prtName, &part);
	if(part) {
		part->particleFlags.isActive = false;
		part->startTime = 0;

		//Free the particles
		if(!part->particleFlags.isSmoke && part->modelDefHandle >= 0) {
			gameRenderWorld->FreeEntityDef( part->modelDefHandle );
			part->modelDefHandle = -1;
		}
	}
}

void idWeapon::Event_StartWeaponLight( const char* lightName) {
	WeaponLight_t* light;
	weaponLights.Get(lightName, &light);
	if( light ) {
		light->lightFlags.isActive = true;
		light->startTime = gameLocal.time;

		// these will be different each fire
		light->light.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
		light->light.shaderParms[ SHADERPARM_DIVERSITY ]	= renderEntity.shaderParms[ SHADERPARM_DIVERSITY ];

		if( !light->lightFlags.isAlwaysOn ){
			light->endTime += light->startTime;
		}
	}
}

void idWeapon::Event_StopWeaponLight( const char* lightName) {
	WeaponLight_t* light;
	weaponLights.Get(lightName, &light);
	if(light) {
		light->lightFlags.isActive = false;
		light->startTime = 0;
		if(light->lightHandle != -1) {
			gameRenderWorld->FreeLightDef( light->lightHandle );
			light->lightHandle = -1;
		}
	}
}
#endif //_DENTONMOD

//----------------- New End
/*
=====================
idWeapon::Event_Melee
=====================
*/
void idWeapon::Event_Melee( void ) {
	idEntity	*ent;
	trace_t		tr;

	if ( !meleeDef ) {
		gameLocal.Error( "No meleeDef on '%s'", weaponDef->dict.GetString( "classname" ) );
	}

	if ( !gameLocal.isClient ) {
		//ivan start
		idVec3 start, end;
		
		if( meleeDef->dict.GetBool( "useWeaponZdir" ) ){
			start = GetPhysics()->GetOrigin();
			end = start + GetPhysics()->GetAxis()[2] * ( meleeDistance * owner->PowerUpModifier( MELEE_DISTANCE ) );
		}else{
			start = playerViewOrigin; 
			end = start + playerViewAxis[0] * ( meleeDistance * owner->PowerUpModifier( MELEE_DISTANCE ) );
		}
		
		/*
		//was:
		idVec3 start = viewWeaponOrigin); 
		idVec3 end = start + playerViewAxis[0] * ( meleeDistance * owner->PowerUpModifier( MELEE_DISTANCE ) );
		gameLocal.clip.TracePoint( tr, start, end, MASK_SHOT_RENDERMODEL, owner );
		*/
		
		idBounds meleebox;
		meleebox.Zero();
		meleebox.ExpandSelf( 8.0f );
		gameLocal.clip.TraceBounds( tr, start, end, meleebox, MASK_SHOT_RENDERMODEL, owner );
		//ivan end
		if ( tr.fraction < 1.0f ) {
			ent = gameLocal.GetTraceEntity( tr );
		} else {
			ent = NULL;
		}

		if ( g_debugWeapon.GetBool() ) {
			gameRenderWorld->DebugLine( colorYellow, start, end, 100 );
			//ivan start
			gameRenderWorld->DebugBounds( colorBlue,meleebox, start, 100 );
			gameRenderWorld->DebugBounds( colorBlue,meleebox, end, 100 );
			//ivan end
			if ( ent ) {
				gameRenderWorld->DebugBounds( colorRed, ent->GetPhysics()->GetBounds(), ent->GetPhysics()->GetOrigin(), 100 );
			}
		}

		bool hit = false;
		const char *hitSound = meleeDef->dict.GetString( "snd_miss" );

		if ( ent ) {

			float push = meleeDef->dict.GetFloat( "push" );
			idVec3 impulse = -push * owner->PowerUpModifier( SPEED ) * tr.c.normal;

			if ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) && ( ent->IsType( idActor::Type ) || ent->IsType( idAFAttachment::Type) ) ) {
				idThread::ReturnInt( 0 );
				return;
			}

			ent->ApplyImpulse( this, tr.c.id, tr.c.point, impulse );

			// weapon stealing - do this before damaging so weapons are not dropped twice
			if ( gameLocal.isMultiplayer
				&& weaponDef && weaponDef->dict.GetBool( "stealing" )
				&& ent->IsType( idPlayer::Type )
				&& !owner->PowerUpActive( BERSERK )
				&& ( gameLocal.gameType != GAME_TDM || gameLocal.serverInfo.GetBool( "si_teamDamage" ) || ( owner->team != static_cast< idPlayer * >( ent )->team ) )
				) {
					owner->StealWeapon( static_cast< idPlayer * >( ent ) );
				}

				if ( ent->fl.takedamage ) {
					idVec3 kickDir, globalKickDir;
					meleeDef->dict.GetVector( "kickDir", "0 0 0", kickDir );
					globalKickDir = muzzleAxis * kickDir;
					ent->Damage( owner, owner, globalKickDir, meleeDefName, owner->PowerUpModifier( MELEE_DAMAGE ), tr.c.id );
					hit = true;
				}

				if ( weaponDef->dict.GetBool( "impact_damage_effect" ) ) {

					if ( ent->spawnArgs.GetBool( "bleed" ) ) {

						hitSound = meleeDef->dict.GetString( owner->PowerUpActive( BERSERK ) ? "snd_hit_berserk" : "snd_hit" );

						ent->AddDamageEffect( tr, impulse, meleeDef->dict.GetString( "classname" ) );

					} else {

						int type = tr.c.material->GetSurfaceType();
						if ( type == SURFTYPE_NONE ) {
							type = GetDefaultSurfaceType();
						}

						const char *materialType = gameLocal.sufaceTypeNames[ type ];

						// start impact sound based on material type
						hitSound = meleeDef->dict.GetString( va( "snd_%s", materialType ) );
						if ( *hitSound == '\0' ) {
							hitSound = meleeDef->dict.GetString( "snd_metal" );
						}

						if ( gameLocal.time > nextStrikeFx ) {
							const char *decal;
							// project decal
							decal = weaponDef->dict.GetString( "mtr_strike" );
							if ( decal && *decal ) {
								gameLocal.ProjectDecal( tr.c.point, -tr.c.normal, 8.0f, true, 6.0, decal );
							}
							nextStrikeFx = gameLocal.time + 200;
						} else {
							hitSound = "";
						}

						strikeSmokeStartTime = gameLocal.time;
						strikePos = tr.c.point;
						strikeAxis = -tr.endAxis;
					}
				}
		}

		if ( *hitSound != '\0' ) {
			const idSoundShader *snd = declManager->FindSound( hitSound );
			StartSoundShader( snd, SND_CHANNEL_BODY2, 0, true, NULL );
		}

		idThread::ReturnInt( hit );
		owner->WeaponFireFeedback( &weaponDef->dict );
		return;
	}

	idThread::ReturnInt( 0 );
	owner->WeaponFireFeedback( &weaponDef->dict );
}

/*
=====================
idWeapon::Event_GetWorldModel
=====================
*/
void idWeapon::Event_GetWorldModel( void ) {
	idThread::ReturnEntity( this ); //ivan - was: worldModel.GetEntity()
}

/*
=====================
idWeapon::Event_AllowDrop
=====================
*/
void idWeapon::Event_AllowDrop( int allow ) {
	if ( allow ) {
		allowDrop = true;
	} else {
		allowDrop = false;
	}
}

/*
================
idWeapon::Event_EjectBrass

Toss a shell model out from the breach if the bone is present
================
*/
void idWeapon::Event_EjectBrass( void ) {
	if ( !g_showBrass.GetBool() || !owner->CanShowWeaponViewmodel() ) {
		return;
	}

	if ( ejectJointWorld == INVALID_JOINT || !brassDict.GetNumKeyVals() ) { //was: ejectJointView
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	idMat3 axis;
	idVec3 origin, linear_velocity, angular_velocity;
	idEntity *ent;

	if ( !GetGlobalJointTransform( ejectJointWorld, origin, axis ) ) { //was: ejectJointView
		return;
	}

	gameLocal.SpawnEntityDef( brassDict, &ent, false );
	if ( !ent || !ent->IsType( idDebris::Type ) ) {
		gameLocal.Error( "'%s' is not an idDebris", weaponDef ? weaponDef->dict.GetString( "def_ejectBrass" ) : "def_ejectBrass" );
	}
	idDebris *debris = static_cast<idDebris *>(ent);
	debris->Create( owner, origin, axis );
	debris->Launch();

	if( spawnArgs.GetBool( "fixed_brass_dir" ) ) {
		linear_velocity = 40 * ( playerViewAxis[0] + playerViewAxis[1] + playerViewAxis[2] );
		angular_velocity.Set( 10 * gameLocal.random.CRandomFloat(), 10 * gameLocal.random.CRandomFloat(), 10 * gameLocal.random.CRandomFloat() );

		debris->GetPhysics()->SetLinearVelocity( linear_velocity );
		debris->GetPhysics()->SetAngularVelocity( angular_velocity );
	}
}

/*
===============
idWeapon::Event_IsInvisible
===============
*/
void idWeapon::Event_IsInvisible( void ) {
	if ( !owner ) {
		idThread::ReturnFloat( 0 );
		return;
	}
	idThread::ReturnFloat( owner->PowerUpActive( INVISIBILITY ) ? 1 : 0 );
}

// New------------------

#ifdef _DENTONMOD

/*
===============
idWeapon::Event_StartZoom
===============
*/
void idWeapon::Event_SetZoom( int status ) {	//New
	if( status ){
		owner->SetWeaponZoom( true );
	}
	else {
		owner->SetWeaponZoom( false );
	}
}
#endif// _DENTONMOD

/*
===============
idWeapon::ClientPredictionThink
===============
*/
void idWeapon::ClientPredictionThink( void ) {
	UpdateAnimation();
}

/*
================
idWeapon::AmmoCount

Returns the total number of rounds regardless of the required ammo  // new
================
*/
int idWeapon::AmmoCount() const {

	if ( owner ) {
		return owner->inventory.HasAmmo( ammoType, 1 );
	} else {
		return 0;
	}
}
