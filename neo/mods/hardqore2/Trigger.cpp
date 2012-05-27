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
#include "script/Script_Thread.h"
#include "Player.h"

#include "Trigger.h"

/*
===============================================================================

  idTrigger

===============================================================================
*/

const idEventDef EV_Enable( "enable", NULL );
const idEventDef EV_Disable( "disable", NULL );

CLASS_DECLARATION( idEntity, idTrigger )
	EVENT( EV_Enable,	idTrigger::Event_Enable )
	EVENT( EV_Disable,	idTrigger::Event_Disable )
END_CLASS

/*
================
idTrigger::DrawDebugInfo
================
*/
void idTrigger::DrawDebugInfo( void ) {
	idMat3		axis = gameLocal.GetLocalPlayer()->viewAngles.ToMat3();
	idVec3		up = axis[ 2 ] * 5.0f;
	idBounds	viewTextBounds( gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin() );
	idBounds	viewBounds( gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin() );
	idBounds	box( idVec3( -4.0f, -4.0f, -4.0f ), idVec3( 4.0f, 4.0f, 4.0f ) );
	idEntity	*ent;
	idEntity	*target;
	int			i;
	bool		show;
	const function_t *func;

	viewTextBounds.ExpandSelf( 128.0f );
	viewBounds.ExpandSelf( 512.0f );
	for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( ent->GetPhysics()->GetContents() & ( CONTENTS_TRIGGER | CONTENTS_FLASHLIGHT_TRIGGER ) ) {
			show = viewBounds.IntersectsBounds( ent->GetPhysics()->GetAbsBounds() );
			if ( !show ) {
				for( i = 0; i < ent->targets.Num(); i++ ) {
					target = ent->targets[ i ].GetEntity();
					if ( target && viewBounds.IntersectsBounds( target->GetPhysics()->GetAbsBounds() ) ) {
						show = true;
						break;
					}
				}
			}

			if ( !show ) {
				continue;
			}

			gameRenderWorld->DebugBounds( colorOrange, ent->GetPhysics()->GetAbsBounds() );
			if ( viewTextBounds.IntersectsBounds( ent->GetPhysics()->GetAbsBounds() ) ) {
				gameRenderWorld->DrawText( ent->name.c_str(), ent->GetPhysics()->GetAbsBounds().GetCenter(), 0.1f, colorWhite, axis, 1 );
				gameRenderWorld->DrawText( ent->GetEntityDefName(), ent->GetPhysics()->GetAbsBounds().GetCenter() + up, 0.1f, colorWhite, axis, 1 );
				if ( ent->IsType( idTrigger::Type ) ) {
					func = static_cast<idTrigger *>( ent )->GetScriptFunction();
				} else {
					func = NULL;
				}

				if ( func ) {
					gameRenderWorld->DrawText( va( "call script '%s'", func->Name() ), ent->GetPhysics()->GetAbsBounds().GetCenter() - up, 0.1f, colorWhite, axis, 1 );
				}
			}

			for( i = 0; i < ent->targets.Num(); i++ ) {
				target = ent->targets[ i ].GetEntity();
				if ( target ) {
					gameRenderWorld->DebugArrow( colorYellow, ent->GetPhysics()->GetAbsBounds().GetCenter(), target->GetPhysics()->GetOrigin(), 10, 0 );
					gameRenderWorld->DebugBounds( colorGreen, box, target->GetPhysics()->GetOrigin() );
					if ( viewTextBounds.IntersectsBounds( target->GetPhysics()->GetAbsBounds() ) ) {
						gameRenderWorld->DrawText( target->name.c_str(), target->GetPhysics()->GetAbsBounds().GetCenter(), 0.1f, colorWhite, axis, 1 );
					}
				}
			}
		}
	}
}

/*
================
idTrigger::Enable
================
*/
void idTrigger::Enable( void ) {
	GetPhysics()->SetContents( CONTENTS_TRIGGER );
	GetPhysics()->EnableClip();
}

/*
================
idTrigger::Disable
================
*/
void idTrigger::Disable( void ) {
	// we may be relinked if we're bound to another object, so clear the contents as well
	GetPhysics()->SetContents( 0 );
	GetPhysics()->DisableClip();
}

/*
================
idTrigger::CallScript
================
*/
void idTrigger::CallScript( void ) const {
	idThread *thread;

	if ( scriptFunction ) {
		thread = new idThread( scriptFunction );
		thread->DelayedStart( 0 );
	}
}

/*
================
idTrigger::GetScriptFunction
================
*/
const function_t *idTrigger::GetScriptFunction( void ) const {
	return scriptFunction;
}

/*
================
idTrigger::Save
================
*/
void idTrigger::Save( idSaveGame *savefile ) const {
	if ( scriptFunction ) {
		savefile->WriteString( scriptFunction->Name() );
	} else {
		savefile->WriteString( "" );
	}
}

/*
================
idTrigger::Restore
================
*/
void idTrigger::Restore( idRestoreGame *savefile ) {
	idStr funcname;
	savefile->ReadString( funcname );
	if ( funcname.Length() ) {
		scriptFunction = gameLocal.program.FindFunction( funcname );
		if ( scriptFunction == NULL ) {
			gameLocal.Warning( "idTrigger_Multi '%s' at (%s) calls unknown function '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), funcname.c_str() );
		}
	} else {
		scriptFunction = NULL;
	}
}

/*
================
idTrigger::Event_Enable
================
*/
void idTrigger::Event_Enable( void ) {
	Enable();
}

/*
================
idTrigger::Event_Disable
================
*/
void idTrigger::Event_Disable( void ) {
	Disable();
}

/*
================
idTrigger::idTrigger
================
*/
idTrigger::idTrigger() {
	scriptFunction = NULL;
}

/*
================
idTrigger::Spawn
================
*/
void idTrigger::Spawn( void ) {
	GetPhysics()->SetContents( CONTENTS_TRIGGER );

	idStr funcname = spawnArgs.GetString( "call", "" );
	if ( funcname.Length() ) {
		scriptFunction = gameLocal.program.FindFunction( funcname );
		if ( scriptFunction == NULL ) {
			gameLocal.Warning( "trigger '%s' at (%s) calls unknown function '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), funcname.c_str() );
		}
	} else {
		scriptFunction = NULL;
	}
}


/*
===============================================================================

  idTrigger_Multi

===============================================================================
*/

const idEventDef EV_TriggerAction( "<triggerAction>", "e" );

CLASS_DECLARATION( idTrigger, idTrigger_Multi )
	EVENT( EV_Touch,			idTrigger_Multi::Event_Touch )
	EVENT( EV_Activate,			idTrigger_Multi::Event_Trigger )
	EVENT( EV_TriggerAction,	idTrigger_Multi::Event_TriggerAction )
END_CLASS


/*
================
idTrigger_Multi::idTrigger_Multi
================
*/
idTrigger_Multi::idTrigger_Multi( void ) {
	wait = 0.0f;
	random = 0.0f;
	delay = 0.0f;
	random_delay = 0.0f;
	nextTriggerTime = 0;
	removeItem = 0;
	touchClient = false;
	touchOther = false;
	triggerFirst = false;
	triggerWithSelf = false;
}

/*
================
idTrigger_Multi::Save
================
*/
void idTrigger_Multi::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( wait );
	savefile->WriteFloat( random );
	savefile->WriteFloat( delay );
	savefile->WriteFloat( random_delay );
	savefile->WriteInt( nextTriggerTime );
	savefile->WriteString( requires );
	savefile->WriteInt( removeItem );
	savefile->WriteBool( touchClient );
	savefile->WriteBool( touchOther );
	savefile->WriteBool( triggerFirst );
	savefile->WriteBool( triggerWithSelf );
}

/*
================
idTrigger_Multi::Restore
================
*/
void idTrigger_Multi::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( wait );
	savefile->ReadFloat( random );
	savefile->ReadFloat( delay );
	savefile->ReadFloat( random_delay );
	savefile->ReadInt( nextTriggerTime );
	savefile->ReadString( requires );
	savefile->ReadInt( removeItem );
	savefile->ReadBool( touchClient );
	savefile->ReadBool( touchOther );
	savefile->ReadBool( triggerFirst );
	savefile->ReadBool( triggerWithSelf );
}

/*
================
idTrigger_Multi::Spawn

"wait" : Seconds between triggerings, 0.5 default, -1 = one time only.
"call" : Script function to call when triggered
"random"	wait variance, default is 0
Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)
================
*/
void idTrigger_Multi::Spawn( void ) {
	spawnArgs.GetFloat( "wait", "0.5", wait );
	spawnArgs.GetFloat( "random", "0", random );
	spawnArgs.GetFloat( "delay", "0", delay );
	spawnArgs.GetFloat( "random_delay", "0", random_delay );

	if ( random && ( random >= wait ) && ( wait >= 0 ) ) {
		random = wait - 1;
		gameLocal.Warning( "idTrigger_Multi '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	if ( random_delay && ( random_delay >= delay ) && ( delay >= 0 ) ) {
		random_delay = delay - 1;
		gameLocal.Warning( "idTrigger_Multi '%s' at (%s) has random_delay >= delay", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	spawnArgs.GetString( "requires", "", requires );
	spawnArgs.GetInt( "removeItem", "0", removeItem );
	spawnArgs.GetBool( "triggerFirst", "0", triggerFirst );
	spawnArgs.GetBool( "triggerWithSelf", "0", triggerWithSelf );

	if ( spawnArgs.GetBool( "anyTouch" ) ) {
		touchClient = true;
		touchOther = true;
	} else if ( spawnArgs.GetBool( "noTouch" ) ) {
		touchClient = false;
		touchOther = false;
	} else if ( spawnArgs.GetBool( "noClient" ) ) {
		touchClient = false;
		touchOther = true;
	} else {
		touchClient = true;
		touchOther = false;
	}

	nextTriggerTime = 0;

	if ( spawnArgs.GetBool( "flashlight_trigger" ) ) {
		GetPhysics()->SetContents( CONTENTS_FLASHLIGHT_TRIGGER );
	} else {
		GetPhysics()->SetContents( CONTENTS_TRIGGER );
	}
}

/*
================
idTrigger_Multi::CheckFacing
================
*/
bool idTrigger_Multi::CheckFacing( idEntity *activator ) {
	if ( spawnArgs.GetBool( "facing" ) ) {
		if ( !activator->IsType( idPlayer::Type ) ) {
			return true;
		}
		idPlayer *player = static_cast< idPlayer* >( activator );
		float dot = player->viewAngles.ToForward() * GetPhysics()->GetAxis()[0];
		float angle = RAD2DEG( idMath::ACos( dot ) );
		if ( angle  > spawnArgs.GetFloat( "angleLimit", "30" ) ) {
			return false;
		}
	}
	return true;
}

/*
================
idTrigger_Multi::TriggerAction
================
*/
void idTrigger_Multi::TriggerAction( idEntity *activator ) {
	ActivateTargets( triggerWithSelf ? this : activator );
	CallScript();

	if ( wait >= 0 ) {
		nextTriggerTime = gameLocal.time + SEC2MS( wait + random * gameLocal.random.CRandomFloat() );
	} else {
		// we can't just remove (this) here, because this is a touch function
		// called while looping through area links...
		nextTriggerTime = gameLocal.time + 1;
		PostEventMS( &EV_Remove, 0 );
	}
}

/*
================
idTrigger_Multi::Event_TriggerAction
================
*/
void idTrigger_Multi::Event_TriggerAction( idEntity *activator ) {
	TriggerAction( activator );
}

/*
================
idTrigger_Multi::Event_Trigger

the trigger was just activated
activated should be the entity that originated the activation sequence (ie. the original target)
activator should be set to the activator so it can be held through a delay
so wait for the delay time before firing
================
*/
void idTrigger_Multi::Event_Trigger( idEntity *activator ) {
	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	// see if this trigger requires an item
	if ( !gameLocal.RequirementMet( activator, requires, removeItem ) ) {
		return;
	}

	if ( !CheckFacing( activator ) ) {
		return;
	}

	if ( triggerFirst ) {
		triggerFirst = false;
		return;
	}

	// don't allow it to trigger twice in a single frame
	nextTriggerTime = gameLocal.time + 1;

	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, activator );
	} else {
		TriggerAction( activator );
	}
}

/*
================
idTrigger_Multi::Event_Touch
================
*/
void idTrigger_Multi::Event_Touch( idEntity *other, trace_t *trace ) {	
	if( triggerFirst ) {
		return;
	}

	bool player = other->IsType( idPlayer::Type );
	if ( player ) {
		if ( !touchClient ) {
			return;
		}
		if ( static_cast< idPlayer * >( other )->spectating ) {
			return;
		}
	} else if ( !touchOther ) {
		return;
	}

	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	// see if this trigger requires an item
	if ( !gameLocal.RequirementMet( other, requires, removeItem ) ) {
		return;
	}

	if ( !CheckFacing( other ) ) {
		return;
	}

	if ( spawnArgs.GetBool( "toggleTriggerFirst" ) ) {
		triggerFirst = true;
	}

	nextTriggerTime = gameLocal.time + 1;
	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, other );
	} else {
		TriggerAction( other );
	}
}

//ivan start


/*
===============================================================================

  idTrigger_CheckPoint
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger_Multi, idTrigger_CheckPoint )
END_CLASS

/*
================
idTrigger_CheckPoint::idTrigger_CheckPoint
================
*/
idTrigger_CheckPoint::idTrigger_CheckPoint( void ) {
}

/*
================
idTrigger_CheckPoint::TriggerAction
================
*/
void idTrigger_CheckPoint::TriggerAction( idEntity *activator ) {
	//behaves like default trigger
	idTrigger_Multi::TriggerAction( activator );
	
	//gameLocal.Printf("idTrigger_CheckPoint::TriggerAction\n");

	//is it a player?
	if ( activator->IsType( idPlayer::Type ) ){
		static_cast< idPlayer* >( activator )->SaveCheckPointPos( this, spawnArgs.GetBool("useMyOrigin", "0") );
	}
}


/*
===============================================================================

  idTrigger_ForceMov
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger_Multi, idTrigger_ForceMov )
END_CLASS

/*
================
idTrigger_ForceMov::idTrigger_ForceMov
================
*/
idTrigger_ForceMov::idTrigger_ForceMov( void ) {
}

/*
================
idTrigger_ForceMov::ActionMovToTarget
================
*/
bool idTrigger_ForceMov::ActionMovToTarget( idPlayer *player ){
	//valid target?
	idEntity *validtarget = GetFirstValidTarget();
	if( !validtarget ){ 
		gameLocal.Warning("%s has no valid targets!", GetName() );
		return false;
	}

	//start movement
	return player->StartForcedMov( validtarget, 0, false, true, spawnArgs.GetBool( "forceStart", "0" ) ); //no abort, total force, optional total force
}
	
/*
================
idTrigger_ForceMov::ActionSafeXpos
================
*/
bool idTrigger_ForceMov::ActionSafeXpos( idPlayer *player ){
	int delta = idMath::Abs( player->GetPhysics()->GetOrigin().x - GetPhysics()->GetOrigin().x );
	if( delta != 0 ){
		if( delta < 10 ){ //near enough to snap in line
			player->fastXpos = GetPhysics()->GetOrigin().x;
			return true;
		}else{
			bool success = player->StartForcedMov( this, 0, false, true, true ); //move toward my origin, no abort, total force, force start
			if( success ){
				player->SetLock2D( true ); //will be locked when the movement ends
			}
			return success;
		}
	}
	return true; //if already aligned, than job already done!
}

/*
================
idTrigger_ForceMov::TriggerAction
================
*/
void idTrigger_ForceMov::TriggerAction( idEntity *activator ) {
	idPlayer *player;
	bool actionDone;

	//gameLocal.Printf("idTrigger_ForceMov::TriggerAction\n");

	//is it a player?
	if ( !activator->IsType( idPlayer::Type ) ){
		gameLocal.Warning("activator is not an idPlayer");
		return;
	}
	player = static_cast< idPlayer* >( activator );

	if( spawnArgs.GetBool( "safeXpos", "0") ){
		actionDone = ActionSafeXpos( player );
	}else{
		actionDone = ActionMovToTarget( player );
	}

	if( actionDone ){ 
		//gameLocal.Warning("idTrigger_ForceMov actionDone");
		CallScript();

		if ( wait >= 0 ) {
			nextTriggerTime = gameLocal.time + SEC2MS( wait + random * gameLocal.random.CRandomFloat() );
		} else {
			// we can't just remove (this) here, because this is a touch function
			// called while looping through area links...
			nextTriggerTime = gameLocal.time + 1;
			PostEventMS( &EV_Remove, 0 );
		}
	}else{ //it's like never triggered because we haven't done what we should had to.
		nextTriggerTime = gameLocal.time + 1; //try again the next frame!
	}
}

/*
===============================================================================

  idTrigger_BlockMov
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger_Multi, idTrigger_BlockMov )
END_CLASS

/*
================
idTrigger_BlockMov::idTrigger_BlockMov
================
*/
idTrigger_BlockMov::idTrigger_BlockMov( void ) {
	
}

/*
================
idTrigger_BlockMov::TriggerAction
================
*/
void idTrigger_BlockMov::TriggerAction( idEntity *activator ) {

	//gameLocal.Printf("idTrigger_BlockMov::TriggerAction\n");

	//is it a player?
	if ( activator->IsType( idPlayer::Type ) ){
		static_cast< idPlayer* >( activator )->BlockForcedMov();
	}
	nextTriggerTime = gameLocal.time + 1; //try again the next frame!
}


/*
===============================================================================

  idTrigger_Interact
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Interact )
	EVENT( EV_Activate,			idTrigger_Interact::Event_Trigger )
	EVENT( EV_Interact,			idTrigger_Interact::Event_Interact )
	EVENT( EV_Touch,			idTrigger_Interact::Event_Touch ) //only for hud info
END_CLASS

/*
================
idTrigger_Interact::idTrigger_Interact
================
*/
idTrigger_Interact::idTrigger_Interact( void ) {
	wait = 0.0f;
	nextInteractTime = 0;
	triggerFirst = false;
	//memset( interactModes, INTERACT_NONE, sizeof( interactModes ) ); 
	interactFlags = 0;
}

/*
================
idTrigger_Interact::Save
================
*/
void idTrigger_Interact::Save( idSaveGame *savefile ) const {
	//int i;
	savefile->WriteFloat( wait );
	savefile->WriteInt( nextInteractTime );
	savefile->WriteBool( triggerFirst );

	/*for( i = 0; i < MAX_TRIGINT_MODES; i++ ) {
		savefile->WriteInt( interactModes[ i ] );
	}*/
	savefile->WriteInt( interactFlags ); 
}

/*
================
idTrigger_Interact::Restore
================
*/
void idTrigger_Interact::Restore( idRestoreGame *savefile ) {
	//int i;
	savefile->ReadFloat( wait );
	savefile->ReadInt( nextInteractTime );
	savefile->ReadBool( triggerFirst );

	/*for( i = 0; i < MAX_TRIGINT_MODES; i++ ) {
		savefile->ReadInt( interactModes[ i ] );
	}*/
	savefile->ReadInt( interactFlags );
}


/*
================
idTrigger_Interact::Spawn
================
*/
void idTrigger_Interact::Spawn( void ) {
	//int i;
	
	spawnArgs.GetFloat( "wait", "0.5", wait );
	spawnArgs.GetBool( "triggerFirst", "0", triggerFirst );

	//for( i = 0; i < MAX_TRIGINT_MODES; i++ ) {
	//	spawnArgs.GetInt( va( "interactMode%d", i ), "0", interactModes[ i ] ); //0 is INTERACT_NONE
	//}

	//add all interact flags
	interactFlags = 0;
	const idKeyValue *kv = spawnArgs.MatchPrefix( "interactMode" );   
	while ( kv ) {
		interactFlags |= spawnArgs.GetInt( kv->GetKey().c_str(), "0"); //TODO: check if it's valid
		kv = spawnArgs.MatchPrefix( "interactMode", kv );
	}      

	nextInteractTime = 0;
}

/*
================
idTrigger_Interact::Event_Touch

NOTE: should only be used for hud info.
================
*/
void idTrigger_Interact::Event_Touch( idEntity *other, trace_t *trace ) {
	if( triggerFirst ) {
		return;
	}

	if ( other->IsType( idPlayer::Type ) ) {
		static_cast< idPlayer * >( other )->AddPossibleInteract( interactFlags );
	}
}

/*
============
idTrigger_Interact::CanInteract
============
*/
bool idTrigger_Interact::CanInteract( int flags ) const {

	//if not activated
	if( triggerFirst ) {
		return false;
	}

	//if enough time passed by
	if ( nextInteractTime > gameLocal.time ) {
		// can't interact until the wait is over
		return false;
	}

	//check if there is a matching mode
	if( interactFlags & flags ){ return true; }

	return false;
}

/*
================
idTrigger_Interact::Event_Interact
================
*/
void idTrigger_Interact::Event_Interact( idEntity *activator, int flags ) {	
	if ( !activator ) return;
	if ( !activator->IsType( idPlayer::Type ) ) return;
	//if ( !CanInteract( flags ) ) return; //NOTE: it has been already checked in idEntity::InteractTouchingTriggers!!
	
	nextInteractTime = gameLocal.time + 1;
	InteractionAction( static_cast< idPlayer* >( activator ), flags );
} 

/*
================
idTrigger_Interact::InteractionAction
================
*/
void idTrigger_Interact::InteractionAction( idPlayer *player, int flags ) { 
	ActivateTargets( player );
	CallScript();
	
	//show text on hud
	const char *infoText = spawnArgs.GetString( "infoText" );
	if ( *infoText != '\0' ) {
		player->ShowInfo( infoText );
	}

	//wait or remove
	if ( wait >= 0 ) {
		nextInteractTime = gameLocal.time + SEC2MS( wait );
	} else {
		nextInteractTime = gameLocal.time + 1;
		PostEventMS( &EV_Remove, 0 ); //remove ourselves - interact 1 time only
	}
}



/*
================
idTrigger_Interact::Event_Trigger
================
*/
void idTrigger_Interact::Event_Trigger( idEntity *activator ) { //when triggered (EV_Activate)
	if ( triggerFirst ) {
		triggerFirst = false;
	}
} 

/*
===============================================================================

  idTrigger_Interact_ChangeXpos
	
===============================================================================
*/

const idEventDef EV_CheckTargets( "<checkTargets>", NULL );

CLASS_DECLARATION( idTrigger_Interact, idTrigger_Interact_ChangeXpos )
	//EVENT( EV_Activate,			idTrigger_Interact_ChangeXpos::Event_Trigger ) //inherited
	//EVENT( EV_Interact,			idTrigger_Interact_ChangeXpos::Event_Interact ) //inherited
	EVENT( EV_CheckTargets,		idTrigger_Interact_ChangeXpos::Event_CheckTargets )
END_CLASS

/*
================
idTrigger_Interact_ChangeXpos::idTrigger_Interact_ChangeXpos
================
*/
idTrigger_Interact_ChangeXpos::idTrigger_Interact_ChangeXpos( void ) {
}

/*
================
idTrigger_Interact_ChangeXpos::Spawn
================
*/
void idTrigger_Interact_ChangeXpos::Spawn( void ) {
	//check targets start
	if ( spawnArgs.MatchPrefix( "target" ) ) {
		if ( gameLocal.GameState() == GAMESTATE_STARTUP ) {
			PostEventMS( &EV_CheckTargets, 0 );
		} else {
			// not during startup, so it's ok to get the targets
			CheckTargets();
		}
	} else { //no targets!
		gameLocal.Warning("%s has no targets!", GetName() );
		interactFlags = 0; //never interact
		Disable(); //never interact again
	}
	//check targets end
}

/*
===============
idTrigger_Interact_ChangeXpos::Event_CheckTargets
===============
*/
void idTrigger_Interact_ChangeXpos::Event_CheckTargets( void ) {
	CheckTargets();
}

/*
===============
idTrigger_Interact_ChangeXpos::CheckTargets
===============
*/
void idTrigger_Interact_ChangeXpos::CheckTargets( void ) {
	bool hasTargetWithHigherX = ( GetTargetGreaterX( true ) != NULL );
	bool hasTargetWithLowerX = ( GetTargetGreaterX( false ) != NULL );

	interactFlags = 0; //reset the flags

	if(hasTargetWithHigherX){
		//gameLocal.Printf("%s hasTargetWithHigherX\n", GetName() );
		interactFlags |= INTERACT_UP;
	}
	if(hasTargetWithLowerX){
		//gameLocal.Printf("%s hasTargetWithLowerX\n", GetName() );
		interactFlags |= INTERACT_DOWN;
	}
}

/*
================
idTrigger_Interact_ChangeXpos::InteractionAction
================
*/
void idTrigger_Interact_ChangeXpos::InteractionAction( idPlayer *player, int flags ) { 
	idEntity	*validtarget;
	bool		greaterX;
	
	greaterX = 0 != ( flags & INTERACT_UP ); //we assume that if it's not UP it is DOWN, because it doesn't respond to other modes. 
	validtarget = GetTargetGreaterX( greaterX );
	if( validtarget ){
		if( player->StartForcedMov( validtarget, 0, spawnArgs.GetBool( "canAbort", "1" ) ) ){
			CallScript();
		}
	}else{ //someone removed a target!!
		gameLocal.Warning("idTrigger_Interact_ChangeXpos::failed to find the target\n");
		//remove the flag for the future so we'll never respond again to this mode
		if(greaterX){
			interactFlags &= ~ INTERACT_UP; 
		}else{
			interactFlags &= ~ INTERACT_DOWN; 
		}
	}

	if ( wait > 0 ) {
		nextInteractTime = gameLocal.time + SEC2MS( wait );
	}else {
		nextInteractTime = gameLocal.time + 1;
		//PostEventMS( &EV_Remove, 0 ); //NEVER remove this kind of trigger!
	}
}

/*
================
idTrigger_Interact_ChangeXpos::GetTargetGreaterX
================
*/
idEntity * idTrigger_Interact_ChangeXpos::GetTargetGreaterX( bool greaterX ) {
	idEntity *ent;
	float deltaX;

	for( int i = 0; i < targets.Num(); i++){
		ent = targets[ i ].GetEntity();
		if( !ent ) continue; 

		deltaX = ent->GetPhysics()->GetOrigin().x - GetPhysics()->GetOrigin().x;
		//gameLocal.Printf("upOrDown: %d, deltaX: %f \n", upOrDown ,deltaX );

		if( greaterX && deltaX > 10.0f ) return ent;
		if( !greaterX && deltaX < -10.0f ) return ent;
	}
	return NULL;
}

#if 0
/*
===============================================================================

  idTrigger_Secret
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Secret )
	EVENT( EV_Activate,			idTrigger_Secret::Event_Trigger )
END_CLASS

/*
================
idTrigger_Secret::idTrigger_Secret
================
*/
idTrigger_Secret::idTrigger_Secret( void ) {
	found = false;
}

/*
================
idTrigger_Secret::Save
================
*/
void idTrigger_Secret::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( found );
}

/*
================
idTrigger_Secret::Restore
================
*/
void idTrigger_Secret::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( found );
}


/*
================
idTrigger_Secret::Spawn
================
*/
void idTrigger_Secret::Spawn( void ) {
	gameLocal.secrets_spawned_counter++;
	Disable(); //never interact with anything.
}

/*
================
idTrigger_Secret::TriggerAction
================
*/
void idTrigger_Secret::TriggerAction( idEntity *activator ) {
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player ) {
		player->AddSecretFound();
	}
	
	ActivateTargets( activator );
	CallScript();

	//PostEventMS( &EV_Remove, 0 );
}

/*
================
idTrigger_Secret::Event_Trigger
================
*/
void idTrigger_Secret::Event_Trigger( idEntity *activator ) {
	if( !found ){
		TriggerAction( activator );
		found = true;
	}
}
#endif

//ivan end

/*
===============================================================================

  idTrigger_EntityName

===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_EntityName )
	EVENT( EV_Touch,			idTrigger_EntityName::Event_Touch )
	EVENT( EV_Activate,			idTrigger_EntityName::Event_Trigger )
	EVENT( EV_TriggerAction,	idTrigger_EntityName::Event_TriggerAction )
END_CLASS

/*
================
idTrigger_EntityName::idTrigger_EntityName
================
*/
idTrigger_EntityName::idTrigger_EntityName( void ) {
	wait = 0.0f;
	random = 0.0f;
	delay = 0.0f;
	random_delay = 0.0f;
	nextTriggerTime = 0;
	triggerFirst = false;
}

/*
================
idTrigger_EntityName::Save
================
*/
void idTrigger_EntityName::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( wait );
	savefile->WriteFloat( random );
	savefile->WriteFloat( delay );
	savefile->WriteFloat( random_delay );
	savefile->WriteInt( nextTriggerTime );
	savefile->WriteBool( triggerFirst );
	savefile->WriteString( entityName );
}

/*
================
idTrigger_EntityName::Restore
================
*/
void idTrigger_EntityName::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( wait );
	savefile->ReadFloat( random );
	savefile->ReadFloat( delay );
	savefile->ReadFloat( random_delay );
	savefile->ReadInt( nextTriggerTime );
	savefile->ReadBool( triggerFirst );
	savefile->ReadString( entityName );
}

/*
================
idTrigger_EntityName::Spawn
================
*/
void idTrigger_EntityName::Spawn( void ) {
	spawnArgs.GetFloat( "wait", "0.5", wait );
	spawnArgs.GetFloat( "random", "0", random );
	spawnArgs.GetFloat( "delay", "0", delay );
	spawnArgs.GetFloat( "random_delay", "0", random_delay );

	if ( random && ( random >= wait ) && ( wait >= 0 ) ) {
		random = wait - 1;
		gameLocal.Warning( "idTrigger_EntityName '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	if ( random_delay && ( random_delay >= delay ) && ( delay >= 0 ) ) {
		random_delay = delay - 1;
		gameLocal.Warning( "idTrigger_EntityName '%s' at (%s) has random_delay >= delay", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	spawnArgs.GetBool( "triggerFirst", "0", triggerFirst );

	entityName = spawnArgs.GetString( "entityname" );
	if ( !entityName.Length() ) {
		gameLocal.Error( "idTrigger_EntityName '%s' at (%s) doesn't have 'entityname' key specified", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	nextTriggerTime = 0;

	if ( !spawnArgs.GetBool( "noTouch" ) ) {
		GetPhysics()->SetContents( CONTENTS_TRIGGER );
	}
}

/*
================
idTrigger_EntityName::TriggerAction
================
*/
void idTrigger_EntityName::TriggerAction( idEntity *activator ) {
	ActivateTargets( activator );
	CallScript();

	if ( wait >= 0 ) {
		nextTriggerTime = gameLocal.time + SEC2MS( wait + random * gameLocal.random.CRandomFloat() );
	} else {
		// we can't just remove (this) here, because this is a touch function
		// called while looping through area links...
		nextTriggerTime = gameLocal.time + 1;
		PostEventMS( &EV_Remove, 0 );
	}
}

/*
================
idTrigger_EntityName::Event_TriggerAction
================
*/
void idTrigger_EntityName::Event_TriggerAction( idEntity *activator ) {
	TriggerAction( activator );
}

/*
================
idTrigger_EntityName::Event_Trigger

the trigger was just activated
activated should be the entity that originated the activation sequence (ie. the original target)
activator should be set to the activator so it can be held through a delay
so wait for the delay time before firing
================
*/
void idTrigger_EntityName::Event_Trigger( idEntity *activator ) {
	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	if ( !activator || ( activator->name != entityName ) ) {
		return;
	}

	if ( triggerFirst ) {
		triggerFirst = false;
		return;
	}

	// don't allow it to trigger twice in a single frame
	nextTriggerTime = gameLocal.time + 1;

	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, activator );
	} else {
		TriggerAction( activator );
	}
}

/*
================
idTrigger_EntityName::Event_Touch
================
*/
void idTrigger_EntityName::Event_Touch( idEntity *other, trace_t *trace ) {
	if( triggerFirst ) {
		return;
	}

	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	if ( !other || ( other->name != entityName ) ) {
		return;
	}

	nextTriggerTime = gameLocal.time + 1;
	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, other );
	} else {
		TriggerAction( other );
	}
}

/*
===============================================================================

  idTrigger_Timer

===============================================================================
*/

const idEventDef EV_Timer( "<timer>", NULL );

CLASS_DECLARATION( idTrigger, idTrigger_Timer )
	EVENT( EV_Timer,		idTrigger_Timer::Event_Timer )
	EVENT( EV_Activate,		idTrigger_Timer::Event_Use )
END_CLASS

/*
================
idTrigger_Timer::idTrigger_Timer
================
*/
idTrigger_Timer::idTrigger_Timer( void ) {
	random = 0.0f;
	wait = 0.0f;
	on = false;
	delay = 0.0f;
}

/*
================
idTrigger_Timer::Save
================
*/
void idTrigger_Timer::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( random );
	savefile->WriteFloat( wait );
	savefile->WriteBool( on );
	savefile->WriteFloat( delay );
	savefile->WriteString( onName );
	savefile->WriteString( offName );
}

/*
================
idTrigger_Timer::Restore
================
*/
void idTrigger_Timer::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( random );
	savefile->ReadFloat( wait );
	savefile->ReadBool( on );
	savefile->ReadFloat( delay );
	savefile->ReadString( onName );
	savefile->ReadString( offName );
}

/*
================
idTrigger_Timer::Spawn

Repeatedly fires its targets.
Can be turned on or off by using.
================
*/
void idTrigger_Timer::Spawn( void ) {
	spawnArgs.GetFloat( "random", "1", random );
	spawnArgs.GetFloat( "wait", "1", wait );
	spawnArgs.GetBool( "start_on", "0", on );
	spawnArgs.GetFloat( "delay", "0", delay );
	onName = spawnArgs.GetString( "onName" );
	offName = spawnArgs.GetString( "offName" );

	if ( random >= wait && wait >= 0 ) {
		random = wait - 0.001;
		gameLocal.Warning( "idTrigger_Timer '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	if ( on ) {
		PostEventSec( &EV_Timer, delay );
	}
}

/*
================
idTrigger_Timer::Enable
================
*/
void idTrigger_Timer::Enable( void ) {
	// if off, turn it on
	if ( !on ) {
		on = true;
		PostEventSec( &EV_Timer, delay );
	}
}

/*
================
idTrigger_Timer::Disable
================
*/
void idTrigger_Timer::Disable( void ) {
	// if on, turn it off
	if ( on ) {
		on = false;
		CancelEvents( &EV_Timer );
	}
}

/*
================
idTrigger_Timer::Event_Timer
================
*/
void idTrigger_Timer::Event_Timer( void ) {
	ActivateTargets( this );

	// set time before next firing
	if ( wait >= 0.0f ) {
		PostEventSec( &EV_Timer, wait + gameLocal.random.CRandomFloat() * random );
	}
}

/*
================
idTrigger_Timer::Event_Use
================
*/
void idTrigger_Timer::Event_Use( idEntity *activator ) {
	// if on, turn it off
	if ( on ) {
		if ( offName.Length() && offName.Icmp( activator->GetName() ) ) {
			return;
		}
		on = false;
		CancelEvents( &EV_Timer );
	} else {
		// turn it on
		if ( onName.Length() && onName.Icmp( activator->GetName() ) ) {
			return;
		}
		on = true;
		PostEventSec( &EV_Timer, delay );
	}
}

/*
===============================================================================

  idTrigger_Count

===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Count )
	EVENT( EV_Activate,	idTrigger_Count::Event_Trigger )
	EVENT( EV_TriggerAction,	idTrigger_Count::Event_TriggerAction )
END_CLASS

/*
================
idTrigger_Count::idTrigger_Count
================
*/
idTrigger_Count::idTrigger_Count( void ) {
	goal = 0;
	count = 0;
	delay = 0.0f;
}

/*
================
idTrigger_Count::Save
================
*/
void idTrigger_Count::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( goal );
	savefile->WriteInt( count );
	savefile->WriteFloat( delay );
}

/*
================
idTrigger_Count::Restore
================
*/
void idTrigger_Count::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( goal );
	savefile->ReadInt( count );
	savefile->ReadFloat( delay );
}

/*
================
idTrigger_Count::Spawn
================
*/
void idTrigger_Count::Spawn( void ) {
	spawnArgs.GetInt( "count", "1", goal );
	spawnArgs.GetFloat( "delay", "0", delay );
	count = 0;
}

/*
================
idTrigger_Count::Event_Trigger
================
*/
void idTrigger_Count::Event_Trigger( idEntity *activator ) {
	// goal of -1 means trigger has been exhausted
	if (goal >= 0) {
		count++;
		if ( count >= goal ) {
			if (spawnArgs.GetBool("repeat")) {
				count = 0;
			} else {
				goal = -1;
			}
			PostEventSec( &EV_TriggerAction, delay, activator );
		}
	}
}

/*
================
idTrigger_Count::Event_TriggerAction
================
*/
void idTrigger_Count::Event_TriggerAction( idEntity *activator ) {
	ActivateTargets( activator );
	CallScript();
	if ( goal == -1 ) {
		PostEventMS( &EV_Remove, 0 );
	}
}

/*
===============================================================================

  idTrigger_Hurt

===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Hurt )
	EVENT( EV_Touch,		idTrigger_Hurt::Event_Touch )
	EVENT( EV_Activate,		idTrigger_Hurt::Event_Toggle )
END_CLASS


/*
================
idTrigger_Hurt::idTrigger_Hurt
================
*/
idTrigger_Hurt::idTrigger_Hurt( void ) {
	on = false;
	delay = 0.0f;
	nextTime = 0;
}

/*
================
idTrigger_Hurt::Save
================
*/
void idTrigger_Hurt::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( on );
	savefile->WriteFloat( delay );
	savefile->WriteInt( nextTime );
}

/*
================
idTrigger_Hurt::Restore
================
*/
void idTrigger_Hurt::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( on );
	savefile->ReadFloat( delay );
	savefile->ReadInt( nextTime );
}

/*
================
idTrigger_Hurt::Spawn

	Damages activator
	Can be turned on or off by using.
================
*/
void idTrigger_Hurt::Spawn( void ) {
	spawnArgs.GetBool( "on", "1", on );
	spawnArgs.GetFloat( "delay", "1.0", delay );
	nextTime = gameLocal.time;
	Enable();
}

/*
================
idTrigger_Hurt::Event_Touch
================
*/
void idTrigger_Hurt::Event_Touch( idEntity *other, trace_t *trace ) {
	const char *damage;

	if ( on && other && gameLocal.time >= nextTime ) {
		damage = spawnArgs.GetString( "def_damage", "damage_painTrigger" );
		other->Damage( NULL, NULL, vec3_origin, damage, 1.0f, INVALID_JOINT );

		ActivateTargets( other );
		CallScript();

		nextTime = gameLocal.time + SEC2MS( delay );
	}
}

/*
================
idTrigger_Hurt::Event_Toggle
================
*/
void idTrigger_Hurt::Event_Toggle( idEntity *activator ) {
	on = !on;
}


/*
===============================================================================

  idTrigger_Fade

===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Fade )
	EVENT( EV_Activate,		idTrigger_Fade::Event_Trigger )
END_CLASS

/*
================
idTrigger_Fade::Event_Trigger
================
*/
void idTrigger_Fade::Event_Trigger( idEntity *activator ) {
	idVec4		fadeColor;
	int			fadeTime;
	idPlayer	*player;

	player = gameLocal.GetLocalPlayer();
	if ( player ) {
		fadeColor = spawnArgs.GetVec4( "fadeColor", "0, 0, 0, 1" );
		fadeTime = SEC2MS( spawnArgs.GetFloat( "fadeTime", "0.5" ) );
		player->playerView.Fade( fadeColor, fadeTime );
		PostEventMS( &EV_ActivateTargets, fadeTime, activator );
	}
}

/*
===============================================================================

  idTrigger_Touch

===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Touch )
	EVENT( EV_Activate,		idTrigger_Touch::Event_Trigger )
END_CLASS


/*
================
idTrigger_Touch::idTrigger_Touch
================
*/
idTrigger_Touch::idTrigger_Touch( void ) {
	clipModel = NULL;
}

/*
================
idTrigger_Touch::Spawn
================
*/
void idTrigger_Touch::Spawn( void ) {
	// get the clip model
	clipModel = new idClipModel( GetPhysics()->GetClipModel() );

	// remove the collision model from the physics object
	GetPhysics()->SetClipModel( NULL, 1.0f );

	if ( spawnArgs.GetBool( "start_on" ) ) {
		BecomeActive( TH_THINK );
	}
}

/*
================
idTrigger_Touch::Save
================
*/
void idTrigger_Touch::Save( idSaveGame *savefile ) {
	savefile->WriteClipModel( clipModel );
}

/*
================
idTrigger_Touch::Restore
================
*/
void idTrigger_Touch::Restore( idRestoreGame *savefile ) {
	savefile->ReadClipModel( clipModel );
}

/*
================
idTrigger_Touch::TouchEntities
================
*/
void idTrigger_Touch::TouchEntities( void ) {
	int numClipModels, i;
	idBounds bounds;
	idClipModel *cm, *clipModelList[ MAX_GENTITIES ];

	if ( clipModel == NULL || scriptFunction == NULL ) {
		return;
	}

	bounds.FromTransformedBounds( clipModel->GetBounds(), clipModel->GetOrigin(), clipModel->GetAxis() );
	numClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	for ( i = 0; i < numClipModels; i++ ) {
		cm = clipModelList[ i ];

		if ( !cm->IsTraceModel() ) {
			continue;
		}

		idEntity *entity = cm->GetEntity();

		if ( !entity ) {
			continue;
		}

		if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
									clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) {
			continue;
		}

		ActivateTargets( entity );

		idThread *thread = new idThread();
		thread->CallFunction( entity, scriptFunction, false );
		thread->DelayedStart( 0 );
	}
}

/*
================
idTrigger_Touch::Think
================
*/
void idTrigger_Touch::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		TouchEntities();
	}
	idEntity::Think();
}

/*
================
idTrigger_Touch::Event_Trigger
================
*/
void idTrigger_Touch::Event_Trigger( idEntity *activator ) {
	if ( thinkFlags & TH_THINK ) {
		BecomeInactive( TH_THINK );
	} else {
		BecomeActive( TH_THINK );
	}
}

/*
================
idTrigger_Touch::Enable
================
*/
void idTrigger_Touch::Enable( void ) {
	BecomeActive( TH_THINK );
}

/*
================
idTrigger_Touch::Disable
================
*/
void idTrigger_Touch::Disable( void ) {
	BecomeInactive( TH_THINK );
}
