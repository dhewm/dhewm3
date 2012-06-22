// Created by Ivan_the_B
//

#ifndef __GAME_BOT_H__
#define __GAME_BOT_H__

/*
===============================================================================

	idAI_Bot and idBotWeapon

===============================================================================
*/

//
// events
//
extern const idEventDef AI_Bot_FireWeapon;

// time before a weapon dropped to the floor disappears
const int AI_BOT_WEAPON_DROP_TIME = -1; //never //was:20 * 1000;

//class idBotNode;
class idBotSquad;

//talk flags - must match .script values
enum {
	AI_BOT_SAY_GENERIC = 0,
	AI_BOT_SAY_WATCHOUT,
	AI_BOT_SAY_SIGHT,
	AI_BOT_SAY_BACKTOIDLE,
	AI_BOT_SAY_SUSPICIOUS,
	AI_BOT_SAY_STARTMOVE,
	AI_BOT_SAY_LOST,
	AI_BOT_SAY_AFTERLOST,
	AI_BOT_SAY_DAMAGED,
	AI_BOT_SAY_FIRE,
	AI_BOT_SAY_MATEDOWN,
	AI_BOT_SAY_CHATTER,
	AI_BOT_SAY_MAXVALUE //always the last one!
};


typedef enum {
	AI_SQUAD_MEMBER_REMOVED,
	AI_SQUAD_MEMBER_ADDED 
}squadMsg_t;


//will not be saved/restored!

class idBotWeapon {
public:
	//spawn settings
	idEntityPtr<idEntity>	ent;
	int						channel;
	jointHandle_t			fireJoint;
	jointHandle_t			bindJoint;
	jointHandle_t			flashJoint; //to do:save

	//weapon
	int						priorityLevel;
	bool					enabled;
	idStr					weaponName;
	float					minSelfDmgDistance;

	//projectile
	const idDict *			projectileDef;
	float					projectileRadius;
	float					projectileSpeed;
	idVec3					projectileVelocity;
	idVec3					projectileGravity;
	mutable idClipModel		*projectileClipModel;

	//attack settings
	float					attack_accuracy;
	float					projectile_spread;
	int						num_projectiles;
	int						fireMode;

	//balancing info
	int						attackCounter;	//current attack counter
	int						balancing_multiplier; //multiplier value
	bool					requestedForBalancing;

	//clip
	int						clipSize;
	int						ammoInClip;
	int						reloadTolerance;

	//light
	const idMaterial *		shader;	
	idVec3					flashColor;
	float					flashRadius;
	int						flashTime;
	
};

class idAI_Bot : public idAI {
public:
	idScriptBool			AI_LEAVE_NODE_TO_FIGHT;
	//idScriptBool			AI_SAY_DAMAGED;
	idScriptFloat			AI_SAY_HINT;

public:
	CLASS_PROTOTYPE( idAI_Bot );
							idAI_Bot();
	virtual					~idAI_Bot();
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );
	void					SetSquad( idBotSquad* newsquad );
	void					ClearSquad( void );
	void					HandleSquadMsg( squadMsg_t type, int parm );
	void					Spawn( void );
	virtual bool			GetAimDir( const idVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, idVec3 &aimDir ) const;
	int						GetHealth( void );

protected:
	bool					canSpeak;
	idList<idBotWeapon>		weapons;
	int						currentWeapon;
	int						lastVisibleEnemyTime;
	bool					lastHitFCWCheckResult; //FromCurrentWeapon 
	int						lastHitFCWCheckTime; //FromCurrentWeapon 
	bool					lastHitNSDCheckResult; //NoSelfDamage
	int						lastHitNSDCheckTime; //NoSelfDamage
	int						lastWeaponChangedTime;
	idScriptBool			AI_WEAPON_CHANGED;
	idScriptBool			AI_WEAPON_NEED_RELOAD;
	//idEntityPtr<idBotNode>	currentNode;
	idEntityPtr<idBotSquad>	squad;
	//float					attackNodesMaxDist;

	static int say_times[ AI_BOT_SAY_MAXVALUE ];
	static const int say_delays[ AI_BOT_SAY_MAXVALUE ];
	static const char * say_names_default[ AI_BOT_SAY_MAXVALUE ];
	static const char * say_names_squad[ AI_BOT_SAY_MAXVALUE ];

	void					LinkScriptVariables( void ); //called in spawn method

	// attacks
	virtual void			CreateProjectileClipModel( void ) const;
	void					CreateProjectileClipModel( int requestedWeapon ) const; 
	//void					DeleteProjectileClipModel( void ) const;

	virtual idProjectile	*CreateProjectile( const idVec3 &pos, const idVec3 &dir );
	virtual idProjectile	*LaunchProjectile( const char *jointname, idEntity *target, bool clampToAttackCone );
	virtual void			GetMuzzle( const char *jointname, idVec3 &muzzle, idMat3 &axis );
	virtual	void			InitMuzzleFlash( void ); 
	void					setWeaponMuzzleFlash( void );
	virtual void			TriggerWeaponEffects( const idVec3 &muzzle );
	virtual	void			UpdateMuzzleFlash( void );
	virtual void			SetEnemyPosition( void );
	bool					GetAimDirByWeapon( const idVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, idVec3 &aimDir, int weaponToTest ) const;

	//weapons
	void					CreateWeapons( void );
	void					AttachWeapon( idEntity *ent, idStr weaponName);
	void					RemoveWeapons( void );
	bool					SelectWeapon( int weaponNum );
	void					SelectInitialWeapon();
	void					ShowOnlyCurrentWeapon();
	int						GetWeaponNumByName(idStr name);
	void					DropWeapon( int weaponNum );

	//utilities
	virtual void			CalculateAttackOffsets( void );
	virtual void			Show( void );
	void					SetDefaultSayTimes( float mult );
	virtual void			SetChatSound( void );
	virtual	void			PlayChatter( void );

	// damage
	virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );

	//events
	void					Event_SelectWeapon( int weaponNum );
	void					Event_IsWeaponEnabled( int weaponNum );
	void					Event_AddWeaponInBalancing( int weaponNum );
	void					Event_GetBalancedWeapon();
	void					Event_RegisterAttackForBalancing( int weaponNum );
	void					Event_SelectAnotherWeapon( int idealLevel, int minLevel );
	//void					Event_SelectAnotherWeaponByNode( idEntity *node );
	void					Event_GetCurrentWeapon( void );
	void					Event_GetWeaponNumByName( const char *weaponName );
	void					Event_CreateMissile( const char *jointname );
	void					Event_LaunchMissile( const idVec3 &muzzle, const idAngles &ang );
	void					Event_FireWeapon( void );
	void					Event_CanHitEnemyFromCurrentWeapon( void );
	void					Event_CanFireToEnemyNoSelfDamage( int requireVisible );
	void					Event_CanHitEnemyFromAnim( const char *animname );
	void					Event_CanHitEnemyFromFireAnim( int weaponToTest );
	void					Event_CanHitEnemyFromJoint( const char *jointname );
	void					Event_ReloadWeapon( void );
	void					Event_CheckReloadTolerance( void );
	void					Event_GetAmmoInClip( void );
	void					Event_GetClipSize( void );
	void					Event_GetIdleAnim( void );
	void					Event_GetReloadAnim( void );
	void					Event_GetFireAnim( void );
	void					Event_LostTimeMoreThan( int time );
	void					Event_WeaponChangedMoreThan( int time ); 
	void					Event_PlayAnimOnWeapon( const char *animname ); 
	//void					Event_TryLockNode( idEntity *node );
	//void					Event_ReleaseNode( void );
	void					Event_FindEnemyAIorPL( int useFOV );
	//void					Event_FindClosestBotNode( idEntity *ignoreNode );
	void					Event_GetNextSquadMate( idEntity *lastMatch );
	void					Event_CanSpeak( float can );
};

ID_INLINE int idAI_Bot::GetHealth( void ) {
	return health;
}

//from bad to good 
enum {
	DISTANCE_COND_TOOFAR		= -4,
	DISTANCE_COND_FIGHT_FAILED	= -3,
	DISTANCE_COND_MIN_FAILED	= -2,
	DISTANCE_COND_MAX_FAILED	= -1,
	DISTANCE_COND_NOENEMY		= 0,
	DISTANCE_COND_OK			= 1
};

/*
===============================================================================

	idBotNode

===============================================================================
*/
/*
class idBotNode : public idEntity {
public:
	CLASS_PROTOTYPE( idBotNode );
						idBotNode();

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );
	bool				IsDisabled( void ) const;
	bool				IsInUse( void ) const;
	bool				AllNextNodesUsed( void );
	idEntity *			GetNextUsableNode( idAI_Bot *botEntity, int lookAheadMaxLevel );
	int					DistanceContidionsTrue( idAI_Bot *testEntity );
	bool				TryLock( idAI_Bot *activator );
	void				Release( void );
	
private:
	float				fight_distance_atnode;
	float				end_distance_min;
	float				end_distance_max;
	float				node_distance_max;
	
	int					end_time_max;
	int					end_time_min;
	int					end_time;

	int					end_health_delta_atnode;
	int					fight_health_delta_reaching;
	int					intial_health;

	bool				disabled;
	bool				inuse;
	bool				reached;
	idEntityPtr<idAI_Bot>	ownerBot;

	void				Event_EvaluateConditions( void );
	void				Event_GetNextUsableNode( idEntity *fromEntity );
	void				Event_CanBeUsedBy( idEntity *botEntity );
	void				Event_Reached( void );
};

ID_INLINE bool idBotNode::IsDisabled( void ) const {
	return disabled;
}

ID_INLINE bool idBotNode::IsInUse( void ) const {
	return inuse;
}
*/

/*
===============================================================================

	idBotSquad

===============================================================================
*/

class idBotSquad : public idEntity {
public:
	CLASS_PROTOTYPE( idBotSquad );
						idBotSquad();

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );

	void				RemoveMember( idAI_Bot *bot );
	idAI_Bot*			GetNextMember( idAI_Bot *lastMatch, idAI_Bot *ignore );

	int					GetNumMembers( void ) const;
	//void				SendMsg( int type ) const; 
	
private:
	idList< idEntityPtr<idAI_Bot> >	members;

	void				Event_FindMembers( void );
	void				Event_Activate( idEntity *activator );
	void				FindMembers( void );
};

ID_INLINE int idBotSquad::GetNumMembers( void ) const{
	return members.Num();
}

#endif /* !__GAME_BOT_H__ */

