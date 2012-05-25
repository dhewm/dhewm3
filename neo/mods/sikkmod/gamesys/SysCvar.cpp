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
#include "framework/Licensee.h"
#include "framework/BuildVersion.h"

#include "GameBase.h"
#include "MultiplayerGame.h"

#include "SysCvar.h"

#if defined( _DEBUG )
	#define	BUILD_DEBUG	"-debug"
#else
	#define	BUILD_DEBUG "-release"
#endif

/*

All game cvars should be defined here.

*/

const char *si_gameTypeArgs[]		= { "singleplayer", "deathmatch", "Tourney", "Team DM", "Last Man", NULL };
const char *si_readyArgs[]			= { "Not Ready", "Ready", NULL };
const char *si_spectateArgs[]		= { "Play", "Spectate", NULL };

const char *ui_skinArgs[]			= { "skins/characters/player/marine_mp", "skins/characters/player/marine_mp_red", "skins/characters/player/marine_mp_blue", "skins/characters/player/marine_mp_green", "skins/characters/player/marine_mp_yellow", NULL };
const char *ui_teamArgs[]			= { "Red", "Blue", NULL };

struct gameVersion_s {
	gameVersion_s( void ) { sprintf( string, "%s.%d%s %s-%s %s %s", ENGINE_VERSION, BUILD_NUMBER, BUILD_DEBUG, BUILD_OS, BUILD_CPU, __DATE__, __TIME__ ); }
	char	string[256];
} gameVersion;

idCVar g_version(					"g_version",				gameVersion.string,	CVAR_GAME | CVAR_ROM, "game version" );

// noset vars
idCVar gamename(					"gamename",					GAME_VERSION,	CVAR_GAME | CVAR_SERVERINFO | CVAR_ROM, "" );
idCVar gamedate(					"gamedate",					__DATE__,		CVAR_GAME | CVAR_ROM, "" );

// server info
idCVar si_name(						"si_name",					"dhewm server",	CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "name of the server" );
idCVar si_gameType(					"si_gameType",		si_gameTypeArgs[ 0 ],	CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "game type - singleplayer, deathmatch, Tourney, Team DM or Last Man", si_gameTypeArgs, idCmdSystem::ArgCompletion_String<si_gameTypeArgs> );
idCVar si_map(						"si_map",					"game/mp/d3dm1",CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "map to be played next on server", idCmdSystem::ArgCompletion_MapName );
idCVar si_maxPlayers(				"si_maxPlayers",			"4",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_INTEGER, "max number of players allowed on the server", 1, 4 );
idCVar si_fragLimit(				"si_fragLimit",				"10",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_INTEGER, "frag limit", 1, MP_PLAYER_MAXFRAGS );
idCVar si_timeLimit(				"si_timeLimit",				"10",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_INTEGER, "time limit in minutes", 0, 60 );
idCVar si_teamDamage(				"si_teamDamage",			"0",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "enable team damage" );
idCVar si_warmup(					"si_warmup",				"0",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "do pre-game warmup" );
idCVar si_usePass(					"si_usePass",				"0",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "enable client password checking" );
idCVar si_pure(						"si_pure",					"1",			CVAR_GAME | CVAR_SERVERINFO | CVAR_BOOL, "server is pure and does not allow modified data" );
idCVar si_spectators(				"si_spectators",			"1",			CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_BOOL, "allow spectators or require all clients to play" );
idCVar si_serverURL(				"si_serverURL",				"",				CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE, "where to reach the server admins and get information about the server" );

// user info
idCVar ui_name(						"ui_name",					"Player",		CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE, "player name" );
idCVar ui_skin(						"ui_skin",				ui_skinArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE, "player skin", ui_skinArgs, idCmdSystem::ArgCompletion_String<ui_skinArgs> );
idCVar ui_team(						"ui_team",				ui_teamArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE, "player team", ui_teamArgs, idCmdSystem::ArgCompletion_String<ui_teamArgs> );
idCVar ui_autoSwitch(				"ui_autoSwitch",			"1",			CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE | CVAR_BOOL, "auto switch weapon" );
idCVar ui_autoReload(				"ui_autoReload",			"1",			CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE | CVAR_BOOL, "auto reload weapon" );
idCVar ui_showGun(					"ui_showGun",				"1",			CVAR_GAME | CVAR_USERINFO | CVAR_ARCHIVE | CVAR_BOOL, "show gun" );
idCVar ui_ready(					"ui_ready",				si_readyArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO, "player is ready to start playing", idCmdSystem::ArgCompletion_String<si_readyArgs> );
idCVar ui_spectate(					"ui_spectate",		si_spectateArgs[ 0 ],	CVAR_GAME | CVAR_USERINFO, "play or spectate", idCmdSystem::ArgCompletion_String<si_spectateArgs> );
idCVar ui_chat(						"ui_chat",					"0",			CVAR_GAME | CVAR_USERINFO | CVAR_BOOL | CVAR_ROM | CVAR_CHEAT, "player is chatting" );

// change anytime vars
idCVar developer(					"developer",				"0",			CVAR_GAME | CVAR_BOOL, "" );

idCVar r_aspectRatio(				"r_aspectRatio",			"0",			CVAR_RENDERER | CVAR_INTEGER | CVAR_ARCHIVE, "aspect ratio of view:\n0 = 4:3\n1 = 16:9\n2 = 16:10", 0, 2 );

idCVar g_cinematic(					"g_cinematic",				"1",			CVAR_GAME | CVAR_BOOL, "skips updating entities that aren't marked 'cinematic' '1' during cinematics" );
idCVar g_cinematicMaxSkipTime(		"g_cinematicMaxSkipTime",	"600",			CVAR_GAME | CVAR_FLOAT, "# of seconds to allow game to run when skipping cinematic.  prevents lock-up when cinematic doesn't end.", 0, 3600 );

idCVar g_muzzleFlash(				"g_muzzleFlash",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show muzzle flashes" );
idCVar g_projectileLights(			"g_projectileLights",		"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show dynamic lights on projectiles" );
idCVar g_bloodEffects(				"g_bloodEffects",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show blood splats, sprays and gibs" );
idCVar g_doubleVision(				"g_doubleVision",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show double vision when taking damage" );
idCVar g_monsters(					"g_monsters",				"1",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_decals(					"g_decals",					"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show decals such as bullet holes" );
idCVar g_knockback(					"g_knockback",				"1000",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar g_skill(						"g_skill",					"1",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar g_nightmare(					"g_nightmare",				"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "if nightmare mode is allowed" );
idCVar g_gravity(					"g_gravity",		DEFAULT_GRAVITY_STRING, CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_skipFX(					"g_skipFX",					"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_skipParticles(				"g_skipParticles",			"0",			CVAR_GAME | CVAR_BOOL, "" );

idCVar g_disasm(					"g_disasm",					"0",			CVAR_GAME | CVAR_BOOL, "disassemble script into base/script/disasm.txt on the local drive when script is compiled" );
idCVar g_debugBounds(				"g_debugBounds",			"0",			CVAR_GAME | CVAR_BOOL, "checks for models with bounds > 2048" );
idCVar g_debugAnim(					"g_debugAnim",				"-1",			CVAR_GAME | CVAR_INTEGER, "displays information on which animations are playing on the specified entity number.  set to -1 to disable." );
idCVar g_debugMove(					"g_debugMove",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugDamage(				"g_debugDamage",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugWeapon(				"g_debugWeapon",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugScript(				"g_debugScript",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugMover(				"g_debugMover",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugTriggers(				"g_debugTriggers",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_debugCinematic(			"g_debugCinematic",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_stopTime(					"g_stopTime",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_damageScale(				"g_damageScale",			"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "scale final damage on player by this factor" );
idCVar g_armorProtection(			"g_armorProtection",		"0.3",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "armor takes this percentage of damage" );
idCVar g_armorProtectionMP(			"g_armorProtectionMP",		"0.6",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "armor takes this percentage of damage in mp" );
idCVar g_useDynamicProtection(		"g_useDynamicProtection",	"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "scale damage and armor dynamically to keep the player alive more often" );
idCVar g_healthTakeTime(			"g_healthTakeTime",			"5",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "how often to take health in nightmare mode" );
idCVar g_healthTakeAmt(				"g_healthTakeAmt",			"5",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "how much health to take in nightmare mode" );
idCVar g_healthTakeLimit(			"g_healthTakeLimit",		"25",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER, "how low can health get taken in nightmare mode" );

idCVar g_showPVS(					"g_showPVS",				"0",			CVAR_GAME | CVAR_INTEGER, "", 0, 2 );
idCVar g_showTargets(				"g_showTargets",			"0",			CVAR_GAME | CVAR_BOOL, "draws entities and thier targets.  hidden entities are drawn grey." );
idCVar g_showTriggers(				"g_showTriggers",			"0",			CVAR_GAME | CVAR_BOOL, "draws trigger entities (orange) and thier targets (green).  disabled triggers are drawn grey." );
idCVar g_showCollisionWorld(		"g_showCollisionWorld",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_showCollisionModels(		"g_showCollisionModels",	"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_showCollisionTraces(		"g_showCollisionTraces",	"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_maxShowDistance(			"g_maxShowDistance",		"128",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_showEntityInfo(			"g_showEntityInfo",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_showviewpos(				"g_showviewpos",			"0",			CVAR_GAME | CVAR_BOOL, "" );
// sikk - g_showcamerainfo was archived and type bool was not set
idCVar g_showcamerainfo(			"g_showcamerainfo",			"0",			CVAR_GAME | CVAR_BOOL, "displays the current frame # for the camera when playing cinematics" );
idCVar g_showTestModelFrame(		"g_showTestModelFrame",		"0",			CVAR_GAME | CVAR_BOOL, "displays the current animation and frame # for testmodels" );
idCVar g_showActiveEntities(		"g_showActiveEntities",		"0",			CVAR_GAME | CVAR_BOOL, "draws boxes around thinking entities.  dormant entities (outside of pvs) are drawn yellow.  non-dormant are green." );
idCVar g_showEnemies(				"g_showEnemies",			"0",			CVAR_GAME | CVAR_BOOL, "draws boxes around monsters that have targeted the the player" );

idCVar g_frametime(					"g_frametime",				"0",			CVAR_GAME | CVAR_BOOL, "displays timing information for each game frame" );
idCVar g_timeentities(				"g_timeEntities",			"0",			CVAR_GAME | CVAR_FLOAT, "when non-zero, shows entities whose think functions exceeded the # of milliseconds specified" );

idCVar ai_debugScript(				"ai_debugScript",			"-1",			CVAR_GAME | CVAR_INTEGER, "displays script calls for the specified monster entity number" );
idCVar ai_debugMove(				"ai_debugMove",				"0",			CVAR_GAME | CVAR_BOOL, "draws movement information for monsters" );
idCVar ai_debugTrajectory(			"ai_debugTrajectory",		"0",			CVAR_GAME | CVAR_BOOL, "draws trajectory tests for monsters" );
idCVar ai_testPredictPath(			"ai_testPredictPath",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar ai_showCombatNodes(			"ai_showCombatNodes",		"0",			CVAR_GAME | CVAR_BOOL, "draws attack cones for monsters" );
idCVar ai_showPaths(				"ai_showPaths",				"0",			CVAR_GAME | CVAR_BOOL, "draws path_* entities" );
idCVar ai_showObstacleAvoidance(	"ai_showObstacleAvoidance",	"0",			CVAR_GAME | CVAR_INTEGER, "draws obstacle avoidance information for monsters.  if 2, draws obstacles for player, as well", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
idCVar ai_blockedFailSafe(			"ai_blockedFailSafe",		"1",			CVAR_GAME | CVAR_BOOL, "enable blocked fail safe handling" );

idCVar g_dvTime(					"g_dvTime",					"1",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_dvAmplitude(				"g_dvAmplitude",			"0.001",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_dvFrequency(				"g_dvFrequency",			"0.5",			CVAR_GAME | CVAR_FLOAT, "" );

idCVar g_kickTime(					"g_kickTime",				"1",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_kickAmplitude(				"g_kickAmplitude",			"0.0001",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_blobTime(					"g_blobTime",				"1",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_blobSize(					"g_blobSize",				"1",			CVAR_GAME | CVAR_FLOAT, "" );

idCVar g_testHealthVision(			"g_testHealthVision",		"0",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_editEntityMode(			"g_editEntityMode",			"0",			CVAR_GAME | CVAR_INTEGER,	"0 = off\n"
																											"1 = lights\n"
																											"2 = sounds\n"
																											"3 = articulated figures\n"
																											"4 = particle systems\n"
																											"5 = monsters\n"
																											"6 = entity names\n"
																											"7 = entity models", 0, 7, idCmdSystem::ArgCompletion_Integer<0,7> );
idCVar g_dragEntity(				"g_dragEntity",				"0",			CVAR_GAME | CVAR_BOOL, "allows dragging physics objects around by placing the crosshair over them and holding the fire button" );
idCVar g_dragDamping(				"g_dragDamping",			"0.5",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_dragShowSelection(			"g_dragShowSelection",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_dropItemRotation(			"g_dropItemRotation",		"",				CVAR_GAME, "" );

idCVar g_vehicleVelocity(			"g_vehicleVelocity",		"1000",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleForce(				"g_vehicleForce",			"50000",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionUp(		"g_vehicleSuspensionUp",	"32",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionDown(		"g_vehicleSuspensionDown",	"20",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionKCompress("g_vehicleSuspensionKCompress","200",		CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleSuspensionDamping(	"g_vehicleSuspensionDamping","400",			CVAR_GAME | CVAR_FLOAT, "" );
idCVar g_vehicleTireFriction(		"g_vehicleTireFriction",	"0.8",			CVAR_GAME | CVAR_FLOAT, "" );

idCVar ik_enable(					"ik_enable",				"1",			CVAR_GAME | CVAR_BOOL, "enable IK" );
idCVar ik_debug(					"ik_debug",					"0",			CVAR_GAME | CVAR_BOOL, "show IK debug lines" );

idCVar af_useLinearTime(			"af_useLinearTime",			"1",			CVAR_GAME | CVAR_BOOL, "use linear time algorithm for tree-like structures" );
idCVar af_useImpulseFriction(		"af_useImpulseFriction",	"0",			CVAR_GAME | CVAR_BOOL, "use impulse based contact friction" );
idCVar af_useJointImpulseFriction(	"af_useJointImpulseFriction","0",			CVAR_GAME | CVAR_BOOL, "use impulse based joint friction" );
idCVar af_useSymmetry(				"af_useSymmetry",			"1",			CVAR_GAME | CVAR_BOOL, "use constraint matrix symmetry" );
idCVar af_skipSelfCollision(		"af_skipSelfCollision",		"0",			CVAR_GAME | CVAR_BOOL, "skip self collision detection" );
idCVar af_skipLimits(				"af_skipLimits",			"0",			CVAR_GAME | CVAR_BOOL, "skip joint limits" );
idCVar af_skipFriction(				"af_skipFriction",			"0",			CVAR_GAME | CVAR_BOOL, "skip friction" );
idCVar af_forceFriction(			"af_forceFriction",			"-1",			CVAR_GAME | CVAR_FLOAT, "force the given friction value" );
idCVar af_maxLinearVelocity(		"af_maxLinearVelocity",		"128",			CVAR_GAME | CVAR_FLOAT, "maximum linear velocity" );
idCVar af_maxAngularVelocity(		"af_maxAngularVelocity",	"1.57",			CVAR_GAME | CVAR_FLOAT, "maximum angular velocity" );
idCVar af_timeScale(				"af_timeScale",				"1",			CVAR_GAME | CVAR_FLOAT, "scales the time" );
idCVar af_jointFrictionScale(		"af_jointFrictionScale",	"0",			CVAR_GAME | CVAR_FLOAT, "scales the joint friction" );
idCVar af_contactFrictionScale(		"af_contactFrictionScale",	"0",			CVAR_GAME | CVAR_FLOAT, "scales the contact friction" );
idCVar af_highlightBody(			"af_highlightBody",			"",				CVAR_GAME, "name of the body to highlight" );
idCVar af_highlightConstraint(		"af_highlightConstraint",	"",				CVAR_GAME, "name of the constraint to highlight" );
idCVar af_showTimings(				"af_showTimings",			"0",			CVAR_GAME | CVAR_BOOL, "show articulated figure cpu usage" );
idCVar af_showConstraints(			"af_showConstraints",		"0",			CVAR_GAME | CVAR_BOOL, "show constraints" );
idCVar af_showConstraintNames(		"af_showConstraintNames",	"0",			CVAR_GAME | CVAR_BOOL, "show constraint names" );
idCVar af_showConstrainedBodies(	"af_showConstrainedBodies",	"0",			CVAR_GAME | CVAR_BOOL, "show the two bodies contrained by the highlighted constraint" );
idCVar af_showPrimaryOnly(			"af_showPrimaryOnly",		"0",			CVAR_GAME | CVAR_BOOL, "show primary constraints only" );
idCVar af_showTrees(				"af_showTrees",				"0",			CVAR_GAME | CVAR_BOOL, "show tree-like structures" );
idCVar af_showLimits(				"af_showLimits",			"0",			CVAR_GAME | CVAR_BOOL, "show joint limits" );
idCVar af_showBodies(				"af_showBodies",			"0",			CVAR_GAME | CVAR_BOOL, "show bodies" );
idCVar af_showBodyNames(			"af_showBodyNames",			"0",			CVAR_GAME | CVAR_BOOL, "show body names" );
idCVar af_showMass(					"af_showMass",				"0",			CVAR_GAME | CVAR_BOOL, "show the mass of each body" );
idCVar af_showTotalMass(			"af_showTotalMass",			"0",			CVAR_GAME | CVAR_BOOL, "show the total mass of each articulated figure" );
idCVar af_showInertia(				"af_showInertia",			"0",			CVAR_GAME | CVAR_BOOL, "show the inertia tensor of each body" );
idCVar af_showVelocity(				"af_showVelocity",			"0",			CVAR_GAME | CVAR_BOOL, "show the velocity of each body" );
idCVar af_showActive(				"af_showActive",			"0",			CVAR_GAME | CVAR_BOOL, "show tree-like structures of articulated figures not at rest" );
idCVar af_testSolid(				"af_testSolid",				"1",			CVAR_GAME | CVAR_BOOL, "test for bodies initially stuck in solid" );

idCVar rb_showTimings(				"rb_showTimings",			"0",			CVAR_GAME | CVAR_BOOL, "show rigid body cpu usage" );
idCVar rb_showBodies(				"rb_showBodies",			"0",			CVAR_GAME | CVAR_BOOL, "show rigid bodies" );
idCVar rb_showMass(					"rb_showMass",				"0",			CVAR_GAME | CVAR_BOOL, "show the mass of each rigid body" );
idCVar rb_showInertia(				"rb_showInertia",			"0",			CVAR_GAME | CVAR_BOOL, "show the inertia tensor of each rigid body" );
idCVar rb_showVelocity(				"rb_showVelocity",			"0",			CVAR_GAME | CVAR_BOOL, "show the velocity of each rigid body" );
idCVar rb_showActive(				"rb_showActive",			"0",			CVAR_GAME | CVAR_BOOL, "show rigid bodies that are not at rest" );

// sikk---> Added "CVAR_ARCHIVE" to all player movement cvars
// The default values for player movement cvars are set in def/player.def
idCVar pm_jumpheight(				"pm_jumpheight",			"48",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "approximate hieght the player can jump" );
idCVar pm_stepsize(					"pm_stepsize",				"16",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "maximum height the player can step up without jumping" );
idCVar pm_crouchspeed(				"pm_crouchspeed",			"80",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "speed the player can move while crouched" );
idCVar pm_walkspeed(				"pm_walkspeed",				"140",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "speed the player can move while walking" );
idCVar pm_runspeed(					"pm_runspeed",				"220",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "speed the player can move while running" );
idCVar pm_noclipspeed(				"pm_noclipspeed",			"200",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "speed the player can move while in noclip" );
idCVar pm_spectatespeed(			"pm_spectatespeed",			"450",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "speed the player can move while spectating" );
idCVar pm_spectatebbox(				"pm_spectatebbox",			"32",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "size of the spectator bounding box" );
idCVar pm_usecylinder(				"pm_usecylinder",			"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_BOOL, "use a cylinder approximation instead of a bounding box for player collision detection" );
idCVar pm_minviewpitch(				"pm_minviewpitch",			"-89",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "amount player's view can look up (negative values are up)" );
idCVar pm_maxviewpitch(				"pm_maxviewpitch",			"89",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "amount player's view can look down" );
idCVar pm_stamina(					"pm_stamina",				"24",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT,					"length of time player can run" );
idCVar pm_staminathreshold(			"pm_staminathreshold",		"4",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "when stamina drops below this value, player gradually slows to a walk" );
idCVar pm_staminarate(				"pm_staminarate",			"0.75",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "rate that player regains stamina. divide pm_stamina by this value to determine how long it takes to fully recharge." );
idCVar pm_crouchheight(				"pm_crouchheight",			"38",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "height of player's bounding box while crouched" );
idCVar pm_crouchviewheight(			"pm_crouchviewheight",		"32",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "height of player's view while crouched" );
idCVar pm_normalheight(				"pm_normalheight",			"74",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "height of player's bounding box while standing" );
idCVar pm_normalviewheight(			"pm_normalviewheight",		"68",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "height of player's view while standing" );
idCVar pm_deadheight(				"pm_deadheight",			"20",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "height of player's bounding box while dead" );
idCVar pm_deadviewheight(			"pm_deadviewheight",		"10",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "height of player's view while dead" );
idCVar pm_crouchrate(				"pm_crouchrate",			"0.87",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "time it takes for player's view to change from standing to crouching" );
idCVar pm_bboxwidth(				"pm_bboxwidth",				"32",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "x/y size of player's bounding box" );
idCVar pm_crouchbob(				"pm_crouchbob",				"0.5",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "bob much faster when crouched" );
idCVar pm_walkbob(					"pm_walkbob",				"0.3",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "bob slowly when walking" );
idCVar pm_runbob(					"pm_runbob",				"0.4",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "bob faster when running" );
idCVar pm_runpitch(					"pm_runpitch",				"0.002",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar pm_runroll(					"pm_runroll",				"0.005",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar pm_bobup(					"pm_bobup",					"0.005",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar pm_bobpitch(					"pm_bobpitch",				"0.002",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar pm_bobroll(					"pm_bobroll",				"0.002",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "" );

idCVar pm_thirdPersonRange(			"pm_thirdPersonRange",		"80",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "camera distance from player in 3rd person" );
idCVar pm_thirdPersonHeight(		"pm_thirdPersonHeight",		"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "height of camera from normal view height in 3rd person" );
idCVar pm_thirdPersonAngle(			"pm_thirdPersonAngle",		"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT, "direction of camera from player in 3rd person in degrees (0 = behind player, 180 = in front)" );
idCVar pm_thirdPersonClip(			"pm_thirdPersonClip",		"1",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_BOOL, "clip third person view into world space" );
idCVar pm_thirdPerson(				"pm_thirdPerson",			"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_BOOL, "enables third person view" );
idCVar pm_thirdPersonDeath(			"pm_thirdPersonDeath",		"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_BOOL, "enables third person view when player dies" );
idCVar pm_modelView(				"pm_modelView",				"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_INTEGER, "draws camera from POV of player model (1 = always, 2 = when dead)", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
idCVar pm_airTics(					"pm_air",					"1800",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_INTEGER, "how long in milliseconds the player can go without air before he starts taking damage" );
// <---sikk

idCVar g_showPlayerShadow(			"g_showPlayerShadow",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "enables shadow of player model" );
idCVar g_showHud(					"g_showHud",				"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "" );
idCVar g_showProjectilePct(			"g_showProjectilePct",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "enables display of player hit percentage" );
idCVar g_showBrass(					"g_showBrass",				"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "enables ejected shells from weapon" );

// sikk---> Added "CVAR_ARCHIVE" to all select game cvars
idCVar g_gun_x(						"g_gunX",					"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar g_gun_y(						"g_gunY",					"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar g_gun_z(						"g_gunZ",					"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar g_viewNodalX(				"g_viewNodalX",				"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar g_viewNodalZ(				"g_viewNodalZ",				"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar g_fov(						"g_fov",					"90",			CVAR_GAME | CVAR_ARCHIVE | CVAR_INTEGER | CVAR_NOCHEAT, "" );
idCVar g_skipViewEffects(			"g_skipViewEffects",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "skip damage and other view effects" );
idCVar g_mpWeaponAngleScale(		"g_mpWeaponAngleScale",		"0",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Control the weapon sway in MP" );
// <---sikk

idCVar g_testParticle(				"g_testParticle",			"0",			CVAR_GAME | CVAR_INTEGER, "test particle visualation, set by the particle editor" );
idCVar g_testParticleName(			"g_testParticleName",		"",				CVAR_GAME, "name of the particle being tested by the particle editor" );
idCVar g_testModelRotate(			"g_testModelRotate",		"0",			CVAR_GAME, "test model rotation speed" );
idCVar g_testPostProcess(			"g_testPostProcess",		"",				CVAR_GAME, "name of material to draw over screen" );
idCVar g_testModelAnimate(			"g_testModelAnimate",		"0",			CVAR_GAME | CVAR_INTEGER, "test model animation,\n"
																							"0 = cycle anim with origin reset\n"
																							"1 = cycle anim with fixed origin\n"
																							"2 = cycle anim with continuous origin\n"
																							"3 = frame by frame with continuous origin\n"
																							"4 = play anim once", 0, 4, idCmdSystem::ArgCompletion_Integer<0,4> );
idCVar g_testModelBlend(			"g_testModelBlend",			"0",			CVAR_GAME | CVAR_INTEGER, "number of frames to blend" );
idCVar g_testDeath(					"g_testDeath",				"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar g_exportMask(				"g_exportMask",				"",				CVAR_GAME, "" );
idCVar g_flushSave(					"g_flushSave",				"0",			CVAR_GAME | CVAR_BOOL, "1 = don't buffer file writing for save games." );

idCVar aas_test(					"aas_test",					"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showAreas(				"aas_showAreas",			"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar aas_showPath(				"aas_showPath",				"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showFlyPath(				"aas_showFlyPath",			"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showWallEdges(			"aas_showWallEdges",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar aas_showHideArea(			"aas_showHideArea",			"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_pullPlayer(				"aas_pullPlayer",			"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_randomPullPlayer(		"aas_randomPullPlayer",		"0",			CVAR_GAME | CVAR_BOOL, "" );
idCVar aas_goalArea(				"aas_goalArea",				"0",			CVAR_GAME | CVAR_INTEGER, "" );
idCVar aas_showPushIntoArea(		"aas_showPushIntoArea",		"0",			CVAR_GAME | CVAR_BOOL, "" );

idCVar g_password(					"g_password",				"",				CVAR_GAME | CVAR_ARCHIVE, "game password" );
idCVar password(					"password",					"",				CVAR_GAME | CVAR_NOCHEAT, "client password used when connecting" );

idCVar g_countDown(					"g_countDown",				"10",			CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "pregame countdown in seconds", 4, 3600 );
idCVar g_gameReviewPause(			"g_gameReviewPause",		"10",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_INTEGER | CVAR_ARCHIVE, "scores review time in seconds (at end game)", 2, 3600 );
idCVar g_TDMArrows(					"g_TDMArrows",				"1",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_BOOL, "draw arrows over teammates in team deathmatch" );
idCVar g_balanceTDM(				"g_balanceTDM",				"1",			CVAR_GAME | CVAR_BOOL, "maintain even teams" );

idCVar net_clientPredictGUI(		"net_clientPredictGUI",		"1",			CVAR_GAME | CVAR_BOOL, "test guis in networking without prediction" );

idCVar g_voteFlags(					"g_voteFlags",				"0",			CVAR_GAME | CVAR_NETWORKSYNC | CVAR_INTEGER | CVAR_ARCHIVE, "vote flags. bit mask of votes not allowed on this server\n"
																					"bit 0 (+1)   restart now\n"
																					"bit 1 (+2)   time limit\n"
																					"bit 2 (+4)   frag limit\n"
																					"bit 3 (+8)   game type\n"
																					"bit 4 (+16)  kick player\n"
																					"bit 5 (+32)  change map\n"
																					"bit 6 (+64)  spectators\n"
																					"bit 7 (+128) next map" );
idCVar g_mapCycle(					"g_mapCycle",				"mapcycle",		CVAR_GAME | CVAR_ARCHIVE, "map cycling script for multiplayer games - see mapcycle.scriptcfg" );

idCVar mod_validSkins(				"mod_validSkins",			"skins/characters/player/marine_mp;skins/characters/player/marine_mp_green;skins/characters/player/marine_mp_blue;skins/characters/player/marine_mp_red;skins/characters/player/marine_mp_yellow",		CVAR_GAME | CVAR_ARCHIVE, "valid skins for the game" );

idCVar net_serverDownload(			"net_serverDownload",		"0",			CVAR_GAME | CVAR_INTEGER | CVAR_ARCHIVE, "enable server download redirects. 0: off 1: redirect to si_serverURL 2: use builtin download. see net_serverDl cvars for configuration" );
idCVar net_serverDlBaseURL(			"net_serverDlBaseURL",		"",				CVAR_GAME | CVAR_ARCHIVE, "base URL for the download redirection" );
idCVar net_serverDlTable(			"net_serverDlTable",		"",				CVAR_GAME | CVAR_ARCHIVE, "pak names for which download is provided, seperated by ;" );


// sikk - New Cvars -
//-------------------------------------------------
// sikk---> Crosshair Cvar
idCVar g_crosshair(					"g_crosshair",					"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"0 = crosshair off, 1 = crosshair on, 2 = crosshair on only when zoomed or npc has focus" );
idCVar g_crosshairType(				"g_crosshairType",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"toggle between static and precise crosshair positioning" );
idCVar g_crosshairLerp(				"g_crosshairLerp",				"0.5",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"smoothness for the movement of the crosshair when g_crosshairType = 1" );
// <---sikk

// sikk---> Dynamic Hud system
idCVar g_hudType(					"g_hudType",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"toggles between default and custom hud" );
idCVar g_useDynamicHud(				"g_useDynamicHud",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"enables dynamic hud (g_hudType == 1 only)" );
idCVar g_dynamicHudTime(			"g_dynamicHudTime",				"10.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"time (in sec) before the hud fades out" );
// <---sikk

// sikk---> IR Goggles/Headlight Mod
idCVar g_goggleType(				"g_goggleType",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"sets the goggle's type: 0 = Infrared; 1 = Thermal" );
idCVar g_batteryLife(				"g_batteryLife",				"60",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "how long the battery lasts (in seconds) for ir goggles/headlight" );
idCVar g_batteryRechargeRate(		"g_batteryRechargeRate",		"120",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "how long it takes the battery to fully recharge (in seconds) for ir goggles/headlight" );
// <---sikk

// sikk---> Global Ambient Light
idCVar g_useAmbientLight(			"g_useAmbientLight",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"enable a global ambient light bound to player" );
idCVar g_ambientLightRadius(		"g_ambientLightRadius",			"1024 1024 1024",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE,	"sets the ambient light's radius (XYZ = 0 to 65536)" );
idCVar g_ambientLightColor(			"g_ambientLightColor",			"0.03125 0.03125 0.03125",	CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE,	"sets the ambient light's color (RGB = 0.0 to 1.0)" );

// sikk---> Explosion FX
idCVar g_useExplosionFX(			"g_useExplosionFX",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"enables explosion screen effects" );
idCVar g_explosionFXTime(			"g_explosionFXTime",			"2.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"time (in secs) the effect lasts" );
idCVar g_explosionFXScale(			"g_explosionFXScale",			"32.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"explosion effect scale" );
// <---sikk

// sikk---> Blood Spray Screen Effect
idCVar g_showBloodSpray(			"g_showBloodSpray",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"show blood spray effect on screen when inflicting damage up close" );
idCVar g_bloodSprayTime(			"g_bloodSprayTime",				"5.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"how long blood spray effect lasts (in seconds)" );
idCVar g_bloodSprayDistance(		"g_bloodSprayDistance",			"96",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"how close you have to be for blood spray effect to \"hit the camera\". Not relevant for chainsaw." );
idCVar g_bloodSprayFrequency(		"g_bloodSprayFrequency",		"0.5",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"how often the blood spray effect can occur. Value Range: 0.0 (never) - 1.0 (always)" );
// <---sikk

// sikk---> Screen Frost
idCVar g_screenFrostTime(			"g_screenFrostTime",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"time (in secs) for frost to fully build on the screen. 0 = disables effect" );
// <---sikk

// sikk---> Tracer Frequency
idCVar g_tracerFrequency(			"g_tracerFrequency",			"0.5",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"how frequent a fired shot will be a tracer. Value Range: 0.0 (no tracers) - 1.0 (all tracers)" );
// <---sikk

// sikk---> Player Head Type
idCVar g_playerHeadType(			"g_playerHeadType",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"sets which head def to use for the player. 0 = Default Player Head; 1 = Marine Helmet" );
// <---sikk

// sikk---> First Person Body
idCVar g_showFirstPersonBody(		"g_showFirstPersonBody",		"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"draws the player body in first person view mode" );
// <---sikk

// sikk---> Portal Sky Box
idCVar g_enablePortalSky(			"g_enablePortalSky",			"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"enables portal sky box support" );
// <---sikk


// sikk---> Health Management System
idCVar g_healthManagementType(		"g_healthManagementType",		"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"selects which health management system to use. 0 = default; 1 = carriable health pack; 2 = regenerating health" );
idCVar g_healthPackTotal(			"g_healthPackTotal",			"100",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"total amount of health reserve in the health pack" );
idCVar g_healthPackUses(			"g_healthPackUses",				"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"number of uses in the health pack. g_healthPackTotal divided by this is how much health is given per use." );
idCVar g_healthPackTime(			"g_healthPackTime",				"3",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"maximum time it takes to heal yourself. " );
idCVar g_healthRegenTime(			"g_healthRegenTime",			"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "how often to regenerate health when using regenerative health system" );
idCVar g_healthRegenDelay(			"g_healthRegenDelay",			"5",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "delay (in seconds) before health starts to regenerate after taking damage when using regenerative health system" );
idCVar g_healthRegenAmt(			"g_healthRegenAmt",				"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "how much health to regenerate per g_healthRegenTime when using regenerative health system" );
idCVar g_healthRegenLimit(			"g_healthRegenLimit",			"100",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "total amount of health that can be regenerated when using regenerative health system" );
idCVar g_healthRegenSteps(			"g_healthRegenSteps",			"4",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "splits g_healthRegenLimit into n number of steps. Value of 1 or less will turn steps off.\nexample1: if g_healthRegenLimit == 100 && g_healthRegenSteps == 4, health regeneration will stop at 25/50/75/100\nexample2: if g_healthRegenLimit == 50 && g_healthRegenSteps == 5, health regeneration will stop at 10/20/30/40/50" );
idCVar g_healthRegenFeedback(		"g_healthRegenFeedback",		"50",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER, "how low must health reach before feedback is drawn when using regenerative health system" );
// <---sikk

// sikk---> Item Management
idCVar g_itemPickupType(			"g_itemPickupType",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"toggles whether items need to be picked up manually with the 'Use' key." );
idCVar g_itemMaxArmorType(			"g_itemMaxArmorType",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets armor capacity type: 0 = Doom 3; 1 = Doom 1/2; 2 = Custom" );
idCVar g_itemHelmetFactor(			"g_itemHelmetFactor",			"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the factor for which security armor is randomly replaced with a marine helmet. Value Range: 0.0 (no replacement) - 1.0 (full replacement)" );
idCVar g_itemValueFactor(			"g_itemValueFactor",			"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the factor for which the value for items are randomly set. Value Range: 0.0 (items have full value) - 1.0 (items have anywhere from one to full value)" );
idCVar g_itemRemovalFactor(			"g_itemRemovalFactor",			"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the factor for which items are randomly removed from the map. Value Range: 0.0 (no items are removed) - 1.0 (all items are removed)" );
idCVar g_itemSearchFactor(			"g_itemSearchFactor",			"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the factor for which items can be found on dead bodies. Value Range: 0.0 (dead bodies contain no items) - 1.0 (dead bodies always contain items)" );
// <---sikk

// sikk---> Ammo Management
idCVar g_ammoCapacityType(			"g_ammoCapacityType",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets ammo capacity type: 0 = Doom 3; 1 = Doom 1/2; 2 = Custom" );
idCVar g_ammoClipSizeType(			"g_ammoClipSizeType",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets ammo clip size type: 0 = Doom 3; 1 = Doom 1/2; 2 = Custom" );
idCVar g_ammoUsageType(				"g_ammoUsageType",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"enables realistic ammo usage when reloading and collecting ammo" );
// <---sikk

// sikk---> Weapon Management
idCVar g_weaponAwareness(			"g_weaponAwareness",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"toggles weapon awareness" );
idCVar g_weaponHandlingType(		"g_weaponHandlingType",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets the weapon handling type: 0 = spread (fixed); 1 = spread (variable); 2 spread (fixed) + recoil; 2 spread (variable) + recoil" );
idCVar g_weaponProjectileOrigin(	"g_weaponProjectileOrigin",		"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"sets whether projectiles are launched from their def defined origin or to launch all projectiles from the weapon's barrel: 0 = Default; 1 = Weapon Barrel" );
// <---sikk

// sikk---> First Person Body
idCVar g_grabMode(					"g_grabMode",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"enables the ability to grab moveable objects" );
// <---sikk


// sikk---> Disable Fall Damage
idCVar g_disableFallDamage(			"g_disableFallDamage",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"disables fall damage" );
// <---sikk

// sikk---> Player Speed Type
idCVar g_playerSpeedType(			"g_playerSpeedType",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets the player's speed configuration: 0 = Doom 3; 1 = Doom 1/2; 2 = Custom" );
// <---sikk

// sikk---> Locational Damage Type
idCVar g_damageType(				"g_damageType",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets damage value type: 0 = Doom 3; 1 = Doom 1/2; 2 = Custom" );
idCVar g_damageZoneType(			"g_damageZoneType",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets locational damage type: 0 = Doom 3; 1 = Doom 1/2; 2 = Custom" );
// <---sikk

// sikk---> Enemy Health Management
idCVar g_enemyHealthType(			"g_enemyHealthType",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets enemy health type: 0 = Doom 3; 1 = Doom 1/2; 2 = Custom" );
idCVar g_enemyHealthScale(			"g_enemyHealthScale",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the health scale for enemies" );
idCVar g_enemyHealthRandom(			"g_enemyHealthRandom",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"sets whether to randomize enemy health values" );
// <---sikk

// sikk---> Spectre Factor
idCVar g_enemySpectreFactor(		"g_enemySpectreFactor",			"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the factor that a Pinky demon will spawn as a Spectre" );
// <---sikk
// sikk---> Pain Elemental Factor
idCVar g_enemyPainElementalFactor(	"g_enemyPainElementalFactor",	"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the factor that a Cacodemon will spawn as a Pain Elemental" );
// <---sikk
// sikk---> Baron of Hell Factor
idCVar g_enemyBaronFactor(			"g_enemyBaronFactor",			"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"sets the factor that a Hellknight will spawn as a Baron of Hell" );
// <---sikk

// sikk---> Monster Burn Away Delay
idCVar g_burnAwayDelay(				"g_burnAwayDelay",				"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"enemy burn away delay. 0.0 = use default value" );
// <---sikk

// sikk---> Cyberdemon Damage Type
idCVar g_cyberdemonDamageType(		"g_cyberdemonDamageType",		"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"sets how the Cyberdemon can be damaged: 0 = Soul Cube only; 1 = All weapons" );
// <---sikk

// sikk---> Inter Rank Aggression
idCVar g_interRankAggression(		"g_interRankAggression",		"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"sets whether enemies of the same rank will fight each other" );
// <---sikk

// sikk---> Zombie Resurrection
idCVar g_zombieResurrectionLimit(	"g_zombieResurrectionLimit",	"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"sets the total number of times a zombie can resurrect. The chance for a zombie to resurrect is still randomized. 0 = Off" );
// <---sikk

// sikk---> Random Encounters System
idCVar g_useRandomEncounters(		"g_useRandomEncounters",		"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"enables random encounters" );
idCVar g_randomEncountersMaxSpawns(	"g_randomEncountersMaxSpawns",	"5",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"number of random enemies that can be alive at one time" );
idCVar g_randomEncountersMinTime(	"g_randomEncountersMinTime",	"30",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"minimum time (in secs) to wait before spawning another random enemy" );
idCVar g_randomEncountersMaxTime(	"g_randomEncountersMaxTime",	"60",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"maximum time (in secs) to wait before spawning another random enemy" );
idCVar g_randomEncountersDormantTime(	"g_randomEncountersDormantTime",	"10",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"maximum time (in secs) a random enemy can be dormant before it is removed" );
// <---sikk


// sikk---> Thirdperson Camera
idCVar pm_thirdPersonOffset(		"pm_thirdPersonOffset",			"0",		CVAR_GAME | CVAR_NETWORKSYNC | CVAR_ARCHIVE | CVAR_FLOAT,				"camera offset from player in 3rd person (-n = left, +n = right)" );
// <---sikk


// sikk---> PostProcess Effects
idCVar r_useSoftShadows(			"r_useSoftShadows",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable Soft Shadows postprocessing effect" );
idCVar r_softShadowsBlurFilter(		"r_softShadowsBlurFilter",		"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Blur method used for the shadow mask:\n0 = No Filter\n1 = Box Filter\n2 = Poisson Filter\n3 = Gaussian Filter" );
idCVar r_softShadowsBlurScale(		"r_softShadowsBlurScale",		"8.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Sample offset scale for the blur filter" );
idCVar r_softShadowsBlurEpsilon(	"r_softShadowsBlurEpsilon",		"4.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the blur depth difference factor for the blur filter" );

idCVar r_useEdgeAA(					"r_useEdgeAA",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Enable edge anti-aliasing: 0 = RGB edge AA; 1 = FXAA" );
idCVar r_edgeAASampleScale(			"r_edgeAASampleScale",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sample offset scale for edge detection" );
idCVar r_edgeAAFilterScale(			"r_edgeAAFilterScale",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the filter offset scale for blurring" );

idCVar r_useHDR(					"r_useHDR",						"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable High Dynamic Range lighting" );
idCVar r_hdrToneMapper(				"r_hdrToneMapper",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Tone mapping method:\n0 = Reinhard (RGB)\n1 = Reinhard (Yxy)\n2 = Exponential\n3 = Filmic (simple)\n4 = Filmic (complex)" );
idCVar r_hdrAdaptationRate(			"r_hdrAdaptationRate",			"60.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Eye adaptation rate" );
idCVar r_hdrMiddleGray(				"r_hdrMiddleGray",				"0.18",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Middle gray value used in HDR tone mapping" );
idCVar r_hdrWhitePoint(				"r_hdrWhitePoint",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Smallest luminance value that will be mapped to white" );
idCVar r_hdrBlueShiftFactor(		"r_hdrBlueShiftFactor",			"0.25",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Blue shift blend factor" );
idCVar r_hdrDither(					"r_hdrDither",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable dithering" );
idCVar r_hdrDitherSize(				"r_hdrDitherSize",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Sets the size of the dither threshold map" );
idCVar r_hdrLumThresholdMax(		"r_hdrLumThresholdMax",			"0.3",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Minimum luminance value threshold" );
idCVar r_hdrLumThresholdMin(		"r_hdrLumThresholdMin",			"0.1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Maximum luminance value threshold" );
idCVar r_hdrBloomToneMapper(		"r_hdrBloomToneMapper",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Tone mapping method, specific to bloom:\n0 = Reinhard (RGB)\n1 = Reinhard (Yxy)\n2 = Exponential\n3 = Filmic (simple)\n4 = Filmic (complex)" );
idCVar r_hdrBloomMiddleGray(		"r_hdrBloomMiddleGray",			"0.18",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Middle gray value used in HDR tone mapping, specific to bloom" );
idCVar r_hdrBloomWhitePoint(		"r_hdrBloomWhitePoint",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Smallest luminance value that will be mapped to white, specific to bloom" );
idCVar r_hdrBloomThreshold(			"r_hdrBloomThreshold",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Bloom luminance threshold" );
idCVar r_hdrBloomOffset(			"r_hdrBloomOffset",				"3.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Bloom luminance offset" );
idCVar r_hdrBloomScale(				"r_hdrBloomScale",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Intensity scale amount for bloom effect" );
idCVar r_hdrBloomSize(				"r_hdrBloomSize",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Size of the bloom effect" );
idCVar r_hdrFlareGamma(				"r_hdrFlareGamma",				"2.2",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Gamma curve for the flare texture" );
idCVar r_hdrFlareScale(				"r_hdrFlareScale",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Intensity scale amount for lens flare effect. 0 = Off" );
idCVar r_hdrFlareSize(				"r_hdrFlareSize",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Size of the lens flare effect" );
idCVar r_hdrGlareStyle(				"r_hdrGlareStyle",				"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Glare style to use with HDR lighting. Value range: 0 - 11\n0 = Off\n1 = Natural\n2 = Star\n3 = Cross\n4 = Snow Cross\n5 = Horizontal\n6 = Vertical\n7 = Star (Spectral)\n8 = Cross (Spectral)\n9 = Snow Cross (Spectral)\n10 = Horizontal (Spectral)\n11 = Vertical (Spectral)" );
idCVar r_hdrGlareScale(				"r_hdrGlareScale",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Intensity scale amount for glare effect" );
idCVar r_hdrGlareSize(				"r_hdrGlareSize",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Size of the glare effect" );

idCVar r_useBloom(					"r_useBloom",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable bloom postprocessing effect" );
idCVar r_bloomBufferSize(			"r_bloomBufferSize",			"3",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Bloom render target size:\n0 = 32x32\n1 = 64x64\n2 = 128x128\n3 = 256x256\n4 = 512x512\n5 = 1024x1024" );
idCVar r_bloomBlurIterations(		"r_bloomBlurIterations",		"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Number of times the blur filter is applied" );
idCVar r_bloomBlurScaleX(			"r_bloomBlurScaleX",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Amount to scale the X axis sample offsets" );
idCVar r_bloomBlurScaleY(			"r_bloomBlurScaleY",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Amount to scale the Y axis sample offsets" );
idCVar r_bloomScale(				"r_bloomScale",					"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Amount to scale the intensity of the bloom effect" );
idCVar r_bloomGamma(				"r_bloomGamma",					"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Gamma curve for the bloom texture" );

idCVar r_useSSIL(					"r_useSSIL",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable Screen-Space Indirect Lighting postprocessing effect" );
idCVar r_ssilRadius(				"r_ssilRadius",					"128",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sample radius for ssil" );
idCVar r_ssilAmount(				"r_ssilAmount",					"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the contribution factor for ssil" );
idCVar r_ssilBlurMethod(			"r_ssilBlurMethod",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Blur method used for the ssil buffer:\n0 = Gaussian\n1 = Bilateral" );
idCVar r_ssilBlurScale(				"r_ssilBlurScale",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the blur scale for the ssil buffer" );
idCVar r_ssilBlurQuality(			"r_ssilBlurQuality",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the blur quality for the ssil buffer" );
idCVar r_ssilBlurEpsilon(			"r_ssilBlurEpsilon",			"4",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the blur depth difference factor for the ssil buffer" );

idCVar r_useSSAO(					"r_useSSAO",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable Screen-Space Ambient Occlusion postprocessing effect" );
idCVar r_ssaoMethod(				"r_ssaoMethod",					"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the ssao method:\n0 = Crytek\n1 = HDAO\n2 = ABAO\n3 = PBAO\n4 = HBAO (low)\n5 = HBAO (medium)\n6 = HBAO (high)\n7 = Ray Marching\n8 = Volumetric Obscurance" );
idCVar r_ssaoRadius(				"r_ssaoRadius",					"16",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sample radius for ssao" );
idCVar r_ssaoBias(					"r_ssaoBias",					"0.075",	CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the angle bias for ssao (darkening factor for Crytek's)" );
idCVar r_ssaoAmount(				"r_ssaoAmount",					"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the contribution factor for ssao" );
idCVar r_ssaoBlurMethod(			"r_ssaoBlurMethod",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Blur method used for the ssao buffer:\n0 = Crytek\n1 = Box\n2 = Gaussian\n3 = Bilateral" );
idCVar r_ssaoBlurScale(				"r_ssaoBlurScale",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the blur scale for the ssao buffer" );
idCVar r_ssaoBlurQuality(			"r_ssaoBlurQuality",			"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the blur quality for the ssao buffer" );
idCVar r_ssaoBlurEpsilon(			"r_ssaoBlurEpsilon",			"16",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the blur depth difference factor for the ssao buffer" );
idCVar r_ssaoBlendPower(			"r_ssaoBlendPower",				"2.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the blend exponent for the ssao to scene final combine" );
idCVar r_ssaoBlendScale(			"r_ssaoBlendScale",				"2.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the blend scale for the ssao to scene final combine" );

idCVar r_useSunShafts(				"r_useSunShafts",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable Screen-Space Volumetric Lighting (Sun Shafts) postprocessing effect" );
idCVar r_sunShaftsSize(				"r_sunShaftsSize",				"16.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sun shafts size" );
idCVar r_sunShaftsStrength(			"r_sunShaftsStrength",			"2.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sun shafts strength" );
idCVar r_sunShaftsMaskStrength(		"r_sunShaftsMaskStrength",		"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sun shafts mask strength" );
idCVar r_sunShaftsQuality(			"r_sunShaftsQuality",			"4",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the sun shafts quality" );
idCVar r_sunOriginX(				"r_sunOriginX",					"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sun's origin along the X axis (used for sun shafts & lens flare)" );
idCVar r_sunOriginY(				"r_sunOriginY",					"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sun's origin along the Y axis (used for sun shafts & lens flare)" );
idCVar r_sunOriginZ(				"r_sunOriginZ",					"0.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the sun's origin along the Z axis (used for sun shafts & lens flare)" );
idCVar r_useLensFlare(				"r_useLensFlare",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable lens flare postprocessing effect" );
idCVar r_lensFlareStrength(			"r_lensFlareStrength",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the lens flare strength" );

idCVar r_useDepthOfField(			"r_useDepthOfField",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Enable depth of field postprocessing effect. Value range: 0 - 2 \n0 = Off\n1 = Automatic Focus\n2 = Manual Focus" );
idCVar r_dofBlurScale(				"r_dofBlurScale",				"4.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the blur scale for depth of field postprocessing effect" );
idCVar r_dofBlurQuality(			"r_dofBlurQuality",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the blur quality for depth of field postprocessing effect:\n0 = Box Filter\n1 = Poisson Filter\n2 = Gaussian Filter\n3 = Bokeh!" );
idCVar r_dofNear(					"r_dofNear",					"-128",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the near distance for depth of field postprocessing effect (r_useDepthOfField = 2 only)" );
idCVar r_dofFar(					"r_dofFar",						"1024",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the far distance for depth of field postprocessing effect (r_useDepthOfField = 2 only)" );
idCVar r_dofFocus(					"r_dofFocus",					"128",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the focus distance for depth of field postprocessing effect (r_useDepthOfField = 2 only)" );
idCVar r_dofConditionAlways(		"r_dofConditionAlways",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Depth of field condition: Always on" );
idCVar r_dofConditionCinematic(		"r_dofConditionCinematic",		"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Depth of field condition: Cinematics" );
idCVar r_dofConditionGUI(			"r_dofConditionGUI",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Depth of field condition: GUI Active" );
idCVar r_dofConditionReload(		"r_dofConditionReload",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Depth of field condition: Reloading" );
idCVar r_dofConditionTalk(			"r_dofConditionTalk",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Depth of field condition: Talking" );
idCVar r_dofConditionZoom(			"r_dofConditionZoom",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Depth of field condition: Weapon Zoom" );

idCVar r_useMotionBlur(				"r_useMotionBlur",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Enable motion blur postprocessing effect.\n0 = off\n1 = Camera\n2 = Accumulation\n3 = Camera + Accumulation" );
idCVar r_motionBlurScale(			"r_motionBlurScale",			"0.1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the motion blur scale. Works similar to shutter speed. Higher values == stronger blurring" );
idCVar r_motionBlurMaskDistance(	"r_motionBlurMaskDistance",		"32",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Don't do motion blur if framerate is below this value" );
idCVar r_motionBlurFPSThreshold(	"r_motionBlurFPSThreshold",		"20",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the motion blur mask distance. Used to mask the view weapon." );
idCVar r_motionBlurMinThreshold(	"r_motionBlurMinThreshold",		"10",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the motion blur min sensitivity threshold. Screen won't blur until threshold is passed. Lower values == higher sensitivity" );
idCVar r_motionBlurMaxThreshold(	"r_motionBlurMaxThreshold",		"45",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the motion blur max sensitivity threshold. Blur strength won't pass threshold." );
idCVar r_motionBlurFactor(			"r_motionBlurFactor",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the motion blur blend factor" );
idCVar r_motionBlurLerp(			"r_motionBlurLerp",				"0.5",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the motion blur blend lerp factor" );
idCVar r_motionBlurQuality(			"r_motionBlurQuality",			"1",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the motion blur quality. Value range: 1 - 4\n1 = 8 samples\n2 = 64 samples (virtual)\n3 = 512 samples (virtual)\n4 = 4096 samples (virtual)" );

idCVar r_useColorGrading(			"r_useColorGrading",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable color grading postprocessing effect. Parameters need to be set manually in the material." );
idCVar r_colorGradingParm(			"r_colorGradingParm",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Parameter to allow specific color grading stage to be used in the material" );
idCVar r_colorGradingType(			"r_colorGradingType",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Color grading type:\n0 = math\n1 = lut" );
idCVar r_colorGradingSharpness(		"r_colorGradingSharpness",		"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Sharpness level when color grading is enabled" );

idCVar r_useCelShading(				"r_useCelShading",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable cel shading postprocessing effect" );
idCVar r_celShadingMethod(			"r_celShadingMethod",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the cel shading edge detection method. Value range: 0 - 2 \n0 = RGB\n1 = Luminance\n2 = Depth(incomplete)" );
idCVar r_celShadingScale(			"r_celShadingScale",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the cel shading scale. Higher values == thicker outline" );
idCVar r_celShadingThreshold(		"r_celShadingThreshold",		"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the cel shading threshold" );

idCVar r_useFilmgrain(				"r_useFilmgrain",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable filmgrain postprocessing effect" );
idCVar r_filmgrainBlendMode(		"r_filmgrainBlendMode",			"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_INTEGER,	"Set the grain blending mode. Value range: 0 - 3\n0 = gl_one, gl_one\n1 = gl_zero, gl_one_minus_src_color\n2 = gl_dst_color, gl_one\n3 = gl_one_minus_dst_color, gl_one" );
idCVar r_filmgrainScale(			"r_filmgrainScale",				"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the grain scale" );
idCVar r_filmgrainStrength(			"r_filmgrainStrength",			"1.0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_FLOAT,	"Set the grain strength. Value range: 0.0 - 1.0" );

idCVar r_useVignetting(				"r_useVignetting",				"0",		CVAR_GAME | CVAR_NOCHEAT | CVAR_ARCHIVE | CVAR_BOOL,	"Enable vignetting postprocessing effect" );
// <---sikk
