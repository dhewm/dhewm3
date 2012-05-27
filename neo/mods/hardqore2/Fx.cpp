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

#include "gamesys/SysCvar.h"
#include "Player.h"
#include "Projectile.h"
#include "WorldSpawn.h"
#include "../framework/DeclEntityDef.h"

#include "Fx.h"

/*
===============================================================================

	idEntityFx

===============================================================================
*/

const idEventDef EV_Fx_FadeFx( "fadeFx" ); //ivan - also remove it
const idEventDef EV_Fx_KillFx( "_killfx" );
const idEventDef EV_Fx_Action( "_fxAction", "e" );		// implemented by subclasses

CLASS_DECLARATION( idEntity, idEntityFx )
EVENT( EV_Activate,		idEntityFx::Event_Trigger )
EVENT( EV_Fx_KillFx,	idEntityFx::Event_ClearFx )
EVENT( EV_Fx_FadeFx,	idEntityFx::Event_FadeFx ) //ivan
END_CLASS


/*
================
idEntityFx::Save
================
*/
void idEntityFx::Save( idSaveGame *savefile ) const {
	int i;

	//ivan start
	savefile->WriteBool( manualRemove );
	savefile->WriteBool( manualFadeIsOn );
	savefile->WriteBool( endlessSounds );
	//ivan end

	savefile->WriteInt( started );
	savefile->WriteInt( nextTriggerTime );
	savefile->WriteFX( fxEffect );
	savefile->WriteString( systemName );

	savefile->WriteInt( actions.Num() );

	for ( i = 0; i < actions.Num(); i++ ) {

		if ( actions[i].lightDefHandle >= 0 ) {
			savefile->WriteBool( true );
			savefile->WriteRenderLight( actions[i].renderLight );
		} else {
			savefile->WriteBool( false );
		}

		if ( actions[i].modelDefHandle >= 0 ) {
			savefile->WriteBool( true );
			savefile->WriteRenderEntity( actions[i].renderEntity );
		} else {
			savefile->WriteBool( false );
		}

		savefile->WriteFloat( actions[i].delay );
		savefile->WriteInt( actions[i].start );
		savefile->WriteBool( actions[i].soundStarted );
		savefile->WriteBool( actions[i].shakeStarted );
		savefile->WriteBool( actions[i].decalDropped );
		savefile->WriteBool( actions[i].launched );
	}
}

/*
================
idEntityFx::Restore
================
*/
void idEntityFx::Restore( idRestoreGame *savefile ) {
	int i;
	int num;
	bool hasObject;

	//ivan start
	savefile->ReadBool( manualRemove );
	savefile->ReadBool( manualFadeIsOn );
	savefile->ReadBool( endlessSounds );
	//ivan end

	savefile->ReadInt( started );
	savefile->ReadInt( nextTriggerTime );
	savefile->ReadFX( fxEffect );
	savefile->ReadString( systemName );

	savefile->ReadInt( num );

	actions.SetNum( num );
	for ( i = 0; i < num; i++ ) {

		savefile->ReadBool( hasObject );
		if ( hasObject ) {
			savefile->ReadRenderLight( actions[i].renderLight );
			actions[i].lightDefHandle = gameRenderWorld->AddLightDef( &actions[i].renderLight );
		} else {
			memset( &actions[i].renderLight, 0, sizeof( renderLight_t ) );
			actions[i].lightDefHandle = -1;
		}

		savefile->ReadBool( hasObject );
		if ( hasObject ) {
			savefile->ReadRenderEntity( actions[i].renderEntity );
			actions[i].modelDefHandle = gameRenderWorld->AddEntityDef( &actions[i].renderEntity );
		} else {
			memset( &actions[i].renderEntity, 0, sizeof( renderEntity_t ) );
			actions[i].modelDefHandle = -1;
		}

		savefile->ReadFloat( actions[i].delay );

		// let the FX regenerate the particleSystem
		actions[i].particleSystem = -1;

		savefile->ReadInt( actions[i].start );
		savefile->ReadBool( actions[i].soundStarted );
		savefile->ReadBool( actions[i].shakeStarted );
		savefile->ReadBool( actions[i].decalDropped );
		savefile->ReadBool( actions[i].launched );
	}
}

/*
================
idEntityFx::Setup
================
*/
void idEntityFx::Setup( const char *fx ) {

	if ( started >= 0 ) {
		return;					// already started
	}

	// early during MP Spawn() with no information. wait till we ReadFromSnapshot for more
	if ( gameLocal.isClient && ( !fx || fx[0] == '\0' ) ) {
		return;
	}

	systemName = fx;
	started = 0;
	endlessSounds = false; //ivan

	fxEffect = static_cast<const idDeclFX *>( declManager->FindType( DECL_FX, systemName.c_str() ) );

	if ( fxEffect ) {
		idFXLocalAction localAction;

		memset( &localAction, 0, sizeof( idFXLocalAction ) );

		actions.AssureSize( fxEffect->events.Num(), localAction );

		for( int i = 0; i<fxEffect->events.Num(); i++ ) {
			const idFXSingleAction& fxaction = fxEffect->events[i];

			idFXLocalAction& laction = actions[i];
			if ( fxaction.random1 || fxaction.random2 ) {
				laction.delay = fxaction.random1 + gameLocal.random.RandomFloat() * ( fxaction.random2 - fxaction.random1 );
			} else {
				laction.delay = fxaction.delay;
			}
			laction.start = -1;
			laction.lightDefHandle = -1;
			laction.modelDefHandle = -1;
			laction.particleSystem = -1;
			laction.shakeStarted = false;
			laction.decalDropped = false;
			laction.launched = false;

			//ivan start
			if ( !endlessSounds && fxaction.type == FX_SOUND && fxaction.duration <= 0 ){
				endlessSounds = true; 
				//gameLocal.Printf("this FX has endlessSounds");
			}
			//ivan end
		}
	}
}

/*
================
idEntityFx::EffectName
================
*/
const char *idEntityFx::EffectName( void ) {
	return fxEffect ? fxEffect->GetName() : NULL;
}

/*
================
idEntityFx::Joint
================
*/
const char *idEntityFx::Joint( void ) {
	return fxEffect ? fxEffect->joint.c_str() : NULL;
}

//ivan start
/*
================
idEntityFx::ResetShaderParms
================
*/
void idEntityFx::ResetShaderParms( void ) {
	if ( !fxEffect ) {
		return;
	}
	for( int i = 0; i < fxEffect->events.Num(); i++ ) {
		const idFXSingleAction& fxaction = fxEffect->events[i];
		idFXLocalAction& laction = actions[i];
		
		if ( laction.lightDefHandle != -1 && fxaction.sibling == -1 && fxaction.type != FX_ATTACHLIGHT ) {
			laction.renderLight.shaderParms[ SHADERPARM_RED ]	= fxaction.lightColor.x;
			laction.renderLight.shaderParms[ SHADERPARM_GREEN ]	= fxaction.lightColor.y;
			laction.renderLight.shaderParms[ SHADERPARM_BLUE ]	= fxaction.lightColor.z;
			laction.renderLight.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
		}
		if ( laction.modelDefHandle != -1 && fxaction.sibling == -1 && fxaction.type != FX_ATTACHENTITY ) {
			laction.renderEntity.shaderParms[ SHADERPARM_RED ]		= 1.0f;
			laction.renderEntity.shaderParms[ SHADERPARM_GREEN ]	= 1.0f;
			laction.renderEntity.shaderParms[ SHADERPARM_BLUE ]		= 1.0f;
			if( manualFadeIsOn ){ //no need to reset it if we are not fading - this would make the particle to change position instantly
				laction.renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
			}
			laction.renderEntity.shaderParms[ SHADERPARM_PARTICLE_STOPTIME ] = 0; //make particles to appear
		}
	}
}

//ivan end

/*
================
idEntityFx::CleanUp
================
*/
void idEntityFx::CleanUp( void ) {
	if ( !fxEffect ) {
		return;
	}
	for( int i = 0; i < fxEffect->events.Num(); i++ ) {
		const idFXSingleAction& fxaction = fxEffect->events[i];
		idFXLocalAction& laction = actions[i];
		CleanUpSingleAction( fxaction, laction );
	}
}

/*
================
idEntityFx::CleanUpSingleAction
================
*/
void idEntityFx::CleanUpSingleAction( const idFXSingleAction& fxaction, idFXLocalAction& laction ) {
	if ( laction.lightDefHandle != -1 && fxaction.sibling == -1 && fxaction.type != FX_ATTACHLIGHT ) {
		gameRenderWorld->FreeLightDef( laction.lightDefHandle );
		laction.lightDefHandle = -1;
	}
	if ( laction.modelDefHandle != -1 && fxaction.sibling == -1 && fxaction.type != FX_ATTACHENTITY ) {
		gameRenderWorld->FreeEntityDef( laction.modelDefHandle );
		laction.modelDefHandle = -1;
	}
	laction.start = -1;
}

/*
================
idEntityFx::Start
================
*/
void idEntityFx::Start( int time ) {
	if ( !fxEffect ) {
		return;
	}

	//ivan start
	// already started
	bool restartSounds = true;
	if ( started >= 0 ){
		if( manualFadeIsOn && endlessSounds ) { //if fading and endless we need to stop them
			StopSound( SND_CHANNEL_ANY, false );
			//gameLocal.Printf("sounds stopped!");
		}else{
			restartSounds = false;
			//gameLocal.Printf("sounds not stopped!");
		}
	}
	manualFadeIsOn = false; 
	//ivan end

	started = time;

	for( int i = 0; i < fxEffect->events.Num(); i++ ) {
		idFXLocalAction& laction = actions[i];
		
		//ivan start
		//was: laction.start = time;
		if( laction.delay < 0 ){ //will be started manually
			laction.start = -1; //start disabled
		}else{
			laction.start = time;
		}
	
		//was: laction.soundStarted = false;
		if( restartSounds ){
			laction.soundStarted = false;
		} //else don't change it!

		//ivan end

		laction.shakeStarted = false;
		laction.particleSystem = -1;
		laction.decalDropped = false;
		laction.launched = false;
	}
}

/*
================
idEntityFx::Stop
================
*/
void idEntityFx::Stop( void ) {
	CleanUp();
	started = -1;
}

//ivan start
/*
================
idEntityFx::FadeDuration

It returns the max 'duration' of the fadeOut stage with "delay -1" (manually started)
================
*/
const int idEntityFx::FadeDuration( void ) {
	int max = 0;

	if ( !fxEffect ) {
		return max;
	}
	for( int i = 0; i < fxEffect->events.Num(); i++ ) {
		const idFXSingleAction& fxaction = fxEffect->events[i];

		if( fxaction.delay > 0 || fxaction.fadeOutTime <= 0 ){ //than it's not part of the manual fade 
			continue; 
		}

		int d = ( fxaction.duration ) * 1000.0f;
		if ( d > max ) {
			max = d;
		}
	}

	//gameLocal.Printf("FadeDuration: %d\n", max);
	return max;
}

//ivan end

/*
================
idEntityFx::Duration
================
*/
const int idEntityFx::Duration( void ) {
	int max = 0;

	if ( !fxEffect ) {
		return max;
	}
	for( int i = 0; i < fxEffect->events.Num(); i++ ) {
		const idFXSingleAction& fxaction = fxEffect->events[i];
		int d = ( fxaction.delay + fxaction.duration ) * 1000.0f;
		if ( d > max ) {
			max = d;
		}
	}

	return max;
}


/*
================
idEntityFx::Done
================
*/
const bool idEntityFx::Done() {
	//ivan end
	if( manualRemove ){ //it never is "done"... unles it has been removed :)
		return false;
	}
	//ivan end
	if (started > 0 && gameLocal.time > started + Duration()) {
		return true;
	}
	return false;
}

/*
================
idEntityFx::ApplyFade
================
*/
void idEntityFx::ApplyFade( const idFXSingleAction& fxaction, idFXLocalAction& laction, const int time, const int actualStart ) {
	if ( fxaction.fadeInTime || fxaction.fadeOutTime ) {
		float fadePct = (float)( time - actualStart ) / ( 1000.0f * ( ( fxaction.fadeInTime != 0 ) ? fxaction.fadeInTime : fxaction.fadeOutTime ) );
		if (fadePct > 1.0) {
			fadePct = 1.0;
		}
		if ( laction.modelDefHandle != -1 ) {
			//ivan start - use entity color
			
			laction.renderEntity.shaderParms[SHADERPARM_RED] = (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct;
			laction.renderEntity.shaderParms[SHADERPARM_GREEN] = (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct;
			laction.renderEntity.shaderParms[SHADERPARM_BLUE] = (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct;
			
			/*
			//TODO: use entity color fr particles?
			laction.renderEntity.shaderParms[SHADERPARM_RED] =	renderEntity.shaderParms[SHADERPARM_RED] * (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct;
			laction.renderEntity.shaderParms[SHADERPARM_GREEN] = renderEntity.shaderParms[SHADERPARM_GREEN] * (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct;
			laction.renderEntity.shaderParms[SHADERPARM_BLUE] = renderEntity.shaderParms[SHADERPARM_BLUE] * (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct;
			*/

			/*
			//fade out particles
			if (( fxaction.fadeInTime == 0 ) && fadePct < 1.0f) { //fading out
				if( laction.renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] == 0 ){
					//gameLocal.Printf("SHADERPARM_PARTICLE_STOPTIME\n");
					laction.renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] = MS2SEC( time );
				}
			}
			*/
			/*
			else{
				gameLocal.Printf("SHADERPARM_PARTICLE_STOPTIME reset\n");
				laction.renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] = 0;
			}
			*/
			//ivan end
			gameRenderWorld->UpdateEntityDef( laction.modelDefHandle, &laction.renderEntity );
		}
		if ( laction.lightDefHandle != -1 ) {
			laction.renderLight.shaderParms[SHADERPARM_RED] = fxaction.lightColor.x * ( (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct );
			laction.renderLight.shaderParms[SHADERPARM_GREEN] = fxaction.lightColor.y * ( (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct );
			laction.renderLight.shaderParms[SHADERPARM_BLUE] = fxaction.lightColor.z * ( (fxaction.fadeInTime) ? fadePct : 1.0f - fadePct );

			//gameLocal.Printf("upd light fadePct %f, shaderParms %f %f %f\n", fadePct,laction.renderLight.shaderParms[SHADERPARM_RED],laction.renderLight.shaderParms[SHADERPARM_GREEN],laction.renderLight.shaderParms[SHADERPARM_BLUE]);

			gameRenderWorld->UpdateLightDef( laction.lightDefHandle, &laction.renderLight );
		}
	}
}

/*
================
idEntityFx::Run
================
*/
void idEntityFx::Run( int time ) {
	int ieff, j;
	idEntity *ent = NULL;
	const idDict *projectileDef = NULL;
	idProjectile *projectile = NULL;

	if ( !fxEffect ) {
		return;
	}

	for( ieff = 0; ieff < fxEffect->events.Num(); ieff++ ) {
		const idFXSingleAction& fxaction = fxEffect->events[ieff];
		idFXLocalAction& laction = actions[ieff];

		//
		// if we're currently done with this one
		//
		if ( laction.start == -1 ) {
			continue;
		}

		//
		// see if it's delayed
		//
		if ( laction.delay > 0 ) {
			if ( laction.start + (time - laction.start) < laction.start + (laction.delay * 1000) ) {
				continue;
			}
		}
		//ivan start - negative delay: they are meant to be started only manually 
		else if ( laction.delay < 0 ) {
			if( !manualFadeIsOn ){ continue; }
			/*
			if( manualFadeIsOn ){
				if( laction.start == 0 ){ 
					gameLocal.Printf("laction.start = time; <-- start now \n");
					laction.start = time; //start now 
				} 
			}else{
				continue;
			}
			*/
		}
		//ivan end

		//
		// each event can have it's own delay and restart
		//
		//was: int actualStart = laction.delay ? laction.start + (int)( laction.delay * 1000 ) : laction.start;
		int actualStart = ( laction.delay > 0 )? laction.start + (int)( laction.delay * 1000 ) : laction.start;
		if( fxaction.duration > 0 ){ //ivan - if added: duration 0 means endless
			float pct = (float)( time - actualStart ) / (1000 * fxaction.duration );
			if ( pct >= 1.0f ) {
				laction.start = -1;
				float totalDelay = 0.0f;
				if ( fxaction.restart ) {
					if ( fxaction.random1 || fxaction.random2 ) {
						totalDelay = fxaction.random1 + gameLocal.random.RandomFloat() * (fxaction.random2 - fxaction.random1);
					} else {
						totalDelay = fxaction.delay;
					}
					laction.delay = totalDelay;
					laction.start = time;
				} 
				continue;
			}
		}

		if ( fxaction.fire.Length() ) {
			for( j = 0; j < fxEffect->events.Num(); j++ ) {
				if ( fxEffect->events[j].name.Icmp( fxaction.fire ) == 0 ) {
					actions[j].delay = 0;
				}
			}
		}

		idFXLocalAction *useAction;
		if ( fxaction.sibling == -1 ) {
			useAction = &laction;
		} else {
			useAction = &actions[fxaction.sibling];
		}
		assert( useAction );

		switch( fxaction.type ) {
			case FX_ATTACHLIGHT:
			case FX_LIGHT: {
				if ( useAction->lightDefHandle == -1 ) {
					if ( fxaction.type == FX_LIGHT ) {
						//gameLocal.Printf("new light\n");
						memset( &useAction->renderLight, 0, sizeof( renderLight_t ) );
						useAction->renderLight.origin = GetPhysics()->GetOrigin() + fxaction.offset;
						useAction->renderLight.axis = GetPhysics()->GetAxis();
						useAction->renderLight.lightRadius[0] = fxaction.lightRadius;
						useAction->renderLight.lightRadius[1] = fxaction.lightRadius;
						useAction->renderLight.lightRadius[2] = fxaction.lightRadius;
						useAction->renderLight.shader = declManager->FindMaterial( fxaction.data, false );
						useAction->renderLight.shaderParms[ SHADERPARM_RED ]	= fxaction.lightColor.x;
						useAction->renderLight.shaderParms[ SHADERPARM_GREEN ]	= fxaction.lightColor.y;
						useAction->renderLight.shaderParms[ SHADERPARM_BLUE ]	= fxaction.lightColor.z;
						useAction->renderLight.shaderParms[ SHADERPARM_TIMESCALE ]	= 1.0f;
						useAction->renderLight.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( time );
						useAction->renderLight.referenceSound = refSound.referenceSound;
						useAction->renderLight.pointLight = true;
						if ( fxaction.noshadows ) {
							useAction->renderLight.noShadows = true;
						}
						useAction->lightDefHandle = gameRenderWorld->AddLightDef( &useAction->renderLight );
					}
					if ( fxaction.noshadows ) {
						for( j = 0; j < fxEffect->events.Num(); j++ ) {
							idFXLocalAction& laction2 = actions[j];
							if ( laction2.modelDefHandle != -1 ) {
								laction2.renderEntity.noShadow = true;
							}
						}
					}
				}
				//ivan start
				else if ( fxaction.trackOrigin ) {
						
					//todo: add code for explicitAxis also here?
					//gameLocal.Printf("light trackOrigin\n");
					useAction->renderLight.origin = GetPhysics()->GetOrigin() + fxaction.offset;
					useAction->renderLight.axis = GetPhysics()->GetAxis();

					//track origin fix
					//gameLocal.Printf("FX_LIGHT UpdateLightDef\n");
					gameRenderWorld->UpdateLightDef( useAction->lightDefHandle, &useAction->renderLight );
					
				}
				//ivan end

				ApplyFade( fxaction, *useAction, time, actualStart );
				break;
			}
			case FX_SOUND: {
				if ( !useAction->soundStarted ) {
					useAction->soundStarted = true;
					//gameLocal.Printf("FX_SOUND StartSoundShader\n");
					const idSoundShader *shader = declManager->FindSound(fxaction.data);
					StartSoundShader( shader, SND_CHANNEL_ANY, 0, false, NULL );
					for( j = 0; j < fxEffect->events.Num(); j++ ) {
						idFXLocalAction& laction2 = actions[j];
						if ( laction2.lightDefHandle != -1 ) {
							//gameLocal.Printf("FX_SOUND UpdateLightDef\n");
							laction2.renderLight.referenceSound = refSound.referenceSound;
							gameRenderWorld->UpdateLightDef( laction2.lightDefHandle, &laction2.renderLight );
						}
					}
				}
				break;
			}
			case FX_DECAL: {
				if ( !useAction->decalDropped ) {
					useAction->decalDropped = true;
					gameLocal.ProjectDecal( GetPhysics()->GetOrigin(), GetPhysics()->GetGravity(), 8.0f, true, fxaction.size, fxaction.data );
				}
				break;
			}
			case FX_SHAKE: {
				if ( !useAction->shakeStarted ) {
					idDict args;
					args.Clear();
					args.SetFloat( "kick_time", fxaction.shakeTime );
					args.SetFloat( "kick_amplitude", fxaction.shakeAmplitude );
					for ( j = 0; j < gameLocal.numClients; j++ ) {
						idPlayer *player = gameLocal.GetClientByNum( j );
						if ( player && ( player->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin() ).LengthSqr() < Square( fxaction.shakeDistance ) ) {
							if ( !gameLocal.isMultiplayer || !fxaction.shakeIgnoreMaster || GetBindMaster() != player ) {
								player->playerView.DamageImpulse( fxaction.offset, &args );
							}
						}
					}
					if ( fxaction.shakeImpulse != 0.0f && fxaction.shakeDistance != 0.0f ) {
						idEntity *ignore_ent = NULL;
						if ( gameLocal.isMultiplayer ) {
							ignore_ent = this;
							if ( fxaction.shakeIgnoreMaster ) {
								ignore_ent = GetBindMaster();
							}
						}
						// lookup the ent we are bound to?
						gameLocal.RadiusPush( GetPhysics()->GetOrigin(), fxaction.shakeDistance, fxaction.shakeImpulse, this, ignore_ent, 1.0f, true );
					}
					useAction->shakeStarted = true;
				}
				break;
			}
			case FX_ATTACHENTITY:
			case FX_PARTICLE:
			case FX_MODEL: {
				if ( useAction->modelDefHandle == -1 ) {
					memset( &useAction->renderEntity, 0, sizeof( renderEntity_t ) );
					useAction->renderEntity.origin = GetPhysics()->GetOrigin() + fxaction.offset;
					useAction->renderEntity.axis = (fxaction.explicitAxis) ? fxaction.axis : GetPhysics()->GetAxis();
					useAction->renderEntity.hModel = renderModelManager->FindModel( fxaction.data );
					useAction->renderEntity.shaderParms[ SHADERPARM_RED ]		= 1.0f;
					useAction->renderEntity.shaderParms[ SHADERPARM_GREEN ]		= 1.0f;
					useAction->renderEntity.shaderParms[ SHADERPARM_BLUE ]		= 1.0f;
					useAction->renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( time );

					// ivan - set stop time to non for particles fxaction.type
					//renderEntity.shaderParms[SHADERPARM_PARTICLE_STOPTIME] = 0;

					useAction->renderEntity.shaderParms[3] = 1.0f;
					useAction->renderEntity.shaderParms[5] = 0.0f;
					if ( useAction->renderEntity.hModel ) {
						useAction->renderEntity.bounds = useAction->renderEntity.hModel->Bounds( &useAction->renderEntity );
					}
					useAction->modelDefHandle = gameRenderWorld->AddEntityDef( &useAction->renderEntity );
				} else if ( fxaction.trackOrigin ) {
						
					//ivan start

					/*
					//was:
					useAction->renderEntity.origin = GetPhysics()->GetOrigin() + fxaction.offset;
					useAction->renderEntity.axis = fxaction.explicitAxis ? fxaction.axis : GetPhysics()->GetAxis();
					*/

					//fix offset: must be relative to our axis!
					useAction->renderEntity.axis = fxaction.explicitAxis ? fxaction.axis : GetPhysics()->GetAxis();
					useAction->renderEntity.origin = GetPhysics()->GetOrigin() + fxaction.offset * useAction->renderEntity.axis;

					//track origin fix
					gameRenderWorld->UpdateEntityDef( useAction->modelDefHandle, &useAction->renderEntity );
					
					//ivan end
				}
				ApplyFade( fxaction, *useAction, time, actualStart );
				break;
			}
			case FX_LAUNCH: {
				if ( gameLocal.isClient ) {
					// client never spawns entities outside of ClientReadSnapshot
					useAction->launched = true;
					break;
				}
				if ( !useAction->launched ) {
					useAction->launched = true;
					projectile = NULL;
					// FIXME: may need to cache this if it is slow
					projectileDef = gameLocal.FindEntityDefDict( fxaction.data, false );
					if ( !projectileDef ) {
						gameLocal.Warning( "projectile \'%s\' not found", fxaction.data.c_str() );
					} else {
						gameLocal.SpawnEntityDef( *projectileDef, &ent, false );
						if ( ent && ent->IsType( idProjectile::Type ) ) {
							projectile = ( idProjectile * )ent;
							projectile->Create( this, GetPhysics()->GetOrigin(), GetPhysics()->GetAxis()[0] );
							projectile->Launch( GetPhysics()->GetOrigin(), GetPhysics()->GetAxis()[0], vec3_origin );
						}
					}
				}
				break;
			}
			//ivan start - SHOCKWAVE from d3xp
			case FX_SHOCKWAVE: {
				if ( gameLocal.isClient ) {
					useAction->shakeStarted = true;
					break;
				}
				if ( !useAction->shakeStarted ) {
					idStr	shockDefName;
					useAction->shakeStarted = true;

					shockDefName = fxaction.data;
					if ( !shockDefName.Length() ) {
						shockDefName = "func_shockwave";
					}

					projectileDef = gameLocal.FindEntityDefDict( shockDefName, false );
					if ( !projectileDef ) {
						gameLocal.Warning( "shockwave \'%s\' not found", shockDefName.c_str() );
					} else {
						gameLocal.SpawnEntityDef( *projectileDef, &ent );
						ent->SetOrigin( GetPhysics()->GetOrigin() + fxaction.offset );
						ent->PostEventMS( &EV_Remove, ent->spawnArgs.GetInt( "duration" ) );
					}
				}
				break;
			}
			//ivan end - SHOCKWAVE from d3xp 
		}
	}
}

/*
================
idEntityFx::idEntityFx
================
*/
idEntityFx::idEntityFx() {
	fxEffect = NULL;
	started = -1;
	nextTriggerTime = -1;
	fl.networkSync = true;

	//ivan start
	manualRemove = false;
	manualFadeIsOn = false;
	endlessSounds = false;
	//ivan end
}

/*
================
idEntityFx::~idEntityFx
================
*/
idEntityFx::~idEntityFx() {
	CleanUp();
	fxEffect = NULL;
	//gameLocal.Printf("~idEntityFx()\n");

	//ivan start
	if( endlessSounds ){
		StopSound( SND_CHANNEL_ANY, false );
		//gameLocal.Printf("StopSound!");
	}
	//ivan end
}

/*
================
idEntityFx::Spawn
================
*/
void idEntityFx::Spawn( void ) {

	if ( g_skipFX.GetBool() ) {
		return;
	}

	const char *fx;
	nextTriggerTime = 0;
	fxEffect = NULL;
	if ( spawnArgs.GetString( "fx", "", &fx ) ) {
		systemName = fx;
	}
	if ( !spawnArgs.GetBool( "triggered" ) ) {
		Setup( fx );
		if ( spawnArgs.GetBool( "test" ) || spawnArgs.GetBool( "start" ) || spawnArgs.GetFloat ( "restart" ) ) {
			PostEventMS( &EV_Activate, 0, this );
		}
	}

	//ivan start
	manualRemove = spawnArgs.GetBool( "manualRemove", "0" ); //don't change default behaviour
	//ivan end
}

/*
================
idEntityFx::Think

  Clears any visual fx started when {item,mob,player} was spawned
================
*/
void idEntityFx::Think( void ) {
	if ( g_skipFX.GetBool() ) {
		return;
	}

	//gameLocal.Printf( "idEntityFx::Think of '%s'\n", GetName() );

	if ( thinkFlags & TH_THINK ) {
		Run( gameLocal.time );
	}

	RunPhysics();
	Present();
}

//ivan start

/*
================
idEntityFx::SetupFade
================
*/
void idEntityFx::SetupFade( void ) {
	int ieff;

	if ( !fxEffect ) {
		return;
	}

	for( ieff = 0; ieff < fxEffect->events.Num(); ieff++ ) {
		const idFXSingleAction& fxaction = fxEffect->events[ieff];
		idFXLocalAction& laction = actions[ieff];
		
		// turn on the waiting ones
		if ( laction.delay < 0 ) {
			laction.start = gameLocal.time; 
		}
		// turn off the ones which are not endless
		else if( fxaction.duration != 0 ){ //duration = 0 means endless. Turn off -1 ones too!
			laction.start = -1; 
		}

		//turn off particles
		if ( laction.modelDefHandle != -1 && fxaction.sibling == -1 && fxaction.type != FX_ATTACHENTITY ) {
			laction.renderEntity.shaderParms[ SHADERPARM_PARTICLE_STOPTIME ] = MS2SEC( gameLocal.time ); 
		}
	}
}


/*
================
idEntityFx::FadeOutFx
================
*/
void idEntityFx::FadeOutFx( void ) {
	if( started < 0 || manualFadeIsOn ){ //if not active or already fading
		return;
	}

	manualFadeIsOn = true;
	SetupFade();

	if( endlessSounds ){
		FadeSound( SND_CHANNEL_ANY, -60, 1 ); //fade out sounds
		//gameLocal.Printf("fading sounds!");
	}

	if( manualRemove ){

		CancelEvents( &EV_Activate ); // make sure it's not going to re-activate itself
		CancelEvents( &EV_Fx_KillFx ); // make sure it's not going to kill or re-activate itself too soon

		PostEventMS( &EV_Fx_KillFx, FadeDuration() );
	}
}

/*
================
idEntityFx::Event_StopFx

  Clears any visual fx started when item(mob) was spawned
================
*/
void idEntityFx::Event_FadeFx( void ) {
	FadeOutFx();
}
//ivan end

/*
================
idEntityFx::Event_ClearFx

  Clears any visual fx started when item(mob) was spawned
================
*/
void idEntityFx::Event_ClearFx( void ) {

	if ( g_skipFX.GetBool() ) {
		return;
	}

	Stop();
	CleanUp();
	BecomeInactive( TH_THINK );

	if ( spawnArgs.GetBool("test") ) {
		PostEventMS( &EV_Activate, 0, this );
	} else {
		if ( spawnArgs.GetFloat( "restart" ) || !spawnArgs.GetBool( "triggered")) {
			float rest = spawnArgs.GetFloat( "restart", "0" );
			if ( rest == 0.0f ) {
				PostEventSec( &EV_Remove, 0.1f );
			} else {
				rest *= gameLocal.random.RandomFloat();
				PostEventSec( &EV_Activate, rest, this );
			}
		}
	}
}

/*
================
idEntityFx::Event_Trigger
================
*/
void idEntityFx::Event_Trigger( idEntity *activator ) {

	if ( g_skipFX.GetBool() ) {
		return;
	}

	float		fxActionDelay;
	const char *fx;

	if ( gameLocal.time < nextTriggerTime ) {
		return;
	}

	if ( spawnArgs.GetString( "fx", "", &fx) ) {
		
		//ivan start
		if( manualRemove ){ //new case
			if( started >= 0 && !manualFadeIsOn && spawnArgs.GetBool( "toggle", "0") ){ //if it is active && toggle is set
				FadeOutFx();
			}else{
				Setup( fx );
				Start( gameLocal.time ); //don't autokill
			}
		}else{ //old case
		//ivan end
			Setup( fx );
			Start( gameLocal.time );
			PostEventMS( &EV_Fx_KillFx, Duration() );
		}
		BecomeActive( TH_THINK );
	}

	fxActionDelay = spawnArgs.GetFloat( "fxActionDelay" );
	if ( fxActionDelay != 0.0f ) {
		nextTriggerTime = gameLocal.time + SEC2MS( fxActionDelay );
	} else {
		// prevent multiple triggers on same frame
		nextTriggerTime = gameLocal.time + 1;
	}
	PostEventSec( &EV_Fx_Action, fxActionDelay, activator );
}

//ivan start
void idEntityFx::StartFxUtility( idEntityFx *nfx, const idVec3 *useOrigin, const idMat3 *useAxis, idEntity *ent, bool bind, bool orientated, jointHandle_t jointnum ){
	if ( nfx->Joint() && *nfx->Joint() ) { //ivan - where the hell is this set?
		//gameLocal.Printf("StartFx --> misterious case. Tell Ivan when this happend!\n" );
		//nfx->BindToJoint( ent, nfx->Joint(), true );
		nfx->BindToJoint( ent, nfx->Joint(), orientated ); //ivan
		nfx->SetOrigin( vec3_origin );
	} 
	//ivan start
	else if( bind && jointnum != INVALID_JOINT ){
		//gameLocal.Printf("StartFx --> Joint ok, orientated: %d\n", (orientated?1:0) );
		nfx->SetAxis( (useAxis) ? *useAxis : ent->GetPhysics()->GetAxis() );
		nfx->BindToJoint( ent, jointnum, orientated );
		nfx->SetOrigin( vec3_origin );
	}
	//ivan end

	else {
		//gameLocal.Printf("StartFx --> no Joint, orientated: %d\n", (orientated?1:0) );
		nfx->SetOrigin( (useOrigin) ? *useOrigin : ent->GetPhysics()->GetOrigin() );
		nfx->SetAxis( (useAxis) ? *useAxis : ent->GetPhysics()->GetAxis() );

		if ( bind ) {
			// never bind to world spawn
			if ( ent != gameLocal.world ) {
				//nfx->Bind( ent, true );
				nfx->Bind( ent, orientated ); //ivan
			}
		}
	}
	nfx->Show();
}
//ivan end

/*
================
idEntityFx::StartFx
================
*/
//idEntityFx *idEntityFx::StartFx( const char *fx, const idVec3 *useOrigin, const idMat3 *useAxis, idEntity *ent, bool bind ) {
idEntityFx *idEntityFx::StartFx( const char *fx, const idVec3 *useOrigin, const idMat3 *useAxis, idEntity *ent, bool bind, bool orientated, jointHandle_t jointnum ) { //ivan
	if ( g_skipFX.GetBool() || !fx || !*fx ) {
		return NULL;
	}

	idDict args;
	args.SetBool( "start", true );
	args.Set( "fx", fx );

	idEntityFx *nfx = static_cast<idEntityFx *>( gameLocal.SpawnEntityType( idEntityFx::Type, &args ) );
	
	//ivan start - code moved in StartFxUtility
	if( nfx ){
		StartFxUtility( nfx, useOrigin, useAxis, ent, bind, orientated, jointnum );
	}
	//ivan end

	return nfx;
}

/*
=================
idEntityFx::WriteToSnapshot
=================
*/
void idEntityFx::WriteToSnapshot( idBitMsgDelta &msg ) const {
	GetPhysics()->WriteToSnapshot( msg );
	WriteBindToSnapshot( msg );
	msg.WriteLong( ( fxEffect != NULL ) ? gameLocal.ServerRemapDecl( -1, DECL_FX, fxEffect->Index() ) : -1 );
	msg.WriteLong( started );
}

/*
=================
idEntityFx::ReadFromSnapshot
=================
*/
void idEntityFx::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	int fx_index, start_time, max_lapse;

	GetPhysics()->ReadFromSnapshot( msg );
	ReadBindFromSnapshot( msg );
	fx_index = gameLocal.ClientRemapDecl( DECL_FX, msg.ReadLong() );
	start_time = msg.ReadLong();

	if ( fx_index != -1 && start_time > 0 && !fxEffect && started < 0 ) {
		spawnArgs.GetInt( "effect_lapse", "1000", max_lapse );
		if ( gameLocal.time - start_time > max_lapse ) {
			// too late, skip the effect completely
			started = 0;
			return;
		}
		const idDeclFX *fx = static_cast<const idDeclFX *>( declManager->DeclByIndex( DECL_FX, fx_index ) );
		if ( !fx ) {
			gameLocal.Error( "FX at index %d not found", fx_index );
		}
		fxEffect = fx;
		Setup( fx->GetName() );
		Start( start_time );
	}
}

/*
=================
idEntityFx::ClientPredictionThink
=================
*/
void idEntityFx::ClientPredictionThink( void ) {
	if ( gameLocal.isNewFrame ) {
		Run( gameLocal.time );
	}
	RunPhysics();
	Present();
}

/*
===============================================================================

  idTeleporter

===============================================================================
*/

CLASS_DECLARATION( idEntityFx, idTeleporter )
	EVENT( EV_Fx_Action,	idTeleporter::Event_DoAction )
END_CLASS

/*
================
idTeleporter::Event_DoAction
================
*/
void idTeleporter::Event_DoAction( idEntity *activator ) {
	idAngles a( 0, spawnArgs.GetFloat( "angle" ), 0 );
	bool forceChangeXpos; //ivan

	//ivan start
	//was: activator->Teleport( GetPhysics()->GetOrigin(), a, NULL );
	//note: now I pass to the player the reference to this Teleporter because he checks the new keys! 

	forceChangeXpos = spawnArgs.GetBool( "forceChangeXpos", "0" );

	if ( forceChangeXpos && activator->IsType( idPlayer::Type ) ) {
		idPlayer *player = static_cast<idPlayer*>( activator );
		idEntity *validtarget = NULL;

		RemoveNullTargets(); //make sure there are only valid targets in the list

		if ( targets.Num() ) {
			validtarget = targets[ 0 ].GetEntity(); //first one
		}
		if( validtarget ){
			player->StopForcedMov(); //make sure ForcedMov is off - this could set a new lockedXpos  
			player->Teleport( GetPhysics()->GetOrigin(), a, this ); //this will set a new lockedXpos 
			player->StartForcedMov( validtarget, 300, false, spawnArgs.GetBool( "totalForce", "0" ) ); //wait 0.3s, no abort, totalForce?
		}else{
			gameLocal.Warning("idTeleporter with forceChangeXpos = 1 but no targets!");
			activator->Teleport( GetPhysics()->GetOrigin(), a, this );
		}
	}else{ //simple teleport
		activator->Teleport( GetPhysics()->GetOrigin(), a, this );
	}
	//ivan end
}

//ivan start

/*
===============================================================================

  idDamagingFx
	
===============================================================================
*/

static const char *dmgFxTypeString[ NUM_DMGFX_TYPES ] = {
	"<none>",
	"flames",
	"electro",
	"toxic"
};


CLASS_DECLARATION( idEntityFx, idDamagingFx )
END_CLASS

/*
================
idDamagingFx::idDamagingFx
================
*/
idDamagingFx::idDamagingFx() {
	nextDamageTime	= 0;
	endDamageTime	= 0;
	damageRate		= 0;
	victim			= NULL;
	damageDefString	= "";
	dmgfxType		= DMGFX_NONE; 
}

/*
================
idDamagingFx::~idDamagingFx
================
*/
idDamagingFx::~idDamagingFx() {
}

/*
================
idDamagingFx::Spawn
================
*/
void idDamagingFx::Spawn( void ) {
	//gameLocal.Printf("idDamagingFx::Spawn\n");
	damageDefString = spawnArgs.GetString( "def_damage", "damage_damagingFx" );

	//damage def must be valid
	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefString.c_str() );
	if ( !damageDef ) {
		gameLocal.Error( "Unknown damageDef '%s'\n", damageDefString.c_str() );
	}

	//get victim ent
	idEntity *ent;
	const char *entname;

	if ( !spawnArgs.GetString( "victim", NULL, &entname ) ) {
		gameLocal.Error( "Couldn't find 'victim' key in entity '%s'", name.c_str() );
		return;
	}

	ent = gameLocal.FindEntity( entname );
	if ( !ent ) {
		gameLocal.Error( "Couldn't find entity '%s' specified in 'victim' key in entity '%s'", entname, name.c_str() );
	}

	victim = ent;

	//fadeOutOnTrigger = true; //make sure
}

/*
================
idDamagingFx::Restart
================
*/
void idDamagingFx::Restart( void ) { 
	//gameLocal.Printf("idDamagingFx::Restart\n");
	CancelEvents( &EV_Activate ); 
	CancelEvents( &EV_Fx_KillFx );

	ResetShaderParms(); 

	//gameLocal.Printf("Restart\n");
	nextTriggerTime = 0; //make sure the event is accepted
	PostEventMS( &EV_Activate, 0, this );
}

/*
================
idDamagingFx::Start
================
*/
void idDamagingFx::Start( int time ) { //time is usually the current time
	//gameLocal.Printf("idDamagingFx::Start\n");

	//damage def must be valid
	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefString.c_str() );
	if ( !damageDef ) {
		gameLocal.Error( "Unknown damageDef '%s'\n", damageDefString.c_str() );
	}

	//end of fx
	endDamageTime = damageDef->GetInt( "totalTime", "-1" ); //-1 is endless
	if( endDamageTime > 0 ){
		endDamageTime = SEC2MS( endDamageTime ) + time;
	} 

	//damageRate
	damageRate = SEC2MS( damageDef->GetInt( "damageRate", "1" ) ); //only pos
	if( damageRate <= 0 ){
		damageRate = 1000;
	} 
	nextDamageTime = damageRate + time;
	//ivan end

	//start the fx 
	idEntityFx::Start( time );
}

/*
================
idDamagingFx::Think
================
*/
void idDamagingFx::Think( void ) {
	//gameLocal.Printf("idDamagingFx::Think\n");
	idEntityFx::Think();
	if( started >= 0 && !manualFadeIsOn ){ //is active
		if( !victim.GetEntity() || victim.GetEntity()->IsHidden() || (endDamageTime > 0 && endDamageTime <= gameLocal.time ) ){
			//gameLocal.Printf("FadeOutFx\n");
			FadeOutFx();
		}else if( nextDamageTime < gameLocal.time ){
			nextDamageTime = damageRate + gameLocal.time;
			//gameLocal.Printf("Damage\n");
			victim.GetEntity()->Damage( this, this, vec3_origin, damageDefString.c_str() , 1.0f, INVALID_JOINT );
		}
	}
}
//spawnArgs.GetString( "def_damage", "damage_damagingFx" )

/*
================
idDamagingFx::StartDamagingFx
================
*/
idDamagingFx *idDamagingFx::StartDamagingFx( int type, idEntity *victimEnt ) { 
	//gameLocal.Printf("idDamagingFx::StartDamagingFx\n");
	idEntity* ent;
	idDamagingFx* nfx;
	if ( g_skipFX.GetBool() || type <= DMGFX_NONE || type >= NUM_DMGFX_TYPES || !victimEnt ) {
		return NULL;
	}

	//check type 
	const char	*defName;
	defName = va( "dmgfx_%s_%s", dmgFxTypeString[type], victimEnt->spawnArgs.GetString( "size4fxs", "default" ) );
	//sprintf( defName, "dmgfx_%s_%s", dmgFxTypeString[type], victimEnt->spawnArgs.GetString( "size4fxs", "default" ) );

	idDict args;
	const idDeclEntityDef *fxDef = gameLocal.FindEntityDef( defName, false );
	if ( fxDef == NULL ) {
		gameLocal.Warning( "No def '%s' found", defName ); //TODO: comment out this warning
		return NULL;
	}

	args = fxDef->dict;
	args.Set( "spawnclass", "idDamagingFx" );
	args.SetBool( "start", true );
	args.SetBool( "manualRemove", true ); //we'll stop it depeding on the damage time, not the fx itself
	args.Set( "victim", victimEnt->GetName() );
	
	if ( gameLocal.SpawnEntityDef( args, &ent, false ) && ent && ent->IsType( idDamagingFx::Type ) ) {
		nfx	= static_cast<idDamagingFx*>(ent);
		nfx->dmgfxType = type;
		//gameLocal.Printf("Spawned %s\n", defName );
        idVec3 tmp = victimEnt->GetPhysics()->GetAbsBounds().GetCenter();
		StartFxUtility( nfx, &tmp, &mat3_identity, victimEnt, true, false );
		return nfx;
	}else{
		gameLocal.Error( "Could not spawn idDamagingFx" );
		return NULL;
	}	
}

/*
================
idDamagingFx::Save
================
*/
void idDamagingFx::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( nextDamageTime );
	savefile->WriteInt( damageRate );
	savefile->WriteInt( endDamageTime );
	savefile->WriteInt( dmgfxType );
	savefile->WriteString( damageDefString );
	victim.Save( savefile );
}

/*
================
idDamagingFx::Restore
================
*/
void idDamagingFx::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( nextDamageTime );
	savefile->ReadInt( damageRate );
	savefile->ReadInt( endDamageTime );
	savefile->ReadInt( dmgfxType );
	savefile->ReadString( damageDefString );
	victim.Restore( savefile );
}
//ivan end
