// Created by Ivan_the_B
//

#include "../../idlib/precompiled.h"
#pragma hdrstop
#include "../Actor.h"
#include "../Entity.h"
#include "../Projectile.h"
#include "../script/Script_Thread.h"
#include "../gamesys/SysCvar.h"
#include "../Game_local.h"
#include "../Moveable.h"
#include "AI.h"
#include "AI_bot.h"

/*
===============================================================================

	idAI_Bot

===============================================================================
*/

const idEventDef AI_Bot_SelectWeapon( "selectBotWeapon", "d", 'd' );
const idEventDef AI_Bot_SelectAnotherWeapon( "selectAnotherWeapon", "dd", 'd' );
const idEventDef AI_Bot_GetCurrentWeapon( "getCurrentBotWeapon", NULL, 'd' );
const idEventDef AI_Bot_GetWeaponNumByName( "getBotWeaponNumByName", "s", 'd' );
const idEventDef AI_Bot_FireWeapon( "fireWeapon", NULL, 'e' );
const idEventDef AI_Bot_CanHitEnemyFromCurrentWeapon( "canHitEnemyFromCurrentWeapon", NULL, 'd' );
const idEventDef AI_Bot_CanFireToEnemyNoSelfDamage( "canFireToEnemyNoSelfDamage", "d", 'd' );
const idEventDef AI_Bot_CanHitEnemyFromFireAnim( "canHitEnemyFromFireAnim", "d", 'd' ); 
const idEventDef AI_Bot_GetIdleAnim( "getIdleAnim", NULL, 's' );
const idEventDef AI_Bot_GetReloadAnim( "getReloadAnim", NULL, 's' );
const idEventDef AI_Bot_GetFireAnim( "getFireAnim", NULL, 's' );
const idEventDef AI_Bot_ReloadWeapon( "reloadWeapon" );
const idEventDef AI_Bot_GetAmmoInClip( "getAmmoInClip", NULL, 'd' );
const idEventDef AI_Bot_GetClipSize( "getClipSize", NULL, 'd' );
const idEventDef AI_Bot_CheckReloadTolerance( "checkReloadTolerance", NULL, 'd' );
const idEventDef AI_Bot_GetRandomTargetTypePrefix( "getRandomTargetTypePrefix", "ss", 'e' );
const idEventDef AI_Bot_LostTimeMoreThan( "lostTimeMoreThan", "d", 'd' );
const idEventDef AI_Bot_WeaponChangedMoreThan( "weaponChangedMoreThan", "d", 'd' );
const idEventDef AI_Bot_PlayAnimOnWeapon( "playAnimOnWeapon", "s" );
const idEventDef AI_Bot_ReleaseNode( "releaseCurrentNode" );
const idEventDef AI_Bot_TryLockNode( "tryLockNode", "e", 'd' );
const idEventDef AI_Bot_FindEnemyAIorPL( "findEnemyAIorPL", "d", 'e' );

/*
//redefined:
const idEventDef AI_CanHitEnemyFromAnim( "canHitEnemyFromAnim", "s", 'd' ); 
const idEventDef AI_CreateMissile( "createMissile", "s", 'e' ); 
const idEventDef AI_LaunchMissile( "launchMissile", "vv", 'e' ); 
const idEventDef AI_CanHitEnemyFromJoint( "canHitEnemyFromJoint", "s", 'd' ); 
*/



CLASS_DECLARATION( idAI, idAI_Bot ) 
	EVENT( AI_Bot_SelectWeapon,					idAI_Bot::Event_SelectWeapon )
	EVENT( AI_Bot_SelectAnotherWeapon,			idAI_Bot::Event_SelectAnotherWeapon )
	EVENT( AI_Bot_GetCurrentWeapon,				idAI_Bot::Event_GetCurrentWeapon )
	EVENT( AI_Bot_GetWeaponNumByName,			idAI_Bot::Event_GetWeaponNumByName ) 
	EVENT( AI_Bot_FireWeapon,					idAI_Bot::Event_FireWeapon )  
	EVENT( AI_Bot_CanHitEnemyFromCurrentWeapon,	idAI_Bot::Event_CanHitEnemyFromCurrentWeapon ) 
	EVENT( AI_Bot_CanFireToEnemyNoSelfDamage,	idAI_Bot::Event_CanFireToEnemyNoSelfDamage )
	EVENT( AI_Bot_CanHitEnemyFromFireAnim,		idAI_Bot::Event_CanHitEnemyFromFireAnim )
	EVENT( AI_Bot_GetIdleAnim,					idAI_Bot::Event_GetIdleAnim )
	EVENT( AI_Bot_GetReloadAnim,				idAI_Bot::Event_GetReloadAnim )
	EVENT( AI_Bot_GetFireAnim,					idAI_Bot::Event_GetFireAnim )
	EVENT( AI_Bot_ReloadWeapon,					idAI_Bot::Event_ReloadWeapon )
	EVENT( AI_Bot_GetAmmoInClip,				idAI_Bot::Event_GetAmmoInClip )
	EVENT( AI_Bot_GetClipSize,					idAI_Bot::Event_GetClipSize )
	EVENT( AI_Bot_CheckReloadTolerance,			idAI_Bot::Event_CheckReloadTolerance ) 
	EVENT( AI_Bot_LostTimeMoreThan,				idAI_Bot::Event_LostTimeMoreThan )
	EVENT( AI_Bot_WeaponChangedMoreThan,		idAI_Bot::Event_WeaponChangedMoreThan )
	EVENT( AI_Bot_PlayAnimOnWeapon,				idAI_Bot::Event_PlayAnimOnWeapon )
	EVENT( AI_Bot_ReleaseNode,					idAI_Bot::Event_ReleaseNode )
	EVENT( AI_Bot_TryLockNode,					idAI_Bot::Event_TryLockNode )
	EVENT( AI_Bot_FindEnemyAIorPL,				idAI_Bot::Event_FindEnemyAIorPL )
	
	EVENT( AI_CanHitEnemyFromAnim,				idAI_Bot::Event_CanHitEnemyFromAnim ) //redefined
	EVENT( AI_CreateMissile,					idAI_Bot::Event_CreateMissile ) //redefined
	EVENT( AI_LaunchMissile,					idAI_Bot::Event_LaunchMissile ) //redefined
	EVENT( AI_CanHitEnemyFromJoint,				idAI_Bot::Event_CanHitEnemyFromJoint ) //redefined
	EVENT( AI_Burn,								idAI_Bot::Event_Burn )
END_CLASS


/*
===============================================================================

	Constructor and Destructor

===============================================================================
*/

/*
=====================
idAI_Bot::idAI_Bot
=====================
*/

idAI_Bot::idAI_Bot() {
	currentWeapon = -1; //nothing
	lastHitFCWCheckResult = false;
	lastHitFCWCheckTime = 0; 
	lastHitNSDCheckResult = false;
	lastHitNSDCheckTime = 0;
	lastVisibleEnemyTime = 0;
	lastWeaponChangedTime = 0;
	currentNode = NULL;
	weapons.SetGranularity( 1 );
}


/*
=====================
idAI_Bot::~idAI_Bot
=====================
*/

idAI_Bot::~idAI_Bot() {
	int i;
	idEntity *ent;

	//gameLocal.Printf("~idAI_Bot!\n");

	DeconstructScriptObject();
	scriptObject.Free();
	
	// remove any attached entities
	for( i = 0; i < weapons.Num(); i++ ) {
		//weapon entity
		ent = weapons[ i ].ent.GetEntity();
		if ( ent ) {
			ent->PostEventMS( &EV_Remove, 0 );
		}

		//projectileClipModel
		delete weapons[ i ].projectileClipModel;
	}
}

/*
=====================
idAI_Bot::Save
=====================
*/
void idAI_Bot::Save( idSaveGame *savefile ) const {
	int i;

	currentNode.Save( savefile );
	savefile->WriteInt( currentWeapon );
	savefile->WriteInt( lastVisibleEnemyTime );
	savefile->WriteBool( lastHitFCWCheckResult );
	savefile->WriteInt( lastHitFCWCheckTime );
	savefile->WriteBool( lastHitNSDCheckResult );
	savefile->WriteInt( lastHitNSDCheckTime );
	savefile->WriteInt( lastWeaponChangedTime );

	savefile->WriteInt( weapons.Num() );
	for( i = 0; i < weapons.Num(); i++ ) {
		//spawn settings
		weapons[i].ent.Save( savefile );
		savefile->WriteInt( weapons[i].channel );
		savefile->WriteJoint( weapons[i].fireJoint );
		savefile->WriteJoint( weapons[i].bindJoint );
		savefile->WriteJoint( weapons[i].flashJoint ); 
	
		//weapon
		savefile->WriteInt( weapons[i].priorityLevel );
		savefile->WriteBool( weapons[i].enabled );
		savefile->WriteString( weapons[i].weaponName );
		savefile->WriteFloat( weapons[i].minSelfDmgDistance );

		//projectile - const idDict * projectileDef
		idStr projectileName;
		idEntity *ent;
		ent = weapons[ i ].ent.GetEntity();
		ent->spawnArgs.GetString( "def_projectile", "", projectileName );
		savefile->WriteString( projectileName );


		//projectile - other stuff
		savefile->WriteFloat( weapons[i].projectileRadius );
		savefile->WriteFloat( weapons[i].projectileSpeed );
		savefile->WriteVec3( weapons[i].projectileVelocity );
		savefile->WriteVec3( weapons[i].projectileGravity );

		//mutable idClipModel *projectileClipModel; NOT SAVED

		//attack settings
		savefile->WriteFloat( weapons[i].attack_accuracy );
		savefile->WriteFloat( weapons[i].projectile_spread );
		savefile->WriteInt( weapons[i].num_projectiles );

		//clip
		savefile->WriteInt( weapons[i].clipSize );
		savefile->WriteInt( weapons[i].ammoInClip );
		savefile->WriteInt( weapons[i].reloadTolerance );

		//light
		savefile->WriteMaterial( weapons[i].shader );
		savefile->WriteVec3( weapons[i].flashColor );
		savefile->WriteFloat( weapons[i].flashRadius );
		savefile->WriteInt( weapons[i].flashTime );

	}


}

/*
=====================
idAI_Bot::Restore
=====================
*/
void idAI_Bot::Restore( idRestoreGame *savefile ) {
	int	num;
	int i;

	//gameLocal.Printf("idAI_Bot::Restore\n");
	
	currentNode.Restore( savefile );

	savefile->ReadInt( currentWeapon );
	savefile->ReadInt( lastVisibleEnemyTime );
	savefile->ReadBool( lastHitFCWCheckResult );
	savefile->ReadInt( lastHitFCWCheckTime );
	savefile->ReadBool( lastHitNSDCheckResult );
	savefile->ReadInt( lastHitNSDCheckTime );
	savefile->ReadInt( lastWeaponChangedTime );

	savefile->ReadInt( num );
	weapons.SetGranularity( 1 );
	weapons.SetNum( num );

	for( i = 0; i < num; i++ ) {
		//spawn settings
		weapons[i].ent.Restore( savefile );
		savefile->ReadInt( weapons[i].channel );
		savefile->ReadJoint( weapons[i].fireJoint );
		savefile->ReadJoint( weapons[i].bindJoint );
		savefile->ReadJoint( weapons[i].flashJoint ); 
	
		//weapon
		savefile->ReadInt( weapons[i].priorityLevel );
		savefile->ReadBool( weapons[i].enabled );
		savefile->ReadString( weapons[i].weaponName );
		savefile->ReadFloat( weapons[i].minSelfDmgDistance );

		//projectile - const idDict * projectileDef
		idStr projectileName;
		savefile->ReadString( projectileName );
		if ( projectileName.Length() ) {
			weapons[ i ].projectileDef = gameLocal.FindEntityDefDict( projectileName );
		} else {
			weapons[ i ].projectileDef = NULL;
		}


		//projectile - other stuff
		savefile->ReadFloat( weapons[i].projectileRadius );
		savefile->ReadFloat( weapons[i].projectileSpeed );
		savefile->ReadVec3( weapons[i].projectileVelocity );
		savefile->ReadVec3( weapons[i].projectileGravity );

		//mutable idClipModel *projectileClipModel; NOT SAVED
		weapons[ i ].projectileClipModel = NULL;

		//attack settings
		savefile->ReadFloat( weapons[i].attack_accuracy );
		savefile->ReadFloat( weapons[i].projectile_spread );
		savefile->ReadInt( weapons[i].num_projectiles );

		//clip
		savefile->ReadInt( weapons[i].clipSize );
		savefile->ReadInt( weapons[i].ammoInClip );
		savefile->ReadInt( weapons[i].reloadTolerance );

		//light
		savefile->ReadMaterial( weapons[i].shader );
		savefile->ReadVec3( weapons[i].flashColor );
		savefile->ReadFloat( weapons[i].flashRadius );
		savefile->ReadInt( weapons[i].flashTime );	

		// Link the script variables back to the scriptobject
		LinkScriptVariables();

		//upd muzzle light
		setWeaponMuzzleFlash();
	}
}


/*
===============================================================================

	Pain and talk stuff

===============================================================================
*/

/*
=====================
idAI::Pain
=====================

bool idAI::Pain( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	
	SAY_DAMAGE = true;

	return idAI::Pain( inflictor, attacker, damage, dir, location );
}
*/


/*
===============================================================================

	Spawn and remove stuff

===============================================================================
*/

/*
=====================
idAI_Bot::Spawn
=====================
*/
void idAI_Bot::Spawn( void ) {
	//gameLocal.Printf("idAI_Bot::Spawn\n");

	CreateWeapons();
	SelectInitialWeapon();
	ShowOnlyCurrentWeapon();
	LinkScriptVariables();
	/*
	//Warning: idAI_Bot::CalculateAttackOffsets is called by idAI::Spawn --> weapons are no available yet!
	//CalculateAttackOffsets(); 
	*/
}

/*
=====================
idAI_Bot::LinkScriptVariables
=====================
*/
void idAI_Bot::LinkScriptVariables( void ) {
	//idAI::LinkScriptVariables(); //useless because idAI::spawn() also calls idAI::LinkScriptVariables()
	AI_WEAPON_CHANGED.LinkTo( scriptObject, "AI_WEAPON_CHANGED" );
	AI_WEAPON_NEED_RELOAD.LinkTo( scriptObject, "AI_WEAPON_NEED_RELOAD" );
	AI_LEAVE_NODE_TO_FIGHT.LinkTo( scriptObject, "AI_LEAVE_NODE_TO_FIGHT" );
	AI_SAY_DAMAGED.LinkTo( scriptObject, "AI_SAY_DAMAGED" );
}

/*
================
idAI_Bot::CreateWeapons
================
*/
void idAI_Bot::CreateWeapons( void ) {
	idEntity *ent;
	idStr	weaponName;

	// spawn any weapons we might have
	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_weapon_", NULL );
	while ( kv ) {
		idDict args;
		
		args.Set( "classname", kv->GetValue().c_str() ); //TODO: check it!?

		// make items non-touchable so the player can't take them out of the character's hands
		args.Set( "no_touch", "1" );

		// don't let them drop to the floor
		args.Set( "dropToFloor", "0" );

		gameLocal.SpawnEntityDef( args, &ent );
		if ( !ent ) {
			gameLocal.Error( "Couldn't spawn '%s' to attach to entity '%s'", kv->GetValue().c_str(), name.c_str() );
		} else {
			// get the weapon name
			weaponName = kv->GetKey();
			weaponName.Strip( "def_weapon_" );
			AttachWeapon( ent, weaponName );
		}
		kv = spawnArgs.MatchPrefix( "def_weapon_", kv );
	}
}


/*
================
idAI_Bot::AttachWeapon
================
*/
void idAI_Bot::AttachWeapon( idEntity *ent, idStr weaponName) { //TODO: spawn only the enabled ones
	idVec3			origin;
	idMat3			axis;
	jointHandle_t	bindJoint;
	//jointHandle_t	fireJoint;
	idStr			jointName;
	idBotWeapon	&attach = weapons.Alloc();
	idAngles		angleOffset;
	idVec3			originOffset;

	idStr			projectileName;
	idAnimator		*entAnimator;

	//weapon number
	currentWeapon = weapons.NumAllocated()-1; //needed to make CreateProjectile working properly

	//bindJoint
	jointName = ent->spawnArgs.GetString( "bindJoint" );
	bindJoint = animator.GetJointHandle( jointName );
	if ( bindJoint == INVALID_JOINT ) {
		gameLocal.Error( "Joint '%s' not found for attaching '%s' on '%s'", jointName.c_str(), ent->GetClassname(), name.c_str() );
	}
	attach.bindJoint = bindJoint;

	angleOffset = ent->spawnArgs.GetAngles( "angles" );
	originOffset = ent->spawnArgs.GetVector( "origin" );

	attach.channel = animator.GetChannelForJoint( bindJoint );
	GetJointWorldTransform( bindJoint, gameLocal.time, origin, axis );
	attach.ent = ent;

	ent->SetOrigin( origin + originOffset * renderEntity.axis );
	idMat3 rotate = angleOffset.ToMat3();
	idMat3 newAxis = rotate * axis;
	ent->SetAxis( newAxis );
	ent->BindToJoint( this, bindJoint, true );
	ent->cinematic = cinematic;
	
	//fire joint
	jointName = ent->spawnArgs.GetString( "fireJoint", "barrel"); //default is barrel
	entAnimator = ent->GetAnimator(); 
	if ( entAnimator && entAnimator->ModelHandle() ) { // check if the entity has an MD5 model
		attach.fireJoint = entAnimator->GetJointHandle( jointName ); //this could also be INVALID_JOINT!
		attach.flashJoint = entAnimator->GetJointHandle( "flash" ); //this could also be INVALID_JOINT!
	}else{
		attach.fireJoint = INVALID_JOINT;
		attach.flashJoint = INVALID_JOINT;
	}

	/*
	//test only
	if ( attach.fireJoint == INVALID_JOINT ) {  
		gameLocal.Printf("fireJoint == INVALID_JOINT\n");
	}else{
		gameLocal.Printf("fireJoint is valid: %d \n", attach.fireJoint);
	}*/
	
	//name 
	attach.weaponName = weaponName;

	//enabled 
	attach.enabled = spawnArgs.GetBool(attach.weaponName.c_str(),"0"); //default disabled

	//projectile
	if ( ent->spawnArgs.GetString( "def_projectile", "", projectileName ) && projectileName.Length() ) {
			//gameLocal.Printf("def_projectile found\n");
			attach.projectileDef = gameLocal.FindEntityDefDict( projectileName );
			CreateProjectile( vec3_origin, viewAxis[ 0 ] ); //this sets the variable projectile
			attach.projectileRadius	= projectile.GetEntity()->GetPhysics()->GetClipModel()->GetBounds().GetRadius();
			attach.projectileVelocity	= idProjectile::GetVelocity( attach.projectileDef );
			attach.projectileGravity	= idProjectile::GetGravity( attach.projectileDef );
			attach.projectileSpeed	= projectileVelocity.Length();
			delete projectile.GetEntity();
			projectile = NULL;

			attach.attack_accuracy = ent->spawnArgs.GetFloat( "attack_accuracy", "1" ); //was 7 
			attach.projectile_spread = ent->spawnArgs.GetFloat( "projectile_spread", "0" );
			attach.num_projectiles = ent->spawnArgs.GetInt( "num_projectiles", "1" );
			attach.clipSize = ent->spawnArgs.GetInt( "clipSize", "0" );  //0 = unlimited
			if(attach.clipSize < 0){ attach.clipSize = 0; }
			attach.ammoInClip = attach.clipSize; //start with full ammo
			attach.reloadTolerance = ent->spawnArgs.GetInt( "reloadTolerance", "0" );
			attach.minSelfDmgDistance = ent->spawnArgs.GetFloat( "minSelfDmgDistance", "0" );

			//light
			attach.flashColor = ent->spawnArgs.GetVector( "flashColor", "0 0 0" );
			attach.flashRadius = ent->spawnArgs.GetFloat( "flashRadius" );	
			attach.flashTime = SEC2MS( ent->spawnArgs.GetFloat( "flashTime", "0.25" ) );

			const char *shader;
			spawnArgs.GetString( "mtr_flashShader", "muzzleflash", &shader );
			attach.shader = declManager->FindMaterial( shader, false );

	}else{
			//gameLocal.Printf("def_projectile not found\n");
			attach.projectileDef = NULL;
			attach.projectileRadius = 0.0f;
			attach.projectileSpeed =  0.0f;
			attach.projectileVelocity = vec3_origin;
			attach.projectileGravity = vec3_origin;

			attach.attack_accuracy = 0.0f;
			attach.projectile_spread = 0.0f;
			attach.num_projectiles = 0;
			attach.clipSize = 0;
			attach.ammoInClip = 0;
			attach.reloadTolerance = 0;
			attach.minSelfDmgDistance = 0;

			attach.flashColor = vec3_origin; 
			attach.flashRadius = 0.0f;
			attach.flashTime = 0;
			attach.shader = NULL;
	}	

	attach.priorityLevel = ent->spawnArgs.GetInt( "priorityLevel", "0" );
	attach.projectileClipModel = NULL; //will be initialized later

	//gameLocal.Printf("fireJoint check: %d \n", weapons[ currentWeapon ].fireJoint);
}


/*
===================
idAI_Bot::CalculateAttackOffsets

calculate joint positions on attack frames so we can do proper "can hit" tests
===================
*/
void idAI_Bot::CalculateAttackOffsets( void ) { //Warning: this is called by idAI::Spawn --> weapons are no available yet!
	const idDeclModelDef	*modelDef;
	int						num;
	int						i;
	int						frame;
	const frameCommand_t	*command;
	idMat3					axis;
	const idAnim			*anim;
	jointHandle_t			joint;

	//gameLocal.Printf("idAI_Bot::CalculateAttackOffsets \n");

	modelDef = animator.ModelDef();
	if ( !modelDef ) {
		return;
	}
	num = modelDef->NumAnims();
	
	// needs to be off while getting the offsets so that we account for the distance the monster moves in the attack anim
	animator.RemoveOriginOffset( false );

	// anim number 0 is reserved for non-existant anims.  to avoid off by one issues, just allocate an extra spot for
	// launch offsets so that anim number can be used without subtracting 1.
	missileLaunchOffset.SetGranularity( 1 );
	missileLaunchOffset.SetNum( num + 1 );
	missileLaunchOffset[ 0 ].Zero();

	for( i = 1; i <= num; i++ ) {
		missileLaunchOffset[ i ].Zero();
		anim = modelDef->GetAnim( i );
		if ( anim ) {
			/*
			frame = anim->FindFrameForFrameCommand( FC_LAUNCHMISSILE, &command );
			if ( frame >= 0 ) {
				joint = animator.GetJointHandle( command->string->c_str() );
				if ( joint == INVALID_JOINT ) {
					gameLocal.Error( "Invalid joint '%s' on 'launch_missile' frame command on frame %d of model '%s'", command->string->c_str(), frame, modelDef->GetName() );
				}
				GetJointTransformForAnim( joint, i, FRAME2MS( frame ), missileLaunchOffset[ i ], axis );
			}*/

			//ivan - fix for new framecommand --> this allows CanHitEnemyFromAnim to work
			frame = anim->FindFrameForFrameCommand( FC_FIREWEAPON, &command );
			if ( frame >= 0 ) {
				//gameLocal.Printf("FC_FIREWEAPON trovato: '%s' \n", anim->Name());
				joint = animator.GetJointHandle( "PISTOL_ATTACHER" ); //TODO: fix this hack.
				if ( joint == INVALID_JOINT ) {
					gameLocal.Error( "Invalid joint on 'fire_weapon' frame command on frame %d of model '%s'", frame, modelDef->GetName() );
				}
				GetJointTransformForAnim( joint, i, FRAME2MS( frame ), missileLaunchOffset[ i ], axis );
			}else{
				//gameLocal.Printf("FC_FIREWEAPON non trovato: '%s' \n", anim->Name());
			}
			//ivan end
		}
	}

	animator.RemoveOriginOffset( true );
}

/*
=====================
idAI_Bot::DropWeapon
=====================
*/
void idAI_Bot::DropWeapon( int weaponNum ) {
	int ammoToAdd; 
	idVec3 velocity;
	idEntity *myweapon;
	idEntity *item;

	if(weaponNum < 0 || weaponNum >= weapons.Num() ){
		return;
	}

	myweapon = weapons[ weaponNum ].ent.GetEntity();
	item = NULL;

	const char *classname = myweapon->spawnArgs.GetString( "def_dropItem" );
	if ( !classname[0] ) {
		return;
	}

	velocity = 150.0f * idAngles( -60, current_yaw, 0 ).ToForward();

	item = idMoveableItem::DropItem( classname, myweapon->GetPhysics()->GetOrigin(), myweapon->GetPhysics()->GetAxis(), velocity , 200, AI_WEAPON_DROP_TIME );
	if ( !item ) {
		return;
	}

	if(weapons[ weaponNum ].clipSize <= 0){ //do nothing if clipSize is unlimited
		//gameLocal.Printf("clipSize <= 0\n");
		return;
	}
		
	const idKeyValue * keyval = item->spawnArgs.MatchPrefix( "inv_ammo_" );	
	if ( keyval ) {
		ammoToAdd = weapons[ weaponNum ].ammoInClip;
		if(ammoToAdd <= 0){ ammoToAdd = 1; } //at least 1
		//gameLocal.Printf("clipSize %d \n",ammoToAdd);
		item->spawnArgs.SetInt( keyval->GetKey(), ammoToAdd );
	}

}

/*
=====================
idAI_Bot::Killed
=====================
*/
void idAI_Bot::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	idAI::Killed( inflictor, attacker, damage, dir, location );
	
	// drop the current weapon
	DropWeapon(currentWeapon);

	//now we can remove all the weapons
	RemoveWeapons();

	//be sure...
	flashJointWorld = INVALID_JOINT; 

	//release the current node
	if ( currentNode.GetEntity() ) {
		currentNode.GetEntity()->Release();
		currentNode = NULL;
	}
}
/*
================
idAI_Bot::RemoveWeapons
================
*/
void idAI_Bot::RemoveWeapons( void ) { 
	int i;
	idEntity *ent;

	// remove any attached weapons
	for( i = 0; i < weapons.Num(); i++ ) {
		//weapon entity
		ent = weapons[ i ].ent.GetEntity();
		if ( ent ) {
			ent->PostEventMS( &EV_Remove, 0 );
		}

		//projectileClipModel
		delete weapons[ i ].projectileClipModel;
	}

	weapons.Clear();
	currentWeapon = -1;
}

/*
===============================================================================

	Weapon selection and utilities

===============================================================================
*/


/*
================
idAI_Bot::SelectInitialWeapon
================
*/
void idAI_Bot::SelectInitialWeapon( void ) { 
	int weaponNum;
	idStr initialweaponValue;

	currentWeapon = -1; //start without weapons
	weaponNum = -1; //default is none

	//get the initialweapon key
	initialweaponValue = spawnArgs.GetString( "initialweapon", "" );
	if( initialweaponValue.Length() ){
		weaponNum = GetWeaponNumByName( initialweaponValue );
		//gameLocal.Printf("The initial requested weapon is: %d \n" ,weaponNum);
		SelectWeapon(weaponNum); //could upd currentWeapon
	}	

	//still no weapon selected... try to select the higher one.
	if(currentWeapon < 0){ 
		for( weaponNum = weapons.Num()-1; weaponNum >= 0; weaponNum-- ) {
			if ( weapons[ weaponNum ].enabled) {
				SelectWeapon(weaponNum); //currentWeapon = weaponNum;
				break;
			}
		}
	}

	//gameLocal.Printf("The initial weapon is: %d \n" ,currentWeapon);
}


/*
================
idAI_Bot::GetWeaponNumByName
================
*/
int idAI_Bot::GetWeaponNumByName(idStr name) { 
	int i;
	for( i = 0; i < weapons.Num(); i++ ) {
		if ( weapons[ i ].weaponName == name) {
			return i;
		}
	}
	return -1; //no weapon
}

/*
================
idAI_Bot::SelectWeapon
================
*/
bool idAI_Bot::SelectWeapon( int weaponNum ) {
	//gameLocal.Printf("SelectWeapon requested: %d \n" ,weaponNum);
	
	if (weaponNum == currentWeapon){
		//gameLocal.Printf("The requested weapon:%d is already selected\n" ,weaponNum);
		return false;
	}
	if( weaponNum < -1 || weaponNum >= weapons.Num()){ //if 3 allocated, max allowed value is 2. Note: -1 is accepted because means 'no weapon'
		gameLocal.Warning("The requested weapon: %d is out of range" ,weaponNum);
		return false;
	}

	if( (weaponNum >= 0)  && !weapons[ weaponNum ].enabled){
		gameLocal.Warning("The requested weapon: %d is disabled" ,weaponNum);
		return false;
	}

	//hide the current one
	if(currentWeapon >= 0){
		weapons[ currentWeapon ].ent.GetEntity()->Hide();
	}

	//show the new one
	if(weaponNum >= 0){
		weapons[ weaponNum ].ent.GetEntity()->Show();
		//ammo
		if(weapons[ weaponNum ].clipSize > 0){ //only if limited clip size
			if(weapons[ weaponNum ].ammoInClip <= 0){
				AI_WEAPON_NEED_RELOAD = true;
			}else{
				AI_WEAPON_NEED_RELOAD = false;
			}
		}
	}else{ //no weapon selected
		AI_WEAPON_NEED_RELOAD = false;
	}

	//remove the current projectileClipModel so that it'll be recreated the next time GetAimDir is called
	//DeleteProjectileClipModel();

	//upd the currentWeapon
	currentWeapon = weaponNum;
	//gameLocal.Printf("SelectWeapon done: %d \n" ,currentWeapon);

	//remember this moment 
	lastWeaponChangedTime = gameLocal.time;

	//flash joint
	if(currentWeapon >= 0){
		flashJointWorld = weapons[ currentWeapon ].flashJoint;
	}else{
		flashJointWorld = INVALID_JOINT;
	}

	//upd light
	setWeaponMuzzleFlash(); //ok because currentWeapon already updated

	//upd script
	AI_WEAPON_CHANGED = true;
	return true;
}


/*
================
idAI_Bot::ShowOnlyCurrentWeapon
================
*/
void idAI_Bot::ShowOnlyCurrentWeapon( void ) {
	int i;
	idEntity *ent;

	// remove any attached entities
	for( i = 0; i < weapons.Num(); i++ ) {
		ent = weapons[ i ].ent.GetEntity();
		if (i == currentWeapon ) {
			weapons[ i ].ent.GetEntity()->Show();
		}else{
			weapons[ i ].ent.GetEntity()->Hide();
		}
	}
}


/*
================
idAI_Bot::TriggerWeaponEffects
================
*/
void idAI_Bot::TriggerWeaponEffects( const idVec3 &muzzle ) {
	idVec3 org;
	idMat3 axis;
	idEntity *myWeaponEnt; 

	if ( !g_muzzleFlash.GetBool() ) {
		return;
	}

	myWeaponEnt = weapons[ currentWeapon ].ent.GetEntity();	
	if(myWeaponEnt->GetRenderEntity()){
		myWeaponEnt->GetRenderEntity()->shaderParms[SHADERPARM_TIMEOFFSET] = -MS2SEC( gameLocal.time );
		myWeaponEnt->GetRenderEntity()->shaderParms[ SHADERPARM_DIVERSITY ] = gameLocal.random.CRandomFloat();
	}

	if ( flashJointWorld != INVALID_JOINT ) { //flashJointWorld has been updated on weapon selection
		//was: GetJointWorldTransform( flashJointWorld, gameLocal.time, org, axis );
	
		//get the reference to the weapon animator
		idAnimator *myWeaponAnimator;
		myWeaponAnimator = myWeaponEnt->GetAnimator(); 

		//do the equivalent of GetJointWorldTransform even without being casted to idAnimatedEntity
		myWeaponAnimator->GetJointTransform( flashJointWorld, gameLocal.time, org, axis );
		myWeaponEnt->ConvertLocalToWorldTransform( org, axis );

		if ( worldMuzzleFlash.lightRadius.x > 0.0f ) {
			worldMuzzleFlash.axis = axis;
			worldMuzzleFlash.shaderParms[SHADERPARM_TIMEOFFSET] = -MS2SEC( gameLocal.time );
			if ( worldMuzzleFlashHandle != - 1 ) {
				gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );
			} else {
				worldMuzzleFlashHandle = gameRenderWorld->AddLightDef( &worldMuzzleFlash );
			}
			muzzleFlashEnd = gameLocal.time + flashTime;
			UpdateVisuals();
		}
	}
}


/*
================
idAI_Bot::UpdateMuzzleFlash
================
*/
void idAI_Bot::UpdateMuzzleFlash( void ) {
	if ( worldMuzzleFlashHandle != -1 ) { 
		if ( gameLocal.time >= muzzleFlashEnd ) {
			gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
			worldMuzzleFlashHandle = -1;
		} else {
			if ( flashJointWorld != INVALID_JOINT ) { //flashJointWorld has been updated on weapon selection
				idVec3 muzzle;
				idEntity *myWeaponEnt;
				idAnimator *myWeaponAnimator;

				//get the reference to the weapon and its animator
				myWeaponEnt = weapons[ currentWeapon ].ent.GetEntity();
				myWeaponAnimator = myWeaponEnt->GetAnimator(); 

				//do the equivalent of GetJointWorldTransform even without being casted to idAnimatedEntity
				myWeaponAnimator->GetJointTransform( flashJointWorld, gameLocal.time, muzzle, worldMuzzleFlash.axis );
				myWeaponEnt->ConvertLocalToWorldTransform( muzzle, worldMuzzleFlash.axis );

				worldMuzzleFlash.origin = muzzle;
				gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );
			}
		}
	}
}

/*
===================
idAI_Bot::InitMuzzleFlash
===================
*/
void idAI_Bot::InitMuzzleFlash( void ) { //called on spawn and on relaod. NOTE: in both cases weapons are not created yet --> just set default values.
	
	//gameLocal.Printf("idAI_Bot::InitMuzzleFlash\n");

	flashTime = 0;

	memset( &worldMuzzleFlash, 0, sizeof ( worldMuzzleFlash ) );

	worldMuzzleFlash.pointLight = true;
	worldMuzzleFlash.shader = NULL;
	worldMuzzleFlash.shaderParms[ SHADERPARM_RED ] = 0.0f;
	worldMuzzleFlash.shaderParms[ SHADERPARM_GREEN ] = 0.0f;
	worldMuzzleFlash.shaderParms[ SHADERPARM_BLUE ] = 0.0f;
	worldMuzzleFlash.shaderParms[ SHADERPARM_ALPHA ] = 1.0f;
	worldMuzzleFlash.shaderParms[ SHADERPARM_TIMESCALE ] = 1.0f;
	worldMuzzleFlash.lightRadius[0] = 0.0f;
	worldMuzzleFlash.lightRadius[1]	= 0.0f;
	worldMuzzleFlash.lightRadius[2]	= 0.0f;
	worldMuzzleFlash.noShadows = spawnArgs.GetBool( "flashNoShadows" );

	worldMuzzleFlashHandle = -1;
}


/*
================
idAI_Bot::setWeaponMuzzleFlash
================
*/
void idAI_Bot::setWeaponMuzzleFlash( void ) {
	const idMaterial *	shader;	
	idVec3				flashColor;
	float				flashRadius;

	if(currentWeapon < 0 || currentWeapon >= weapons.Num()){
		shader = NULL;
		flashColor = vec3_origin;
		flashRadius = 0.0f;
		flashTime = 0; //global
	}else{
		shader = weapons[ currentWeapon ].shader;
		flashColor = weapons[ currentWeapon ].flashColor;
		flashRadius = weapons[ currentWeapon ].flashRadius;
		flashTime = weapons[ currentWeapon ].flashTime; //global
	}
	
	worldMuzzleFlash.shader = shader;
	worldMuzzleFlash.shaderParms[ SHADERPARM_RED ] = flashColor[0];
	worldMuzzleFlash.shaderParms[ SHADERPARM_GREEN ] = flashColor[1];
	worldMuzzleFlash.shaderParms[ SHADERPARM_BLUE ] = flashColor[2];
	worldMuzzleFlash.lightRadius[0] = flashRadius;
	worldMuzzleFlash.lightRadius[1]	= flashRadius;
	worldMuzzleFlash.lightRadius[2]	= flashRadius;
}

/*
===============================================================================

	Projectiles stuff

===============================================================================
*/


/*
=====================
idAI_Bot::DeleteProjectileClipModel 
=====================

void idAI_Bot::DeleteProjectileClipModel( void ) const { 
	//remove the current projectileClipModel so that it'll be recreated the next time GetAimDir or other functions are called
	delete projectileClipModel;
	projectileClipModel = NULL;
}
*/

/*
=====================
idAI_Bot::CreateProjectileClipModel 
=====================
*/
void idAI_Bot::CreateProjectileClipModel( void ) const { //warning! a weapon needs to be selected when you call this!
	if ( weapons[ currentWeapon ].projectileClipModel == NULL ) {
		idBounds projectileBounds( vec3_origin );
		projectileBounds.ExpandSelf( weapons[ currentWeapon ].projectileRadius );
		weapons[ currentWeapon ].projectileClipModel = new idClipModel( idTraceModel( projectileBounds ) );
	}
}

/*
=====================
idAI_Bot::CreateProjectileClipModel 
=====================
*/
void idAI_Bot::CreateProjectileClipModel( int requestedWeapon ) const { //warning! make sure the requested weapon exists when you call this!
	if ( weapons[ requestedWeapon ].projectileClipModel == NULL ) {
		idBounds projectileBounds( vec3_origin );
		projectileBounds.ExpandSelf( weapons[ requestedWeapon ].projectileRadius );
		weapons[ requestedWeapon ].projectileClipModel = new idClipModel( idTraceModel( projectileBounds ) );
	}
}


/*
=====================
idAI_Bot::CreateProjectile
=====================
*/
idProjectile *idAI_Bot::CreateProjectile( const idVec3 &pos, const idVec3 &dir ) { //NOTE: make sure a weapon is selected and it has a weapon before calling this
	idEntity *ent;
	const char *clsname;
	const idDict *	currentProjectileDef;

	if ( !projectile.GetEntity() ) {
		currentProjectileDef = weapons[ currentWeapon ].projectileDef; 
		gameLocal.SpawnEntityDef( *currentProjectileDef, &ent, false );

		if ( !ent ) {
			clsname = currentProjectileDef->GetString( "classname" );
			gameLocal.Error( "Could not spawn entityDef '%s'", clsname );
		}
		
		if ( !ent->IsType( idProjectile::Type ) ) {
			clsname = ent->GetClassname();
			gameLocal.Error( "'%s' is not an idProjectile", clsname );
		}
		projectile = ( idProjectile * )ent;
	}
	projectile.GetEntity()->Create( this, pos, dir );
	return projectile.GetEntity();
}



/*
=====================
idAI_Bot::LaunchProjectile
=====================
*/
idProjectile *idAI_Bot::LaunchProjectile( const char *jointname, idEntity *target, bool clampToAttackCone ) {
	idVec3				muzzle;
	idVec3				dir;
	idVec3				start;
	trace_t				tr;
	idBounds			projBounds;
	float				distance;
	const idClipModel	*projClip;
	float				attack_accuracy;
	float				attack_cone;
	float				projectile_spread;
	float				diff;
	float				angle;
	float				spin;
	idAngles			ang;
	int					num_projectiles;
	int					i;
	idMat3				axis;
	idVec3				tmp;
	idProjectile		*lastProjectile;

	if ( currentWeapon < 0){
		gameLocal.Warning( "%s (%s) is trying to fire but doesn't have a weapon selected", name.c_str(), GetEntityDefName() );
		return NULL;
	}

	if ( !weapons[ currentWeapon ].projectileDef ) {
		gameLocal.Warning( "%s (%s) : the current weapon doesn't have a projectile specified", name.c_str(), GetEntityDefName() );
		return NULL;
	}

	attack_accuracy = weapons[ currentWeapon ].attack_accuracy;
	attack_cone = spawnArgs.GetFloat( "attack_cone", "70" );
	projectile_spread = weapons[ currentWeapon ].projectile_spread;
	num_projectiles = weapons[ currentWeapon ].num_projectiles;

	GetMuzzle( jointname, muzzle, axis );

	if ( !projectile.GetEntity() ) {
		CreateProjectile( muzzle, axis[ 0 ] );
	}

	lastProjectile = projectile.GetEntity();

	if ( target != NULL ) {
		tmp = target->GetPhysics()->GetAbsBounds().GetCenter() - muzzle;
		tmp.Normalize();
		axis = tmp.ToMat3();
	} else {
		axis = viewAxis;
	}

	// rotate it because the cone points up by default
	tmp = axis[2];
	axis[2] = axis[0];
	axis[0] = -tmp;

	// make sure the projectile starts inside the monster bounding box
	const idBounds &ownerBounds = physicsObj.GetAbsBounds();
	projClip = lastProjectile->GetPhysics()->GetClipModel();
	projBounds = projClip->GetBounds().Rotate( axis );

	// check if the owner bounds is bigger than the projectile bounds
	if ( ( ( ownerBounds[1][0] - ownerBounds[0][0] ) > ( projBounds[1][0] - projBounds[0][0] ) ) &&
		( ( ownerBounds[1][1] - ownerBounds[0][1] ) > ( projBounds[1][1] - projBounds[0][1] ) ) &&
		( ( ownerBounds[1][2] - ownerBounds[0][2] ) > ( projBounds[1][2] - projBounds[0][2] ) ) ) {
		if ( (ownerBounds - projBounds).RayIntersection( muzzle, viewAxis[ 0 ], distance ) ) {
			start = muzzle + distance * viewAxis[ 0 ];
		} else {
			start = ownerBounds.GetCenter();
		}
	} else {
		// projectile bounds bigger than the owner bounds, so just start it from the center
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation( tr, start, muzzle, projClip, axis, MASK_SHOT_RENDERMODEL, this );
	muzzle = tr.endpos;

	// set aiming direction
	GetAimDirByWeapon( muzzle, target, this, dir, currentWeapon);
	ang = dir.ToAngles();

	// adjust his aim so it's not perfect.  uses sine based movement so the tracers appear less random in their spread.
	float t = MS2SEC( gameLocal.time + entityNumber * 497 );
	ang.pitch += idMath::Sin16( t * 5.1 ) * attack_accuracy;
	ang.yaw	+= idMath::Sin16( t * 6.7 ) * attack_accuracy;

	if ( clampToAttackCone ) {
		// clamp the attack direction to be within monster's attack cone so he doesn't do
		// things like throw the missile backwards if you're behind him
		diff = idMath::AngleDelta( ang.yaw, current_yaw );
		if ( diff > attack_cone ) {
			ang.yaw = current_yaw + attack_cone;
		} else if ( diff < -attack_cone ) {
			ang.yaw = current_yaw - attack_cone;
		}
	}

	axis = ang.ToMat3();

	float spreadRad = DEG2RAD( projectile_spread );
	for( i = 0; i < num_projectiles; i++ ) {
		// spread the projectiles out
		angle = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
		spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
		dir = axis[ 0 ] + axis[ 2 ] * ( angle * idMath::Sin( spin ) ) - axis[ 1 ] * ( angle * idMath::Cos( spin ) );
		dir.Normalize();

		// launch the projectile
		if ( !projectile.GetEntity() ) {
			CreateProjectile( muzzle, dir );
		}
		lastProjectile = projectile.GetEntity();
		lastProjectile->Launch( muzzle, dir, vec3_origin );
		projectile = NULL;
	}

	//weapon fxs
	TriggerWeaponEffects( muzzle );

	//ammo
	if(weapons[ currentWeapon ].clipSize > 0){ //only if limited clip size
		weapons[ currentWeapon ].ammoInClip--;
		if(weapons[ currentWeapon ].ammoInClip <= 0){
			AI_WEAPON_NEED_RELOAD = true;
		}
	}

	lastAttackTime = gameLocal.time;

	return lastProjectile;
}


/*
===============================================================================

	Misc

===============================================================================	
*/

/*
=====================
idAI_Bot::GetAimDir
=====================
*/
bool idAI_Bot::GetAimDir( const idVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, idVec3 &aimDir ) const {
	return GetAimDirByWeapon( firePos, aimAtEnt, ignore, aimDir, currentWeapon);
}


/*
=====================
idAI_Bot::GetAimDirByWeapon
=====================
*/
bool idAI_Bot::GetAimDirByWeapon( const idVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, idVec3 &aimDir, int weaponToTest) const {
	idVec3	targetPos1;
	idVec3	targetPos2;
	idVec3	delta;
	float	max_height;
	bool	result;

	//if no weapon is selected
	if ( weaponToTest < 0 || weaponToTest >= weapons.Num()){ 
		gameLocal.Warning( "idAI_Bot::GetAimDirByWeapon: the weapon to test is out of range" );
		return false;
	}

	// if no aimAtEnt or projectile set
	if ( !aimAtEnt || !weapons[ weaponToTest ].projectileDef ) {
		aimDir = viewAxis[ 0 ] * physicsObj.GetGravityAxis();
		return false;
	}

	if ( weapons[ weaponToTest ].projectileClipModel == NULL ) {
		CreateProjectileClipModel( weaponToTest );
	}

	if ( aimAtEnt == enemy.GetEntity() ) {
		static_cast<idActor *>( aimAtEnt )->GetAIAimTargets( lastVisibleEnemyPos, targetPos1, targetPos2 );
	} else if ( aimAtEnt->IsType( idActor::Type ) ) {
		static_cast<idActor *>( aimAtEnt )->GetAIAimTargets( aimAtEnt->GetPhysics()->GetOrigin(), targetPos1, targetPos2 );
	} else {
		targetPos1 = aimAtEnt->GetPhysics()->GetAbsBounds().GetCenter();
		targetPos2 = targetPos1;
	}

	// try aiming for chest
	delta = firePos - targetPos2;
	max_height = delta.LengthFast() * projectile_height_to_distance_ratio;
	result = PredictTrajectory( firePos, targetPos2, weapons[ weaponToTest ].projectileSpeed, weapons[ weaponToTest ].projectileGravity, weapons[ weaponToTest ].projectileClipModel, MASK_SHOT_RENDERMODEL, max_height, ignore, aimAtEnt, ai_debugTrajectory.GetBool() ? 1000 : 0, aimDir );
	if ( result || !aimAtEnt->IsType( idActor::Type ) ) {
		return result;
	}

	// try aiming for head
	delta = firePos - targetPos1;
	max_height = delta.LengthFast() * projectile_height_to_distance_ratio;
	result = PredictTrajectory( firePos, targetPos1, weapons[ weaponToTest ].projectileSpeed, weapons[ weaponToTest ].projectileGravity, weapons[ weaponToTest ].projectileClipModel, MASK_SHOT_RENDERMODEL, max_height, ignore, aimAtEnt, ai_debugTrajectory.GetBool() ? 1000 : 0, aimDir );

	return result;
}

/*
================
idAI_Bot::GetMuzzle
================
*/
void idAI_Bot::GetMuzzle( const char *jointname, idVec3 &muzzle, idMat3 &axis ) { 
	jointHandle_t joint;

	if ( !jointname || !jointname[ 0 ] ) { //new behaviour: is nothing is passed, use the current weapon settings
		if(currentWeapon >= 0){
			//WEAPON CASE 1: weapon fireJoint!
			if(weapons[ currentWeapon ].fireJoint != INVALID_JOINT){ //if fireJoint is valid, we are sure the entity has an MD5 model
				idEntity *myWeaponEnt;
				idAnimator *myWeaponAnimator;

				//get the reference to the weapon and its animator
				myWeaponEnt = weapons[ currentWeapon ].ent.GetEntity();
				myWeaponAnimator = myWeaponEnt->GetAnimator(); 

				//use the fireJoint!
				joint = weapons[ currentWeapon ].fireJoint; //myWeaponAnimator->GetJointHandle( "barrel" ); //

				//do the equivalent of GetJointWorldTransform even without being casted to idAnimatedEntity
				myWeaponAnimator->GetJointTransform( joint, gameLocal.time, muzzle, axis );
				myWeaponEnt->ConvertLocalToWorldTransform( muzzle, axis );
				
				//gameLocal.Printf("GetMuzzle che usa fireJoint\n");
			}
			//WEAPON CASE 2: my bindJoint !
			else{ 
				joint = weapons[ currentWeapon ].bindJoint; //bindJoint is always valid
				GetJointWorldTransform( joint, gameLocal.time, muzzle, axis );
				//gameLocal.Printf("GetMuzzle che usa bindJoint\n");
			}
		//NO-WEAPON CASE 1: my origin
		}else{ 
			muzzle = physicsObj.GetOrigin() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 14;
			muzzle -= physicsObj.GetGravityNormal() * physicsObj.GetBounds()[ 1 ].z * 0.5f;
		}
	//NO-WEAPON CASE 2: my specified joint
	} else {
		joint = animator.GetJointHandle( jointname );
		if ( joint == INVALID_JOINT ) {
			gameLocal.Error( "Unknown joint '%s' on %s", jointname, GetEntityDefName() );
		}
		GetJointWorldTransform( joint, gameLocal.time, muzzle, axis );
	}
}


/*
================
idAI_Bot::Show
================
*/
void idAI_Bot::Show( void ) {
	idAI::Show();
	ShowOnlyCurrentWeapon();
}


/*
===============================================================================

	Weapon events

===============================================================================
*/


/*
================
idAI_Bot::Event_SelectWeapon
================
*/
void idAI_Bot::Event_SelectWeapon( int weaponNum ){
	//gameLocal.Printf("Event_SelectWeapon requested: %d \n" ,weaponNum);
	idThread::ReturnInt( SelectWeapon( weaponNum ) );
}

/*
================
idAI_Bot::Event_SelectAnotherWeapon
================
*/
void idAI_Bot::Event_SelectAnotherWeapon( int idealLevel, int minLevel ){ 
	int testWeapon;
	int nextWeapon;
	int wCounter;
	int wNum;

	wNum = weapons.Num();

	//ideal loop
	testWeapon = currentWeapon;
	nextWeapon = currentWeapon;

	for( wCounter = 0; wCounter < wNum; wCounter++ ) { //do exactly a complete loop (also test the current weapon so that it's ok even if we start from -1)
		
		//find the next one
		testWeapon++;
		if(testWeapon >= wNum){testWeapon = 0;}

		//test it
		if ( (weapons[ testWeapon ].enabled) && (weapons[ testWeapon ].priorityLevel >= idealLevel) ) {
			nextWeapon = testWeapon;
			break;
		}
	}

	if(nextWeapon != currentWeapon){ //found!
		idThread::ReturnInt( SelectWeapon( nextWeapon ) ); //it's enabled and on range, so no way this could be false.
		return;
	}

	//minlevel loop
	testWeapon = currentWeapon;
	nextWeapon = currentWeapon;

	for( wCounter = 0; wCounter < wNum; wCounter++ ) { //do exactly a complete loop (also test the current weapon so that it's ok even if we start from -1)
		
		//find the next one
		testWeapon++;
		if(testWeapon >= wNum){testWeapon = 0;}

		//test it
		if ( (weapons[ testWeapon ].enabled) && (weapons[ testWeapon ].priorityLevel >= minLevel) ) {
			nextWeapon = testWeapon;
			break;
		}
	}

	if(nextWeapon != currentWeapon){ //found!
		idThread::ReturnInt( SelectWeapon( nextWeapon ) ); //it's enabled and on range, so no way this can be false.
		return;
	}

	//nothing can satisfy the conditions
	idThread::ReturnInt( false );

}

/*
================
idAI_Bot::Event_ReloadWeapon
================
*/
void idAI_Bot::Event_ReloadWeapon( void ){
	AI_WEAPON_NEED_RELOAD = false;
	if( currentWeapon < 0) return;
	weapons[ currentWeapon ].ammoInClip = weapons[ currentWeapon ].clipSize;
}

/*
================
idAI_Bot::Event_CheckReloadTolerance
================
*/
void idAI_Bot::Event_CheckReloadTolerance( void ){ //returns true if we should reload
	if( currentWeapon < 0){ //no weapon selected
		idThread::ReturnInt( false );
		return;
	}

	if( weapons[ currentWeapon ].clipSize == 0){ //unlimited clip
		idThread::ReturnInt( false );
		return;
	}

	if(	weapons[ currentWeapon ].ammoInClip > weapons[ currentWeapon ].reloadTolerance ){ //still enough ammo
		idThread::ReturnInt( false );
		return;
	}
	
	//reloading is a good idea at this point...
	idThread::ReturnInt( true );
	return;
}


/*
================
idAI_Bot::Event_GetAmmoInClip
================
*/
void idAI_Bot::Event_GetAmmoInClip( void ){
	if( currentWeapon < 0){
		idThread::ReturnInt( 0 );
		return;
	}

	idThread::ReturnInt( weapons[ currentWeapon ].ammoInClip );
}

/*
================
idAI_Bot::Event_GetClipSize
================
*/
void idAI_Bot::Event_GetClipSize( void ){
	if( currentWeapon < 0){
		idThread::ReturnInt( 0 );
		return;
	}

	idThread::ReturnInt( weapons[ currentWeapon ].clipSize );
}


/*
================
idAI_Bot::Event_GetCurrentWeapon
================
*/
void idAI_Bot::Event_GetCurrentWeapon( void ){
	idThread::ReturnInt( currentWeapon );
}

/*
================
idAI_Bot::Event_GetWeaponNumByName
================
*/
void idAI_Bot::Event_GetWeaponNumByName( const char *weaponName ){
	int weaponNum;
	weaponNum = GetWeaponNumByName( idStr( weaponName ) );
	idThread::ReturnInt( weaponNum );
}

/*
===============================================================================

	Anims events

===============================================================================
*/

/*
=====================
idAI_Bot::Event_GetIdleAnim
=====================
*/

void idAI_Bot::Event_GetIdleAnim( void ) {
	const char *animname;
	if(currentWeapon < 0){
		animname = "idle";
	}else{
		animname = va( "%s_idle", weapons[ currentWeapon ].weaponName.c_str() ); 
	}
	idThread::ReturnString( animname );
}

/*
=====================
idAI_Bot::Event_GetReloadAnim
=====================
*/

void idAI_Bot::Event_GetReloadAnim( void ) {
	const char *animname;
	if(currentWeapon < 0){
		animname = "idle"; //in case someone tries to reload but no weapon is selected :)
	}else{
		animname = va( "%s_reload", weapons[ currentWeapon ].weaponName.c_str() ); 
	}
	idThread::ReturnString( animname );
}

/*
=====================
idAI_Bot::Event_GetReloadAnim
=====================
*/

void idAI_Bot::Event_GetFireAnim( void ) {
	const char *animname;
	if(currentWeapon < 0){
		animname = "idle"; //in case someone tries to fire but no weapon is selected :)
	}else{
		animname = va( "%s_fire", weapons[ currentWeapon ].weaponName.c_str() ); 
	}
	idThread::ReturnString( animname );
}

/*
===============================================================================

	Fire events

===============================================================================
*/

/*
=====================
idAI_Bot::Event_FireWeapon
=====================
*/
void idAI_Bot::Event_FireWeapon( void ) {
	idProjectile *proj;

	proj = LaunchProjectile( "", enemy.GetEntity(), true ); //"" will force the use of weapon specific settings
	idThread::ReturnEntity( proj );
}

/*
=====================
idAI_Bot::Event_CreateMissile
=====================
*/
void idAI_Bot::Event_CreateMissile( const char *jointname ) {
	idVec3 muzzle;
	idMat3 axis;

	//gameLocal.Printf("idAI_Bot::Event_CreateMissile \n");

	//ivan start
	if ( currentWeapon < 0){
		gameLocal.Warning( "idAI_Bot::Event_CreateMissile: %s (%s) is trying to fire but doesn't have a weapon selected", name.c_str(), GetEntityDefName() );
		return idThread::ReturnEntity( NULL );
	}

	if ( !weapons[ currentWeapon ].projectileDef ) {
		gameLocal.Warning( "idAI_Bot::Event_CreateMissile: %s (%s) : the current weapon doesn't have a projectile specified", name.c_str(), GetEntityDefName() );
		return idThread::ReturnEntity( NULL );
	}
	//ivan end

	GetMuzzle( jointname, muzzle, axis );
	CreateProjectile( muzzle, viewAxis[ 0 ] * physicsObj.GetGravityAxis() );
	if ( projectile.GetEntity() ) {
		if ( !jointname || !jointname[ 0 ] ) {
			projectile.GetEntity()->Bind( this, true );
		} else {
			projectile.GetEntity()->BindToJoint( this, jointname, true );
		}
	}
	idThread::ReturnEntity( projectile.GetEntity() );
}


/*
=====================
idAI_Bot::Event_LaunchMissile
=====================
*/
void idAI_Bot::Event_LaunchMissile( const idVec3 &org, const idAngles &ang ) {
	idVec3		start;
	trace_t		tr;
	idBounds	projBounds;
	const idClipModel *projClip;
	idMat3		axis;
	float		distance;

	//gameLocal.Printf("idAI_Bot::Event_LaunchMissile \n");

	//ivan start
	if ( currentWeapon < 0){
		gameLocal.Warning( "idAI_Bot::Event_LaunchMissile: %s (%s) is trying to fire but doesn't have a weapon selected", name.c_str(), GetEntityDefName() );
		idThread::ReturnEntity( NULL );
		return;
	}

	if ( !weapons[ currentWeapon ].projectileDef ) {
		gameLocal.Warning( "idAI_Bot::Event_LaunchMissile: %s (%s) : the current weapon doesn't have a projectile specified", name.c_str(), GetEntityDefName() );
		idThread::ReturnEntity( NULL );
		return;
	}
	//ivan end

	axis = ang.ToMat3();
	if ( !projectile.GetEntity() ) {
		CreateProjectile( org, axis[ 0 ] );
	}

	// make sure the projectile starts inside the monster bounding box
	const idBounds &ownerBounds = physicsObj.GetAbsBounds();
	projClip = projectile.GetEntity()->GetPhysics()->GetClipModel();
	projBounds = projClip->GetBounds().Rotate( projClip->GetAxis() );

	// check if the owner bounds is bigger than the projectile bounds
	if ( ( ( ownerBounds[1][0] - ownerBounds[0][0] ) > ( projBounds[1][0] - projBounds[0][0] ) ) &&
		( ( ownerBounds[1][1] - ownerBounds[0][1] ) > ( projBounds[1][1] - projBounds[0][1] ) ) &&
		( ( ownerBounds[1][2] - ownerBounds[0][2] ) > ( projBounds[1][2] - projBounds[0][2] ) ) ) {
		if ( (ownerBounds - projBounds).RayIntersection( org, viewAxis[ 0 ], distance ) ) {
			start = org + distance * viewAxis[ 0 ];
		} else {
			start = ownerBounds.GetCenter();
		}
	} else {
		// projectile bounds bigger than the owner bounds, so just start it from the center
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation( tr, start, org, projClip, projClip->GetAxis(), MASK_SHOT_RENDERMODEL, this );

	// launch the projectile
	idThread::ReturnEntity( projectile.GetEntity() );
	projectile.GetEntity()->Launch( tr.endpos, axis[ 0 ], vec3_origin );
	projectile = NULL;

	TriggerWeaponEffects( tr.endpos );

	lastAttackTime = gameLocal.time;
}
 
/*
===============================================================================

	CanHit events

===============================================================================
*/

/*
=====================
idAI_Bot::Event_CanHitEnemyFromCurrentWeapon
=====================
*/
void idAI_Bot::Event_CanHitEnemyFromCurrentWeapon( void ) {
	trace_t	tr;
	idVec3	muzzle;
	idMat3	axis;
	idVec3	start;
	//float	distance;

	idActor *enemyEnt = enemy.GetEntity();
	if ( !AI_ENEMY_VISIBLE || !enemyEnt ) {
		idThread::ReturnInt( false );
		return;
	}

	//ivan start
	if( currentWeapon < 0 ){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromCurrentWeapon: no weapon selected!");
		idThread::ReturnInt( false );
		return;
	}

	if( !weapons[ currentWeapon ].projectileDef ){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromCurrentWeapon: no projectile defined!");
		idThread::ReturnInt( false );
		return;
	}
	//ivan end

	
	// don't check twice per frame - makes sense because no parameters can be passed
	if ( gameLocal.time == lastHitFCWCheckTime ) {
		idThread::ReturnInt( lastHitFCWCheckResult );
		return;
	}

	lastHitFCWCheckTime = gameLocal.time;

	//get muzzle and destination positions
	GetMuzzle( "", muzzle, axis ); //"" will force use the weapon fire position
	idVec3 toPos = enemyEnt->GetEyePosition();
	
	if ( weapons[ currentWeapon ].projectileClipModel == NULL ) {
		CreateProjectileClipModel();
	}

	gameLocal.clip.Translation( tr, muzzle, toPos, weapons[ currentWeapon ].projectileClipModel, mat3_identity, MASK_SHOT_BOUNDINGBOX, this );
	if ( tr.fraction >= 1.0f || ( gameLocal.GetTraceEntity( tr ) == enemyEnt ) ) {
		//gameLocal.Printf("OK FromCurrentWeapon!\n");
		lastHitFCWCheckResult = true;
	} else {
		//gameLocal.Printf("NO FromCurrentWeapon!\n");
		lastHitFCWCheckResult = false;
	}

	idThread::ReturnInt( lastHitFCWCheckResult );
}

/*
=====================
idAI_Bot::Event_CanFireToEnemyNoSelfDamage
=====================
*/
void idAI_Bot::Event_CanFireToEnemyNoSelfDamage( int requireVisible ) {
	trace_t	tr;
	idVec3	muzzle;
	idMat3	axis;
	idVec3	start;
	//float	distance;

	idActor *enemyEnt = enemy.GetEntity();
	if ( (!AI_ENEMY_VISIBLE && requireVisible != 0 )  || !enemyEnt ) {
		idThread::ReturnInt( false );
		return;
	}

	//ivan start
	if( currentWeapon < 0 ){
		gameLocal.Warning("idAI_Bot::Event_CanFireToEnemyNoSelfDamage: no weapon selected!");
		idThread::ReturnInt( false );
		return;
	}

	if( !weapons[ currentWeapon ].projectileDef ){
		gameLocal.Warning("idAI_Bot::Event_CanFireToEnemyNoSelfDamage: no projectile defined!");
		idThread::ReturnInt( false );
		return;
	}
	//ivan end

	
	// don't check twice per frame - makes sense because if (requireVisible = true && !AI_ENEMY_VISIBLE) the timer is not updated, so in any case if we are here we are doing the same thing again  
	if ( gameLocal.time == lastHitNSDCheckTime ) {
		idThread::ReturnInt( lastHitNSDCheckResult );
		return;
	}
	lastHitNSDCheckTime = gameLocal.time;

	//get muzzle and destination positions
	GetMuzzle( "", muzzle, axis ); //"" will force use the weapon fire position
	idVec3 toPos = enemyEnt->GetEyePosition();

	if ( weapons[ currentWeapon ].projectileClipModel == NULL ) {
		CreateProjectileClipModel();
	}

	gameLocal.clip.Translation( tr, muzzle, toPos, weapons[ currentWeapon ].projectileClipModel, mat3_identity, MASK_SHOT_BOUNDINGBOX, this );
	
	float dist = ( muzzle - tr.endpos ).LengthFast();
	
	if ( tr.fraction >= 1.0f || ( gameLocal.GetTraceEntity( tr ) == enemyEnt ) ) {
		//gameLocal.Printf("OK NoSelfDamage!\n");
		lastHitNSDCheckResult = true;
	} else if (dist > weapons[ currentWeapon ].minSelfDmgDistance ){ //60.0f
		//gameLocal.Printf("OK NoSelfDamage perche distante %f!\n", dist);
		lastHitNSDCheckResult = true;
	} else {
		//gameLocal.Printf("NO NoSelfDamage!\n");
		lastHitNSDCheckResult = false;
	}

	idThread::ReturnInt( lastHitNSDCheckResult );
}


/*
=====================
idAI_Bot::Event_CanFireNoSelfDamageFromAnim
=====================

void idAI_Bot::Event_CanFireNoSelfDamageFromAnim( const char *animname, int requireVisible ) {
	
}
*/

/*
=====================
idAI_Bot::Event_CanHitEnemyFromFireAnim
=====================
*/
void idAI_Bot::Event_CanHitEnemyFromFireAnim( int weaponToTest ) { //note: it uses the proj of the requested weapon
	int		anim;
	idVec3	dir;
	idVec3	local_dir;
	idVec3	fromPos;
	idMat3	axis;
	idVec3	start;
	trace_t	tr;
	float	distance;
	const char *animname; 

	//gameLocal.Printf("idAI_Bot::Event_CanHitEnemyFromFireAnim called! \n" );

	idActor *enemyEnt = enemy.GetEntity();
	if ( !AI_ENEMY_VISIBLE || !enemyEnt ) {
		idThread::ReturnInt( false );
		return;
	}	

	//ivan start
	if(weaponToTest < 0 || weaponToTest >= weapons.Num()){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromFireAnim: requested weapon is out of range!");
		idThread::ReturnInt( false );
		return;
	}

	if( !weapons[ weaponToTest ].projectileDef ){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromFireAnim: requested weapon has no projectile defined!");
		idThread::ReturnInt( false );
		return;
	}
	//ivan end

	animname = va( "%s_fire", weapons[ weaponToTest ].weaponName.c_str() ); 
	anim = GetAnim( ANIMCHANNEL_LEGS, animname );
	if ( !anim ) {
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromFireAnim: requested weapon has no fire anim defined!");
		idThread::ReturnInt( false );
		return;
	}

	// just do a ray test if close enough
	if ( enemyEnt->GetPhysics()->GetAbsBounds().IntersectsBounds( physicsObj.GetAbsBounds().Expand( 16.0f ) ) ) {
		Event_CanHitEnemy();
		return;
	}

	// calculate the world transform of the launch position
	
	const idVec3 &org = physicsObj.GetOrigin();
	dir = lastVisibleEnemyPos - org;
	physicsObj.GetGravityAxis().ProjectVector( dir, local_dir );
	local_dir.z = 0.0f;
	local_dir.ToVec2().Normalize();
	axis = local_dir.ToMat3();
	fromPos = physicsObj.GetOrigin() + missileLaunchOffset[ anim ] * axis;
	
	
	//ivan:
	//fromPos = physicsObj.GetOrigin() + missileLaunchOffset[ anim ] * viewAxis * physicsObj.GetGravityAxis();


	if ( weapons[ weaponToTest ].projectileClipModel == NULL ) { //TODO: use the requested one!!!
		CreateProjectileClipModel(weaponToTest);
	}

	// check if the owner bounds is bigger than the projectile bounds
	const idBounds &ownerBounds = physicsObj.GetAbsBounds();
	const idBounds &projBounds = weapons[ weaponToTest ].projectileClipModel->GetBounds();
	if ( ( ( ownerBounds[1][0] - ownerBounds[0][0] ) > ( projBounds[1][0] - projBounds[0][0] ) ) &&
		( ( ownerBounds[1][1] - ownerBounds[0][1] ) > ( projBounds[1][1] - projBounds[0][1] ) ) &&
		( ( ownerBounds[1][2] - ownerBounds[0][2] ) > ( projBounds[1][2] - projBounds[0][2] ) ) ) {
		if ( (ownerBounds - projBounds).RayIntersection( org, viewAxis[ 0 ], distance ) ) {
			start = org + distance * viewAxis[ 0 ];
		} else {
			start = ownerBounds.GetCenter();
		}
	} else {
		// projectile bounds bigger than the owner bounds, so just start it from the center
		start = ownerBounds.GetCenter();
	}	

	gameLocal.clip.Translation( tr, start, fromPos, weapons[ weaponToTest ].projectileClipModel, mat3_identity, MASK_SHOT_RENDERMODEL, this );
	fromPos = tr.endpos;

	if ( GetAimDirByWeapon( fromPos, enemy.GetEntity(), this, dir, weaponToTest) ) {
		//gameLocal.Printf("idAI_Bot::Event_CanHitEnemyFromAnim true!\n");
		idThread::ReturnInt( true );
	} else {
		//gameLocal.Printf("idAI_Bot::Event_CanHitEnemyFromAnim false!\n");
		idThread::ReturnInt( false );
	}
}


/*
=====================
idAI_Bot::Event_CanHitEnemyFromAnim
=====================
*/
void idAI_Bot::Event_CanHitEnemyFromAnim( const char *animname ) { //deprecated: it uses the current projectile!
	int		anim;
	idVec3	dir;
	idVec3	local_dir;
	idVec3	fromPos;
	idMat3	axis;
	idVec3	start;
	trace_t	tr;
	float	distance;

	gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromAnim is deprecated: it uses the current projectile!");

	idActor *enemyEnt = enemy.GetEntity();
	if ( !AI_ENEMY_VISIBLE || !enemyEnt ) {
		idThread::ReturnInt( false );
		return;
	}

	//ivan start
	if( currentWeapon < 0 ){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromAnim: no weapon selected!");
		idThread::ReturnInt( false );
		return;
	}

	if( !weapons[ currentWeapon ].projectileDef ){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromAnim: no projectile defined!");
		idThread::ReturnInt( false );
		return;
	}
	//ivan end

	anim = GetAnim( ANIMCHANNEL_LEGS, animname );
	if ( !anim ) {
		idThread::ReturnInt( false );
		return;
	}

	// just do a ray test if close enough
	if ( enemyEnt->GetPhysics()->GetAbsBounds().IntersectsBounds( physicsObj.GetAbsBounds().Expand( 16.0f ) ) ) {
		Event_CanHitEnemy();
		return;
	}

	// calculate the world transform of the launch position
	const idVec3 &org = physicsObj.GetOrigin();
	dir = lastVisibleEnemyPos - org;
	physicsObj.GetGravityAxis().ProjectVector( dir, local_dir );
	local_dir.z = 0.0f;
	local_dir.ToVec2().Normalize();
	axis = local_dir.ToMat3();
	fromPos = physicsObj.GetOrigin() + missileLaunchOffset[ anim ] * axis;

	if ( weapons[ currentWeapon ].projectileClipModel == NULL ) {
		CreateProjectileClipModel();
	}

	// check if the owner bounds is bigger than the projectile bounds
	const idBounds &ownerBounds = physicsObj.GetAbsBounds();
	const idBounds &projBounds = weapons[ currentWeapon ].projectileClipModel->GetBounds();
	if ( ( ( ownerBounds[1][0] - ownerBounds[0][0] ) > ( projBounds[1][0] - projBounds[0][0] ) ) &&
		( ( ownerBounds[1][1] - ownerBounds[0][1] ) > ( projBounds[1][1] - projBounds[0][1] ) ) &&
		( ( ownerBounds[1][2] - ownerBounds[0][2] ) > ( projBounds[1][2] - projBounds[0][2] ) ) ) {
		if ( (ownerBounds - projBounds).RayIntersection( org, viewAxis[ 0 ], distance ) ) {
			start = org + distance * viewAxis[ 0 ];
		} else {
			start = ownerBounds.GetCenter();
		}
	} else {
		// projectile bounds bigger than the owner bounds, so just start it from the center
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation( tr, start, fromPos, weapons[ currentWeapon ].projectileClipModel, mat3_identity, MASK_SHOT_RENDERMODEL, this );
	fromPos = tr.endpos;

	if ( GetAimDirByWeapon( fromPos, enemy.GetEntity(), this, dir, currentWeapon ) ) {
		idThread::ReturnInt( true );
	} else {
		idThread::ReturnInt( false );
	}
}


/*
=====================
idAI_Bot::Event_CanHitEnemyFromJoint
=====================
*/
void idAI_Bot::Event_CanHitEnemyFromJoint( const char *jointname ) { //deprecated: it uses the current projectile!
	trace_t	tr;
	idVec3	muzzle;
	idMat3	axis;
	idVec3	start;
	float	distance;

	gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromJoint is deprecated: it uses the current projectile!");

	idActor *enemyEnt = enemy.GetEntity();
	if ( !AI_ENEMY_VISIBLE || !enemyEnt ) {
		idThread::ReturnInt( false );
		return;
	}

	//ivan start
	if( currentWeapon < 0 ){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromJoint: no weapon selected!");
		idThread::ReturnInt( false );
		return;
	}

	if( !weapons[ currentWeapon ].projectileDef ){
		gameLocal.Warning("idAI_Bot::Event_CanHitEnemyFromJoint: no projectile defined!");
		idThread::ReturnInt( false );
		return;
	}
	//ivan end

	// don't check twice per frame - we cannot check more than 1 joint per frame!
	if ( gameLocal.time == lastHitCheckTime ) {
		idThread::ReturnInt( lastHitCheckResult );
		return;
	}

	lastHitCheckTime = gameLocal.time;

	const idVec3 &org = physicsObj.GetOrigin();
	idVec3 toPos = enemyEnt->GetEyePosition();
	jointHandle_t joint = animator.GetJointHandle( jointname );
	if ( joint == INVALID_JOINT ) {
		gameLocal.Error( "Unknown joint '%s' on %s", jointname, GetEntityDefName() );
	}
	animator.GetJointTransform( joint, gameLocal.time, muzzle, axis );
	muzzle = org + ( muzzle + modelOffset ) * viewAxis * physicsObj.GetGravityAxis();

	if ( weapons[ currentWeapon ].projectileClipModel == NULL ) {
		CreateProjectileClipModel();
	}

	// check if the owner bounds is bigger than the projectile bounds
	const idBounds &ownerBounds = physicsObj.GetAbsBounds();
	const idBounds &projBounds = weapons[ currentWeapon ].projectileClipModel->GetBounds();
	if ( ( ( ownerBounds[1][0] - ownerBounds[0][0] ) > ( projBounds[1][0] - projBounds[0][0] ) ) &&
		( ( ownerBounds[1][1] - ownerBounds[0][1] ) > ( projBounds[1][1] - projBounds[0][1] ) ) &&
		( ( ownerBounds[1][2] - ownerBounds[0][2] ) > ( projBounds[1][2] - projBounds[0][2] ) ) ) {
		if ( (ownerBounds - projBounds).RayIntersection( org, viewAxis[ 0 ], distance ) ) {
			start = org + distance * viewAxis[ 0 ];
		} else {
			start = ownerBounds.GetCenter();
		}
	} else {
		// projectile bounds bigger than the owner bounds, so just start it from the center
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation( tr, start, muzzle, weapons[ currentWeapon ].projectileClipModel, mat3_identity, MASK_SHOT_BOUNDINGBOX, this );
	muzzle = tr.endpos;

	gameLocal.clip.Translation( tr, muzzle, toPos, weapons[ currentWeapon ].projectileClipModel, mat3_identity, MASK_SHOT_BOUNDINGBOX, this );
	if ( tr.fraction >= 1.0f || ( gameLocal.GetTraceEntity( tr ) == enemyEnt ) ) {
		lastHitCheckResult = true;
	} else {
		lastHitCheckResult = false;
	}

	idThread::ReturnInt( lastHitCheckResult );
}

/*
=====================
idAI_Bot::Event_FindEnemyAI
=====================
*/
void idAI_Bot::Event_FindEnemyAIorPL( int useFOV ) {
	idEntity	*ent;
	idActor		*actor;
	idActor		*bestEnemy;
	float		bestDist;
	float		dist;
	idVec3		delta;
	pvsHandle_t pvs;
	int			i;

	pvs = gameLocal.pvs.SetupCurrentPVS( GetPVSAreas(), GetNumPVSAreas() );

	bestDist = idMath::INFINITY;
	bestEnemy = NULL;
	for ( ent = gameLocal.activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
		if ( ent->fl.hidden || ent->fl.isDormant || !ent->IsType( idActor::Type ) ) {
			continue;
		}

		actor = static_cast<idActor *>( ent );
		if ( ( actor->health <= 0 ) || !( ReactionTo( actor ) & ATTACK_ON_SIGHT ) ) {
			continue;
		}

		if ( !gameLocal.pvs.InCurrentPVS( pvs, actor->GetPVSAreas(), actor->GetNumPVSAreas() ) ) {
			continue;
		}

		delta = physicsObj.GetOrigin() - actor->GetPhysics()->GetOrigin();
		dist = delta.LengthSqr();
		if ( ( dist < bestDist ) && CanSee( actor, useFOV != 0 ) ) {
			bestDist = dist;
			bestEnemy = actor;
		}
	}

	gameLocal.pvs.FreeCurrentPVS( pvs );

	if(bestEnemy != NULL){
		idThread::ReturnEntity( bestEnemy );
		return;
	}

	//else player!

	if ( gameLocal.InPlayerPVS( this ) ) {
		for ( i = 0; i < gameLocal.numClients ; i++ ) {
			ent = gameLocal.entities[ i ];

			if ( !ent || !ent->IsType( idActor::Type ) ) {
				continue;
			}

			actor = static_cast<idActor *>( ent );
			if ( ( actor->health <= 0 ) || !( ReactionTo( actor ) & ATTACK_ON_SIGHT ) ) {
				continue;
			}

			if ( CanSee( actor, useFOV != 0 ) ) {
				idThread::ReturnEntity( actor );
				return;
			}
		}
	}

	idThread::ReturnEntity( NULL );
}

/*
=====================
idAI::Event_Burn
=====================
*/
void idAI_Bot::Event_Burn( void ) {
	idAI::Event_Burn();
	idEntity *myWeaponEnt = weapons[ currentWeapon ].ent.GetEntity();
	if(myWeaponEnt){
		myWeaponEnt->GetRenderEntity()->shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f;
	}
}

/*
===============================================================================

	Misc

===============================================================================
*/

/*
=====================
idAI_Bot::GetEnemyRange
=====================

float idAI_Bot::GetEnemyRange( void ) {
	float dist;
	idActor *enemyEnt = enemy.GetEntity();

	if ( enemyEnt ) {
		dist = ( enemyEnt->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin() ).Length();
	} else {
		// Just some really high number
		dist = idMath::INFINITY;
	}

	return dist;
}
*/

/*
=====================
idAI_Bot::SetEnemyPosition
=====================
*/
void idAI_Bot::SetEnemyPosition( void ) {
	idAI::SetEnemyPosition();
	lastVisibleEnemyTime = gameLocal.time;
}

/*
=====================
idAI_Bot::Event_LostTimeMoreThan
=====================
*/
void idAI_Bot::Event_LostTimeMoreThan( int time ) {
	if(lastVisibleEnemyTime + SEC2MS( time ) < gameLocal.time){
		//gameLocal.Printf("Event_LostTimeMoreThan true !\n" );
		idThread::ReturnInt( true ); 
	}else{
		//gameLocal.Printf("Event_LostTimeMoreThan false !\n" );
		idThread::ReturnInt( false ); 
	}
}

/*
=====================
idAI_Bot::Event_WeaponChangedMoreThan
=====================
*/
void idAI_Bot::Event_WeaponChangedMoreThan( int time ) {
	if(lastWeaponChangedTime + SEC2MS( time ) < gameLocal.time){
		//gameLocal.Printf("Event_WeaponChangedMoreThan true !\n" );
		idThread::ReturnInt( true ); 
	}else{
		//gameLocal.Printf("Event_WeaponChangedMoreThan false !\n" );
		idThread::ReturnInt( false ); 
	}
}

/*
=====================
idAI_Bot::Event_PlayAnimOnWeapon
=====================
*/
void idAI_Bot::Event_PlayAnimOnWeapon( const char *animname ) {
	if(currentWeapon < 0 ){
		return;
	}
	
	int anim;
	idEntity *myWeaponEnt;
	idAnimator *myWeaponAnimator;
	
	//get the reference to the weapon and its animator
	myWeaponEnt = weapons[ currentWeapon ].ent.GetEntity();	
	myWeaponAnimator = myWeaponEnt->GetAnimator(); 

	// only if the entity has an MD5 model
	if ( myWeaponAnimator && myWeaponAnimator->ModelHandle() ) { 
		anim = myWeaponAnimator->GetAnim( animname );
		if ( !anim ) {
			gameLocal.Warning( "missing '%s' animation on current weapon!", animname );
			myWeaponAnimator->Clear( 0, gameLocal.time, 0 );
		} else {
			myWeaponAnimator->PlayAnim( 0, anim, gameLocal.time, 0 );
		}
	}
}



/*
=====================
idAI_Bot::idAI_Bot
=====================
*/

void idAI_Bot::Event_ReleaseNode( void ) {
	if ( currentNode.GetEntity() ) {
		currentNode.GetEntity()->Release();
		currentNode = NULL;
	}
}


/*
=====================
idAI_Bot::Event_TryLockNode
=====================
*/

void idAI_Bot::Event_TryLockNode( idEntity *node ) {
	bool resultOK;

	if( !node || !node->IsType( idBotNode::Type )){
		gameLocal.Warning("trylock failed: the entity is not an idBotNode!" );
		idThread::ReturnInt( false ); return;
	}

	//cast it
	idBotNode * castedNode;
	castedNode = static_cast<idBotNode *>( node );

	//try
	resultOK = castedNode->TryLock(this);

	if(resultOK){
		//release the current node if test ok
		if( currentNode.GetEntity()){
			currentNode.GetEntity()->Release(); 
		}

		//upd the current node
		currentNode = castedNode;
	}

	idThread::ReturnInt( resultOK );

}



/***********************************************************************

idBotNode

***********************************************************************/

const idEventDef EV_BotNode_EvaluateConditions( "evaluateConditions", NULL, 'd' );
const idEventDef EV_BotNode_GetNextUsableNode( "getNextUsableNode", "e", 'e' );
const idEventDef EV_BotNode_CanBeUsedBy( "canBeUsedBy", "e", 'd' );
const idEventDef EV_BotNode_Reached( "reached" );

CLASS_DECLARATION( idEntity, idBotNode )
	EVENT( EV_BotNode_EvaluateConditions,		idBotNode::Event_EvaluateConditions )
	EVENT( EV_BotNode_GetNextUsableNode,		idBotNode::Event_GetNextUsableNode ) 
	EVENT( EV_BotNode_CanBeUsedBy,				idBotNode::Event_CanBeUsedBy ) 
	EVENT( EV_BotNode_Reached,					idBotNode::Event_Reached )
END_CLASS

/*
=====================
idBotNode::idBotNode
=====================
*/
idBotNode::idBotNode( void ) {
	fight_distance_atnode = 0.0f;
	end_distance_min = 0.0f; 
	end_distance_max = 0.0f;
	node_distance_max = 0.0f;
	end_time = 0;
	end_time_max = 0;
	end_time_min = 0;
	end_health_delta_atnode = 0;
	fight_health_delta_reaching = 0;
	intial_health = 0;
	disabled = false;
	inuse = false;
	reached = false;
	ownerBot = NULL;
}


/*
=====================
idBotNode::Save
=====================
*/
void idBotNode::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( fight_distance_atnode );
	savefile->WriteFloat( end_distance_min );
	savefile->WriteFloat( end_distance_max );
	savefile->WriteFloat( node_distance_max ); 

	savefile->WriteInt( end_time_max );
	savefile->WriteInt( end_time_min );
	savefile->WriteInt( end_time );

	savefile->WriteInt( end_health_delta_atnode );
	savefile->WriteInt( fight_health_delta_reaching ); 
	savefile->WriteInt( intial_health );

	savefile->WriteBool( disabled );
	savefile->WriteBool( inuse );
	savefile->WriteBool( reached );

	ownerBot.Save( savefile );
}

/*
=====================
idBotNode::Restore
=====================
*/
void idBotNode::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( fight_distance_atnode );
	savefile->ReadFloat( end_distance_min );
	savefile->ReadFloat( end_distance_max );
	savefile->ReadFloat( node_distance_max ); 

	savefile->ReadInt( end_time_max );
	savefile->ReadInt( end_time_min );
	savefile->ReadInt( end_time );

	savefile->ReadInt( end_health_delta_atnode );
	savefile->ReadInt( fight_health_delta_reaching ); 
	savefile->ReadInt( intial_health );

	savefile->ReadBool( disabled );
	savefile->ReadBool( inuse );
	savefile->ReadBool( reached );

	ownerBot.Restore( savefile );
}


/*
=====================
idBotNode::Spawn
=====================
*/
void idBotNode::Spawn( void ) {
	fight_distance_atnode = spawnArgs.GetFloat( "fight_distance_atnode" ,"0");
	end_distance_min = spawnArgs.GetFloat( "end_distance_min","0" );
	end_distance_max = spawnArgs.GetFloat( "end_distance_max","0" );
	if( end_distance_min > 0 && (end_distance_min <= fight_distance_atnode)){
		gameLocal.Warning("idBotNode '%s' Spawn: end_distance_min <= fight_distance_atnode! --> end_distance_min will never be used", GetName());
		gameLocal.Warning("FIX: end_distance_min changed to fight_distance_atnode*2");
		end_distance_min = fight_distance_atnode*2; //fix
	}

	end_time_max = spawnArgs.GetInt( "end_time_max","0" );
	end_time_min = spawnArgs.GetInt( "end_time_min","0" );	
	if( end_time_min < 0){end_time_min = 0;}
	if( end_time_max < end_time_min ){end_time_max = end_time_min;}

	end_health_delta_atnode = spawnArgs.GetInt( "end_health_delta_atnode","0" );
	fight_health_delta_reaching = spawnArgs.GetInt( "fight_health_delta_reaching","0" );

	disabled = spawnArgs.GetBool( "start_off" );
	node_distance_max = spawnArgs.GetFloat( "node_distance_max","0" );
}

/*
=====================
idBotNode::TryLock
=====================
*/
bool idBotNode::TryLock( idAI_Bot *activator ) {
	if( !activator ){
		gameLocal.Warning("trylock failed on '%s' : activator not valid!", GetName() );
		return false;
	}
	
	if( disabled || inuse ){
		gameLocal.Warning("trylock failed on '%s' : disabled or inuse!", GetName() );
		return false;
	}

	//time
	if( end_time_min > 0){
		end_time = gameLocal.time + SEC2MS( end_time_min + gameLocal.random.RandomInt(end_time_max - end_time_min) );
	}

	//health
	intial_health = activator->GetHealth();

	//new owner
	ownerBot = activator;
	inuse = true;
	reached = false;
	//gameLocal.Warning("trylock ok on '%s' !", GetName() );
	return true; 
}

/*
=====================
idBotNode::Release
=====================
*/
void idBotNode::Release( void ) {
	if(inuse){
		if ( spawnArgs.GetBool( "use_once" ) ) {
			disabled = true;
		}

		inuse = false;
		reached = false;
		ownerBot = NULL;
		//gameLocal.Printf("node '%s' released!\n", GetName() );
	}
}

/*
=====================
idBotNode::Event_Reached
=====================
*/
void idBotNode::Event_Reached( void ) {
	if(inuse && !reached){
		reached = true;
		intial_health = ownerBot.GetEntity()->GetHealth();
	}
}

/*
=====================
idBotNode::Event_CanBeUsedBy
=====================
*/
void idBotNode::Event_CanBeUsedBy( idEntity *botEntity ) {
	if(disabled || inuse || !botEntity || !botEntity->IsType( idAI_Bot::Type )){
		//gameLocal.Printf("cannot use the node '%s'!\n", GetName() );
		idThread::ReturnInt( false ); return;
	}
	//gameLocal.Printf("can use the node '%s' !\n", GetName() );
	idThread::ReturnInt( true ); 

}


/*
=====================
idBotNode::Event_GetNextNode
=====================
*/
void idBotNode::Event_GetNextUsableNode( idEntity *botEntity ) {
	if( !botEntity || !botEntity->IsType( idAI_Bot::Type )){
		idThread::ReturnInt( false ); return;
	}

	idThread::ReturnEntity( GetNextUsableNode( static_cast<idAI_Bot *>( botEntity ) , 2 ) );
}

/*
=====================
idBotNode::AllNextNodesUsed
=====================
*/
bool idBotNode::AllNextNodesUsed( void ){
	int	i;
	idEntity *ent;
	idBotNode * myNode;
	bool allUsed = true;

	//gameLocal.Printf("idBotNode::AllNextNodesUsed called!\n");

	for( i = 0; i < targets.Num(); i++ ) {
		ent = targets[ i ].GetEntity();		
		if( ent ){
			if ( ent->IsType( idBotNode::Type ) ) {		
				myNode = static_cast<idBotNode *>( ent );
				if( !myNode->IsInUse() ){
					allUsed = false;
					//gameLocal.Printf("uno libero trovato: '%s'\n",myNode->GetName() );
					break;
				}
			}
		}
	}

	return allUsed ;
}


/*
=====================
idBotNode::GetNextUsableNode
=====================
*/
idEntity * idBotNode::GetNextUsableNode( idAI_Bot *botEntity, int lookAheadMaxLevel ) {
	int	i;
	int	numBotNodes;
	int	numOkEnts;
	int which;
	bool addItToList;
	idBotNode * myNode;

	//gameLocal.Printf("idBotNode::GetNextUsableNode!\n");

	idEntity *ent;
	idEntity *ents[ MAX_GENTITIES ];
	idBotNode *botNodeEnts[ MAX_GENTITIES ];

	numBotNodes = 0;
	
	//create a list containing the next idBotNodes
	for( i = 0; i < targets.Num(); i++ ) {
		ent = targets[ i ].GetEntity();		
		if( ent ){
			if ( ent->IsType( idBotNode::Type ) ) {		
				myNode = static_cast<idBotNode *>( ent );

				botNodeEnts[ numBotNodes++ ] = myNode;
				if ( numBotNodes >= MAX_GENTITIES ) {
					break;
				}
			}
		}
	}

	if ( !numBotNodes ) { //no nodes next
		//gameLocal.Printf("no nodi dopo!\n");
		return NULL;
	}

	numOkEnts = 0;

	//create a sub list containing the next VALID idBotNodes
	for( i = 0; i < numBotNodes; i++ ) {
		myNode = botNodeEnts[ i ];	
		addItToList = true;

		if( myNode->IsInUse() || ( myNode->DistanceContidionsTrue( botEntity ) < DISTANCE_COND_NOENEMY )){
			addItToList = false;
		}

		if (idStr::Cmp( myNode->GetEntityDefName(), "ai_bot_attack_path_corner" ) == 0) {
			if( myNode->AllNextNodesUsed() ){ addItToList = false; } //if it leads to used targets
		}

		if(addItToList){
			ents[ numOkEnts++ ] = myNode;
			if ( numOkEnts >= MAX_GENTITIES ) {
				break;
			}
		}

	}

	if ( numOkEnts ) {
		which = gameLocal.random.RandomInt( numOkEnts );
		//gameLocal.Printf("preso uno random dei %d nodi trovati: '%s' \n", numOkEnts, ents[ which ]->GetName() ); 
		return ents[ which ] ;
	}

	//nothing? Try looking ahead!
	lookAheadMaxLevel--;
	if(lookAheadMaxLevel > 0){
		for( i = 0; i < numBotNodes; i++ ) {
			ent = botNodeEnts[ i ]->GetNextUsableNode( botEntity, lookAheadMaxLevel );
			if(ent){ 
				//gameLocal.Printf("trovato uno con ricorsione: '%s' \n", ent->GetName() );
				return ent; 
			}
		}
	}

	//nothing
	//gameLocal.Printf("nothing found :( \n");
	return NULL;
}

/*
=====================
idBotNode::DistanceContidionsTrue
=====================
*/
int idBotNode::DistanceContidionsTrue( idAI_Bot *testEntity ) {
	idEntity * testEnemy; 
	float entityRange;
	float enemyRange;

	//entity stuff	
	if( node_distance_max > 0.0f ){ 
		entityRange = ( testEntity->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin() ).LengthFast();
		if( entityRange > node_distance_max ){
			//gameLocal.Printf("The node is too far\n" );
			return DISTANCE_COND_TOOFAR;
		}
	}
	
	//enemy stuff
	testEnemy = testEntity->GetEnemy();

	if(!testEnemy){
		//gameLocal.Printf("DistanceContidions !testEnemy...\n" );
		return DISTANCE_COND_NOENEMY; 
	}

	enemyRange = ( testEnemy->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin() ).LengthFast();

	if( fight_distance_atnode > 0.0f ){ //this is the most important one and should be the first.
		if( enemyRange < fight_distance_atnode ){
			//gameLocal.Printf("DistanceContidions fight_distance_atnode no\n" );
			return DISTANCE_COND_FIGHT_FAILED;
		}
	}

	if( end_distance_min > 0.0f){ // this check is enabled
		if( enemyRange < end_distance_min ){
			//gameLocal.Printf("DistanceContidions end_distance_min no\n" );
			return DISTANCE_COND_MIN_FAILED;
		}
	}

	if( end_distance_max > 0.0f ){ //this check is enabled
		if( enemyRange > end_distance_max ){
			//gameLocal.Printf("DistanceContidions end_distance_max no\n" );
			return DISTANCE_COND_MAX_FAILED;
		}
	}
	
	//gameLocal.Printf("DistanceContidions all ok\n" );
	return DISTANCE_COND_OK;
}

/*
=====================
idBotNode::Event_EvaluateConditions
=====================
*/
void idBotNode::Event_EvaluateConditions( void ) { //true if one of them is verified!
	idAI_Bot* currentOwner;
	int result;
	
	if ( !inuse ) {
		//gameLocal.Printf("Event_EvaluateConditions: node not in use!\n" );
		idThread::ReturnInt( true ); return;
	}

	assert( ownerBot );
	currentOwner = ownerBot.GetEntity();


	//time
	if( end_time_min > 0){ //not unlimited
		if(end_time < gameLocal.time){ 
			//gameLocal.Printf("time cond!\n" );
			idThread::ReturnInt( true ); return;
		}
	}

	//health
	int deltaToUse;
	if(reached){
		deltaToUse = end_health_delta_atnode;
	}else{
		deltaToUse = fight_health_delta_reaching;
	}
	if( deltaToUse > 0){
		if( deltaToUse < (intial_health - currentOwner->GetHealth())){
			//gameLocal.Printf("health cond!\n" );
			currentOwner->AI_SAY_DAMAGED = true;
			if(!reached){ currentOwner->AI_LEAVE_NODE_TO_FIGHT = true; } 
			idThread::ReturnInt( true ); return;
		}
	}
	

	//distance
	result = DistanceContidionsTrue( currentOwner );
	if( result == DISTANCE_COND_FIGHT_FAILED ){ 	
		currentOwner->AI_LEAVE_NODE_TO_FIGHT = true;
	}
	if( result != DISTANCE_COND_OK && result != DISTANCE_COND_NOENEMY ){
		//gameLocal.Printf("distance cond!\n" );
		idThread::ReturnInt( true ); return;
	}

	//else
	//gameLocal.Printf("Event_EvaluateConditions all false...go on!\n" );
	idThread::ReturnInt( false );
}
