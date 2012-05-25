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

#ifndef __SYS_CVAR_H__
#define __SYS_CVAR_H__

#include "framework/CVarSystem.h"

extern idCVar	developer;

extern idCVar	g_cinematic;
extern idCVar	g_cinematicMaxSkipTime;

extern idCVar	r_aspectRatio;

extern idCVar	g_monsters;
extern idCVar	g_decals;
extern idCVar	g_knockback;
extern idCVar	g_skill;
extern idCVar	g_gravity;
extern idCVar	g_skipFX;
extern idCVar	g_skipParticles;
extern idCVar	g_bloodEffects;
extern idCVar	g_projectileLights;
extern idCVar	g_doubleVision;
extern idCVar	g_muzzleFlash;

extern idCVar	g_disasm;
extern idCVar	g_debugBounds;
extern idCVar	g_debugAnim;
extern idCVar	g_debugMove;
extern idCVar	g_debugDamage;
extern idCVar	g_debugWeapon;
extern idCVar	g_debugScript;
extern idCVar	g_debugMover;
extern idCVar	g_debugTriggers;
extern idCVar	g_debugCinematic;
extern idCVar	g_stopTime;
extern idCVar	g_armorProtection;
extern idCVar	g_armorProtectionMP;
extern idCVar	g_damageScale;
extern idCVar	g_useDynamicProtection;
extern idCVar	g_healthTakeTime;
extern idCVar	g_healthTakeAmt;
extern idCVar	g_healthTakeLimit;

extern idCVar	g_showPVS;
extern idCVar	g_showTargets;
extern idCVar	g_showTriggers;
extern idCVar	g_showCollisionWorld;
extern idCVar	g_showCollisionModels;
extern idCVar	g_showCollisionTraces;
extern idCVar	g_maxShowDistance;
extern idCVar	g_showEntityInfo;
extern idCVar	g_showviewpos;
extern idCVar	g_showcamerainfo;
extern idCVar	g_showTestModelFrame;
extern idCVar	g_showActiveEntities;
extern idCVar	g_showEnemies;

extern idCVar	g_frametime;
extern idCVar	g_timeentities;

extern idCVar	ai_debugScript;
extern idCVar	ai_debugMove;
extern idCVar	ai_debugTrajectory;
extern idCVar	ai_testPredictPath;
extern idCVar	ai_showCombatNodes;
extern idCVar	ai_showPaths;
extern idCVar	ai_showObstacleAvoidance;
extern idCVar	ai_blockedFailSafe;

extern idCVar	g_dvTime;
extern idCVar	g_dvAmplitude;
extern idCVar	g_dvFrequency;

extern idCVar	g_kickTime;
extern idCVar	g_kickAmplitude;
extern idCVar	g_blobTime;
extern idCVar	g_blobSize;

extern idCVar	g_testHealthVision;
extern idCVar	g_editEntityMode;
extern idCVar	g_dragEntity;
extern idCVar	g_dragDamping;
extern idCVar	g_dragShowSelection;
extern idCVar	g_dropItemRotation;

extern idCVar	g_vehicleVelocity;
extern idCVar	g_vehicleForce;
extern idCVar	g_vehicleSuspensionUp;
extern idCVar	g_vehicleSuspensionDown;
extern idCVar	g_vehicleSuspensionKCompress;
extern idCVar	g_vehicleSuspensionDamping;
extern idCVar	g_vehicleTireFriction;

extern idCVar	ik_enable;
extern idCVar	ik_debug;

extern idCVar	af_useLinearTime;
extern idCVar	af_useImpulseFriction;
extern idCVar	af_useJointImpulseFriction;
extern idCVar	af_useSymmetry;
extern idCVar	af_skipSelfCollision;
extern idCVar	af_skipLimits;
extern idCVar	af_skipFriction;
extern idCVar	af_forceFriction;
extern idCVar	af_maxLinearVelocity;
extern idCVar	af_maxAngularVelocity;
extern idCVar	af_timeScale;
extern idCVar	af_jointFrictionScale;
extern idCVar	af_contactFrictionScale;
extern idCVar	af_highlightBody;
extern idCVar	af_highlightConstraint;
extern idCVar	af_showTimings;
extern idCVar	af_showConstraints;
extern idCVar	af_showConstraintNames;
extern idCVar	af_showConstrainedBodies;
extern idCVar	af_showPrimaryOnly;
extern idCVar	af_showTrees;
extern idCVar	af_showLimits;
extern idCVar	af_showBodies;
extern idCVar	af_showBodyNames;
extern idCVar	af_showMass;
extern idCVar	af_showTotalMass;
extern idCVar	af_showInertia;
extern idCVar	af_showVelocity;
extern idCVar	af_showActive;
extern idCVar	af_testSolid;

extern idCVar	rb_showTimings;
extern idCVar	rb_showBodies;
extern idCVar	rb_showMass;
extern idCVar	rb_showInertia;
extern idCVar	rb_showVelocity;
extern idCVar	rb_showActive;

extern idCVar	pm_jumpheight;
extern idCVar	pm_stepsize;
extern idCVar	pm_crouchspeed;
extern idCVar	pm_walkspeed;
extern idCVar	pm_runspeed;
extern idCVar	pm_noclipspeed;
extern idCVar	pm_spectatespeed;
extern idCVar	pm_spectatebbox;
extern idCVar	pm_usecylinder;
extern idCVar	pm_minviewpitch;
extern idCVar	pm_maxviewpitch;
extern idCVar	pm_stamina;
extern idCVar	pm_staminathreshold;
extern idCVar	pm_staminarate;
extern idCVar	pm_crouchheight;
extern idCVar	pm_crouchviewheight;
extern idCVar	pm_normalheight;
extern idCVar	pm_normalviewheight;
extern idCVar	pm_deadheight;
extern idCVar	pm_deadviewheight;
extern idCVar	pm_crouchrate;
extern idCVar	pm_bboxwidth;
extern idCVar	pm_crouchbob;
extern idCVar	pm_walkbob;
extern idCVar	pm_runbob;
extern idCVar	pm_runpitch;
extern idCVar	pm_runroll;
extern idCVar	pm_bobup;
extern idCVar	pm_bobpitch;
extern idCVar	pm_bobroll;
extern idCVar	pm_thirdPersonRange;
extern idCVar	pm_thirdPersonHeight;
extern idCVar	pm_thirdPersonAngle;
extern idCVar	pm_thirdPersonClip;
extern idCVar	pm_thirdPerson;
extern idCVar	pm_thirdPersonDeath;
extern idCVar	pm_modelView;
extern idCVar	pm_airTics;

extern idCVar	g_showPlayerShadow;
extern idCVar	g_showHud;
extern idCVar	g_showProjectilePct;
extern idCVar	g_showBrass;
extern idCVar	g_gun_x;
extern idCVar	g_gun_y;
extern idCVar	g_gun_z;
extern idCVar	g_viewNodalX;
extern idCVar	g_viewNodalZ;
extern idCVar	g_fov;
extern idCVar	g_testDeath;
extern idCVar	g_skipViewEffects;
extern idCVar   g_mpWeaponAngleScale;

extern idCVar	g_testParticle;
extern idCVar	g_testParticleName;

extern idCVar	g_testPostProcess;

extern idCVar	g_testModelRotate;
extern idCVar	g_testModelAnimate;
extern idCVar	g_testModelBlend;
extern idCVar	g_exportMask;
extern idCVar	g_flushSave;

extern idCVar	aas_test;
extern idCVar	aas_showAreas;
extern idCVar	aas_showPath;
extern idCVar	aas_showFlyPath;
extern idCVar	aas_showWallEdges;
extern idCVar	aas_showHideArea;
extern idCVar	aas_pullPlayer;
extern idCVar	aas_randomPullPlayer;
extern idCVar	aas_goalArea;
extern idCVar	aas_showPushIntoArea;

extern idCVar	net_clientPredictGUI;

extern idCVar	g_voteFlags;
extern idCVar	g_mapCycle;
extern idCVar	g_balanceTDM;

extern idCVar	si_timeLimit;
extern idCVar	si_fragLimit;
extern idCVar	si_gameType;
extern idCVar	si_map;
extern idCVar	si_spectators;

extern idCVar	net_clientSelfSmoothing;
extern idCVar	net_clientLagOMeter;

extern const char *si_gameTypeArgs[];

extern const char *ui_skinArgs[];


// sikk - New Cvars -
//-------------------------------------------------
// sikk---> Crosshair Cvars
extern idCVar	g_crosshair;
extern idCVar	g_crosshairType;
extern idCVar	g_crosshairLerp;
// <---sikk

// sikk---> Hud management
extern idCVar	g_hudType;
extern idCVar	g_useDynamicHud;
extern idCVar	g_dynamicHudTime;
// <---sikk

// sikk---> IR Goggles/Headlight Mod
extern idCVar	g_goggleType;
extern idCVar	g_batteryLife;
extern idCVar	g_batteryRechargeRate;
// <---sikk

// sikk---> Global Ambient Light
extern idCVar	g_useAmbientLight;
extern idCVar	g_ambientLightRadius;
extern idCVar	g_ambientLightColor;
// <---sikk

// sikk---> Explosion FX
extern idCVar	g_useExplosionFX;
extern idCVar	g_explosionFXTime;
extern idCVar	g_explosionFXScale;
// <---sikk

// sikk---> Blood Spray Screen Effect
extern idCVar	g_showBloodSpray;
extern idCVar	g_bloodSprayTime;
extern idCVar	g_bloodSprayDistance;
extern idCVar	g_bloodSprayFrequency;
// <---sikk

extern idCVar	g_screenFrostTime;		// sikk - Screen Frost

extern idCVar	g_tracerFrequency;		// sikk - Tracer Frequency

extern idCVar	g_playerHeadType;		// sikk - Player Head Type

extern idCVar	g_showFirstPersonBody;	// sikk - First Person Body

extern idCVar	g_enablePortalSky;		// sikk - Portal Sky Box


// sikk---> Health Management System
extern idCVar	g_healthManagementType;
extern idCVar	g_healthPackTotal;
extern idCVar	g_healthPackUses;
extern idCVar	g_healthPackTime;
extern idCVar	g_healthRegenTime;
extern idCVar	g_healthRegenDelay;
extern idCVar	g_healthRegenAmt;
extern idCVar	g_healthRegenLimit;
extern idCVar	g_healthRegenSteps;
extern idCVar	g_healthRegenFeedback;
// <---sikk

// sikk---> Item Management
extern idCVar	g_itemPickupType;
extern idCVar	g_itemMaxArmorType;
extern idCVar	g_itemHelmetFactor;
extern idCVar	g_itemValueFactor;
extern idCVar	g_itemRemovalFactor;
extern idCVar	g_itemSearchFactor;
// <---sikk

// sikk---> Ammo Management
extern idCVar	g_ammoCapacityType;
extern idCVar	g_ammoClipSizeType;
extern idCVar	g_ammoUsageType;
// <---sikk

// sikk---> Weapon Management
extern idCVar	g_weaponAwareness;
extern idCVar	g_weaponHandlingType;
extern idCVar	g_weaponProjectileOrigin;
// <---sikk

extern idCVar	g_grabMode;					// sikk - Object Manipulation

extern idCVar	g_disableFallDamage;		// sikk - Disable Fall Damage

extern idCVar	g_playerSpeedType;			// sikk - Player Speed Type

extern idCVar	g_damageType;				// sikk - Damage Type
extern idCVar	g_damageZoneType;			// sikk - Locational Damage Type

// sikk---> Enemy Health Management
extern idCVar	g_enemyHealthType;
extern idCVar	g_enemyHealthScale;
extern idCVar	g_enemyHealthRandom;
// <---sikk

extern idCVar	g_enemySpectreFactor;		// sikk - Spectre Factor
extern idCVar	g_enemyPainElementalFactor;	// sikk - Pain Elemental Factor
extern idCVar	g_enemyBaronFactor;			// sikk - Baron of Hell Factor

extern idCVar	g_burnAwayDelay;			// sikk - Monster Burn Away Delay

extern idCVar	g_cyberdemonDamageType;		// sikk - Cyberdemon Damage Type

extern idCVar	g_interRankAggression;		// sikk - Inter Rank Aggression

extern idCVar	g_zombieResurrectionLimit;	// sikk - Zombie Resurrection

// sikk---> Random Encounters System
extern idCVar	g_useRandomEncounters;
extern idCVar	g_randomEncountersMaxSpawns;
extern idCVar	g_randomEncountersMinTime;
extern idCVar	g_randomEncountersMaxTime;
extern idCVar	g_randomEncountersDormantTime;
// <---sikk


extern idCVar	pm_thirdPersonOffset;		// sikk - Thirdpesron Camera


// sikk---> PostProcess Effects
extern idCVar	r_useSoftShadows;
extern idCVar	r_softShadowsBlurFilter;
extern idCVar	r_softShadowsBlurScale;
extern idCVar	r_softShadowsBlurEpsilon;

extern idCVar	r_useEdgeAA;
extern idCVar	r_edgeAASampleScale;
extern idCVar	r_edgeAAFilterScale;

extern idCVar	r_useHDR;
extern idCVar	r_hdrToneMapper;
extern idCVar	r_hdrAdaptationRate;
extern idCVar	r_hdrMiddleGray;
extern idCVar	r_hdrWhitePoint;
extern idCVar	r_hdrBlueShiftFactor;
extern idCVar	r_hdrDither;
extern idCVar	r_hdrDitherSize;
extern idCVar	r_hdrLumThresholdMax;
extern idCVar	r_hdrLumThresholdMin;
extern idCVar	r_hdrBloomToneMapper;
extern idCVar	r_hdrBloomMiddleGray;
extern idCVar	r_hdrBloomWhitePoint;
extern idCVar	r_hdrBloomThreshold;
extern idCVar	r_hdrBloomOffset;
extern idCVar	r_hdrBloomScale;
extern idCVar	r_hdrBloomSize;
extern idCVar	r_hdrFlareGamma;
extern idCVar	r_hdrFlareScale;
extern idCVar	r_hdrFlareSize;
extern idCVar	r_hdrGlareStyle;
extern idCVar	r_hdrGlareScale;
extern idCVar	r_hdrGlareSize;

extern idCVar	r_useBloom;
extern idCVar	r_bloomBufferSize;
extern idCVar	r_bloomBlurIterations;
extern idCVar	r_bloomBlurScaleX;
extern idCVar	r_bloomBlurScaleY;
extern idCVar	r_bloomScale;
extern idCVar	r_bloomGamma;

extern idCVar	r_useSSIL;
extern idCVar	r_ssilRadius;
extern idCVar	r_ssilAmount;
extern idCVar	r_ssilBlurMethod;
extern idCVar	r_ssilBlurScale;
extern idCVar	r_ssilBlurQuality;
extern idCVar	r_ssilBlurEpsilon;

extern idCVar	r_useSSAO;
extern idCVar	r_ssaoMethod;
extern idCVar	r_ssaoRadius;
extern idCVar	r_ssaoAmount;
extern idCVar	r_ssaoBias;
extern idCVar	r_ssaoBlurMethod;
extern idCVar	r_ssaoBlurScale;
extern idCVar	r_ssaoBlurQuality;
extern idCVar	r_ssaoBlurEpsilon;
extern idCVar	r_ssaoBlendPower;
extern idCVar	r_ssaoBlendScale;

extern idCVar	r_useSunShafts;
extern idCVar	r_sunShaftsSize;
extern idCVar	r_sunShaftsStrength;
extern idCVar	r_sunShaftsMaskStrength;
extern idCVar	r_sunShaftsQuality;
extern idCVar	r_sunOriginX;
extern idCVar	r_sunOriginY;
extern idCVar	r_sunOriginZ;
extern idCVar	r_useLensFlare;
extern idCVar	r_lensFlareStrength;

extern idCVar	r_useMotionBlur;
extern idCVar	r_motionBlurScale;
extern idCVar	r_motionBlurMaskDistance;
extern idCVar	r_motionBlurFPSThreshold;
extern idCVar	r_motionBlurMinThreshold;
extern idCVar	r_motionBlurMaxThreshold;
extern idCVar	r_motionBlurFactor;
extern idCVar	r_motionBlurLerp;
extern idCVar	r_motionBlurQuality;

extern idCVar	r_useDepthOfField;
extern idCVar	r_dofBlurScale;
extern idCVar	r_dofBlurQuality;
extern idCVar	r_dofNear;
extern idCVar	r_dofFar;
extern idCVar	r_dofFocus;
extern idCVar	r_dofConditionAlways;
extern idCVar	r_dofConditionCinematic;
extern idCVar	r_dofConditionGUI;
extern idCVar	r_dofConditionTalk;
extern idCVar	r_dofConditionReload;
extern idCVar	r_dofConditionZoom;

extern idCVar	r_useColorGrading;
extern idCVar	r_colorGradingParm;
extern idCVar	r_colorGradingType;
extern idCVar	r_colorGradingSharpness;

extern idCVar	r_useCelShading;
extern idCVar	r_celShadingMethod;
extern idCVar	r_celShadingScale;
extern idCVar	r_celShadingThreshold;

extern idCVar	r_useFilmgrain;
extern idCVar	r_filmgrainBlendMode;
extern idCVar	r_filmgrainScale;
extern idCVar	r_filmgrainStrength;

extern idCVar	r_useVignetting;
// <---sikk

#endif /* !__SYS_CVAR_H__ */
