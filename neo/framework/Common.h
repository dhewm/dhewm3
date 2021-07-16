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

#ifndef __COMMON_H__
#define __COMMON_H__

#include "framework/CVarSystem.h"

/*
==============================================================

  Common

==============================================================
*/

typedef enum {
	EDITOR_NONE					= 0,
	EDITOR_RADIANT				= BIT(1),
	EDITOR_GUI					= BIT(2),
	EDITOR_DEBUGGER				= BIT(3),
	EDITOR_SCRIPT				= BIT(4),
	EDITOR_LIGHT				= BIT(5),
	EDITOR_SOUND				= BIT(6),
	EDITOR_DECL					= BIT(7),
	EDITOR_AF					= BIT(8),
	EDITOR_PARTICLE				= BIT(9),
	EDITOR_PDA					= BIT(10),
	EDITOR_AAS					= BIT(11),
	EDITOR_MATERIAL				= BIT(12)
} toolFlag_t;

#define STRTABLE_ID				"#str_"
#define STRTABLE_ID_LENGTH		5

extern idCVar		com_version;
extern idCVar		com_skipRenderer;
extern idCVar		com_asyncInput;
extern idCVar		com_asyncSound;
extern idCVar		com_machineSpec;
extern idCVar		com_purgeAll;
extern idCVar		com_developer;
extern idCVar		com_allowConsole;
extern idCVar		com_speeds;
extern idCVar		com_showFPS;
extern idCVar		com_showMemoryUsage;
extern idCVar		com_showAsyncStats;
extern idCVar		com_showSoundDecoders;
extern idCVar		com_makingBuild;
extern idCVar		com_updateLoadSize;
extern idCVar		com_enableDebuggerServer;
extern idCVar		com_dbgClientAdr;
extern idCVar		com_dbgServerAdr;

extern int			time_gameFrame;			// game logic time
extern int			time_gameDraw;			// game present time
extern int			time_frontend;			// renderer frontend time
extern int			time_backend;			// renderer backend time

extern int			com_frameTime;			// time for the current frame in milliseconds
extern volatile int	com_ticNumber;			// 60 hz tics, incremented by async function
extern int			com_editors;			// current active editor(s)
extern bool			com_editorActive;		// true if an editor has focus

extern bool			com_debuggerSupported;	// only set to true when the updateDebugger function is set. see GetAdditionalFunction()

#ifdef _WIN32
const char			DMAP_MSGID[] = "DMAPOutput";
const char			DMAP_DONE[] = "DMAPDone";
extern HWND			com_hwndMsg;
extern bool			com_outputMsg;
#endif

struct MemInfo_t {
	idStr			filebase;

	int				total;
	int				assetTotals;

	// memory manager totals
	int				memoryManagerTotal;

	// subsystem totals
	int				gameSubsystemTotal;
	int				renderSubsystemTotal;

	// asset totals
	int				imageAssetsTotal;
	int				modelAssetsTotal;
	int				soundAssetsTotal;
};

class idLangDict;
class idInterpreter;
class idProgram;

class idCommon {
public:
	virtual						~idCommon( void ) {}

								// Initialize everything.
								// if the OS allows, pass argc/argv directly (without executable name)
								// otherwise pass the command line in a single string (without executable name)
	virtual void				Init( int argc, char **argv ) = 0;

								// Shuts down everything.
	virtual void				Shutdown( void ) = 0;

								// Shuts down everything.
	virtual void				Quit( void ) = 0;

								// Returns true if common initialization is complete.
	virtual bool				IsInitialized( void ) const = 0;

								// Called repeatedly as the foreground thread for rendering and game logic.
	virtual void				Frame( void ) = 0;

								// Called repeatedly by blocking function calls with GUI interactivity.
	virtual void				GUIFrame( bool execCmd, bool network ) = 0;

								// Called 60 times a second from a background thread for sound mixing,
								// and input generation. Not called until idCommon::Init() has completed.
	virtual void				Async( void ) = 0;

								// Checks for and removes command line "+set var arg" constructs.
								// If match is NULL, all set commands will be executed, otherwise
								// only a set with the exact name.  Only used during startup.
								// set once to clear the cvar from +set for early init code
	virtual void				StartupVariable( const char *match, bool once ) = 0;

								// Initializes a tool with the given dictionary.
	virtual void				InitTool( const toolFlag_t tool, const idDict *dict ) = 0;

								// Activates or deactivates a tool.
	virtual void				ActivateTool( bool active ) = 0;

								// Writes the user's configuration to a file
	virtual void				WriteConfigToFile( const char *filename ) = 0;

								// Writes cvars with the given flags to a file.
	virtual void				WriteFlaggedCVarsToFile( const char *filename, int flags, const char *setCmd ) = 0;


								// Begins redirection of console output to the given buffer.
	virtual void				BeginRedirect( char *buffer, int buffersize, void (*flush)( const char * ) ) = 0;

								// Stops redirection of console output.
	virtual void				EndRedirect( void ) = 0;

								// Update the screen with every message printed.
	virtual void				SetRefreshOnPrint( bool set ) = 0;

								// Prints message to the console, which may cause a screen update if com_refreshOnPrint is set.
	virtual void				Printf( const char *fmt, ... )id_attribute((format(printf,2,3))) = 0;

								// Same as Printf, with a more usable API - Printf pipes to this.
	virtual void				VPrintf( const char *fmt, va_list arg ) = 0;

								// Prints message that only shows up if the "developer" cvar is set,
								// and NEVER forces a screen update, which could cause reentrancy problems.
	virtual void				DPrintf( const char *fmt, ... ) id_attribute((format(printf,2,3))) = 0;

								// Prints WARNING %s message and adds the warning message to a queue for printing later on.
	virtual void				Warning( const char *fmt, ... ) id_attribute((format(printf,2,3))) = 0;

								// Prints WARNING %s message in yellow that only shows up if the "developer" cvar is set.
	virtual void				DWarning( const char *fmt, ...) id_attribute((format(printf,2,3))) = 0;

								// Prints all queued warnings.
	virtual void				PrintWarnings( void ) = 0;

								// Removes all queued warnings.
	virtual void				ClearWarnings( const char *reason ) = 0;

								// Issues a C++ throw. Normal errors just abort to the game loop,
								// which is appropriate for media or dynamic logic errors.
	virtual void				Error( const char *fmt, ... ) id_attribute((format(printf,2,3))) = 0;

								// Fatal errors quit all the way to a system dialog box, which is appropriate for
								// static internal errors or cases where the system may be corrupted.
	virtual void				FatalError( const char *fmt, ... ) id_attribute((format(printf,2,3))) = 0;

								// Returns a pointer to the dictionary with language specific strings.
	virtual const idLangDict *	GetLanguageDict( void ) = 0;

								// Returns key bound to the command
	virtual const char *		KeysFromBinding( const char *bind ) = 0;

								// Returns the binding bound to the key
	virtual const char *		BindingFromKey( const char *key ) = 0;

								// Directly sample a button.
	virtual int					ButtonState( int key ) = 0;

								// Directly sample a keystate.
	virtual int					KeyState( int key ) = 0;

	/* Some Mods (like Ruiner and DarkMod when it still was a mod) used "SourceHook"
	 * to override Doom3 Methods to call their own code before the original method
	 * was executed.. this is super ugly and probably not super portable either.
	 *
	 * So let's offer something that's slightly less ugly: A function pointer based
	 * interface to provide similar (but known!) hacks.
	 * For example, Ruiner used SourceHook to intercept idCmdSystem::BufferCommandText()
	 * and recreate some cooked rendering data in case reloadImages or vid_restart was executed.
	 * Now, instead of doing ugly hacks with SourceHook, Ruiner can just call
	 *   common->SetCallback( idCommon::CB_ReloadImages,
	 *                        (idCommon::FunctionPointer)functionToCall,
	 *                        (void*)argForFunctionToCall );
	 *
	 * (the Mod needs to check if SetCallback() returned true; if it didn't the used version
	 *  of dhewm3 doesn't support the given CallBackType and the Mod must either error out
	 *  or handle the case that the callback doesn't work)
	 *
	 * Of course this means that for every new SourceHook hack a Mod (that's ported to dhewm3)
	 * uses, a corresponding entry must be added to enum CallbackType and it must be handled,
	 * which implies that the Mod will only properly work with the latest dhewm3 git code
	 * or the next release..
	 * I guess most mods don't need this hack though, so I think it's feasible.
	 *
	 * Note that this allows adding new types of callbacks without breaking the API and ABI
	 * between dhewm3 and the Game DLLs; the alternative would be something like
	 * idCommon::RegisterReloadImagesCallback(), and maybe other similar methods later, which
	 * would break the ABI and API each time and all Mods would have to be adjusted, even if
	 * they don't even need that functionality (because they never needed SourceHook or similar).
	 *
	 * Similar to SetCallback() I've also added GetAdditionalFunction() to get a function pointer
	 * from dhewm3 that Mods can call (and that's not exported via the normal interface classes).
	 * Right now it's only used for a Doom3 Demo specific hack only relevant for base.dll (not for Mods)
	 */

	typedef void* (*FunctionPointer)(void*); // needs to be cast to/from real type!
	enum CallbackType {
		// called on reloadImages and vid_restart commands (before anything "real" happens)
		// expecting callback to be like void cb(void* userarg, const idCmdArgs& cmdArgs)
		// where cmdArgs contains the command+arguments that was called
		CB_ReloadImages = 1,
	};

	// returns true if setting the callback was successful, else false
	// When a game DLL is unloaded the callbacks are automatically removed from the Engine
	// so you usually don't have to worry about that; but you can call this with cb = NULL
	// and userArg = NULL to remove a callback manually (e.g. if userArg refers to an object you deleted)
	virtual bool				SetCallback(CallbackType cbt, FunctionPointer cb, void* userArg) = 0;

	enum FunctionType {
		// the function's signature is bool fn(void) - no arguments.
		// it returns true if we're currently running the doom3 demo
		// not relevant for mods, only for game/ aka base.dll/base.so/...
		FT_IsDemo = 1,
		// the function's signature is bool fn(idInterpreter,idProgram,int) with arguments:
		// idInterpreter *interpreter, idProgram *program, int instructionPointer
		// it returns true if the game debugger is active.
		// relevant for mods.
		FT_UpdateDebugger,
	};

	// returns true if that function is available in this version of dhewm3
	// *out_fnptr will be the function (you'll have to cast it probably)
	// *out_userArg will be an argument you have to pass to the function, if appropriate (else NULL)
	virtual bool				GetAdditionalFunction(FunctionType ft, FunctionPointer* out_fnptr, void** out_userArg) = 0;
};

extern idCommon *		common;

#endif /* !__COMMON_H__ */
