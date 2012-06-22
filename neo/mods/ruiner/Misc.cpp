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
/*

Various utility objects and functions.

*/

#include "sys/platform.h"
#include "renderer/ModelManager.h"

#include "gamesys/SysCvar.h"
#include "script/Script_Thread.h"
#include "ai/AI.h"
#include "Player.h"
#include "Camera.h"
#include "WorldSpawn.h"
#include "Moveable.h"
#include "SmokeParticles.h"

#include "Misc.h"

/*
===============================================================================

idSpawnableEntity

A simple, spawnable entity with a model and no functionable ability of it's own.
For example, it can be used as a placeholder during development, for marking
locations on maps for script, or for simple placed models without any behavior
that can be bound to other entities.  Should not be subclassed.
===============================================================================
*/

CLASS_DECLARATION( idEntity, idSpawnableEntity )
END_CLASS

/*
======================
idSpawnableEntity::Spawn
======================
*/
void idSpawnableEntity::Spawn() {
	// this just holds dict information
}

/*
===============================================================================

	idPlayerStart

===============================================================================
*/

const idEventDef EV_TeleportStage( "<TeleportStage>", "e" );

CLASS_DECLARATION( idEntity, idPlayerStart )
	EVENT( EV_Activate,			idPlayerStart::Event_TeleportPlayer )
	EVENT( EV_TeleportStage,	idPlayerStart::Event_TeleportStage )
END_CLASS

/*
===============
idPlayerStart::idPlayerStart
================
*/
idPlayerStart::idPlayerStart( void ) {
	teleportStage = 0;
}

/*
===============
idPlayerStart::Spawn
================
*/
void idPlayerStart::Spawn( void ) {
	teleportStage = 0;
}

/*
================
idPlayerStart::Save
================
*/
void idPlayerStart::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( teleportStage );
}

/*
================
idPlayerStart::Restore
================
*/
void idPlayerStart::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( teleportStage );
}

/*
================
idPlayerStart::ClientReceiveEvent
================
*/
bool idPlayerStart::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {
	int entityNumber;

	switch( event ) {
		case EVENT_TELEPORTPLAYER: {
			entityNumber = msg.ReadBits( GENTITYNUM_BITS );
			idPlayer *player = static_cast<idPlayer *>( gameLocal.entities[entityNumber] );
			if ( player != NULL && player->IsType( idPlayer::Type ) ) {
				Event_TeleportPlayer( player );
			}
			return true;
		}
		default:
			break;
	}

	return idEntity::ClientReceiveEvent( event, time, msg );
}

/*
===============
idPlayerStart::Event_TeleportStage

FIXME: add functionality to fx system ( could be done with player scripting too )
================
*/
void idPlayerStart::Event_TeleportStage( idEntity *_player ) {
	idPlayer *player;
	if ( !_player->IsType( idPlayer::Type ) ) {
		common->Warning( "idPlayerStart::Event_TeleportStage: entity is not an idPlayer\n" );
		return;
	}
	player = static_cast<idPlayer*>(_player);
	float teleportDelay = spawnArgs.GetFloat( "teleportDelay" );
	switch ( teleportStage ) {
		case 0:
			player->playerView.Flash( colorWhite, 125 );
			player->SetInfluenceLevel( INFLUENCE_LEVEL3 );
			player->SetInfluenceView( spawnArgs.GetString( "mtr_teleportFx" ), NULL, 0.0f, NULL );
			gameSoundWorld->FadeSoundClasses( 0, -20.0f, teleportDelay );
			player->StartSound( "snd_teleport_start", SND_CHANNEL_BODY2, 0, false, NULL );
			teleportStage++;
			PostEventSec( &EV_TeleportStage, teleportDelay, player );
			break;
		case 1:
			gameSoundWorld->FadeSoundClasses( 0, 0.0f, 0.25f );
			teleportStage++;
			PostEventSec( &EV_TeleportStage, 0.25f, player );
			break;
		case 2:
			player->SetInfluenceView( NULL, NULL, 0.0f, NULL );
			TeleportPlayer( player );
			player->StopSound( SND_CHANNEL_BODY2, false );
			player->SetInfluenceLevel( INFLUENCE_NONE );
			teleportStage = 0;
			break;
		default:
			break;
	}
}

/*
===============
idPlayerStart::TeleportPlayer
================
*/
void idPlayerStart::TeleportPlayer( idPlayer *player ) {
	float pushVel = spawnArgs.GetFloat( "push", "300" );
	float f = spawnArgs.GetFloat( "visualEffect", "0" );
	const char *viewName = spawnArgs.GetString( "visualView", "" );
	idEntity *ent = viewName ? gameLocal.FindEntity( viewName ) : NULL;

	if ( f && ent ) {
		// place in private camera view for some time
		// the entity needs to teleport to where the camera view is to have the PVS right
		player->Teleport( ent->GetPhysics()->GetOrigin(), ang_zero, this );
		player->StartSound( "snd_teleport_enter", SND_CHANNEL_ANY, 0, false, NULL );
		player->SetPrivateCameraView( static_cast<idCamera*>(ent) );
		// the player entity knows where to spawn from the previous Teleport call
		if ( !gameLocal.isClient ) {
			player->PostEventSec( &EV_Player_ExitTeleporter, f );
		}
	} else {
		// direct to exit, Teleport will take care of the killbox
		player->Teleport( GetPhysics()->GetOrigin(), GetPhysics()->GetAxis().ToAngles(), NULL );

		// multiplayer hijacked this entity, so only push the player in multiplayer
		if ( gameLocal.isMultiplayer ) {
			player->GetPhysics()->SetLinearVelocity( GetPhysics()->GetAxis()[0] * pushVel );
		}
	}
}

/*
===============
idPlayerStart::Event_TeleportPlayer
================
*/
void idPlayerStart::Event_TeleportPlayer( idEntity *activator ) {
	idPlayer *player;

	if ( activator->IsType( idPlayer::Type ) ) {
		player = static_cast<idPlayer*>( activator );
	} else {
		player = gameLocal.GetLocalPlayer();
	}
	if ( player ) {
		if ( spawnArgs.GetBool( "visualFx" ) ) {

			teleportStage = 0;
			Event_TeleportStage( player );

		} else {

			if ( gameLocal.isServer ) {
				idBitMsg	msg;
				byte		msgBuf[MAX_EVENT_PARAM_SIZE];

				msg.Init( msgBuf, sizeof( msgBuf ) );
				msg.BeginWriting();
				msg.WriteBits( player->entityNumber, GENTITYNUM_BITS );
				ServerSendEvent( EVENT_TELEPORTPLAYER, &msg, false, -1 );
			}

			TeleportPlayer( player );
		}
	}
}

/*
===============================================================================

	idActivator

===============================================================================
*/

CLASS_DECLARATION( idEntity, idActivator )
	EVENT( EV_Activate,		idActivator::Event_Activate )
END_CLASS

/*
===============
idActivator::Save
================
*/
void idActivator::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( stay_on );
}

/*
===============
idActivator::Restore
================
*/
void idActivator::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( stay_on );

	if ( stay_on ) {
		BecomeActive( TH_THINK );
	}
}

/*
===============
idActivator::Spawn
================
*/
void idActivator::Spawn( void ) {
	bool start_off;

	spawnArgs.GetBool( "stay_on", "0", stay_on );
	spawnArgs.GetBool( "start_off", "0", start_off );

	GetPhysics()->SetClipBox( idBounds( vec3_origin ).Expand( 4 ), 1.0f );
	GetPhysics()->SetContents( 0 );

	if ( !start_off ) {
		BecomeActive( TH_THINK );
	}
}

/*
===============
idActivator::Think
================
*/
void idActivator::Think( void ) {
	RunPhysics();
	if ( thinkFlags & TH_THINK ) {
		if ( TouchTriggers() ) {
			if ( !stay_on ) {
				BecomeInactive( TH_THINK );
			}
		}
	}
	Present();
}

/*
===============
idActivator::Activate
================
*/
void idActivator::Event_Activate( idEntity *activator ) {
	if ( thinkFlags & TH_THINK ) {
		BecomeInactive( TH_THINK );
	} else {
		BecomeActive( TH_THINK );
	}
}


/*
===============================================================================

idPathCorner

===============================================================================
*/

CLASS_DECLARATION( idEntity, idPathCorner )
	EVENT( AI_RandomPath,		idPathCorner::Event_RandomPath )
END_CLASS

/*
=====================
idPathCorner::Spawn
=====================
*/
void idPathCorner::Spawn( void ) {
}

/*
=====================
idPathCorner::DrawDebugInfo
=====================
*/
void idPathCorner::DrawDebugInfo( void ) {
	idEntity *ent;
	idBounds bnds( idVec3( -4.0, -4.0f, -8.0f ), idVec3( 4.0, 4.0f, 64.0f ) );

	for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( !ent->IsType( idPathCorner::Type ) ) {
			continue;
		}

		idVec3 org = ent->GetPhysics()->GetOrigin();
		gameRenderWorld->DebugBounds( colorRed, bnds, org, 0 );
	}
}

/*
============
idPathCorner::RandomPath
============
*/
idPathCorner *idPathCorner::RandomPath( const idEntity *source, const idEntity *ignore ) {
	int	i;
	int	num;
	int which;
	idEntity *ent;
	idPathCorner *path[ MAX_GENTITIES ];

	num = 0;
	for( i = 0; i < source->targets.Num(); i++ ) {
		ent = source->targets[ i ].GetEntity();
		if ( ent && ( ent != ignore ) && ent->IsType( idPathCorner::Type ) ) {
			path[ num++ ] = static_cast<idPathCorner *>( ent );
			if ( num >= MAX_GENTITIES ) {
				break;
			}
		}
	}

	if ( !num ) {
		return NULL;
	}

	which = gameLocal.random.RandomInt( num );
	return path[ which ];
}

/*
=====================
idPathCorner::Event_RandomPath
=====================
*/
void idPathCorner::Event_RandomPath( void ) {
	idPathCorner *path;

	path = RandomPath( this, NULL );
	idThread::ReturnEntity( path );
}

/*
===============================================================================

  idDamagable

===============================================================================
*/

const idEventDef EV_RestoreDamagable( "<RestoreDamagable>" );
const idEventDef EV_HideBrokenEntities( "<HideBrokenEntities>" );
const idEventDef EV_ChainBecomeBroken( "<ChainBecomeBroken>", "ee" ); //ivan 

CLASS_DECLARATION( idEntity, idDamagable )
	EVENT( EV_Activate,				idDamagable::Event_BecomeBroken )
	EVENT( EV_ChainBecomeBroken,	idDamagable::Event_ChainBecomeBroken ) //ivan
	EVENT( EV_HideBrokenEntities,	idDamagable::Event_HideBrokenEntities ) //ivan
	EVENT( EV_RestoreDamagable,		idDamagable::Event_RestoreDamagable )
END_CLASS

/*
================
idDamagable::idDamagable
================
*/
idDamagable::idDamagable( void ) {
	count = 0;
	nextTriggerTime = 0;
	//ivan start
	particleModelDefHandle = -1;
	lightDefHandle = -1;
	memset( &particleRenderEntity, 0, sizeof( particleRenderEntity ) );
	memset( &light, 0, sizeof( light ) );
	lightTime = 0;
	hasBrokenModel = false;
	//ivan end
}

//ivan start
/*
================
idDamagable::~idDamagable
================
*/
idDamagable::~idDamagable( void ) {
	count = 0;
	nextTriggerTime = 0;
	//ivan start
	if ( particleModelDefHandle >= 0 ){
		gameRenderWorld->FreeEntityDef( particleModelDefHandle );
	}
	if ( lightDefHandle >= 0 ) {
		gameRenderWorld->FreeLightDef( lightDefHandle );
	}
	//ivan end
}

//ivan end

/*
================
idDamagable::Save
================
*/
void idDamagable::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( count );
	savefile->WriteInt( nextTriggerTime );
	//ivan start
	savefile->WriteInt( particleModelDefHandle );
	savefile->WriteInt( lightDefHandle );
	savefile->WriteRenderEntity( particleRenderEntity );
	savefile->WriteRenderLight( light );
	savefile->WriteInt( lightTime );
	savefile->WriteBool( hasBrokenModel );
	//ivan end
}

/*
================
idDamagable::Restore
================
*/
void idDamagable::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( count );
	savefile->ReadInt( nextTriggerTime );
	//ivan start
	savefile->ReadInt( (int &)particleModelDefHandle );
	savefile->ReadInt( (int &)lightDefHandle );
	savefile->ReadRenderEntity( particleRenderEntity );
	savefile->ReadRenderLight( light );
	savefile->ReadInt( lightTime );
	savefile->ReadBool( hasBrokenModel );
	//ivan end
}

/*
================
idDamagable::Spawn
================
*/
void idDamagable::Spawn( void ) {
	idStr broken;

	health = spawnArgs.GetInt( "health", "5" );
	spawnArgs.GetInt( "count", "1", count );
	nextTriggerTime = 0;

	// make sure the model gets cached
	spawnArgs.GetString( "broken", "", broken );
	if ( broken.Length() ){
		hasBrokenModel = true;
		if( !renderModelManager->CheckModel( broken ) ) {
			gameLocal.Error( "idDamagable '%s' at (%s): cannot load broken model '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), broken.c_str() );
		}
	}else{
		hasBrokenModel = false;
	}

	fl.takedamage = true;
	GetPhysics()->SetContents( CONTENTS_SOLID );

	//ivan start
	particleModelDefHandle = -1;
	lightDefHandle = -1;
	lightTime = 0;
	memset( &particleRenderEntity, 0, sizeof( particleRenderEntity ) );
	memset( &light, 0, sizeof( light ) );

	if( spawnArgs.MatchPrefix( "broken_ent" ) ){
		PostEventMS( &EV_HideBrokenEntities, 0 );
	}
	//ivan end
}

//ivan start

/*
================
idDamagable::Think
================
*/
void idDamagable::Think( void ) {
	//gameLocal.Printf("idDamagable::Think\n");
	idEntity::Think();

	//light
	if ( lightDefHandle >= 0 ){
		if ( gameLocal.time - lightTime > 250 ) {
			gameRenderWorld->FreeLightDef( lightDefHandle );
			lightDefHandle = -1;
		}
	}else{
		BecomeInactive( TH_THINK );
	}
}

/*
================
idDamagable::AddParticles
================
*/
void idDamagable::AddParticles( const char *name ) {
	idPhysics *physicsObj = GetPhysics();

	if ( name && *name ) {
		if ( particleModelDefHandle >= 0 ){
			gameRenderWorld->FreeEntityDef( particleModelDefHandle );
		}
		memset( &particleRenderEntity, 0, sizeof ( particleRenderEntity ) );
		const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, name ) );
		if ( modelDef ) {
			particleRenderEntity.origin = physicsObj->GetAbsBounds().GetCenter();
			particleRenderEntity.axis = mat3_identity;
			particleRenderEntity.hModel = modelDef->ModelHandle();
			float rgb = 1.0f; 
			particleRenderEntity.shaderParms[ SHADERPARM_RED ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_GREEN ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_BLUE ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_ALPHA ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.realClientTime );
			particleRenderEntity.shaderParms[ SHADERPARM_DIVERSITY ] = gameLocal.random.RandomInt( 90 ); 
			if ( !particleRenderEntity.hModel ) {
				particleRenderEntity.hModel = renderModelManager->FindModel( name );
			}
			particleModelDefHandle = gameRenderWorld->AddEntityDef( &particleRenderEntity );
		}
	}
}

/*
================
idDamagable::AddLight
================
*/
void idDamagable::AddLight( const char *name ) {
	idPhysics *physicsObj = GetPhysics(); 

	if ( lightDefHandle >= 0 ){
		gameRenderWorld->FreeLightDef( lightDefHandle );
	}
	memset( &light, 0, sizeof ( light ) );
	light.axis = mat3_identity;
	light.lightRadius.x = spawnArgs.GetFloat( "light_radius" );
	light.lightRadius.y = light.lightRadius.z = light.lightRadius.x;
	light.origin = physicsObj->GetOrigin();
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
void idDamagable::ExplodingEffects( void ) {
	const char *temp;

	StartSound( "snd_explode", SND_CHANNEL_ANY, 0, false, NULL );

	temp = spawnArgs.GetString( "mtr_lightexplode" );
	if ( *temp != '\0' ) {
		AddLight( temp );
	}

	temp = spawnArgs.GetString( "mtr_burnmark" );
	if ( *temp != '\0' ) {
		gameLocal.ProjectDecal( GetPhysics()->GetOrigin(), GetPhysics()->GetGravity(), 128.0f, true, 96.0f, temp );
	}
	// put the explosion particle effect to the end -- By Clone JCD
	temp = spawnArgs.GetString( "model_detonate" );
	if ( *temp != '\0' ) {
		AddParticles( temp );
	}
}

/*
================
idDamagable::SpawnDebris
================
*/
void idDamagable::SpawnDebris( const char *prefix ) {
	idPhysics *physicsObj = GetPhysics(); //ivan

	//revility breakables start
	const idKeyValue *kv = spawnArgs.MatchPrefix( prefix );   
	while ( kv ) {
		const idDict *debris_args = gameLocal.FindEntityDefDict( kv->GetValue(), false );
		if ( debris_args ) {
			idEntity *ent;
			idVec3 dir;
			idDebris *debris;
			//if ( first ) {
			dir = physicsObj->GetAxis()[1]; //ivan -> fix
			//   first = false;
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
			debris->Create( this, physicsObj->GetOrigin(), dir.ToMat3() ); //ivan -> fix
			debris->Launch();
			debris->GetRenderEntity()->shaderParms[ SHADERPARM_TIME_OF_DEATH ] = ( gameLocal.time + 1500 ) * 0.001f;
			debris->UpdateVisuals();

		}
		kv = spawnArgs.MatchPrefix( prefix, kv );
	}      
	//revility breakables end
}

/*
================
idDamagable::BrokenEntitiesVisible
================
*/
void idDamagable::BrokenEntitiesVisible( bool show ) {
	idEntity *ent;

	const idKeyValue *kv = spawnArgs.MatchPrefix( "broken_ent" );   
	while ( kv ) {
		ent = gameLocal.FindEntity( kv->GetValue().c_str() );
		if ( ent ) {
			if(show) ent->Show();
			else ent->Hide();
		}else{
			gameLocal.Warning( "Couldn't find entity '%s' specified in '%s' key in entity '%s'", kv->GetValue().c_str(), kv->GetKey().c_str(), name.c_str() );
		}
		
		//loop
		kv = spawnArgs.MatchPrefix( "broken_ent", kv );
	}
}

/*
================
idDamagable::SpawnDrops
================
*/
void idDamagable::SpawnDrops( void ){
	idVec3 offset;
	idStr offsetKey;
	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_dropItem", NULL );
	while( kv ) {

		//get the offset
		offsetKey = kv->GetKey().c_str() + 4;
		offsetKey += "Offset";
		offset = spawnArgs.GetVector( offsetKey.c_str(), "0 0 0" );

		idMoveableItem::DropItem( kv->GetValue().c_str(), GetPhysics()->GetAbsBounds().GetCenter() + offset, GetPhysics()->GetAxis(), vec3_origin, 0, 0, false ); //don't drop to floor so offset is respected
		kv = spawnArgs.MatchPrefix( "def_dropItem", kv );
	}
}
//ivan end

/*
================
idDamagable::BecomeBroken
================
*/
void idDamagable::BecomeBroken( idEntity *activator, idEntity *chainInflictor ) {
	float	forceShader7Val;
	int		numShader7States;
	int		cycleShader7State;
	float	wait;
	bool	lastStep; //ivan 
	//idStr	broken;

	if ( gameLocal.time < nextTriggerTime ) {
		return;
	}

	//reset next trigger time
	spawnArgs.GetFloat( "wait", "0.1", wait );
	nextTriggerTime = gameLocal.time + SEC2MS( wait );

	//check if it's the last step
	lastStep = false; //ivan
	if ( count > 0 ) {
		count--;
		if ( !count ) { //this was the last step!
			fl.takedamage = false; //ivan note: make sure we don't take damage for the last step
			lastStep = true;  //ivan
		} else {
			health = spawnArgs.GetInt( "health", "5" );
		}
	}

	//ivan start
	if( lastStep ){
		
		//gameLocal.Printf("lastStep\n");

		if ( hasBrokenModel ) {
			SetModel( spawnArgs.GetString( "broken", "" ) );
		}else{
			if ( spawnArgs.GetBool( "deadHidden", "1" ) ) { //hide only if we don't have a broken model
				Hide();
			}
			if ( !spawnArgs.GetBool( "deadSolid", "1" ) ) {
			  GetPhysics()->SetContents( 0 );
			}
		}

		//splash damage
		const char *splash = spawnArgs.GetString( "def_splash_damage", "" ); //damagable_exploding_base
		if ( splash && *splash ) {
			int oldContents;
			oldContents = GetPhysics()->GetContents();
			GetPhysics()->SetContents( 0 ); //allows RadiusDamage to go through us

			//ivan - ignore the chain inflictor!
			//was: gameLocal.RadiusDamage( GetPhysics()->GetOrigin(), this, activator, this, this, splash );
			//note: chainInflictor can be null. We are ignored anyway, because takedamage is false when we are here. :)
			gameLocal.RadiusDamage( GetPhysics()->GetOrigin(), this, activator, chainInflictor, this, splash ); 
		

			if( oldContents ){
				GetPhysics()->SetContents( oldContents );
			}
		}

		ExplodingEffects();
		SpawnDebris( "def_expl_debris" );
		BrokenEntitiesVisible( true ); //Triggering targets could be enough if 1-time only...
		SpawnDrops(); //drop items

	}//end lastStep
	else{ //intermediate step
		//gameLocal.Printf("intermediate step\n");

		if ( hasBrokenModel ) {
			SetModel( spawnArgs.GetString( "broken", "" ) );
		}

		SpawnDebris( "def_dmg_debris" );
	}
	//ivan end

	//shaders stuff
	spawnArgs.GetInt( "numstates", "1", numShader7States );
	spawnArgs.GetInt( "cycle", "0", cycleShader7State );
	spawnArgs.GetFloat( "forcestate", "0", forceShader7Val );

	// set the state parm
	if ( cycleShader7State ) {
		renderEntity.shaderParms[ SHADERPARM_MODE ]++;
		if ( renderEntity.shaderParms[ SHADERPARM_MODE ] > numShader7States ) {
			renderEntity.shaderParms[ SHADERPARM_MODE ] = 0;
		}
	} else if ( forceShader7Val ) {
		renderEntity.shaderParms[ SHADERPARM_MODE ] = forceShader7Val;
	} else {
		renderEntity.shaderParms[ SHADERPARM_MODE ] = gameLocal.random.RandomInt( numShader7States ) + 1;
	}

	// offset the start time of the shader to sync it to the gameLocal time
	//ivan start
	//setting SHADERPARM_TIMEOFFSET to local time is not a good ideas for flares... 
	//was: renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
	if( spawnArgs.GetBool( "updParm4", "1" ) ){
		renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
	}

	//cancel pending events?
	//CancelEvents( &EV_ChainBecomeBroken );
	//we don't need. Even if this happens, nextTriggerTime will save us. 

	//ivan end

	ActivateTargets( activator );

	if ( lastStep ){ //ivan
		if( spawnArgs.GetBool( "restore" ) ) { //lastStep added - was: "hideWhenBroken"
			//Hide(); //ivan - we are already hidden if we should.
			
			//ivan -  extra delay for restore
			wait = SEC2MS( spawnArgs.GetFloat( "restoreDelay", "1" ) ) + gameLocal.time;
			if( wait > nextTriggerTime ){ nextTriggerTime = wait; }

			PostEventMS( &EV_RestoreDamagable, nextTriggerTime - gameLocal.time );
			BecomeActive( TH_THINK );
		}
	}
	//ivan start
	else{ //reset health and start from the beginning!
		health = spawnArgs.GetInt( "health", "5" );
	}
	//ivan end
}

/*
================
idDamagable::Killed
================
*/
void idDamagable::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	int chainDelay;
	//gameLocal.Printf("idDamagable::Killed by %s from %s\n", inflictor->GetName(), attacker->GetName() );

	if ( gameLocal.time < nextTriggerTime ) {
		health += damage;
		return;
	}

	//ivan start 
	//was: BecomeBroken( attacker );

	chainDelay = 0;
	if( inflictor && inflictor->IsType( idDamagable::Type ) ){ //in a chain
		chainDelay = SEC2MS( spawnArgs.GetFloat( "chainDelay", "0" ) );
	}

	if( chainDelay > 0 ){
		//gameLocal.Printf("chain detected\n");
		
		PostEventMS( &EV_ChainBecomeBroken, chainDelay, attacker, inflictor ); //delayed
		nextTriggerTime = gameLocal.time + 1; //make sure not triggered or killed again this frame
	}else{
		BecomeBroken( attacker, NULL );
	}
	//ivan end
	
}

/*
================
idDamagable::Event_BecomeBroken
================
*/
void idDamagable::Event_BecomeBroken( idEntity *activator ) {
	BecomeBroken( activator , NULL );
}

/*
================
idDamagable::Event_ChainBecomeBroken
================
*/
void idDamagable::Event_ChainBecomeBroken( idEntity *activator, idEntity *chainInflictor ) {
	BecomeBroken( activator, chainInflictor );
}

/*
================
idDamagable::Event_HideBrokenEntities
================
*/
void idDamagable::Event_HideBrokenEntities( void ) {
	BrokenEntitiesVisible( false );
}



/*
================
idDamagable::Event_RestoreDamagable
================
*/
void idDamagable::Event_RestoreDamagable( void ) {
	health = spawnArgs.GetInt( "health", "5" );
	
	//ivan start - fix
	//gameLocal.Printf("idDamagable::Event_RestoreDamagable\n");

	if ( hasBrokenModel ) { //we have a broken model
		//restore the not-broken one
		idStr model;
		spawnArgs.GetString( "model", "", model );
		if ( model.Length() ) { 
			SetModel( model );
		}
	}
	
	BrokenEntitiesVisible( false ); //hide broken entities

	GetPhysics()->SetContents( CONTENTS_SOLID ); //solid

	fl.takedamage = true;

	spawnArgs.GetInt( "count", "1", count ); //reset count!

	renderEntity.shaderParms[ SHADERPARM_MODE ] = 0; //reset SHADERPARM_MODE
	//ivan end
	Show();
}


/*
===============================================================================

  idExplodable

===============================================================================
*/

CLASS_DECLARATION( idEntity, idExplodable )
	EVENT( EV_Activate,	idExplodable::Event_Explode )
END_CLASS

/*
================
idExplodable::Spawn
================
*/
void idExplodable::Spawn( void ) {
	Hide();
}

/*
================
idExplodable::Event_Explode
================
*/
void idExplodable::Event_Explode( idEntity *activator ) {
	const char *temp;

	if ( spawnArgs.GetString( "def_damage", "damage_explosion", &temp ) ) {
		gameLocal.RadiusDamage( GetPhysics()->GetOrigin(), activator, activator, this, this, temp );
	}

	StartSound( "snd_explode", SND_CHANNEL_ANY, 0, false, NULL );

	// Show() calls UpdateVisuals, so we don't need to call it ourselves after setting the shaderParms
	renderEntity.shaderParms[SHADERPARM_RED]		= 1.0f;
	renderEntity.shaderParms[SHADERPARM_GREEN]		= 1.0f;
	renderEntity.shaderParms[SHADERPARM_BLUE]		= 1.0f;
	renderEntity.shaderParms[SHADERPARM_ALPHA]		= 1.0f;
	renderEntity.shaderParms[SHADERPARM_TIMEOFFSET] = -MS2SEC( gameLocal.time );
	renderEntity.shaderParms[SHADERPARM_DIVERSITY]	= 0.0f;
	Show();

	PostEventMS( &EV_Remove, 2000 );

	ActivateTargets( activator );
}


/*
===============================================================================

  idSpring

===============================================================================
*/

CLASS_DECLARATION( idEntity, idSpring )
	EVENT( EV_PostSpawn,	idSpring::Event_LinkSpring )
END_CLASS

/*
================
idSpring::Think
================
*/
void idSpring::Think( void ) {
	idVec3 start, end, origin;
	idMat3 axis;

	// run physics
	RunPhysics();

	if ( thinkFlags & TH_THINK ) {
		// evaluate force
		spring.Evaluate( gameLocal.time );

		start = p1;
		if ( ent1->GetPhysics() ) {
			axis = ent1->GetPhysics()->GetAxis();
			origin = ent1->GetPhysics()->GetOrigin();
			start = origin + start * axis;
		}

		end = p2;
		if ( ent2->GetPhysics() ) {
			axis = ent2->GetPhysics()->GetAxis();
			origin = ent2->GetPhysics()->GetOrigin();
			end = origin + p2 * axis;
		}

		gameRenderWorld->DebugLine( idVec4(1, 1, 0, 1), start, end, 0, true );
	}

	Present();
}

/*
================
idSpring::Event_LinkSpring
================
*/
void idSpring::Event_LinkSpring( void ) {
	idStr name1, name2;

	spawnArgs.GetString( "ent1", "", name1 );
	spawnArgs.GetString( "ent2", "", name2 );

	if ( name1.Length() ) {
		ent1 = gameLocal.FindEntity( name1 );
		if ( !ent1 ) {
			gameLocal.Error( "idSpring '%s' at (%s): cannot find first entity '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), name1.c_str() );
		}
	}
	else {
		ent1 = gameLocal.entities[ENTITYNUM_WORLD];
	}

	if ( name2.Length() ) {
		ent2 = gameLocal.FindEntity( name2 );
		if ( !ent2 ) {
			gameLocal.Error( "idSpring '%s' at (%s): cannot find second entity '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), name2.c_str() );
		}
	}
	else {
		ent2 = gameLocal.entities[ENTITYNUM_WORLD];
	}
	spring.SetPosition( ent1->GetPhysics(), id1, p1, ent2->GetPhysics(), id2, p2 );
	BecomeActive( TH_THINK );
}

/*
================
idSpring::Spawn
================
*/
void idSpring::Spawn( void ) {
	float Kstretch, damping, restLength;

	spawnArgs.GetInt( "id1", "0", id1 );
	spawnArgs.GetInt( "id2", "0", id2 );
	spawnArgs.GetVector( "point1", "0 0 0", p1 );
	spawnArgs.GetVector( "point2", "0 0 0", p2 );
	spawnArgs.GetFloat( "constant", "100.0f", Kstretch );
	spawnArgs.GetFloat( "damping", "10.0f", damping );
	spawnArgs.GetFloat( "restlength", "0.0f", restLength );

	spring.InitSpring( Kstretch, 0.0f, damping, restLength );

	ent1 = ent2 = NULL;

	PostEventMS( &EV_PostSpawn, 0 );
}

/*
===============================================================================

  idForceField

===============================================================================
*/

const idEventDef EV_Toggle( "Toggle", NULL );

CLASS_DECLARATION( idEntity, idForceField )
	EVENT( EV_Activate,		idForceField::Event_Activate )
	EVENT( EV_Toggle,		idForceField::Event_Toggle )
	EVENT( EV_FindTargets,	idForceField::Event_FindTargets )
END_CLASS

/*
===============
idForceField::Toggle
================
*/
void idForceField::Toggle( void ) {
	if ( thinkFlags & TH_THINK ) {
		BecomeInactive( TH_THINK );
	} else {
		BecomeActive( TH_THINK );
	}
}

/*
================
idForceField::Think
================
*/
void idForceField::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		// evaluate force
		forceField.Evaluate( gameLocal.time );
	}
	Present();
}

/*
================
idForceField::Save
================
*/
void idForceField::Save( idSaveGame *savefile ) const {
	savefile->WriteStaticObject( forceField );
}

/*
================
idForceField::Restore
================
*/
void idForceField::Restore( idRestoreGame *savefile ) {
	savefile->ReadStaticObject( forceField );
}

/*
================
idForceField::Spawn
================
*/
void idForceField::Spawn( void ) {
	idVec3 uniform;
	float explosion, implosion, randomTorque;
	float dist_offset, dist_radius, oldVelocityPct; //ivan

	if ( spawnArgs.GetFloat( "randomTorque", "0", randomTorque ) ) {
		forceField.RandomTorque( randomTorque );
	}

	// -- direction type --
	if ( spawnArgs.GetVector( "uniform", "0 0 0", uniform ) ) {
		forceField.Uniform( uniform );
	} else if ( spawnArgs.GetFloat( "explosion", "0", explosion ) ) {
		forceField.Explosion( explosion );
	} else if ( spawnArgs.GetFloat( "implosion", "0", implosion ) ) {
		forceField.Implosion( implosion );
	}

	//ivan start

	// -- magnitude type --
	if ( spawnArgs.GetBool( "increase_w_dist", "0" ) ) {
		forceField.SetMagnitudeType( FORCEFIELD_MAGNITUDE_DISTANCE );
	} else if ( spawnArgs.GetBool( "decrease_w_dist", "0" ) ) {
		forceField.SetMagnitudeType( FORCEFIELD_MAGNITUDE_DISTANCE_INV );
	} else {
		forceField.SetMagnitudeType( FORCEFIELD_MAGNITUDE_FIXED );
	}

	// -- magnitude settings --
	dist_offset = spawnArgs.GetFloat( "dist_offset", "0" );
	dist_radius = spawnArgs.GetFloat( "dist_radius", "0" );
	if( dist_offset < 0.0f ){ dist_offset = 0.0f; }
	if( dist_radius <= 0.0f ){ 
		//dist_radius = (GetPhysics()->GetBounds()[1].z - GetPhysics()->GetBounds()[0].z); //* 0.8f//(maxs + -mins)/2  height
		float maxs_lenght = GetPhysics()->GetBounds()[1].z;
		float mins_lenght = -GetPhysics()->GetBounds()[0].z;
		dist_radius = (maxs_lenght > mins_lenght) ? maxs_lenght : mins_lenght; //use the bigger one
	} 
	//gameLocal.Printf("%s - dist_radius: %f\n",GetName(), dist_radius);
	forceField.SetDistanceBounds( dist_offset, dist_radius );

	//ivan end

	// -- apply type --	
	if ( spawnArgs.GetBool( "applyForce", "0" ) ) {
		forceField.SetApplyType( FORCEFIELD_APPLY_FORCE );
	} else if ( spawnArgs.GetBool( "applyImpulse", "0" ) ) {
		forceField.SetApplyType( FORCEFIELD_APPLY_IMPULSE );
	} else {
		forceField.SetApplyType( FORCEFIELD_APPLY_VELOCITY );
		//ivan start
		if ( spawnArgs.GetFloat( "oldVelocityPct", "0", oldVelocityPct ) ) {
			forceField.SetOldVelocityPct( oldVelocityPct );
		} 
		//ivan end
	}

	// -- other --
	forceField.SetPlayerOnly( spawnArgs.GetBool( "playerOnly", "0" ) );
	forceField.SetMonsterOnly( spawnArgs.GetBool( "monsterOnly", "0" ) );

	// set the collision model on the force field
	forceField.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ) );

	// remove the collision model from the physics object
	GetPhysics()->SetClipModel( NULL, 1.0f );

	if ( spawnArgs.GetBool( "start_on" ) ) {
		BecomeActive( TH_THINK );
	}
}

/*
===============
idForceField::Event_Toggle
================
*/
void idForceField::Event_Toggle( void ) {
	Toggle();
}

/*
================
idForceField::Event_Activate
================
*/
void idForceField::Event_Activate( idEntity *activator ) {
	float wait;

	Toggle();
	if ( spawnArgs.GetFloat( "wait", "0.01", wait ) ) {
		PostEventSec( &EV_Toggle, wait );
	}
}

/*
================
idForceField::Event_FindTargets
================
*/
void idForceField::Event_FindTargets( void ) {
	FindTargets();
	RemoveNullTargets();
	if ( targets.Num() ) {
		forceField.Uniform( targets[0].GetEntity()->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin() );
	}
}

/*
===============================================================================

	idAnimated

===============================================================================
*/

const idEventDef EV_Animated_Start( "<start>" );
const idEventDef EV_LaunchMissiles( "launchMissiles", "ssssdf" );
const idEventDef EV_LaunchMissilesUpdate( "<launchMissiles>", "dddd" );
const idEventDef EV_AnimDone( "<AnimDone>", "d" );
const idEventDef EV_StartRagdoll( "startRagdoll" );

CLASS_DECLARATION( idAFEntity_Gibbable, idAnimated )
	EVENT( EV_Activate,				idAnimated::Event_Activate )
	EVENT( EV_Animated_Start,		idAnimated::Event_Start )
	EVENT( EV_StartRagdoll,			idAnimated::Event_StartRagdoll )
	EVENT( EV_AnimDone,				idAnimated::Event_AnimDone )
	EVENT( EV_Footstep,				idAnimated::Event_Footstep )
	EVENT( EV_FootstepLeft,			idAnimated::Event_Footstep )
	EVENT( EV_FootstepRight,		idAnimated::Event_Footstep )
	EVENT( EV_LaunchMissiles,		idAnimated::Event_LaunchMissiles )
	EVENT( EV_LaunchMissilesUpdate,	idAnimated::Event_LaunchMissilesUpdate )
END_CLASS

/*
===============
idAnimated::idAnimated
================
*/
idAnimated::idAnimated() {
	anim = 0;
	blendFrames = 0;
	soundJoint = INVALID_JOINT;
	activated = false;
	combatModel = NULL;
	activator = NULL;
	current_anim_index = 0;
	num_anims = 0;

}

/*
===============
idAnimated::idAnimated
================
*/
idAnimated::~idAnimated() {
	delete combatModel;
	combatModel = NULL;
}

/*
===============
idAnimated::Save
================
*/
void idAnimated::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( current_anim_index );
	savefile->WriteInt( num_anims );
	savefile->WriteInt( anim );
	savefile->WriteInt( blendFrames );
	savefile->WriteJoint( soundJoint );
	activator.Save( savefile );
	savefile->WriteBool( activated );
}

/*
===============
idAnimated::Restore
================
*/
void idAnimated::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( current_anim_index );
	savefile->ReadInt( num_anims );
	savefile->ReadInt( anim );
	savefile->ReadInt( blendFrames );
	savefile->ReadJoint( soundJoint );
	activator.Restore( savefile );
	savefile->ReadBool( activated );
}

/*
===============
idAnimated::Spawn
================
*/
void idAnimated::Spawn( void ) {
	idStr		animname;
	int			anim2;
	float		wait;
	const char	*joint;

	joint = spawnArgs.GetString( "sound_bone", "origin" );
	soundJoint = animator.GetJointHandle( joint );
	if ( soundJoint == INVALID_JOINT ) {
		gameLocal.Warning( "idAnimated '%s' at (%s): cannot find joint '%s' for sound playback", name.c_str(), GetPhysics()->GetOrigin().ToString(0), joint );
	}

	LoadAF();

	// allow bullets to collide with a combat model
	if ( spawnArgs.GetBool( "combatModel", "0" ) ) {
		combatModel = new idClipModel( modelDefHandle );
	}

	// allow the entity to take damage
	if ( spawnArgs.GetBool( "takeDamage", "0" ) ) {
		fl.takedamage = true;
	}

	blendFrames = 0;

	current_anim_index = 0;
	spawnArgs.GetInt( "num_anims", "0", num_anims );

	blendFrames = spawnArgs.GetInt( "blend_in" );

	animname = spawnArgs.GetString( num_anims ? "anim1" : "anim" );
	if ( !animname.Length() ) {
		anim = 0;
	} else {
		anim = animator.GetAnim( animname );
		if ( !anim ) {
			gameLocal.Error( "idAnimated '%s' at (%s): cannot find anim '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), animname.c_str() );
		}
	}

	if ( spawnArgs.GetBool( "hide" ) ) {
		Hide();

		if ( !num_anims ) {
			blendFrames = 0;
		}
	} else if ( spawnArgs.GetString( "start_anim", "", animname ) ) {
		anim2 = animator.GetAnim( animname );
		if ( !anim2 ) {
			gameLocal.Error( "idAnimated '%s' at (%s): cannot find anim '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), animname.c_str() );
		}
		animator.CycleAnim( ANIMCHANNEL_ALL, anim2, gameLocal.time, 0 );
	} else if ( anim ) {
		// init joints to the first frame of the animation
		animator.SetFrame( ANIMCHANNEL_ALL, anim, 1, gameLocal.time, 0 );

		if ( !num_anims ) {
			blendFrames = 0;
		}
	}

	spawnArgs.GetFloat( "wait", "-1", wait );

	if ( wait >= 0 ) {
		PostEventSec( &EV_Activate, wait, this );
	}
}

/*
===============
idAnimated::LoadAF
===============
*/
bool idAnimated::LoadAF( void ) {
	idStr fileName;

	if ( !spawnArgs.GetString( "ragdoll", "*unknown*", fileName ) ) {
		return false;
	}
	af.SetAnimator( GetAnimator() );
	return af.Load( this, fileName );
}

/*
===============
idAnimated::GetPhysicsToSoundTransform
===============
*/
bool idAnimated::GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis ) {
	animator.GetJointTransform( soundJoint, gameLocal.time, origin, axis );
	axis = renderEntity.axis;
	return true;
}

/*
================
idAnimated::StartRagdoll
================
*/
bool idAnimated::StartRagdoll( void ) {
	// if no AF loaded
	if ( !af.IsLoaded() ) {
		return false;
	}

	// if the AF is already active
	if ( af.IsActive() ) {
		return true;
	}

	// disable any collision model used
	GetPhysics()->DisableClip();

	// start using the AF
	af.StartFromCurrentPose( spawnArgs.GetInt( "velocityTime", "0" ) );

	return true;
}

/*
=====================
idAnimated::PlayNextAnim
=====================
*/
void idAnimated::PlayNextAnim( void ) {
	const char *animname;
	int len;
	int cycle;

	if ( current_anim_index >= num_anims ) {
		Hide();
		if ( spawnArgs.GetBool( "remove" ) ) {
			PostEventMS( &EV_Remove, 0 );
		} else {
			current_anim_index = 0;
		}
		return;
	}

	Show();
	current_anim_index++;

	spawnArgs.GetString( va( "anim%d", current_anim_index ), NULL, &animname );
	if ( !animname ) {
		anim = 0;
		animator.Clear( ANIMCHANNEL_ALL, gameLocal.time, FRAME2MS( blendFrames ) );
		return;
	}

	anim = animator.GetAnim( animname );
	if ( !anim ) {
		gameLocal.Warning( "missing anim '%s' on %s", animname, name.c_str() );
		return;
	}

	if ( g_debugCinematic.GetBool() ) {
		gameLocal.Printf( "%d: '%s' start anim '%s'\n", gameLocal.framenum, GetName(), animname );
	}

	spawnArgs.GetInt( "cycle", "1", cycle );
	if ( ( current_anim_index == num_anims ) && spawnArgs.GetBool( "loop_last_anim" ) ) {
		cycle = -1;
	}

	animator.CycleAnim( ANIMCHANNEL_ALL, anim, gameLocal.time, FRAME2MS( blendFrames ) );
	animator.CurrentAnim( ANIMCHANNEL_ALL )->SetCycleCount( cycle );

	len = animator.CurrentAnim( ANIMCHANNEL_ALL )->PlayLength();
	if ( len >= 0 ) {
		PostEventMS( &EV_AnimDone, len, current_anim_index );
	}

	// offset the start time of the shader to sync it to the game time
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );

	animator.ForceUpdate();
	UpdateAnimation();
	UpdateVisuals();
	Present();
}

/*
===============
idAnimated::Event_StartRagdoll
================
*/
void idAnimated::Event_StartRagdoll( void ) {
	StartRagdoll();
}

/*
===============
idAnimated::Event_AnimDone
================
*/
void idAnimated::Event_AnimDone( int animindex ) {
	if ( g_debugCinematic.GetBool() ) {
		const idAnim *animPtr = animator.GetAnim( anim );
		gameLocal.Printf( "%d: '%s' end anim '%s'\n", gameLocal.framenum, GetName(), animPtr ? animPtr->Name() : "" );
	}

	if ( ( animindex >= num_anims ) && spawnArgs.GetBool( "remove" ) ) {
		Hide();
		PostEventMS( &EV_Remove, 0 );
	} else if ( spawnArgs.GetBool( "auto_advance" ) ) {
		PlayNextAnim();
	} else {
		activated = false;
	}

	ActivateTargets( activator.GetEntity() );
}

/*
===============
idAnimated::Event_Activate
================
*/
void idAnimated::Event_Activate( idEntity *_activator ) {
	if ( num_anims ) {
		PlayNextAnim();
		activator = _activator;
		return;
	}

	if ( activated ) {
		// already activated
		return;
	}

	activated = true;
	activator = _activator;
	ProcessEvent( &EV_Animated_Start );
}

/*
===============
idAnimated::Event_Start
================
*/
void idAnimated::Event_Start( void ) {
	int cycle;
	int len;

	Show();

	if ( num_anims ) {
		PlayNextAnim();
		return;
	}

	if ( anim ) {
		if ( g_debugCinematic.GetBool() ) {
			const idAnim *animPtr = animator.GetAnim( anim );
			gameLocal.Printf( "%d: '%s' start anim '%s'\n", gameLocal.framenum, GetName(), animPtr ? animPtr->Name() : "" );
		}
		spawnArgs.GetInt( "cycle", "1", cycle );
		animator.CycleAnim( ANIMCHANNEL_ALL, anim, gameLocal.time, FRAME2MS( blendFrames ) );
		animator.CurrentAnim( ANIMCHANNEL_ALL )->SetCycleCount( cycle );

		len = animator.CurrentAnim( ANIMCHANNEL_ALL )->PlayLength();
		if ( len >= 0 ) {
			PostEventMS( &EV_AnimDone, len, 1 );
		}
	}

	// offset the start time of the shader to sync it to the game time
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );

	animator.ForceUpdate();
	UpdateAnimation();
	UpdateVisuals();
	Present();
}

/*
===============
idAnimated::Event_Footstep
===============
*/
void idAnimated::Event_Footstep( void ) {
	StartSound( "snd_footstep", SND_CHANNEL_BODY, 0, false, NULL );
}

/*
=====================
idAnimated::Event_LaunchMissilesUpdate
=====================
*/
void idAnimated::Event_LaunchMissilesUpdate( int launchjoint, int targetjoint, int numshots, int framedelay ) {
	idVec3			launchPos;
	idVec3			targetPos;
	idMat3			axis;
	idVec3			dir;
	idEntity *		ent;
	idProjectile *	projectile;
	const idDict *	projectileDef;
	const char *	projectilename;

	projectilename = spawnArgs.GetString( "projectilename" );
	projectileDef = gameLocal.FindEntityDefDict( projectilename, false );
	if ( !projectileDef ) {
		gameLocal.Warning( "idAnimated '%s' at (%s): 'launchMissiles' called with unknown projectile '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), projectilename );
		return;
	}

	StartSound( "snd_missile", SND_CHANNEL_WEAPON, 0, false, NULL );

	animator.GetJointTransform( ( jointHandle_t )launchjoint, gameLocal.time, launchPos, axis );
	launchPos = renderEntity.origin + launchPos * renderEntity.axis;

	animator.GetJointTransform( ( jointHandle_t )targetjoint, gameLocal.time, targetPos, axis );
	targetPos = renderEntity.origin + targetPos * renderEntity.axis;

	dir = targetPos - launchPos;
	dir.Normalize();

	gameLocal.SpawnEntityDef( *projectileDef, &ent, false );
	if ( !ent || !ent->IsType( idProjectile::Type ) ) {
		gameLocal.Error( "idAnimated '%s' at (%s): in 'launchMissiles' call '%s' is not an idProjectile", name.c_str(), GetPhysics()->GetOrigin().ToString(0), projectilename );
	}
	projectile = ( idProjectile * )ent;
	projectile->Create( this, launchPos, dir );
	projectile->Launch( launchPos, dir, vec3_origin );

	if ( numshots > 0 ) {
		PostEventMS( &EV_LaunchMissilesUpdate, FRAME2MS( framedelay ), launchjoint, targetjoint, numshots - 1, framedelay );
	}
}

/*
=====================
idAnimated::Event_LaunchMissiles
=====================
*/
void idAnimated::Event_LaunchMissiles( const char *projectilename, const char *sound, const char *launchjoint, const char *targetjoint, int numshots, int framedelay ) {
	const idDict *	projectileDef;
	jointHandle_t	launch;
	jointHandle_t	target;

	projectileDef = gameLocal.FindEntityDefDict( projectilename, false );
	if ( !projectileDef ) {
		gameLocal.Warning( "idAnimated '%s' at (%s): unknown projectile '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), projectilename );
		return;
	}

	launch = animator.GetJointHandle( launchjoint );
	if ( launch == INVALID_JOINT ) {
		gameLocal.Warning( "idAnimated '%s' at (%s): unknown launch joint '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), launchjoint );
		gameLocal.Error( "Unknown joint '%s'", launchjoint );
	}

	target = animator.GetJointHandle( targetjoint );
	if ( target == INVALID_JOINT ) {
		gameLocal.Warning( "idAnimated '%s' at (%s): unknown target joint '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), targetjoint );
	}

	spawnArgs.Set( "projectilename", projectilename );
	spawnArgs.Set( "missilesound", sound );

	CancelEvents( &EV_LaunchMissilesUpdate );
	ProcessEvent( &EV_LaunchMissilesUpdate, launch, target, numshots - 1, framedelay );
}


/*
===============================================================================

	idStaticEntity

	Some static entities may be optimized into inline geometry by dmap

===============================================================================
*/

CLASS_DECLARATION( idEntity, idStaticEntity )
	EVENT( EV_Activate,				idStaticEntity::Event_Activate )
END_CLASS

/*
===============
idStaticEntity::idStaticEntity
===============
*/
idStaticEntity::idStaticEntity( void ) {
	spawnTime = 0;
	active = false;
	fadeFrom.Set( 1, 1, 1, 1 );
	fadeTo.Set( 1, 1, 1, 1 );
	fadeStart = 0;
	fadeEnd	= 0;
	runGui = false;
}

/*
===============
idStaticEntity::Save
===============
*/
void idStaticEntity::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( spawnTime );
	savefile->WriteBool( active );
	savefile->WriteVec4( fadeFrom );
	savefile->WriteVec4( fadeTo );
	savefile->WriteInt( fadeStart );
	savefile->WriteInt( fadeEnd );
	savefile->WriteBool( runGui );
}

/*
===============
idStaticEntity::Restore
===============
*/
void idStaticEntity::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( spawnTime );
	savefile->ReadBool( active );
	savefile->ReadVec4( fadeFrom );
	savefile->ReadVec4( fadeTo );
	savefile->ReadInt( fadeStart );
	savefile->ReadInt( fadeEnd );
	savefile->ReadBool( runGui );
}

/*
===============
idStaticEntity::Spawn
===============
*/
void idStaticEntity::Spawn( void ) {
	bool solid;
	bool hidden;

	// an inline static model will not do anything at all
	if ( spawnArgs.GetBool( "inline" ) || gameLocal.world->spawnArgs.GetBool( "inlineAllStatics" ) ) {
		Hide();
		return;
	}

	solid = spawnArgs.GetBool( "solid" );
	hidden = spawnArgs.GetBool( "hide" );

	if ( solid && !hidden ) {
		GetPhysics()->SetContents( CONTENTS_SOLID );
	} else {
		GetPhysics()->SetContents( 0 );
	}

	spawnTime = gameLocal.time;
	active = false;

	idStr model = spawnArgs.GetString( "model" );
	if ( model.Find( ".prt" ) >= 0 ) {
		// we want the parametric particles out of sync with each other
		renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = gameLocal.random.RandomInt( 32767 );
	}

	fadeFrom.Set( 1, 1, 1, 1 );
	fadeTo.Set( 1, 1, 1, 1 );
	fadeStart = 0;
	fadeEnd	= 0;

	// NOTE: this should be used very rarely because it is expensive
	runGui = spawnArgs.GetBool( "runGui" );
	if ( runGui ) {
		BecomeActive( TH_THINK );
	}
}

/*
================
idStaticEntity::ShowEditingDialog
================
*/
void idStaticEntity::ShowEditingDialog( void ) {
	common->InitTool( EDITOR_PARTICLE, &spawnArgs );
}
/*
================
idStaticEntity::Think
================
*/
void idStaticEntity::Think( void ) {
	idEntity::Think();
	if ( thinkFlags & TH_THINK ) {
		if ( runGui && renderEntity.gui[0] ) {
			idPlayer *player = gameLocal.GetLocalPlayer();
			if ( player ) {
				if ( !player->objectiveSystemOpen ) {
					renderEntity.gui[0]->StateChanged( gameLocal.time, true );
					if ( renderEntity.gui[1] ) {
						renderEntity.gui[1]->StateChanged( gameLocal.time, true );
					}
					if ( renderEntity.gui[2] ) {
						renderEntity.gui[2]->StateChanged( gameLocal.time, true );
					}
				}
			}
		}
		if ( fadeEnd > 0 ) {
			idVec4 color;
			if ( gameLocal.time < fadeEnd ) {
				color.Lerp( fadeFrom, fadeTo, ( float )( gameLocal.time - fadeStart ) / ( float )( fadeEnd - fadeStart ) );
			} else {
				color = fadeTo;
				fadeEnd = 0;
				BecomeInactive( TH_THINK );
			}
			SetColor( color );
		}
	}
}

/*
================
idStaticEntity::Fade
================
*/
void idStaticEntity::Fade( const idVec4 &to, float fadeTime ) {
	GetColor( fadeFrom );
	fadeTo = to;
	fadeStart = gameLocal.time;
	fadeEnd = gameLocal.time + SEC2MS( fadeTime );
	BecomeActive( TH_THINK );
}

/*
================
idStaticEntity::Hide
================
*/
void idStaticEntity::Hide( void ) {
	idEntity::Hide();
	GetPhysics()->SetContents( 0 );
}

/*
================
idStaticEntity::Show
================
*/
void idStaticEntity::Show( void ) {
	idEntity::Show();
	if ( spawnArgs.GetBool( "solid" ) ) {
		GetPhysics()->SetContents( CONTENTS_SOLID );
	}
}

/*
================
idStaticEntity::Event_Activate
================
*/
void idStaticEntity::Event_Activate( idEntity *activator ) {
	idStr activateGui;

	spawnTime = gameLocal.time;
	active = !active;

	const idKeyValue *kv = spawnArgs.FindKey( "hide" );
	if ( kv ) {
		if ( IsHidden() ) {
			Show();
		} else {
			Hide();
		}
	}

	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( spawnTime );
	renderEntity.shaderParms[5] = active;
	// this change should be a good thing, it will automatically turn on
	// lights etc.. when triggered so that does not have to be specifically done
	// with trigger parms.. it MIGHT break things so need to keep an eye on it
	renderEntity.shaderParms[ SHADERPARM_MODE ] = ( renderEntity.shaderParms[ SHADERPARM_MODE ] ) ?  0.0f : 1.0f;
	BecomeActive( TH_UPDATEVISUALS );
}

/*
================
idStaticEntity::WriteToSnapshot
================
*/
void idStaticEntity::WriteToSnapshot( idBitMsgDelta &msg ) const {
	GetPhysics()->WriteToSnapshot( msg );
	WriteBindToSnapshot( msg );
	WriteColorToSnapshot( msg );
	WriteGUIToSnapshot( msg );
	msg.WriteBits( IsHidden()?1:0, 1 );
}

/*
================
idStaticEntity::ReadFromSnapshot
================
*/
void idStaticEntity::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	bool hidden;

	GetPhysics()->ReadFromSnapshot( msg );
	ReadBindFromSnapshot( msg );
	ReadColorFromSnapshot( msg );
	ReadGUIFromSnapshot( msg );
	hidden = msg.ReadBits( 1 ) == 1;
	if ( hidden != IsHidden() ) {
		if ( hidden ) {
			Hide();
		} else {
			Show();
		}
	}
	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}


//ivan start
/*
===============================================================================

	idTrailWrapper

	Some static entities may be optimized into inline geometry by dmap

===============================================================================
*/

CLASS_DECLARATION( idEntity, idTrailWrapper )
	EVENT( EV_Activate,				idTrailWrapper::Event_Activate )
END_CLASS

/*
===============
idTrailWrapper::idTrailWrapper
===============
*/
idTrailWrapper::idTrailWrapper( void ) {
	trailGen	= NULL;
	trailSize	= 0;
	oldLowPoint = vec3_origin;
	oldHighPoint = vec3_origin;
	newLowPoint = vec3_origin;
	newHighPoint = vec3_origin;
}

/*
===============
idTrailWrapper::~idTrailWrapper
===============
*/
idTrailWrapper::~idTrailWrapper( void ) {
	gameLocal.trailsManager->RemoveTrailGen( trailGen );
}

/*
===============
idTrailWrapper::Save
===============
*/
void idTrailWrapper::Save( idSaveGame *savefile ) const {
	//trailGen is saved elsewhere. Here we save only the id to retrieve it later
	savefile->WriteInt( gameLocal.trailsManager->GetSafeUniqueId( trailGen ) ); 
	savefile->WriteInt( trailSize );

	/*
	//not saved:
	oldLowPoint
	oldHighPoint
	newLowPoint
	newHighPoint
	*/
}

/*
===============
idTrailWrapper::Restore
===============
*/
void idTrailWrapper::Restore( idRestoreGame *savefile ) {
	int trailId; //trailGen was saved elsewhere. Here we read its id to retrieve it.
	savefile->ReadInt( trailId ); 
	trailGen = gameLocal.trailsManager->FindTrailByUniqueId( trailId );
	savefile->ReadInt( trailSize );

	oldLowPoint = vec3_origin;
	oldHighPoint = vec3_origin;
	newLowPoint = vec3_origin;
	newHighPoint = vec3_origin;
}

/*
===============
idTrailWrapper::Spawn
===============
*/
void idTrailWrapper::Spawn( void ) {
	trailSize = spawnArgs.GetInt( "trailSize", "25" );
	if( spawnArgs.GetBool( "startActive", "0" ) ){
		StartTrail();
	}

	GetPhysics()->SetClipBox( idBounds( vec3_origin ).Expand( 4 ), 1.0f );
	GetPhysics()->SetContents( 0 );
}


/*
================
idTrailWrapper::UpdateTrail
================
*/
void idTrailWrapper::UpdateTrail( void ) { //it assumes trailGen is valid and enabled
	//if( trailGen && trailGen->IsEnabled() ){	
		newLowPoint = GetPhysics()->GetOrigin();
		newHighPoint = newLowPoint + GetPhysics()->GetAxis()[2] * trailSize;
		
		if( oldLowPoint.Compare( newLowPoint, 1.0f ) && oldHighPoint.Compare( newHighPoint, 1.0f ) ){
			trailGen->RemoveOldestPoints();
		}else{
			trailGen->AddNewPoints( newLowPoint , newHighPoint ); 
		}

		//remember
		oldLowPoint = newLowPoint;
		oldHighPoint = newHighPoint;
	//}
}

/*
================
idTrailWrapper::Think
================
*/
void idTrailWrapper::Think( void ) {
	idEntity::Think();

	//think only until we need to upd the trail
	if ( thinkFlags & TH_THINK ) {
		if( trailGen ){
			if( trailGen->IsEnabled() ){
				UpdateTrail();
			}else{
				BecomeInactive( TH_THINK );
			}
		}else{
			BecomeInactive( TH_THINK );
		}
	}
	
}

/*
=================
idTrailWrapper::InitTrail
=================
*/
void idTrailWrapper::InitTrail( void ) {	
	if( !trailGen ){
		const idKeyValue* kv = spawnArgs.FindKey( "def_trail" );
		if( kv ){
			trailGen = gameLocal.trailsManager->NewTrailGen();
			if( trailGen ){ //if there is no free space for a new trail...just don't use it 
				trailGen->StartTrail( kv->GetValue() );
				BecomeActive( TH_THINK );
			}
		}else{
			gameLocal.Warning("idTrailWrapper '%s' has not trail defined! ", GetName() );
		}
	}
}


/*
=================
idTrailWrapper::StartTrail
=================
*/
void idTrailWrapper::StartTrail( void ) {	
	if( !trailGen ){
		InitTrail();
	}
	if( trailGen ){
		trailGen->RestartTrail();
		BecomeActive( TH_THINK );
	}
}

/*
================
idTrailWrapper::Fade
================
*/
void idTrailWrapper::FadeTrail( void ) {
	if( !trailGen ){
		InitTrail();
	}
	if( trailGen ){
		trailGen->FadeTrail();
		BecomeActive( TH_THINK );
	}
}

/*
================
idTrailWrapper::StopTrail
================
*/
void idTrailWrapper::StopTrail( void ) {
	if( trailGen ){
		trailGen->StopTrail();
		BecomeInactive( TH_THINK );
	}
}

/*
================
idTrailWrapper::Hide
================
*/
void idTrailWrapper::Hide( void ) {
	idEntity::Hide();
	StopTrail();
}


/*
================
idTrailWrapper::Show
================
*/
void idTrailWrapper::Show( void ) {
	idEntity::Show();
	StartTrail();
}


/*
================
idTrailWrapper::Event_Activate
================
*/
void idTrailWrapper::Event_Activate( idEntity *activator ) {
	
	//toggle state
	if( trailGen && trailGen->IsEnabled() && !trailGen->IsFading() ){
		FadeTrail();
	}else{
		StartTrail();
	}
}

//ivan end

/*
===============================================================================

idFuncEmitter

===============================================================================
*/


CLASS_DECLARATION( idStaticEntity, idFuncEmitter )
EVENT( EV_Activate,				idFuncEmitter::Event_Activate )
END_CLASS

/*
===============
idFuncEmitter::idFuncEmitter
===============
*/
idFuncEmitter::idFuncEmitter( void ) {
	hidden = false;
}

/*
===============
idFuncEmitter::Spawn
===============
*/
void idFuncEmitter::Spawn( void ) {
	if ( spawnArgs.GetBool( "start_off" ) ) {
		hidden = true;
		renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] = MS2SEC( 1 );
		UpdateVisuals();
	} else {
		hidden = false;
	}
}

/*
===============
idFuncEmitter::Save
===============
*/
void idFuncEmitter::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( hidden );
}

/*
===============
idFuncEmitter::Restore
===============
*/
void idFuncEmitter::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( hidden );
}

/*
================
idFuncEmitter::Event_Activate
================
*/
void idFuncEmitter::Event_Activate( idEntity *activator ) {
	if ( hidden || spawnArgs.GetBool( "cycleTrigger" ) ) {
		renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] = 0;
		renderEntity.shaderParms[SHADERPARM_TIMEOFFSET] = -MS2SEC( gameLocal.time );
		hidden = false;
	} else {
		renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] = MS2SEC( gameLocal.time );
		hidden = true;
	}
	UpdateVisuals();
}

/*
================
idFuncEmitter::WriteToSnapshot
================
*/
void idFuncEmitter::WriteToSnapshot( idBitMsgDelta &msg ) const {
	msg.WriteBits( hidden ? 1 : 0, 1 );
	msg.WriteFloat( renderEntity.shaderParms[ SHADERPARM_PARTICLE_STOPTIME ] );
	msg.WriteFloat( renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] );
}

/*
================
idFuncEmitter::ReadFromSnapshot
================
*/
void idFuncEmitter::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	hidden = msg.ReadBits( 1 ) != 0;
	renderEntity.shaderParms[ SHADERPARM_PARTICLE_STOPTIME ] = msg.ReadFloat();
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = msg.ReadFloat();
	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}


/*
===============================================================================

idFuncSplat

===============================================================================
*/


const idEventDef EV_Splat( "<Splat>" );
CLASS_DECLARATION( idFuncEmitter, idFuncSplat )
EVENT( EV_Activate,		idFuncSplat::Event_Activate )
EVENT( EV_Splat,		idFuncSplat::Event_Splat )
END_CLASS

/*
===============
idFuncSplat::idFuncSplat
===============
*/
idFuncSplat::idFuncSplat( void ) {
}

/*
===============
idFuncSplat::Spawn
===============
*/
void idFuncSplat::Spawn( void ) {
}

/*
================
idFuncSplat::Event_Splat
================
*/
void idFuncSplat::Event_Splat( void ) {
	const char *splat = NULL;
	int count = spawnArgs.GetInt( "splatCount", "1" );
	for ( int i = 0; i < count; i++ ) {
		splat = spawnArgs.RandomPrefix( "mtr_splat", gameLocal.random );
		if ( splat && *splat ) {
			float size = spawnArgs.GetFloat( "splatSize", "128" );
			float dist = spawnArgs.GetFloat( "splatDistance", "128" );
			float angle = spawnArgs.GetFloat( "splatAngle", "0" );
			gameLocal.ProjectDecal( GetPhysics()->GetOrigin(), GetPhysics()->GetAxis()[2], dist, true, size, splat, angle );
		}
	}
	StartSound( "snd_splat", SND_CHANNEL_ANY, 0, false, NULL );
}

/*
================
idFuncSplat::Event_Activate
================
*/
void idFuncSplat::Event_Activate( idEntity *activator ) {
	idFuncEmitter::Event_Activate( activator );
	PostEventSec( &EV_Splat, spawnArgs.GetFloat( "splatDelay", "0.25" ) );
	StartSound( "snd_spurt", SND_CHANNEL_ANY, 0, false, NULL );
}

/*
===============================================================================

idFuncSmoke

===============================================================================
*/

CLASS_DECLARATION( idEntity, idFuncSmoke )
EVENT( EV_Activate,				idFuncSmoke::Event_Activate )
END_CLASS

/*
===============
idFuncSmoke::idFuncSmoke
===============
*/
idFuncSmoke::idFuncSmoke() {
	smokeTime = 0;
	smoke = NULL;
	restart = false;
}

/*
===============
idFuncSmoke::Save
===============
*/
void idFuncSmoke::Save(	idSaveGame *savefile ) const {
	savefile->WriteInt( smokeTime );
	savefile->WriteParticle( smoke );
	savefile->WriteBool( restart );
}

/*
===============
idFuncSmoke::Restore
===============
*/
void idFuncSmoke::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( smokeTime );
	savefile->ReadParticle( smoke );
	savefile->ReadBool( restart );
}

/*
===============
idFuncSmoke::Spawn
===============
*/
void idFuncSmoke::Spawn( void ) {
	const char *smokeName = spawnArgs.GetString( "smoke" );
	if ( *smokeName != '\0' ) {
		smoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	} else {
		smoke = NULL;
	}
	if ( spawnArgs.GetBool( "start_off" ) ) {
		smokeTime = 0;
		restart = false;
	} else if ( smoke ) {
		smokeTime = gameLocal.time;
		BecomeActive( TH_UPDATEPARTICLES );
		restart = true;
	}
	GetPhysics()->SetContents( 0 );
}

/*
================
idFuncSmoke::Event_Activate
================
*/
void idFuncSmoke::Event_Activate( idEntity *activator ) {
	if ( thinkFlags & TH_UPDATEPARTICLES ) {
		//ivan start - ensure correct behaviour
		restart = !restart;
		//was: restart = false;
		//ivan end
		return;
	} else {
		BecomeActive( TH_UPDATEPARTICLES );
		restart = true;
		smokeTime = gameLocal.time;
	}
}

/*
===============
idFuncSmoke::Think
================
*/
void idFuncSmoke::Think( void ) {

	// if we are completely closed off from the player, don't do anything at all
	if ( CheckDormant() || smoke == NULL || smokeTime == -1 ) {
		return;
	}

	if ( ( thinkFlags & TH_UPDATEPARTICLES) && !IsHidden() ) {
		if ( !gameLocal.smokeParticles->EmitSmoke( smoke, smokeTime, gameLocal.random.CRandomFloat(), GetPhysics()->GetOrigin(), GetPhysics()->GetAxis() ) ) {
			if ( restart ) {
				smokeTime = gameLocal.time;
			} else {
				smokeTime = 0;
				BecomeInactive( TH_UPDATEPARTICLES );
			}
		}
	}

}


/*
===============================================================================

	idTextEntity

===============================================================================
*/

CLASS_DECLARATION( idEntity, idTextEntity )
END_CLASS

/*
================
idTextEntity::Spawn
================
*/
void idTextEntity::Spawn( void ) {
	// these are cached as the are used each frame
	text = spawnArgs.GetString( "text" );
	playerOriented = spawnArgs.GetBool( "playerOriented" );
	bool force = spawnArgs.GetBool( "force" );
	if ( developer.GetBool() || force ) {
		BecomeActive(TH_THINK);
	}
}

/*
================
idTextEntity::Save
================
*/
void idTextEntity::Save( idSaveGame *savefile ) const {
	savefile->WriteString( text );
	savefile->WriteBool( playerOriented );
}

/*
================
idTextEntity::Restore
================
*/
void idTextEntity::Restore( idRestoreGame *savefile ) {
	savefile->ReadString( text );
	savefile->ReadBool( playerOriented );
}

/*
================
idTextEntity::Think
================
*/
void idTextEntity::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		gameRenderWorld->DrawText( text, GetPhysics()->GetOrigin(), 0.25, colorWhite, playerOriented ? gameLocal.GetLocalPlayer()->viewAngles.ToMat3() : GetPhysics()->GetAxis().Transpose(), 1 );
		for ( int i = 0; i < targets.Num(); i++ ) {
			if ( targets[i].GetEntity() ) {
				gameRenderWorld->DebugArrow( colorBlue, GetPhysics()->GetOrigin(), targets[i].GetEntity()->GetPhysics()->GetOrigin(), 1 );
			}
		}
	} else {
		BecomeInactive( TH_ALL );
	}
}


/*
===============================================================================

	idVacuumSeperatorEntity

	Can be triggered to let vacuum through a portal (blown out window)

===============================================================================
*/

CLASS_DECLARATION( idEntity, idVacuumSeparatorEntity )
	EVENT( EV_Activate,		idVacuumSeparatorEntity::Event_Activate )
END_CLASS


/*
================
idVacuumSeparatorEntity::idVacuumSeparatorEntity
================
*/
idVacuumSeparatorEntity::idVacuumSeparatorEntity( void ) {
	portal = 0;
}

/*
================
idVacuumSeparatorEntity::Save
================
*/
void idVacuumSeparatorEntity::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( (int)portal );
	savefile->WriteInt( gameRenderWorld->GetPortalState( portal ) );
}

/*
================
idVacuumSeparatorEntity::Restore
================
*/
void idVacuumSeparatorEntity::Restore( idRestoreGame *savefile ) {
	int state;

	savefile->ReadInt( (int &)portal );
	savefile->ReadInt( state );

	gameLocal.SetPortalState( portal, state );
}

/*
================
idVacuumSeparatorEntity::Spawn
================
*/
void idVacuumSeparatorEntity::Spawn() {
	idBounds b;

	b = idBounds( spawnArgs.GetVector( "origin" ) ).Expand( 16 );
	portal = gameRenderWorld->FindPortal( b );
	if ( !portal ) {
		gameLocal.Warning( "VacuumSeparator '%s' didn't contact a portal", spawnArgs.GetString( "name" ) );
		return;
	}
	gameLocal.SetPortalState( portal, PS_BLOCK_AIR | PS_BLOCK_LOCATION );
}

/*
================
idVacuumSeparatorEntity::Event_Activate
================
*/
void idVacuumSeparatorEntity::Event_Activate( idEntity *activator ) {
	if ( !portal ) {
		return;
	}
	gameLocal.SetPortalState( portal, PS_BLOCK_NONE );
}


/*
===============================================================================

idLocationSeparatorEntity

===============================================================================
*/

CLASS_DECLARATION( idEntity, idLocationSeparatorEntity )
END_CLASS

/*
================
idLocationSeparatorEntity::Spawn
================
*/
void idLocationSeparatorEntity::Spawn() {
	idBounds b;

	b = idBounds( spawnArgs.GetVector( "origin" ) ).Expand( 16 );
	qhandle_t portal = gameRenderWorld->FindPortal( b );
	if ( !portal ) {
		gameLocal.Warning( "LocationSeparator '%s' didn't contact a portal", spawnArgs.GetString( "name" ) );
	}
	gameLocal.SetPortalState( portal, PS_BLOCK_LOCATION );
}


/*
===============================================================================

	idVacuumEntity

	Levels should only have a single vacuum entity.

===============================================================================
*/

CLASS_DECLARATION( idEntity, idVacuumEntity )
END_CLASS

/*
================
idVacuumEntity::Spawn
================
*/
void idVacuumEntity::Spawn() {
	if ( gameLocal.vacuumAreaNum != -1 ) {
		gameLocal.Warning( "idVacuumEntity::Spawn: multiple idVacuumEntity in level" );
		return;
	}

	idVec3 org = spawnArgs.GetVector( "origin" );

	gameLocal.vacuumAreaNum = gameRenderWorld->PointInArea( org );
}


/*
===============================================================================

idLocationEntity

===============================================================================
*/

CLASS_DECLARATION( idEntity, idLocationEntity )
END_CLASS

/*
======================
idLocationEntity::Spawn
======================
*/
void idLocationEntity::Spawn() {
	idStr realName;

	// this just holds dict information

	// if "location" not already set, use the entity name.
	if ( !spawnArgs.GetString( "location", "", realName ) ) {
		spawnArgs.Set( "location", name );
	}
}

/*
======================
idLocationEntity::GetLocation
======================
*/
const char *idLocationEntity::GetLocation( void ) const {
	return spawnArgs.GetString( "location" );
}

/*
//ivan: chain

//first
A
A

//add B
A <-(target)-- B
A --(master)-> B

//add C
A <-(target)-- B <-(target)-- C
A --(master)-> B --(master)-> C

The last one never has a master.

*/

/*
===============================================================================

	idBeam

===============================================================================
*/

const idEventDef EV_FadeBeamColor ( "<fadeOutBeam>" ); //ivan
CLASS_DECLARATION( idEntity, idBeam )
	EVENT( EV_PostSpawn,			idBeam::Event_MatchTarget )
	EVENT( EV_Activate,				idBeam::Event_Activate )
	EVENT( EV_FadeBeamColor,		idBeam::Event_FadeColor ) //ivan
END_CLASS

/*
===============
idBeam::idBeam
===============
*/
idBeam::idBeam() {
	target = NULL;
	master = NULL;

	//ivan start
	fadeTime = 0;
	fadeOutIntervals.Zero();
	//ivan end
}

//ivan start

/*
==============
idBeam::Event_FadeColor
==============
*/
void idBeam::Event_FadeColor( void ){ 
	FadeColor();
}

/*
==============
idBeam::FadeColor
==============
*/
void idBeam::FadeColor( void ){
	idVec4 beamColor;
	idVec4 fadeColor;
	int time;

	//fade time
	spawnArgs.GetInt( "fadeTime", "0", time );

	//dest color
	spawnArgs.GetVec4( "end_color", "0 0 0 0", fadeColor );  //default black
	//fadeColor.Zero();

	if( time <= 0 ) {
		renderEntity.shaderParms[ SHADERPARM_RED ]		= fadeColor[ 0 ];
		renderEntity.shaderParms[ SHADERPARM_GREEN ]	= fadeColor[ 1 ];
		renderEntity.shaderParms[ SHADERPARM_BLUE ]		= fadeColor[ 2 ];
		renderEntity.shaderParms[ SHADERPARM_ALPHA ]	= fadeColor[ 3 ];
		return;
	}

	//current color
	beamColor[ 0 ] = renderEntity.shaderParms[ SHADERPARM_RED ];
	beamColor[ 1 ] = renderEntity.shaderParms[ SHADERPARM_GREEN ];
	beamColor[ 2 ] = renderEntity.shaderParms[ SHADERPARM_BLUE ];
	beamColor[ 3 ] = renderEntity.shaderParms[ SHADERPARM_ALPHA ];

	//if we are here time != 0 
	fadeOutIntervals = ( beamColor - fadeColor) / time; //intervals for each ms
	fadeOutIntervals *= USERCMD_MSEC; //intervals for each frame
	fadeTime = gameLocal.time + time;

	BecomeActive( TH_THINK ); //start thinking
}

/*
==============
idBeam::AddChainNodeAtPos
==============
*/
idBeam* idBeam::AddChainNodeAtPos( const idVec3 &pos ) {

	if ( master.GetEntity() ) {
		gameLocal.Warning("This beam already has a master!");
		return NULL;
	}

	idBeam* newNode;
	idDict args;
	args.Copy( spawnArgs ); //copy my settings!
	args.SetVector( "origin", pos );
	args.Set( "target", GetName() ); //once spawned, I'll be his target and he'll be my master
	newNode = ( idBeam * )gameLocal.SpawnEntityType( idBeam::Type, &args );
		
	//gameLocal.Printf("Added new node at '%s'\n", pos.ToString() );

	return newNode;
}
//ivan end

/*
===============
idBeam::Save
===============
*/
void idBeam::Save( idSaveGame *savefile ) const {
	target.Save( savefile );
	master.Save( savefile );
	//ivan start
	savefile->WriteInt( fadeTime ); 
	savefile->WriteVec4( fadeOutIntervals ); 
	//ivan end
}

/*
===============
idBeam::Restore
===============
*/
void idBeam::Restore( idRestoreGame *savefile ) {
	target.Restore( savefile );
	master.Restore( savefile );
	//ivan start
	savefile->ReadInt( fadeTime ); 
	savefile->ReadVec4( fadeOutIntervals ); 
	//ivan end
}

/*
===============
idBeam::Spawn
===============
*/
void idBeam::Spawn( void ) {
	float width;

	if ( spawnArgs.GetFloat( "width", "0", width ) ) {
		renderEntity.shaderParms[ SHADERPARM_BEAM_WIDTH ] = width;
	}

	SetModel( "_BEAM" );
	Hide();

	PostEventMS( &EV_PostSpawn, 0 );
}

/*
================
idBeam::Think
================
*/
void idBeam::Think( void ) {
	idBeam *masterEnt;

	if ( !IsHidden() && !target.GetEntity() ) {
		// hide if our target is removed
		Hide();
	}

	RunPhysics();

	masterEnt = master.GetEntity();
	if ( masterEnt ) {
		const idVec3 &origin = GetPhysics()->GetOrigin();
		masterEnt->SetBeamTarget( origin );
	}

	//ivan start - fade
	if( fadeTime > gameLocal.time ) {	
		renderEntity.shaderParms[ SHADERPARM_RED ]		-= fadeOutIntervals[0];
		renderEntity.shaderParms[ SHADERPARM_GREEN ]	-= fadeOutIntervals[1];
		renderEntity.shaderParms[ SHADERPARM_BLUE ]		-= fadeOutIntervals[2];
		renderEntity.shaderParms[ SHADERPARM_ALPHA ]	-= fadeOutIntervals[3];

		BecomeActive( TH_UPDATEVISUALS ); //Present() will upd the renderEntity

	} else if( fadeTime > 0){ 
		fadeTime = 0;
		BecomeInactive( TH_THINK );

		if( spawnArgs.GetBool( "removeAfterFade", "0") ){
			PostEventMS( &EV_Remove, 0 );
		}
	}
	//ivan end

	Present();
}

/*
================
idBeam::SetMaster
================
*/
void idBeam::SetMaster( idBeam *masterbeam ) {
	master = masterbeam;
}

/*
================
idBeam::SetBeamTarget
================
*/
void idBeam::SetBeamTarget( const idVec3 &origin ) {
	if ( ( renderEntity.shaderParms[ SHADERPARM_BEAM_END_X ] != origin.x ) || ( renderEntity.shaderParms[ SHADERPARM_BEAM_END_Y ] != origin.y ) || ( renderEntity.shaderParms[ SHADERPARM_BEAM_END_Z ] != origin.z ) ) {
		renderEntity.shaderParms[ SHADERPARM_BEAM_END_X ] = origin.x;
		renderEntity.shaderParms[ SHADERPARM_BEAM_END_Y ] = origin.y;
		renderEntity.shaderParms[ SHADERPARM_BEAM_END_Z ] = origin.z;
		UpdateVisuals();
	}
}

/*
================
idBeam::Show
================
*/
void idBeam::Show( void ) {
	idBeam *targetEnt;

	idEntity::Show();

	targetEnt = target.GetEntity();
	if ( targetEnt ) {
		const idVec3 &origin = targetEnt->GetPhysics()->GetOrigin();
		SetBeamTarget( origin );
	}
}

/*
================
idBeam::Event_MatchTarget
================
*/
void idBeam::Event_MatchTarget( void ) {
	int i;
	idEntity *targetEnt;
	idBeam *targetBeam;

	if ( !targets.Num() ) {
		return;
	}

	targetBeam = NULL;
	for( i = 0; i < targets.Num(); i++ ) {
		targetEnt = targets[ i ].GetEntity();
		if ( targetEnt && targetEnt->IsType( idBeam::Type ) ) {
			targetBeam = static_cast<idBeam *>( targetEnt );
			break;
		}
	}

	if ( !targetBeam ) {
		gameLocal.Error( "Could not find valid beam target for '%s'", name.c_str() );
	}

	target = targetBeam;
	targetBeam->SetMaster( this );
	if ( !spawnArgs.GetBool( "start_off" ) ) {
		Show();
	}
}

/*
================
idBeam::Event_Activate
================
*/
void idBeam::Event_Activate( idEntity *activator ) {
	if ( IsHidden() ) {
		Show();
	} else {
		Hide();
	}
}

/*
================
idBeam::WriteToSnapshot
================
*/
void idBeam::WriteToSnapshot( idBitMsgDelta &msg ) const {
	GetPhysics()->WriteToSnapshot( msg );
	WriteBindToSnapshot( msg );
	WriteColorToSnapshot( msg );
	msg.WriteFloat( renderEntity.shaderParms[SHADERPARM_BEAM_END_X] );
	msg.WriteFloat( renderEntity.shaderParms[SHADERPARM_BEAM_END_Y] );
	msg.WriteFloat( renderEntity.shaderParms[SHADERPARM_BEAM_END_Z] );
}

/*
================
idBeam::ReadFromSnapshot
================
*/
void idBeam::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	GetPhysics()->ReadFromSnapshot( msg );
	ReadBindFromSnapshot( msg );
	ReadColorFromSnapshot( msg );
	renderEntity.shaderParms[SHADERPARM_BEAM_END_X] = msg.ReadFloat();
	renderEntity.shaderParms[SHADERPARM_BEAM_END_Y] = msg.ReadFloat();
	renderEntity.shaderParms[SHADERPARM_BEAM_END_Z] = msg.ReadFloat();
	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}


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
	// Nothing to save
}

/*
================
idLiquid::Restore
================
*/
void idLiquid::Restore( idRestoreGame *savefile ) {
	//FIXME: NO!
	Spawn();
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
	GetPhysics()->SetContents( CONTENTS_TRIGGER );
*/
}

/*
================
idLiquid::Event_Touch
================
*/
void idLiquid::Event_Touch( idEntity *other, trace_t *trace ) {
	// FIXME: for QuakeCon
/*
	idVec3 pos;

	pos = other->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
	model->IntersectBounds( other->GetPhysics()->GetBounds().Translate( pos ), -10.0f );
*/
}


/*
===============================================================================

	idShaking

===============================================================================
*/

CLASS_DECLARATION( idEntity, idShaking )
	EVENT( EV_Activate,				idShaking::Event_Activate )
END_CLASS

/*
===============
idShaking::idShaking
===============
*/
idShaking::idShaking() {
	active = false;
}

/*
===============
idShaking::Save
===============
*/
void idShaking::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( active );
	savefile->WriteStaticObject( physicsObj );
}

/*
===============
idShaking::Restore
===============
*/
void idShaking::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( active );
	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );
}

/*
===============
idShaking::Spawn
===============
*/
void idShaking::Spawn( void ) {
	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), 1.0f );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetClipMask( MASK_SOLID );
	SetPhysics( &physicsObj );

	active = false;
	if ( !spawnArgs.GetBool( "start_off" ) ) {
		BeginShaking();
	}
}

/*
================
idShaking::BeginShaking
================
*/
void idShaking::BeginShaking( void ) {
	int			phase;
	idAngles	shake;
	int			period;

	active = true;
	phase = gameLocal.random.RandomInt( 1000 );
	shake = spawnArgs.GetAngles( "shake", "0.5 0.5 0.5" );
	period = spawnArgs.GetFloat( "period", "0.05" ) * 1000;
	physicsObj.SetAngularExtrapolation( extrapolation_t(EXTRAPOLATION_DECELSINE|EXTRAPOLATION_NOSTOP), phase, period * 0.25f, GetPhysics()->GetAxis().ToAngles(), shake, ang_zero );
}

/*
================
idShaking::Event_Activate
================
*/
void idShaking::Event_Activate( idEntity *activator ) {
	if ( !active ) {
		BeginShaking();
	} else {
		active = false;
		physicsObj.SetAngularExtrapolation( EXTRAPOLATION_NONE, 0, 0, physicsObj.GetAxis().ToAngles(), ang_zero, ang_zero );
	}
}

/*
===============================================================================

	idEarthQuake

===============================================================================
*/

CLASS_DECLARATION( idEntity, idEarthQuake )
	EVENT( EV_Activate,				idEarthQuake::Event_Activate )
END_CLASS

/*
===============
idEarthQuake::idEarthQuake
===============
*/
idEarthQuake::idEarthQuake() {
	wait = 0.0f;
	random = 0.0f;
	nextTriggerTime = 0;
	shakeStopTime = 0;
	triggered = false;
	playerOriented = false;
	disabled = false;
	shakeTime = 0.0f;
}

/*
===============
idEarthQuake::Save
===============
*/
void idEarthQuake::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( nextTriggerTime );
	savefile->WriteInt( shakeStopTime );
	savefile->WriteFloat( wait );
	savefile->WriteFloat( random );
	savefile->WriteBool( triggered );
	savefile->WriteBool( playerOriented );
	savefile->WriteBool( disabled );
	savefile->WriteFloat( shakeTime );
}

/*
===============
idEarthQuake::Restore
===============
*/
void idEarthQuake::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( nextTriggerTime );
	savefile->ReadInt( shakeStopTime );
	savefile->ReadFloat( wait );
	savefile->ReadFloat( random );
	savefile->ReadBool( triggered );
	savefile->ReadBool( playerOriented );
	savefile->ReadBool( disabled );
	savefile->ReadFloat( shakeTime );

	if ( shakeStopTime > gameLocal.time ) {
		BecomeActive( TH_THINK );
	}
}

/*
===============
idEarthQuake::Spawn
===============
*/
void idEarthQuake::Spawn( void ) {
	nextTriggerTime = 0;
	shakeStopTime = 0;
	wait = spawnArgs.GetFloat( "wait", "15" );
	random = spawnArgs.GetFloat( "random", "5" );
	triggered = spawnArgs.GetBool( "triggered" );
	playerOriented = spawnArgs.GetBool( "playerOriented" );
	disabled = false;
	shakeTime = spawnArgs.GetFloat( "shakeTime", "0" );

	if ( !triggered ){
		PostEventSec( &EV_Activate, spawnArgs.GetFloat( "wait" ), this );
	}
	BecomeInactive( TH_THINK );
}

/*
================
idEarthQuake::Event_Activate
================
*/
void idEarthQuake::Event_Activate( idEntity *activator ) {

	if ( nextTriggerTime > gameLocal.time ) {
		return;
	}

	if ( disabled && activator == this ) {
		return;
	}

	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player == NULL ) {
		return;
	}

	nextTriggerTime = 0;

	if ( !triggered && activator != this ){
		// if we are not triggered ( i.e. random ), disable or enable
		disabled ^= 1;
		if (disabled) {
			return;
		} else {
			PostEventSec( &EV_Activate, wait + random * gameLocal.random.CRandomFloat(), this );
		}
	}

	ActivateTargets( activator );

	const idSoundShader *shader = declManager->FindSound( spawnArgs.GetString( "snd_quake" ) );
	if ( playerOriented ) {
		player->StartSoundShader( shader, SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
	} else {
		StartSoundShader( shader, SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
	}

	if ( shakeTime > 0.0f ) {
		shakeStopTime = gameLocal.time + SEC2MS( shakeTime );
		BecomeActive( TH_THINK );
	}

	if ( wait > 0.0f ) {
		if ( !triggered ) {
			PostEventSec( &EV_Activate, wait + random * gameLocal.random.CRandomFloat(), this );
		} else {
			nextTriggerTime = gameLocal.time + SEC2MS( wait + random * gameLocal.random.CRandomFloat() );
		}
	} else if ( shakeTime == 0.0f ) {
		PostEventMS( &EV_Remove, 0 );
	}
}


/*
===============
idEarthQuake::Think
================
*/
void idEarthQuake::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		if ( gameLocal.time > shakeStopTime ) {
			BecomeInactive( TH_THINK );
			if ( wait <= 0.0f ) {
				PostEventMS( &EV_Remove, 0 );
			}
			return;
		}
		float shakeVolume = gameSoundWorld->CurrentShakeAmplitudeForPosition( gameLocal.time, gameLocal.GetLocalPlayer()->firstPersonViewOrigin );
		gameLocal.RadiusPush( GetPhysics()->GetOrigin(), 256, 1500 * shakeVolume, this, this, 1.0f, true );
	}
	BecomeInactive( TH_UPDATEVISUALS );
}

/*
===============================================================================

	idFuncPortal

===============================================================================
*/

CLASS_DECLARATION( idEntity, idFuncPortal )
	EVENT( EV_Activate,				idFuncPortal::Event_Activate )
END_CLASS

/*
===============
idFuncPortal::idFuncPortal
===============
*/
idFuncPortal::idFuncPortal() {
	portal = 0;
	state = false;
}

/*
===============
idFuncPortal::Save
===============
*/
void idFuncPortal::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( (int)portal );
	savefile->WriteBool( state );
}

/*
===============
idFuncPortal::Restore
===============
*/
void idFuncPortal::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( (int &)portal );
	savefile->ReadBool( state );
	gameLocal.SetPortalState( portal, state ? PS_BLOCK_ALL : PS_BLOCK_NONE );
}

/*
===============
idFuncPortal::Spawn
===============
*/
void idFuncPortal::Spawn( void ) {
	portal = gameRenderWorld->FindPortal( GetPhysics()->GetAbsBounds().Expand( 32.0f ) );
	if ( portal > 0 ) {
		state = spawnArgs.GetBool( "start_on" );
		gameLocal.SetPortalState( portal, state ? PS_BLOCK_ALL : PS_BLOCK_NONE );
	}
}

/*
================
idFuncPortal::Event_Activate
================
*/
void idFuncPortal::Event_Activate( idEntity *activator ) {
	if ( portal > 0 ) {
		state = !state;
		gameLocal.SetPortalState( portal, state ? PS_BLOCK_ALL : PS_BLOCK_NONE );
	}
}

/*
===============================================================================

	idFuncAASPortal

===============================================================================
*/

CLASS_DECLARATION( idEntity, idFuncAASPortal )
	EVENT( EV_Activate,				idFuncAASPortal::Event_Activate )
END_CLASS

/*
===============
idFuncAASPortal::idFuncAASPortal
===============
*/
idFuncAASPortal::idFuncAASPortal() {
	state = false;
}

/*
===============
idFuncAASPortal::Save
===============
*/
void idFuncAASPortal::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( state );
}

/*
===============
idFuncAASPortal::Restore
===============
*/
void idFuncAASPortal::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( state );
	gameLocal.SetAASAreaState( GetPhysics()->GetAbsBounds(), AREACONTENTS_CLUSTERPORTAL, state );
}

/*
===============
idFuncAASPortal::Spawn
===============
*/
void idFuncAASPortal::Spawn( void ) {
	state = spawnArgs.GetBool( "start_on" );
	gameLocal.SetAASAreaState( GetPhysics()->GetAbsBounds(), AREACONTENTS_CLUSTERPORTAL, state );
}

/*
================
idFuncAASPortal::Event_Activate
================
*/
void idFuncAASPortal::Event_Activate( idEntity *activator ) {
	state ^= 1;
	gameLocal.SetAASAreaState( GetPhysics()->GetAbsBounds(), AREACONTENTS_CLUSTERPORTAL, state );
}

/*
===============================================================================

	idFuncAASObstacle

===============================================================================
*/

CLASS_DECLARATION( idEntity, idFuncAASObstacle )
	EVENT( EV_Activate,				idFuncAASObstacle::Event_Activate )
END_CLASS

/*
===============
idFuncAASObstacle::idFuncAASObstacle
===============
*/
idFuncAASObstacle::idFuncAASObstacle() {
	state = false;
}

/*
===============
idFuncAASObstacle::Save
===============
*/
void idFuncAASObstacle::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( state );
}

/*
===============
idFuncAASObstacle::Restore
===============
*/
void idFuncAASObstacle::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( state );
	gameLocal.SetAASAreaState( GetPhysics()->GetAbsBounds(), AREACONTENTS_OBSTACLE, state );
}

/*
===============
idFuncAASObstacle::Spawn
===============
*/
void idFuncAASObstacle::Spawn( void ) {
	state = spawnArgs.GetBool( "start_on" );
	gameLocal.SetAASAreaState( GetPhysics()->GetAbsBounds(), AREACONTENTS_OBSTACLE, state );
}

/*
================
idFuncAASObstacle::Event_Activate
================
*/
void idFuncAASObstacle::Event_Activate( idEntity *activator ) {
	state ^= 1;
	gameLocal.SetAASAreaState( GetPhysics()->GetAbsBounds(), AREACONTENTS_OBSTACLE, state );
}



/*
===============================================================================

idFuncRadioChatter

===============================================================================
*/

const idEventDef EV_ResetRadioHud( "<resetradiohud>", "e" );


CLASS_DECLARATION( idEntity, idFuncRadioChatter )
EVENT( EV_Activate,				idFuncRadioChatter::Event_Activate )
EVENT( EV_ResetRadioHud,		idFuncRadioChatter::Event_ResetRadioHud )
END_CLASS

/*
===============
idFuncRadioChatter::idFuncRadioChatter
===============
*/
idFuncRadioChatter::idFuncRadioChatter() {
	time = 0.0;
}

/*
===============
idFuncRadioChatter::Save
===============
*/
void idFuncRadioChatter::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( time );
}

/*
===============
idFuncRadioChatter::Restore
===============
*/
void idFuncRadioChatter::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( time );
}

/*
===============
idFuncRadioChatter::Spawn
===============
*/
void idFuncRadioChatter::Spawn( void ) {
	time = spawnArgs.GetFloat( "time", "5.0" );
}

/*
================
idFuncRadioChatter::Event_Activate
================
*/
void idFuncRadioChatter::Event_Activate( idEntity *activator ) {
	idPlayer *player;
	const char	*sound;
	const idSoundShader *shader;
	int length;

	if ( activator->IsType( idPlayer::Type ) ) {
		player = static_cast<idPlayer *>( activator );
	} else {
		player = gameLocal.GetLocalPlayer();
	}

	player->hud->HandleNamedEvent( "radioChatterUp" );

	sound = spawnArgs.GetString( "snd_radiochatter", "" );
	if ( sound && *sound ) {
		shader = declManager->FindSound( sound );
		player->StartSoundShader( shader, SND_CHANNEL_RADIO, SSF_GLOBAL, false, &length );
		time = MS2SEC( length + 150 );
	}
	// we still put the hud up because this is used with no sound on
	// certain frame commands when the chatter is triggered
	PostEventSec( &EV_ResetRadioHud, time, player );

}

/*
================
idFuncRadioChatter::Event_ResetRadioHud
================
*/
void idFuncRadioChatter::Event_ResetRadioHud( idEntity *activator ) {
	idPlayer *player = ( activator->IsType( idPlayer::Type ) ) ? static_cast<idPlayer *>( activator ) : gameLocal.GetLocalPlayer();
	player->hud->HandleNamedEvent( "radioChatterDown" );
	ActivateTargets( activator );
}


/*
===============================================================================

	idPhantomObjects

===============================================================================
*/

CLASS_DECLARATION( idEntity, idPhantomObjects )
	EVENT( EV_Activate,				idPhantomObjects::Event_Activate )
END_CLASS

/*
===============
idPhantomObjects::idPhantomObjects
===============
*/
idPhantomObjects::idPhantomObjects() {
	target			= NULL;
	end_time		= 0;
	throw_time		= 0.0f;
	shake_time		= 0.0f;
	shake_ang.Zero();
	speed			= 0.0f;
	min_wait		= 0;
	max_wait		= 0;
	fl.neverDormant	= false;
}

/*
===============
idPhantomObjects::Save
===============
*/
void idPhantomObjects::Save( idSaveGame *savefile ) const {
	int i;

	savefile->WriteInt( end_time );
	savefile->WriteFloat( throw_time );
	savefile->WriteFloat( shake_time );
	savefile->WriteVec3( shake_ang );
	savefile->WriteFloat( speed );
	savefile->WriteInt( min_wait );
	savefile->WriteInt( max_wait );
	target.Save( savefile );
	savefile->WriteInt( targetTime.Num() );
	for( i = 0; i < targetTime.Num(); i++ ) {
		savefile->WriteInt( targetTime[ i ] );
	}

	for( i = 0; i < lastTargetPos.Num(); i++ ) {
		savefile->WriteVec3( lastTargetPos[ i ] );
	}
}

/*
===============
idPhantomObjects::Restore
===============
*/
void idPhantomObjects::Restore( idRestoreGame *savefile ) {
	int num;
	int i;

	savefile->ReadInt( end_time );
	savefile->ReadFloat( throw_time );
	savefile->ReadFloat( shake_time );
	savefile->ReadVec3( shake_ang );
	savefile->ReadFloat( speed );
	savefile->ReadInt( min_wait );
	savefile->ReadInt( max_wait );
	target.Restore( savefile );

	savefile->ReadInt( num );
	targetTime.SetGranularity( 1 );
	targetTime.SetNum( num );
	lastTargetPos.SetGranularity( 1 );
	lastTargetPos.SetNum( num );

	for( i = 0; i < num; i++ ) {
		savefile->ReadInt( targetTime[ i ] );
	}

	if ( savefile->GetBuildNumber() == INITIAL_RELEASE_BUILD_NUMBER ) {
		// these weren't saved out in the first release
		for( i = 0; i < num; i++ ) {
			lastTargetPos[ i ].Zero();
		}
	} else {
		for( i = 0; i < num; i++ ) {
			savefile->ReadVec3( lastTargetPos[ i ] );
		}
	}
}

/*
===============
idPhantomObjects::Spawn
===============
*/
void idPhantomObjects::Spawn( void ) {
	throw_time = spawnArgs.GetFloat( "time", "5" );
	speed = spawnArgs.GetFloat( "speed", "1200" );
	shake_time = spawnArgs.GetFloat( "shake_time", "1" );
	throw_time -= shake_time;
	if ( throw_time < 0.0f ) {
		throw_time = 0.0f;
	}
	min_wait = SEC2MS( spawnArgs.GetFloat( "min_wait", "1" ) );
	max_wait = SEC2MS( spawnArgs.GetFloat( "max_wait", "3" ) );

	shake_ang = spawnArgs.GetVector( "shake_ang", "65 65 65" );
	Hide();
	GetPhysics()->SetContents( 0 );
}

/*
================
idPhantomObjects::Event_Activate
================
*/
void idPhantomObjects::Event_Activate( idEntity *activator ) {
	int i;
	float time;
	float frac;
	float scale;

	if ( thinkFlags & TH_THINK ) {
		BecomeInactive( TH_THINK );
		return;
	}

	RemoveNullTargets();
	if ( !targets.Num() ) {
		return;
	}

	if ( !activator || !activator->IsType( idActor::Type ) ) {
		target = gameLocal.GetLocalPlayer();
	} else {
		target = static_cast<idActor *>( activator );
	}

	end_time = gameLocal.time + SEC2MS( spawnArgs.GetFloat( "end_time", "0" ) );

	targetTime.SetNum( targets.Num() );
	lastTargetPos.SetNum( targets.Num() );

	const idVec3 &toPos = target.GetEntity()->GetEyePosition();

	// calculate the relative times of all the objects
	time = 0.0f;
	for( i = 0; i < targetTime.Num(); i++ ) {
		targetTime[ i ] = SEC2MS( time );
		lastTargetPos[ i ] = toPos;

		frac = 1.0f - ( float )i / ( float )targetTime.Num();
		time += ( gameLocal.random.RandomFloat() + 1.0f ) * 0.5f * frac + 0.1f;
	}

	// scale up the times to fit within throw_time
	scale = throw_time / time;
	for( i = 0; i < targetTime.Num(); i++ ) {
		targetTime[ i ] = gameLocal.time + SEC2MS( shake_time )+ targetTime[ i ] * scale;
	}

	BecomeActive( TH_THINK );
}

/*
===============
idPhantomObjects::Think
================
*/
void idPhantomObjects::Think( void ) {
	int			i;
	int			num;
	float		time;
	idVec3		vel;
	idVec3		ang;
	idEntity	*ent;
	idActor		*targetEnt;
	idPhysics	*entPhys;
	trace_t		tr;

	// if we are completely closed off from the player, don't do anything at all
	if ( CheckDormant() ) {
		return;
	}

	if ( !( thinkFlags & TH_THINK ) ) {
		BecomeInactive( thinkFlags & ~TH_THINK );
		return;
	}

	targetEnt = target.GetEntity();
	if ( !targetEnt || ( targetEnt->health <= 0 ) || ( end_time && ( gameLocal.time > end_time ) ) || gameLocal.inCinematic ) {
		BecomeInactive( TH_THINK );
	}

	const idVec3 &toPos = targetEnt->GetEyePosition();

	num = 0;
	for ( i = 0; i < targets.Num(); i++ ) {
		ent = targets[ i ].GetEntity();
		if ( !ent ) {
			continue;
		}

		if ( ent->fl.hidden ) {
			// don't throw hidden objects
			continue;
		}

		if ( !targetTime[ i ] ) {
			// already threw this object
			continue;
		}

		num++;

		time = MS2SEC( targetTime[ i ] - gameLocal.time );
		if ( time > shake_time ) {
			continue;
		}

		entPhys = ent->GetPhysics();
		const idVec3 &entOrg = entPhys->GetOrigin();

		gameLocal.clip.TracePoint( tr, entOrg, toPos, MASK_OPAQUE, ent );
		if ( tr.fraction >= 1.0f || ( gameLocal.GetTraceEntity( tr ) == targetEnt ) ) {
			lastTargetPos[ i ] = toPos;
		}

		if ( time < 0.0f ) {
			idAI::PredictTrajectory( entPhys->GetOrigin(), lastTargetPos[ i ], speed, entPhys->GetGravity(),
				entPhys->GetClipModel(), entPhys->GetClipMask(), 256.0f, ent, targetEnt, ai_debugTrajectory.GetBool() ? 1 : 0, vel );
			vel *= speed;
			entPhys->SetLinearVelocity( vel );
			if ( !end_time ) {
				targetTime[ i ] = 0;
			} else {
				targetTime[ i ] = gameLocal.time + gameLocal.random.RandomInt( max_wait - min_wait ) + min_wait;
			}
			if ( ent->IsType( idMoveable::Type ) ) {
				idMoveable *ment = static_cast<idMoveable*>( ent );
				ment->EnableDamage( true, 2.5f );
			}
		} else {
			// this is not the right way to set the angular velocity, but the effect is nice, so I'm keeping it. :)
			ang.Set( gameLocal.random.CRandomFloat() * shake_ang.x, gameLocal.random.CRandomFloat() * shake_ang.y, gameLocal.random.CRandomFloat() * shake_ang.z );
			ang *= ( 1.0f - time / shake_time );
			entPhys->SetAngularVelocity( ang );
		}
	}

	if ( !num ) {
		BecomeInactive( TH_THINK );
	}
}

//ivan start
#ifdef IVAN_TODO

/*
===============================================================================

	idProjLauncher

===============================================================================
*/

const idEventDef EV_FireProjectile( "fireProjectile", "v", 'e' ); 
const idEventDef EV_FireProjAtTarget( "fireProjAtTarget", "E", 'e' ); //E = NULL ok too

CLASS_DECLARATION( idEntity, idProjLauncher )
	EVENT( EV_Activate,				idProjLauncher::Event_Activate )
	EVENT( EV_FireProjectile,		idProjLauncher::Event_FireProjectile ) //ff 1.1
	EVENT( EV_FireProjAtTarget,		idProjLauncher::Event_FireProjAtTarget ) //ff 1.1
END_CLASS

/*
===============
idProjLauncher::idProjLauncher
===============
*/
idProjLauncher::idProjLauncher() {
	projectileDef		= NULL;
}

/*
===============
idProjLauncher::Save
===============
*/
void idProjLauncher::Save( idSaveGame *savefile ) const {
	idStr projectileName;
	spawnArgs.GetString( "def_projectile", "", projectileName );
	savefile->WriteString( projectileName );
}

/*
===============
idProjLauncher::Restore
===============
*/
void idProjLauncher::Restore( idRestoreGame *savefile ) {
	idStr projectileName;
	savefile->ReadString( projectileName );
	if ( projectileName.Length() ) {
		projectileDef = gameLocal.FindEntityDefDict( projectileName );
	} else {
		projectileDef = NULL;
	}
}

/*
===============
idProjLauncher::Spawn
===============
*/
void idProjLauncher::Spawn( void ) {
	idEntity *ent;
	const char *clsname;
	idStr projectileName;

	//find the def
	if ( spawnArgs.GetString( "def_projectile", "", projectileName ) && projectileName.Length() ) {
		projectileDef = gameLocal.FindEntityDefDict( projectileName );

		if ( !projectileDef->GetNumKeyVals() ) {
			gameLocal.Warning( "No projectile defined '%s'", projectileName );
			projectileDef = NULL;
			return;
		}
	}else{
		projectileDef = NULL;
		return;
	}

	//try to spawn and check if it's ok
	gameLocal.SpawnEntityDef( *projectileDef, &ent, false );
	if ( !ent ) {
		clsname = projectileDef->GetString( "classname" );
		gameLocal.Error( "Could not spawn entityDef '%s'", clsname );
	}
		
	if ( !ent->IsType( idProjectile::Type ) ) {
		clsname = ent->GetClassname();
		gameLocal.Error( "'%s' is not an idProjectile", clsname );
	}

	//proj is ok, delete it
	delete ent;
	ent = NULL;
}

/*
=====================
idProjLauncher::GetAimDir
=====================
*/
idVec3 idProjLauncher::GetAimDir( idEntity *aimAtEnt ) {
	idVec3	mainTargetPos;
	idVec3	secTargetPos; //not used, but necessary
	idVec3	aimDir;
	idVec3	firePos;

	firePos = GetPhysics()->GetOrigin();

	if ( aimAtEnt ) { //target entity ok!
		if ( aimAtEnt->IsType( idPlayer::Type ) ) { //player - head
			static_cast<idPlayer *>( aimAtEnt )->GetAIAimTargets( aimAtEnt->GetPhysics()->GetOrigin(), mainTargetPos, secTargetPos );
		} else if ( aimAtEnt->IsType( idActor::Type ) ) { //AI - chest
			static_cast<idActor *>( aimAtEnt )->GetAIAimTargets( aimAtEnt->GetPhysics()->GetOrigin(), secTargetPos, mainTargetPos );
		} else { //center
			mainTargetPos = aimAtEnt->GetPhysics()->GetAbsBounds().GetCenter();
			//secTargetPos = vec3_origin;
		}
		
		aimDir = mainTargetPos - firePos;
		aimDir.Normalize();

	} else { //no valid aimAtEnt entity!
		aimDir = GetPhysics()->GetAxis().ToAngles().ToForward();
	}

	return aimDir;
}

/*
================
idProjLauncher::FireProjectile
================
*/
idProjectile* idProjLauncher::FireProjectile( idVec3 &dir ) {
	idProjectile	*proj;
	idEntity		*ent;
	idEntity		*owner;
	idVec3			firePos;

	if ( gameLocal.isClient ) { return NULL; }

	if ( !projectileDef ) { return NULL; }

	gameLocal.SpawnEntityDef( *projectileDef, &ent, false );

	if ( !ent || !ent->IsType( idProjectile::Type ) ) {
		gameLocal.Error( "idProjLauncher was not able to fire the Projectile");
	}

	if ( projectileDef->GetBool( "net_instanthit" ) ) {
		ent->fl.networkSync = false;
	}

	firePos = GetPhysics()->GetOrigin();
	
	owner = GetBindMaster();
	if(!owner){ owner = this; }

	proj = static_cast<idProjectile *>(ent);
	proj->Create( owner, firePos, dir );
	proj->Launch( firePos, dir, vec3_origin ); //0.0f, 1.0f, 1.0f 
	return proj;
}
/*
================
idProjLauncher::Event_Activate
================
*/
void idProjLauncher::Event_Activate( idEntity *activator ) { //triggered
	idVec3 dir;
	dir = GetPhysics()->GetAxis().ToAngles().ToForward();
	
	FireProjectile( dir );
}

/*
================
idProjLauncher::Event_FireProjectile
================
*/
void idProjLauncher::Event_FireProjectile( idAngles &fireAng ) {
	idProjectile	*proj;
	idVec3			dir;

	dir = fireAng.ToForward();
	proj = FireProjectile( dir );

	idThread::ReturnEntity( proj );
}

/*
================
idProjLauncher::Event_FireProjAtTarget
================
*/
void idProjLauncher::Event_FireProjAtTarget( idEntity* aimAtEnt) {
	idProjectile	*proj;
	idVec3			dir;

	dir = GetAimDir( aimAtEnt );
	proj = FireProjectile( dir );

	idThread::ReturnEntity( proj );
}
#endif 
//ivan end


#ifdef _PORTALSKY
/*
===============================================================================

idPortalSky

===============================================================================
*/

CLASS_DECLARATION( idEntity, idPortalSky )
	EVENT( EV_PostSpawn,			idPortalSky::Event_PostSpawn )
	EVENT( EV_Activate,				idPortalSky::Event_Activate )
END_CLASS

/*
===============
idPortalSky::idPortalSky
===============
*/
idPortalSky::idPortalSky( void ) {

}

/*
===============
idPortalSky::~idPortalSky
===============
*/
idPortalSky::~idPortalSky( void ) {

}

/*
===============
idPortalSky::Spawn
===============
*/
void idPortalSky::Spawn( void ) {
	if ( !spawnArgs.GetBool( "triggered" ) ) {
		PostEventMS( &EV_PostSpawn, 1 );
	}
}

/*
================
idPortalSky::Event_PostSpawn
================
*/
void idPortalSky::Event_PostSpawn() {
	gameLocal.SetPortalSkyEnt( this );
}

/*
================
idPortalSky::Event_Activate
================
*/
void idPortalSky::Event_Activate( idEntity *activator ) {
	gameLocal.SetPortalSkyEnt( this );
}

#endif /* _PORTALSKY */
