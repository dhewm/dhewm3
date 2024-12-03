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
along with Doom 3 Source Code.	If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/sys_sdl.h"
#include "sys/platform.h"
#include "idlib/containers/List.h"
#include "idlib/Heap.h"
#include "framework/Common.h"
#include "framework/Console.h"
#include "framework/KeyInput.h"
#include "framework/Session_local.h"
#include "renderer/RenderSystem.h"
#include "renderer/tr_local.h"

#include "sys/sys_public.h"
#include "sys/sys_imgui.h"

#if !SDL_VERSION_ATLEAST(2, 0, 0)
#define SDL_Keycode SDLKey
#define SDLK_APPLICATION SDLK_COMPOSE
#define SDLK_SCROLLLOCK SDLK_SCROLLOCK
#define SDLK_LGUI SDLK_LSUPER
#define SDLK_RGUI SDLK_RSUPER
#define SDLK_KP_0 SDLK_KP0
#define SDLK_KP_1 SDLK_KP1
#define SDLK_KP_2 SDLK_KP2
#define SDLK_KP_3 SDLK_KP3
#define SDLK_KP_4 SDLK_KP4
#define SDLK_KP_5 SDLK_KP5
#define SDLK_KP_6 SDLK_KP6
#define SDLK_KP_7 SDLK_KP7
#define SDLK_KP_8 SDLK_KP8
#define SDLK_KP_9 SDLK_KP9
#define SDLK_NUMLOCKCLEAR SDLK_NUMLOCK
#define SDLK_PRINTSCREEN SDLK_PRINT
#endif

// unholy hacks to make it work with SDL3 (and also SDL2 and partly SDL1.2)
#if SDL_VERSION_ATLEAST(3, 0, 0)
// in SDL3 SDL_GetKeyFromScancode() got additional arguments that I don't care about
#define SDL_GetKeyFromScancode(SC)  SDL_GetKeyFromScancode(SC, 0, true)

#define SDLK_z SDLK_Z

#define SDL_TEXTINPUT SDL_EVENT_TEXT_INPUT
#define SDL_KEYDOWN SDL_EVENT_KEY_DOWN
#define SDL_KEYUP SDL_EVENT_KEY_UP

#define SDL_USEREVENT SDL_EVENT_USER
#define SDL_QUIT SDL_EVENT_QUIT

#define SDL_MOUSEBUTTONDOWN SDL_EVENT_MOUSE_BUTTON_DOWN
#define SDL_MOUSEBUTTONUP SDL_EVENT_MOUSE_BUTTON_UP
#define SDL_MOUSEMOTION SDL_EVENT_MOUSE_MOTION
#define SDL_MOUSEWHEEL SDL_EVENT_MOUSE_WHEEL

#define SDL_JoystickGUID SDL_GUID
#define SDL_JoystickGetGUID SDL_GetJoystickGUID
#define SDL_JoystickName SDL_GetJoystickName
#define SDL_JoystickGetGUIDString SDL_GUIDToString

#define SDL_JOYDEVICEADDED SDL_EVENT_JOYSTICK_ADDED
#define SDL_JOYDEVICEREMOVED SDL_EVENT_JOYSTICK_REMOVED

// game controllers have been renamed to gamepads and the
// corresponding SDL_Event members are now prefixed with g instead of c
#define cbutton gbutton
#define caxis gaxis

#define SDL_GameController SDL_Gamepad
#define SDL_GameControllerAxis SDL_GamepadAxis
#define SDL_GameControllerButton SDL_GamepadButton

#define SDL_GameControllerGetType SDL_GetGamepadType
#define SDL_GameControllerName SDL_GetGamepadName
#define SDL_GameControllerGetJoystick SDL_GetGamepadJoystick
#define SDL_GameControllerGetVendor SDL_GetGamepadVendor
#define SDL_GameControllerGetProduct SDL_GetGamepadProduct
#define SDL_GameControllerOpen SDL_OpenGamepad

#define SDL_CONTROLLERAXISMOTION SDL_EVENT_GAMEPAD_AXIS_MOTION
#define SDL_CONTROLLERBUTTONDOWN SDL_EVENT_GAMEPAD_BUTTON_DOWN
#define SDL_CONTROLLERBUTTONUP SDL_EVENT_GAMEPAD_BUTTON_UP

#define SDL_CONTROLLER_AXIS_LEFTX SDL_GAMEPAD_AXIS_LEFTX
#define SDL_CONTROLLER_AXIS_LEFTY SDL_GAMEPAD_AXIS_LEFTY
#define SDL_CONTROLLER_AXIS_MAX SDL_GAMEPAD_AXIS_MAX
#define SDL_CONTROLLER_AXIS_RIGHTX SDL_GAMEPAD_AXIS_RIGHTX
#define SDL_CONTROLLER_AXIS_RIGHTY SDL_GAMEPAD_AXIS_RIGHTY
#define SDL_CONTROLLER_AXIS_TRIGGERLEFT SDL_GAMEPAD_AXIS_LEFT_TRIGGER
#define SDL_CONTROLLER_AXIS_TRIGGERRIGHT SDL_GAMEPAD_AXIS_RIGHT_TRIGGER
#define SDL_CONTROLLER_BUTTON_A SDL_GAMEPAD_BUTTON_SOUTH
#define SDL_CONTROLLER_BUTTON_B SDL_GAMEPAD_BUTTON_EAST
#define SDL_CONTROLLER_BUTTON_X SDL_GAMEPAD_BUTTON_WEST
#define SDL_CONTROLLER_BUTTON_Y SDL_GAMEPAD_BUTTON_NORTH
#define SDL_CONTROLLER_BUTTON_BACK SDL_GAMEPAD_BUTTON_BACK
#define SDL_CONTROLLER_BUTTON_DPAD_DOWN SDL_GAMEPAD_BUTTON_DPAD_DOWN
#define SDL_CONTROLLER_BUTTON_DPAD_LEFT SDL_GAMEPAD_BUTTON_DPAD_LEFT
#define SDL_CONTROLLER_BUTTON_DPAD_RIGHT SDL_GAMEPAD_BUTTON_DPAD_RIGHT
#define SDL_CONTROLLER_BUTTON_DPAD_UP SDL_GAMEPAD_BUTTON_DPAD_UP
#define SDL_CONTROLLER_BUTTON_GUIDE SDL_GAMEPAD_BUTTON_GUIDE
#define SDL_CONTROLLER_BUTTON_INVALID SDL_GAMEPAD_BUTTON_INVALID
#define SDL_CONTROLLER_BUTTON_LEFTSHOULDER SDL_GAMEPAD_BUTTON_LEFT_SHOULDER
#define SDL_CONTROLLER_BUTTON_LEFTSTICK SDL_GAMEPAD_BUTTON_LEFT_STICK
#define SDL_CONTROLLER_BUTTON_MISC1 SDL_GAMEPAD_BUTTON_MISC1
#define SDL_CONTROLLER_BUTTON_PADDLE1 SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1
#define SDL_CONTROLLER_BUTTON_PADDLE2 SDL_GAMEPAD_BUTTON_LEFT_PADDLE1
#define SDL_CONTROLLER_BUTTON_PADDLE3 SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2
#define SDL_CONTROLLER_BUTTON_PADDLE4 SDL_GAMEPAD_BUTTON_LEFT_PADDLE2
#define SDL_CONTROLLER_BUTTON_RIGHTSHOULDER SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER
#define SDL_CONTROLLER_BUTTON_RIGHTSTICK SDL_GAMEPAD_BUTTON_RIGHT_STICK
#define SDL_CONTROLLER_BUTTON_START SDL_GAMEPAD_BUTTON_START
#define SDL_CONTROLLER_BUTTON_TOUCHPAD SDL_GAMEPAD_BUTTON_TOUCHPAD

#define SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT
#define SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR
#define SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT
#define SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO
#define SDL_CONTROLLER_TYPE_PS3 SDL_GAMEPAD_TYPE_PS3
#define SDL_CONTROLLER_TYPE_PS4 SDL_GAMEPAD_TYPE_PS4
#define SDL_CONTROLLER_TYPE_PS5 SDL_GAMEPAD_TYPE_PS5
#define SDL_CONTROLLER_TYPE_UNKNOWN SDL_GAMEPAD_TYPE_STANDARD
#define SDL_CONTROLLER_TYPE_VIRTUAL SDL_GAMEPAD_TYPE_VIRTUAL
#define SDL_CONTROLLER_TYPE_XBOX360 SDL_GAMEPAD_TYPE_XBOX360
#define SDL_CONTROLLER_TYPE_XBOXONE SDL_GAMEPAD_TYPE_XBOXONE

#define IS_SDL_BTN_DOWN(EV)  EV.down

#else // SDL2 and SDL1.2

#define IS_SDL_BTN_DOWN(EV)  (EV.state == SDL_PRESSED)

#endif // SDL_VERSION_ATLEAST(3, 0, 0) - unholy hacks

extern idCVar in_useGamepad; // from UsercmdGen.cpp
extern idCVar joy_deadZone;  // ditto

// NOTE: g++-4.7 doesn't like when this is static (for idCmdSystem::ArgCompletion_String<kbdNames>)
const char *_in_kbdNames[] = {
#if SDL_VERSION_ATLEAST(2, 0, 0) // auto-detection is only available for SDL2
	"auto",
#endif
	"english", "french", "german", "italian", "spanish", "turkish", "norwegian", "brazilian", NULL
};

static idCVar in_kbd( "in_kbd", _in_kbdNames[0], CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT, "keyboard layout", _in_kbdNames, idCmdSystem::ArgCompletion_String<_in_kbdNames> );
// TODO: I'd really like to make in_ignoreConsoleKey default to 1, but I guess there would be too much confusion :-/
static idCVar in_ignoreConsoleKey( "in_ignoreConsoleKey", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT | CVAR_BOOL,
                                   "Console only opens with Shift+Esc, not ` or ^ etc" );

static idCVar in_nograb( "in_nograb", "0", CVAR_SYSTEM | CVAR_NOCHEAT, "prevents input grabbing" );
idCVar in_grabKeyboard( "in_grabKeyboard", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT | CVAR_BOOL,
                        "if enabled, grabs all keyboard input if mouse is grabbed (so keyboard shortcuts from the OS like Alt-Tab or Windows Key won't work)" );

idCVar joy_gamepadLayout( "joy_gamepadLayout", "-1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT | CVAR_INTEGER,
                          "Button layout of gamepad. -1: auto (needs SDL 2.0.12 or newer), 0: XBox-style, 1: Nintendo-style, 2: PS4/5-style, 3: PS2/3-style", idCmdSystem::ArgCompletion_Integer < -1, 3 > );

// set in handleMouseGrab(), used in Sys_GetEvent() to decide what kind of internal mouse event to generate
static bool in_relativeMouseMode = true;
// set in Sys_GetEvent() on window focus gained/lost events
static bool in_hasFocus = true;

static enum D3_Gamepad_Type {
	D3_GAMEPAD_XINPUT,			// XBox/XInput standard, the default
	D3_GAMEPAD_NINTENDO,		// nintendo-like (A/B and X/Y are switched)
	D3_GAMEPAD_PLAYSTATION,		// PS-like (geometric symbols instead of A/B/X/Y)
	D3_GAMEPAD_PLAYSTATION_OLD	// PS2/PS3-like: the back button is called "select" instead of "share"
} gamepadType = D3_GAMEPAD_XINPUT;

struct kbd_poll_t {
	int key;
	bool state;

	kbd_poll_t() : key( 0 ), state( false ) {
	}

	kbd_poll_t( int k, bool s ) {
		key = k;
		state = s;
	}
};

struct mouse_poll_t {
	int action;
	int value;

	mouse_poll_t() : action( 0 ), value( 0 ) {}

	mouse_poll_t( int a, int v ) {
		action = a;
		value = v;
	}
};

struct joystick_poll_t {
	int action;
	int value;

	joystick_poll_t() : action( 0 ), value( 0 ) {} // TODO: or -1?

	joystick_poll_t( int a, int v ) {
		action = a;
		value = v;
	}
};

static idList<kbd_poll_t> kbd_polls;
static idList<mouse_poll_t> mouse_polls;
static idList<joystick_poll_t> joystick_polls;

static bool buttonStates[K_LAST_KEY];
static float joyAxis[MAX_JOYSTICK_AXIS];

static idList<sysEvent_t> event_overflow;

struct scancodename_t {
	int sdlScancode;
	const char *name;
};

// scancodenames[keynum - K_FIRST_SCANCODE] belongs to keynum
static scancodename_t scancodemappings[] = {
	// NOTE: must be kept in sync with the K_SC_* section of keyNum_t in framework/KeyInput.h !
#if SDL_VERSION_ATLEAST(2, 0, 0)
#define D3_SC_MAPPING(X) { SDL_SCANCODE_ ## X , "SC_" #X }
#else // SDL1.2 doesn't have scancodes
#define D3_SC_MAPPING(X) { 0 , "SC_" #X }
#endif
	D3_SC_MAPPING( A ), // { SDL_SCANCODE_A, "SC_A" },
	D3_SC_MAPPING( B ),
	D3_SC_MAPPING( C ),
	D3_SC_MAPPING( D ),
	D3_SC_MAPPING( E ),
	D3_SC_MAPPING( F ),
	D3_SC_MAPPING( G ),
	D3_SC_MAPPING( H ),
	D3_SC_MAPPING( I ),
	D3_SC_MAPPING( J ),
	D3_SC_MAPPING( K ),
	D3_SC_MAPPING( L ),
	D3_SC_MAPPING( M ),
	D3_SC_MAPPING( N ),
	D3_SC_MAPPING( O ),
	D3_SC_MAPPING( P ),
	D3_SC_MAPPING( Q ),
	D3_SC_MAPPING( R ),
	D3_SC_MAPPING( S ),
	D3_SC_MAPPING( T ),
	D3_SC_MAPPING( U ),
	D3_SC_MAPPING( V ),
	D3_SC_MAPPING( W ),
	D3_SC_MAPPING( X ),
	D3_SC_MAPPING( Y ),
	D3_SC_MAPPING( Z ),
	// leaving out SDL_SCANCODE_1 ... _0, we handle them separately already
	// also return, escape, backspace, tab, space, already handled as keycodes
	D3_SC_MAPPING( MINUS ),
	D3_SC_MAPPING( EQUALS ),
	D3_SC_MAPPING( LEFTBRACKET ),
	D3_SC_MAPPING( RIGHTBRACKET ),
	D3_SC_MAPPING( BACKSLASH ),
	D3_SC_MAPPING( NONUSHASH ),
	D3_SC_MAPPING( SEMICOLON ),
	D3_SC_MAPPING( APOSTROPHE ),
	D3_SC_MAPPING( GRAVE ),
	D3_SC_MAPPING( COMMA ),
	D3_SC_MAPPING( PERIOD ),
	D3_SC_MAPPING( SLASH ),
	// leaving out lots of keys incl. from keypad, we already handle them as normal keys
	D3_SC_MAPPING( NONUSBACKSLASH ),
	D3_SC_MAPPING( INTERNATIONAL1 ), /**< used on Asian keyboards, see footnotes in USB doc */
	D3_SC_MAPPING( INTERNATIONAL2 ),
	D3_SC_MAPPING( INTERNATIONAL3 ), /**< Yen */
	D3_SC_MAPPING( INTERNATIONAL4 ),
	D3_SC_MAPPING( INTERNATIONAL5 ),
	D3_SC_MAPPING( INTERNATIONAL6 ),
	D3_SC_MAPPING( INTERNATIONAL7 ),
	D3_SC_MAPPING( INTERNATIONAL8 ),
	D3_SC_MAPPING( INTERNATIONAL9 ),
	D3_SC_MAPPING( THOUSANDSSEPARATOR ),
	D3_SC_MAPPING( DECIMALSEPARATOR ),
	D3_SC_MAPPING( CURRENCYUNIT ),
	D3_SC_MAPPING( CURRENCYSUBUNIT )
#undef D3_SC_MAPPING
};

// for keynums between K_FIRST_SCANCODE and K_LAST_SCANCODE
// returns e.g. "SC_A" for K_SC_A
const char *Sys_GetScancodeName( int key ) {
	if ( key >= K_FIRST_SCANCODE && key <= K_LAST_SCANCODE ) {
		int scIdx = key - K_FIRST_SCANCODE;
		return scancodemappings[scIdx].name;
	}
	return NULL;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static bool isAscii( const char *str_ ) {
	const unsigned char *str = ( const unsigned char * )str_;
	while ( *str != '\0' ) {
		if ( *str > 127 ) {
			return false;
		}
		++str;
	}
	return true;
}
#endif // SDL2

// start button isn't bindable, but I want to use its name in the imgui-based menu
const char *D3_GetGamepadStartButtonName() {
	int layout = joy_gamepadLayout.GetInteger();

	if ( layout == -1 ) {
		layout = gamepadType;
	}

	switch ( layout ) {
	default:
		common->Warning( "joy_gamepadLayout has invalid value %d !\n", joy_gamepadLayout.GetInteger() );
	// fall-through
	case D3_GAMEPAD_PLAYSTATION_OLD:
	case D3_GAMEPAD_XINPUT:
		return "Pad Start";
	case D3_GAMEPAD_NINTENDO:
		return "Pad (+)";

	case D3_GAMEPAD_PLAYSTATION:
		return "Pad Options";
	}
}

const char *Sys_GetLocalizedJoyKeyName( int key ) {
	// Note: trying to keep the returned names short, because the Doom3 binding window doesn't have much space for names..

#if SDL_VERSION_ATLEAST(2, 0, 0) // gamecontroller/gamepad not supported in SDL1
	if ( key >= K_FIRST_JOY && key <= K_LAST_JOY ) {

		if ( key <= K_JOY_BTN_BACK ) {
#if 0 //SDL_VERSION_ATLEAST(3, 0, 0)
			// TODO: or use the SDL2 code and just set joy_gamepadLayout automatically based on SDL_GetGamepadType() ?
			SDL_GamepadButton gpbtn = ( SDL_GamepadButton )( SDL_GAMEPAD_BUTTON_SOUTH + ( key - K_JOY_BTN_SOUTH ) );
			SDL_GamepadType sdlGamepadType = TODO;
			SDL_GamepadButtonLabel label = SDL_GetGamepadButtonLabelForType( sdlGamepadType, gpbtn );
			switch ( label ) {
			case SDL_GAMEPAD_BUTTON_LABEL_A:
				return "Pad A";
			case SDL_GAMEPAD_BUTTON_LABEL_B:
				return "Pad B";
			case SDL_GAMEPAD_BUTTON_LABEL_X:
				return "Pad X";
			case SDL_GAMEPAD_BUTTON_LABEL_Y:
				return "Pad Y";
			case SDL_GAMEPAD_BUTTON_LABEL_CROSS:
				return "Pad Cross";
			case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:
				return "Pad Circle";
			case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:
				return "Pad Square";
			case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE:
				return "Pad Triangle";
			}

#else // SDL2
			//                                          South,   East,       West,         North        Back
			static const char *xboxBtnNames[5]     = { "Pad A", "Pad B",    "Pad X",      "Pad Y",   "Pad Back" };
			static const char *nintendoBtnNames[5] = { "Pad B", "Pad A",    "Pad Y",      "Pad X",   "Pad -" };
			static const char *psBtnNames[5] = { "Pad Cross", "Pad Circle", "Pad Square", "Pad Triangle", "Pad Share" };

			int layout = joy_gamepadLayout.GetInteger();

			if ( layout == -1 ) {
				layout = gamepadType;
			}
			unsigned btnIdx = key - K_JOY_BTN_SOUTH;

			assert( btnIdx < 5 );

			switch ( layout ) {
			default:
				common->Warning( "joy_gamepadLayout has invalid value %d !\n", joy_gamepadLayout.GetInteger() );
			// fall-through
			case D3_GAMEPAD_XINPUT:
				return xboxBtnNames[btnIdx];
			case D3_GAMEPAD_NINTENDO:
				return nintendoBtnNames[btnIdx];
			case D3_GAMEPAD_PLAYSTATION_OLD:
				if ( key == K_JOY_BTN_BACK ) {
					return "Pad Select";
				}
			// the other button names are identical for PS2/3 and PS4/5
			// fall-through
			case D3_GAMEPAD_PLAYSTATION:
				return psBtnNames[btnIdx];
			}
#endif // face button names for SDL2
		}

		// the labels for the remaining keys are the same for SDL2 and SDL3 (and all controllers)
		switch ( key ) {
		case K_JOY_BTN_GUIDE: // can't be used in dhewm3, because it opens steam on some systems
		case K_JOY_BTN_START: // can't be used for bindings, because it's hardcoded to generate Esc
			return NULL;

		case K_JOY_BTN_LSTICK:
			return "Pad LStick";
		case K_JOY_BTN_RSTICK:
			return "Pad RStick";
		case K_JOY_BTN_LSHOULDER:
			return "Pad LShoulder";
		case K_JOY_BTN_RSHOULDER:
			return "Pad RShoulder";

		case K_JOY_DPAD_UP:
			return "DPad Up";
		case K_JOY_DPAD_DOWN:
			return "DPad Down";
		case K_JOY_DPAD_LEFT:
			return "DPad Left";
		case K_JOY_DPAD_RIGHT:
			return "DPad Right";

		case K_JOY_BTN_MISC1:
			return "Pad Misc";
		case K_JOY_BTN_RPADDLE1:
			return "Pad P1";
		case K_JOY_BTN_LPADDLE1:
			return "Pad P3";
		case K_JOY_BTN_RPADDLE2:
			return "Pad P2";
		case K_JOY_BTN_LPADDLE2:
			return "Pad P4";

		// Note: Would be nicer with "Pad " (or even "Gamepad ") at the beginning,
		//       but then it's too long for the keybinding window :-/
		case K_JOY_STICK1_UP:
			return "Stick1 Up";
		case K_JOY_STICK1_DOWN:
			return "Stick1 Down";
		case K_JOY_STICK1_LEFT:
			return "Stick1 Left";
		case K_JOY_STICK1_RIGHT:
			return "Stick1 Right";

		case K_JOY_STICK2_UP:
			return "Stick2 Up";
		case K_JOY_STICK2_DOWN:
			return "Stick2 Down";
		case K_JOY_STICK2_LEFT:
			return "Stick2 Left";
		case K_JOY_STICK2_RIGHT:
			return "Stick2 Right";

		case K_JOY_TRIGGER1:
			return "Trigger 1";
		case K_JOY_TRIGGER2:
			return "Trigger 2";

		default:
			assert( 0 && "missing a case in Sys_GetLocalizedJoyKeyName()!" );
		}
	}
#endif // SDL2+
	return NULL;
}

static const char *getLocalizedScancodeName( int key, bool useUtf8 ) {
	if ( key >= K_FIRST_SCANCODE && key <= K_LAST_SCANCODE ) {
		int scIdx = key - K_FIRST_SCANCODE;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_Scancode sc = ( SDL_Scancode ) scancodemappings[scIdx].sdlScancode;
		SDL_Keycode k = SDL_GetKeyFromScancode( sc );

		if ( k >= 0xA1 && k <= 0xFF ) {
			static char shortStr[3] = {};

			if ( useUtf8 ) {
				// SDL_Keycodes are unicode chars (where applicable),
				// so at least for Latin-1 turn them directly into UTF-8
				if ( k >= 0xE0 && k <= 0xFE && k != 0xF7 ) {
					// turn lowercase chars into their uppercase equivalents
					k -= 32;
				}
				shortStr[0] = ( unsigned char )( 0xC0 + ( k >> 6 ) );
				shortStr[1] = ( unsigned char )( 0x80 + ( k & 0x3F ) );
				return shortStr;
			} else {
				// luckily, the "High-ASCII" (ISO-8559-1) chars supported by Doom3
				// have the same values as the corresponding SDL_Keycodes.
				shortStr[0] = ( unsigned char )k;
				shortStr[1] = 0;
				return shortStr;
			}
		} else if ( k != SDLK_UNKNOWN ) {
			const char *ret = SDL_GetKeyName( k );
			if ( ret && *ret != '\0' ) {
				if ( useUtf8 ) {
					return ret;
				}

				// the keyname from SDL2 is in UTF-8, which Doom3 can't print (except with ImGui),
				// so only return the name directly if it's ASCII, otherwise try to translate it
				// to ISO8859-1, and if that fails fall back to SC_*
				if ( isAscii( ret ) ) {
					return ret;
				}
				static char isoName[32];

				// try to convert name to ISO8859-1 (Doom3's supported "High ASCII")
				// TODO: pass '?' as invalidChar?
				if ( D3_UTF8toISO8859_1( ret, isoName, sizeof( isoName ) ) && isoName[0] != '\0' ) {
					return isoName;
				}
			}
		}
#endif  // SDL1.2 doesn't support this, use unlocalized name (also as fallback if we couldn't get a keyname)
		return scancodemappings[scIdx].name;

	}
	return NULL;
}

// returns localized name of the key (between K_FIRST_SCANCODE and K_LAST_SCANCODE),
// regarding the current keyboard layout - if that name is in ASCII or corresponds
// to a "High-ASCII" char supported by Doom3.
// Otherwise return same name as Sys_GetScancodeName()
// !! Returned string is only valid until next call to this function !!
const char *Sys_GetLocalizedScancodeName( int key ) {
	return getLocalizedScancodeName( key, false );
}

const char *Sys_GetLocalizedScancodeNameUTF8( int key ) {
	return getLocalizedScancodeName( key, true );
}

// returns keyNum_t (K_SC_* constant) for given scancode name (like "SC_A")
// only makes sense to call it if name starts with "SC_" (or "sc_")
// returns -1 if not found
int Sys_GetKeynumForScancodeName( const char *name ) {
	for ( int scIdx = 0; scIdx < K_NUM_SCANCODES; ++scIdx ) {
		if ( idStr::Icmp( name, scancodemappings[scIdx].name ) == 0 ) {
			return scIdx + K_FIRST_SCANCODE;
		}
	}
	return -1;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static int getKeynumForSDLscancode( SDL_Scancode scancode ) {
	int sc = scancode;
	for ( int scIdx = 0; scIdx < K_NUM_SCANCODES; ++scIdx ) {
		if ( scancodemappings[scIdx].sdlScancode == sc ) {
			return scIdx + K_FIRST_SCANCODE;
		}
	}
	return 0;
}
#endif

static byte mapkey( SDL_Keycode key ) {
	switch ( key ) {
	case SDLK_BACKSPACE:
		return K_BACKSPACE;
	case SDLK_PAUSE:
		return K_PAUSE;
	}

	if ( key <= SDLK_z ) {
		return key & 0xff;
	}

	switch ( key ) {
	case SDLK_APPLICATION:
		return K_COMMAND;
	case SDLK_CAPSLOCK:
		return K_CAPSLOCK;
	case SDLK_SCROLLLOCK:
		return K_SCROLL;
	case SDLK_POWER:
		return K_POWER;

	case SDLK_UP:
		return K_UPARROW;
	case SDLK_DOWN:
		return K_DOWNARROW;
	case SDLK_LEFT:
		return K_LEFTARROW;
	case SDLK_RIGHT:
		return K_RIGHTARROW;

	case SDLK_LGUI:
		return K_LWIN;
	case SDLK_RGUI:
		return K_RWIN;
	case SDLK_MENU:
		return K_MENU;

	case SDLK_LALT:
		return K_ALT;
	case SDLK_RALT:
		return K_RIGHT_ALT;
	case SDLK_RCTRL:
		return K_RIGHT_CTRL;
	case SDLK_LCTRL:
		return K_CTRL;
	case SDLK_RSHIFT:
		return K_RIGHT_SHIFT;
	case SDLK_LSHIFT:
		return K_SHIFT;
	case SDLK_INSERT:
		return K_INS;
	case SDLK_DELETE:
		return K_DEL;
	case SDLK_PAGEDOWN:
		return K_PGDN;
	case SDLK_PAGEUP:
		return K_PGUP;
	case SDLK_HOME:
		return K_HOME;
	case SDLK_END:
		return K_END;

	case SDLK_F1:
		return K_F1;
	case SDLK_F2:
		return K_F2;
	case SDLK_F3:
		return K_F3;
	case SDLK_F4:
		return K_F4;
	case SDLK_F5:
		return K_F5;
	case SDLK_F6:
		return K_F6;
	case SDLK_F7:
		return K_F7;
	case SDLK_F8:
		return K_F8;
	case SDLK_F9:
		return K_F9;
	case SDLK_F10:
		return K_F10;
	case SDLK_F11:
		return K_F11;
	case SDLK_F12:
		return K_F12;
	// K_INVERTED_EXCLAMATION;
	case SDLK_F13:
		return K_F13;
	case SDLK_F14:
		return K_F14;
	case SDLK_F15:
		return K_F15;

	case SDLK_KP_7:
		return K_KP_HOME;
	case SDLK_KP_8:
		return K_KP_UPARROW;
	case SDLK_KP_9:
		return K_KP_PGUP;
	case SDLK_KP_4:
		return K_KP_LEFTARROW;
	case SDLK_KP_5:
		return K_KP_5;
	case SDLK_KP_6:
		return K_KP_RIGHTARROW;
	case SDLK_KP_1:
		return K_KP_END;
	case SDLK_KP_2:
		return K_KP_DOWNARROW;
	case SDLK_KP_3:
		return K_KP_PGDN;
	case SDLK_KP_ENTER:
		return K_KP_ENTER;
	case SDLK_KP_0:
		return K_KP_INS;
	case SDLK_KP_PERIOD:
		return K_KP_DEL;
	case SDLK_KP_DIVIDE:
		return K_KP_SLASH;
	// K_SUPERSCRIPT_TWO;
	case SDLK_KP_MINUS:
		return K_KP_MINUS;
	// K_ACUTE_ACCENT;
	case SDLK_KP_PLUS:
		return K_KP_PLUS;
	case SDLK_NUMLOCKCLEAR:
		return K_KP_NUMLOCK;
	case SDLK_KP_MULTIPLY:
		return K_KP_STAR;
	case SDLK_KP_EQUALS:
		return K_KP_EQUALS;

	// K_MASCULINE_ORDINATOR;
	// K_GRAVE_A;
	// K_AUX1;
	// K_CEDILLA_C;
	// K_GRAVE_E;
	// K_AUX2;
	// K_AUX3;
	// K_AUX4;
	// K_GRAVE_I;
	// K_AUX5;
	// K_AUX6;
	// K_AUX7;
	// K_AUX8;
	// K_TILDE_N;
	// K_GRAVE_O;
	// K_AUX9;
	// K_AUX10;
	// K_AUX11;
	// K_AUX12;
	// K_AUX13;
	// K_AUX14;
	// K_GRAVE_U;
	// K_AUX15;
	// K_AUX16;

	case SDLK_PRINTSCREEN:
		return K_PRINT_SCR;
	case SDLK_MODE:
		// FIXME: is this really right alt? (also mapping SDLK_RALT to K_RIGHT_ALT)
		return K_RIGHT_ALT;
	}
	return 0;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)

#if ! SDL_VERSION_ATLEAST(2, 0, 14)
// Hack: to support newer SDL2 runtime versions than the one built against,
//       define these controller buttons if needed
enum {
	SDL_CONTROLLER_BUTTON_MISC1 = 15, /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
	SDL_CONTROLLER_BUTTON_PADDLE1,  /* Xbox Elite paddle P1 */
	SDL_CONTROLLER_BUTTON_PADDLE2,  /* Xbox Elite paddle P3 */
	SDL_CONTROLLER_BUTTON_PADDLE3,  /* Xbox Elite paddle P2 */
	SDL_CONTROLLER_BUTTON_PADDLE4,  /* Xbox Elite paddle P4 */
	SDL_CONTROLLER_BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
};
#endif // ! SDL_VERSION_ATLEAST(2, 0, 14)

static sys_jEvents mapjoybutton( SDL_GameControllerButton button ) {
	switch ( button ) {
	case SDL_CONTROLLER_BUTTON_A:
		return J_BTN_SOUTH;
	case SDL_CONTROLLER_BUTTON_B:
		return J_BTN_EAST;
	case SDL_CONTROLLER_BUTTON_X:
		return J_BTN_WEST;
	case SDL_CONTROLLER_BUTTON_Y:
		return J_BTN_NORTH;
	case SDL_CONTROLLER_BUTTON_BACK:
		return J_BTN_BACK;
	case SDL_CONTROLLER_BUTTON_GUIDE:
		// TODO: this one should probably not be bindable?
		//return J_BTN_GUIDE;
		break;
	case SDL_CONTROLLER_BUTTON_START:
		return J_BTN_START;
	case SDL_CONTROLLER_BUTTON_LEFTSTICK:
		return J_BTN_LSTICK;
	case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
		return J_BTN_RSTICK;
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		return J_BTN_LSHOULDER;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		return J_BTN_RSHOULDER;
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		return J_DPAD_UP;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		return J_DPAD_DOWN;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		return J_DPAD_LEFT;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		return J_DPAD_RIGHT;

	case SDL_CONTROLLER_BUTTON_MISC1:
		return J_BTN_MISC1;
	case SDL_CONTROLLER_BUTTON_PADDLE1:
		return J_BTN_RPADDLE1;
	case SDL_CONTROLLER_BUTTON_PADDLE2:
		return J_BTN_RPADDLE2;
	case SDL_CONTROLLER_BUTTON_PADDLE3:
		return J_BTN_LPADDLE1;
	case SDL_CONTROLLER_BUTTON_PADDLE4:
		return J_BTN_LPADDLE2;
	default:
		common->Warning( "unknown game controller button %u", button );
		break;
	}
	return MAX_JOY_EVENT;
}

static sys_jEvents mapjoyaxis( SDL_GameControllerAxis axis ) {
	switch ( axis ) {
	case SDL_CONTROLLER_AXIS_LEFTX:
		return J_AXIS_LEFT_X;
	case SDL_CONTROLLER_AXIS_LEFTY:
		return J_AXIS_LEFT_Y;
	case SDL_CONTROLLER_AXIS_RIGHTX:
		return J_AXIS_RIGHT_X;
	case SDL_CONTROLLER_AXIS_RIGHTY:
		return J_AXIS_RIGHT_Y;
	case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
		return J_AXIS_LEFT_TRIG;
	case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
		return J_AXIS_RIGHT_TRIG;
	default:
		common->Warning( "unknown game controller axis %u", axis );
		break;
	}
	return J_AXIS_MAX;
}

#if ! SDL_VERSION_ATLEAST(2, 24, 0)
// Hack: to support newer SDL2 runtime versions than the one compiled against,
// define some controller types that were added after 2.0.12
enum {
#if ! SDL_VERSION_ATLEAST(2, 0, 14)
	SDL_CONTROLLER_TYPE_PS5 = 7,
#endif

	// leaving out luna and stadia (from 2.0.16)
	// and nvidia shield (from 2.24), they're similar enough to XBox/XInput

	// the following were added in 2.24
	SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT = 11,
	SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,
	SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR
};
#endif // ! SDL_VERSION_ATLEAST(2, 24, 0)

static void setGamepadType( SDL_GameController *gc ) {
#if SDL_VERSION_ATLEAST(2, 0, 12)
	const char *typestr = NULL;
	switch ( SDL_GameControllerGetType( gc ) ) {
	default: // the other controller like luna, stadia, whatever, have a very similar layout
	case SDL_CONTROLLER_TYPE_UNKNOWN:
	case SDL_CONTROLLER_TYPE_XBOX360:
	case SDL_CONTROLLER_TYPE_XBOXONE:
		gamepadType = D3_GAMEPAD_XINPUT;
		typestr = "XBox-like";
		break;

	case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
	case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
	case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
	case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
		gamepadType = D3_GAMEPAD_NINTENDO;
		typestr = "Nintendo-like";
		break;

	case SDL_CONTROLLER_TYPE_PS3:
		gamepadType = D3_GAMEPAD_PLAYSTATION_OLD;
		typestr = "Playstation2/3-like";
		break;
	case SDL_CONTROLLER_TYPE_PS4:
	case SDL_CONTROLLER_TYPE_PS5:
		gamepadType = D3_GAMEPAD_PLAYSTATION;
		typestr = "Playstation-like";
		break;
	}
	common->Printf( "Detected Gamepad %s as type %s\n", SDL_GameControllerName( gc ), typestr );
	SDL_Joystick *joy = SDL_GameControllerGetJoystick( gc );
	SDL_JoystickGUID guid = SDL_JoystickGetGUID( joy );
	char guidstr[34] = {};
	SDL_JoystickGetGUIDString( guid, guidstr, sizeof( guidstr ) );
	Uint16 vendor = SDL_GameControllerGetVendor( gc );
	Uint16 product = SDL_GameControllerGetProduct( gc );
	const char *joyname = SDL_JoystickName( joy );
	common->Printf( "  USB IDs: %.4hx:%.4hx Joystick Name: \"%s\" GUID: %s\n", vendor, product, joyname, guidstr );

#endif // SDL_VERSION_ATLEAST(2, 0, 12)
}

#endif // SDL2+ gamecontroller code

static void PushConsoleEvent( const char *s ) {
	char *b;
	size_t len;

	len = strlen( s ) + 1;
	b = ( char * )Mem_Alloc( len );
	strcpy( b, s );

	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = SE_CONSOLE;
	event.user.data1 = ( void * )len;
	event.user.data2 = b;

	SDL_PushEvent( &event );
}

/*
=================
Sys_InitInput
=================
*/
void Sys_InitInput() {
	kbd_polls.SetGranularity( 64 );
	mouse_polls.SetGranularity( 64 );

	assert( sizeof( scancodemappings ) / sizeof( scancodemappings[0] ) == K_NUM_SCANCODES && "scancodemappings incomplete?" );

#if !SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_EnableUNICODE( 1 );
	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );
#endif

	in_kbd.SetModified();
	Sys_GetConsoleKey( false ); // initialize consoleKeymappingIdx from in_kbd

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// NOTE: SDL3 doesn't support that hint/env var, but whatever, support the env var anyway
	const char *grabKeyboardEnv = SDL_getenv( "SDL_GRAB_KEYBOARD" );

	if ( grabKeyboardEnv ) {
		common->Printf( "The SDL_GRAB_KEYBOARD environment variable is set, setting the in_grabKeyboard CVar to the same value (%s)\n", grabKeyboardEnv );
		in_grabKeyboard.SetString( grabKeyboardEnv );
	} else {
		in_grabKeyboard.SetModified();
	}
#else // SDL1.2 doesn't support this
	in_grabKeyboard.ClearModified();
#endif

	joystick_polls.SetGranularity( 64 );
	event_overflow.SetGranularity( 64 );

	memset( buttonStates, 0, sizeof( buttonStates ) );
	memset( joyAxis, 0, sizeof( joyAxis ) );

#if SDL_VERSION_ATLEAST(3, 0, 0)
	int numJoysticks = 0;

	SDL_JoystickID *joysticks = SDL_GetJoysticks( &numJoysticks );

	for ( int i = 0; i < numJoysticks; ++i ) {
		SDL_GameController *gc = SDL_GameControllerOpen( joysticks[i] );

		if ( gc != NULL ) {
			setGamepadType( gc );
		}
	}
	SDL_free( joysticks );

#elif SDL_VERSION_ATLEAST(2, 0, 0)
	// use button positions instead of button labels,
	// Sys_GetLocalizedJoyKeyName() will do the translation
	// (I think this also was the default before 2.0.12?)
	SDL_SetHint( "SDL_GAMECONTROLLER_USE_BUTTON_LABELS", "0" );

	const int NumJoysticks = SDL_NumJoysticks();

	for ( int i = 0; i < NumJoysticks; ++i ) {
		SDL_GameController *gc = SDL_GameControllerOpen( i );

		if ( gc != NULL ) {
			setGamepadType( gc );
		}
	}
#endif // gamecontroller/gamepad not supported in SDL1
}

/*
=================
Sys_ShutdownInput
=================
*/
void Sys_ShutdownInput() {
	kbd_polls.Clear();
	mouse_polls.Clear();
	joystick_polls.Clear();
	event_overflow.Clear();
}

/*
===========
Sys_InitScanTable
===========
*/
// Windows has its own version due to the tools
#ifndef _WIN32
void Sys_InitScanTable() {
}
#endif


struct ConsoleKeyMapping {
	const char *langName;
	unsigned char key;
	unsigned char keyShifted;
};

static ConsoleKeyMapping consoleKeyMappings[] = {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	{ "auto",   	 0,	0   },  // special case: set current keycode for SDL_SCANCODE_GRAVE (no shifted keycode, though)
#endif
	{ "english",	'`',	'~' },
	{ "french", 	'<',	'>' },
	{ "german", 	'^',	176 }, // °
	{ "italian",	'\\',	'|' },
	{ "spanish",	186,	170 }, // º ª
	{ "turkish",	'"',	233 }, // é
	{ "norwegian",	124,	167 }, // | §
	{ "brazilian",	'\'',	'"' },
};
static int consoleKeyMappingIdx = 0;

static void initConsoleKeyMapping() {
	const int numMappings = sizeof( consoleKeyMappings ) / sizeof( consoleKeyMappings[0] );
	idStr lang = in_kbd.GetString();

	consoleKeyMappingIdx = 0;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	consoleKeyMappings[0].key = 0;
	if ( lang.Length() == 0 || lang.Icmp( "auto" ) == 0 ) {
		// auto-detection (SDL2-only)
		int keycode = SDL_GetKeyFromScancode( SDL_SCANCODE_GRAVE );

		if ( keycode > 0 && keycode <= 0xFF ) {
			// the SDL keycode and dhewm3 keycode should be identical for the mappings,
			// as it's ISO-8859-1 ("High ASCII") chars
			for ( int i = 1; i < numMappings; ++i ) {
				if ( consoleKeyMappings[i].key == keycode ) {
					consoleKeyMappingIdx = i;
					common->Printf( "Detected keyboard layout as \"%s\"\n", consoleKeyMappings[i].langName );
					break;
				}
			}

			if ( consoleKeyMappingIdx == 0 ) { // not found in known mappings
				consoleKeyMappings[0].key = keycode;
			}
		}
	} else
#endif
	{
		for ( int i = 1; i < numMappings; ++i ) {
			if ( lang.Icmp( consoleKeyMappings[i].langName ) == 0 ) {
				consoleKeyMappingIdx = i;
#if SDL_VERSION_ATLEAST(2, 0, 0)
				int keycode = SDL_GetKeyFromScancode( SDL_SCANCODE_GRAVE );

				if ( keycode && keycode != consoleKeyMappings[i].key ) {
					common->Warning( "in_kbd is set to \"%s\", but the actual keycode of the 'console key' is %c (%d), not %c (%d), so this might not work that well..\n",
					                 lang.c_str(), ( unsigned char )keycode, keycode, consoleKeyMappings[i].key, consoleKeyMappings[i].key );
				}
#endif
				break;
			}
		}
	}
}

/*
===============
Sys_GetConsoleKey
===============
*/
unsigned char Sys_GetConsoleKey( bool shifted ) {

	if ( in_ignoreConsoleKey.GetBool() ) {
		return 0;
	}

	if ( in_kbd.IsModified() ) {
		initConsoleKeyMapping();
		in_kbd.ClearModified();
	}
	return shifted ? consoleKeyMappings[consoleKeyMappingIdx].keyShifted : consoleKeyMappings[consoleKeyMappingIdx].key;
}

/*
===============
Sys_MapCharForKey
===============
*/
unsigned char Sys_MapCharForKey( int key ) {
	return key & 0xff;
}

/*
===============
Sys_GrabMouseCursor
Note: Usually grabbing is handled in idCommonLocal::Frame() -> Sys_GenerateEvents() -> handleMouseGrab()
      This function should only be used to release the mouse before long operations where
      common->Frame() won't be called for a while
===============
*/
void Sys_GrabMouseCursor( bool grabIt ) {
	int flags = grabIt ? ( GRAB_GRABMOUSE | GRAB_HIDECURSOR | GRAB_RELATIVEMOUSE ) : 0;

	GLimp_GrabInput( flags );
}


/*
===============
Sys_SetInteractiveIngameGuiActive
Tell the input system that currently an interactive *ingame* UI has focus,
so there is an active cursor.
Used for an ungodly hack to make gamepad button south (A) behave like
left mouse button in that case, so "clicking" with gamepad in the PDA
(and ingame GUIs) works as expected.
Not set for proper menus like main menu etc - the gamepad hacks for that
are in idUserInterfaceLocal::HandleEvent().
Call with ui = NULL to clear the state.
I hope this won't explode in my face :-p
===============
*/
bool D3_IN_interactiveIngameGuiActive = false;
void Sys_SetInteractiveIngameGuiActive( bool active, idUserInterface *ui ) {
	static idList<idUserInterface *> lastuis;

	if ( ui == NULL ) {
		// special case for clearing
		D3_IN_interactiveIngameGuiActive = false;
		lastuis.Clear();
		return;
	}
	int idx = lastuis.FindIndex( ui );

	if ( sessLocal.GetActiveMenu() == NULL && active ) {
		// add ui to lastuis, if it has been activated and no proper menu
		// (like main menu) is currently open
		if ( idx == -1 ) {
			lastuis.Append( ui );
		}
	} else if ( idx != -1 ) {
		// if the UI is in lastuis and has been deactivated, or there
		// is a proper menu opened, remove it from the list.
		// this both handles the regular deactivate case and also works around
		// main-menu-in-multiplayer weirdness: that menu calls idUserInterface::Activate()
		// with activate = true twice, but on first call sessLocal.GetActiveMenu() is NULL
		// so we want to remove it once we realize that it really is a "proper" menu after all.
		// And because it's possible that we have an ingame UI focussed while opening
		// the multiplayer-main-menu, we keep a list of lastuis, instead of just one,
		// so D3_IN_interactiveIngameGuiActive remains true in that case
		// (the ingame UI is still in the list); the lastuis list is also needed
		// for the case of opening the PDA while an ingame GUI is focused
		lastuis.RemoveIndex( idx );
	}
	D3_IN_interactiveIngameGuiActive = lastuis.Num() != 0;
}

static void PushButton( int key, bool value ) {
	// So we don't keep sending the same SE_KEY message over and over again
	if ( buttonStates[key] != value ) {
		buttonStates[key] = value;
		sysEvent_t res = { SE_KEY, key, value ? 1 : 0, 0, NULL };
		// this is done to generate two events per controller axis event
		// one SE_JOYSTICK and one SE_KEY
		event_overflow.Append( res );
	}
}

/*
================
Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent() {
	SDL_Event ev;
	sysEvent_t res = { };
	int key;
	bool isDown = false;
	static const sysEvent_t res_none = { SE_NONE, 0, 0, 0, NULL };

	// process any overflow.
	if ( event_overflow.Num() > 0 ) {
		res = event_overflow[0];
		event_overflow.RemoveIndex( 0 );
		return res;
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// s holds the string from the last SDL_TEXTINPUT event, to generate SE_CHARS D3 events,
	// one event per call to this function until all chars have been handled
	// it used to have SDL_TEXTINPUTEVENT_TEXT_SIZE (32) bytes, but in SDL3 the string can have
	// arbitrary size, however I assume that 128 should still be more than enough
	static char s[128] = {0};
	static size_t s_pos = 0;

	if ( s[0] != '\0' ) {
		res.evType = SE_CHAR;
		res.evValue = ( unsigned char )s[s_pos];

		++s_pos;

		if ( !s[s_pos] || s_pos == sizeof( s ) ) {
			memset( s, 0, sizeof( s ) );
			s_pos = 0;
		}
		return res;
	}
#endif

	// c holds a single char for a SE_CHAR event, probably coming from a SDL_KEYDOWN event
	// (that was also returned as SE_KEY), or from a SDL_TEXTINPUT event that contained just one char.
	// It's 0 when not currently holding a char to generate an event
	static byte c = 0;

	if ( c ) {
		res.evType = SE_CHAR;
		res.evValue = c;
		c = 0;

		return res;
	}

	// loop until there is an event we care about (will return then) or no more events
	while ( SDL_PollEvent( &ev ) ) {
		if ( D3::ImGuiHooks::ProcessEvent( &ev ) ) {
			// ImGui has used the event, so it shouldn't also be handled by the game
			continue;
		}

		switch ( ev.type ) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		case SDL_EVENT_WINDOW_FOCUS_GAINED: {
			// unset modifier, in case alt-tab was used to leave window and ALT is still set
			// as that can cause fullscreen-toggling when pressing enter...
			SDL_Keymod currentmod = SDL_GetModState();

			int newmod = SDL_KMOD_NONE;

			if ( currentmod & SDL_KMOD_CAPS ) { // preserve capslock
				newmod |= SDL_KMOD_CAPS;
			}
			SDL_SetModState( ( SDL_Keymod )newmod );
		}

		// new context because visual studio complains about newmod and currentmod not initialized because of the case SDL_WINDOWEVENT_FOCUS_LOST
		in_hasFocus = true;

		// start playing the game sound world again (when coming from editor)
		session->SetPlayingSoundWorld();
		continue; // handle next event

		case SDL_EVENT_WINDOW_FOCUS_LOST:
			in_hasFocus = false;
			continue; // handle next event

		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			GLimp_UpdateWindowSize();
			continue;  // handle next event

#elif SDL_VERSION_ATLEAST(2, 0, 0)
		case SDL_WINDOWEVENT:
			switch ( ev.window.event ) {
			case SDL_WINDOWEVENT_FOCUS_GAINED: {
				// unset modifier, in case alt-tab was used to leave window and ALT is still set
				// as that can cause fullscreen-toggling when pressing enter...
				SDL_Keymod currentmod = SDL_GetModState();

				int newmod = KMOD_NONE;

				if ( currentmod & KMOD_CAPS ) { // preserve capslock
					newmod |= KMOD_CAPS;
				}
				SDL_SetModState( ( SDL_Keymod )newmod );
			}

				// new context because visual studio complains about newmod and currentmod not initialized because of the case SDL_WINDOWEVENT_FOCUS_LOST
			in_hasFocus = true;

				// start playing the game sound world again (when coming from editor)
			session->SetPlayingSoundWorld();

			break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				in_hasFocus = false;
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				GLimp_UpdateWindowSize();
				break;
			}
			continue; // handle next event
#else
		case SDL_ACTIVEEVENT: {
			if ( ev.active.gain ) {
				in_hasFocus = true;

				// unset modifier, in case alt-tab was used to leave window and ALT is still set
				// as that can cause fullscreen-toggling when pressing enter...
				SDLMod currentmod = SDL_GetModState();

				int newmod = KMOD_NONE;

				if ( currentmod & KMOD_CAPS ) { // preserve capslock
					newmod |= KMOD_CAPS;
				}
				SDL_SetModState( ( SDLMod )newmod );
			} else {
				in_hasFocus = false;
			}
		}
		continue; // handle next event

		case SDL_VIDEOEXPOSE:
			continue; // handle next event
#endif

		case SDL_KEYDOWN:
#if SDL_VERSION_ATLEAST(3, 0, 0)
			if ( ev.key.key == SDLK_RETURN && ( ev.key.mod & SDL_KMOD_ALT ) > 0 ) {
#else
			if ( ev.key.keysym.sym == SDLK_RETURN && ( ev.key.keysym.mod & KMOD_ALT ) > 0 ) {
#endif
				cvarSystem->SetCVarBool( "r_fullscreen", !renderSystem->IsFullScreen() );
				PushConsoleEvent( "vid_restart partial" );
				return res_none;
			}

		// fall through
		case SDL_KEYUP:
#if !SDL_VERSION_ATLEAST(2, 0, 0) // SDL1.2
			key = mapkey( ev.key.keysym.sym );

			if ( !key ) {
				if ( !in_ignoreConsoleKey.GetBool() ) {
					// check if its an unmapped console key
					int c = Sys_GetConsoleKey( ( ev.key.keysym.mod & KMOD_SHIFT ) != 0 );

					if ( ev.key.keysym.unicode == c ) {
						key = c;
					}
				}

				if ( !key ) {
					if ( ev.type == SDL_KEYDOWN ) {
						common->Warning( "unmapped SDL key %d (0x%x) - if possible use SDL2 for better keyboard support",
						                 ev.key.keysym.sym, ev.key.keysym.unicode );
					}
					continue; // handle next event
				}
			}
#else // SDL2+
			{
#if SDL_VERSION_ATLEAST(3, 0, 0)
				SDL_Scancode sc = ev.key.scancode;
				SDL_Keycode keycode = ev.key.key;
#else // SDL2
				SDL_Scancode sc = ev.key.keysym.scancode;
				SDL_Keycode keycode = ev.key.keysym.sym;
#endif
				// workaround for AZERTY-keyboards, which don't have 1, 2, ..., 9, 0 in first row:
				// always map those physical keys (scancodes) to those keycodes anyway
				// see also https://bugzilla.libsdl.org/show_bug.cgi?id=3188
				if ( sc == SDL_SCANCODE_0 ) {
					key = '0';
				} else if ( sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9 ) {
					// note that the SDL_SCANCODEs are SDL_SCANCODE_1, _2, ..., _9, SDL_SCANCODE_0
					// while in ASCII it's '0', '1', ..., '9' => handle 0 and 1-9 separately
					// (doom3 uses the ASCII values for those keys)
					key = '1' + ( sc - SDL_SCANCODE_1 );
				} else {
					key = mapkey( keycode );
				}

				if ( !in_ignoreConsoleKey.GetBool() && sc == SDL_SCANCODE_GRAVE ) {
					// that key between Esc, Tab and 1 is the console key
					key = K_CONSOLE;
				}

				if ( !key ) {
					// if the key couldn't be mapped so far, try to map the scancode to K_SC_*
					key = getKeynumForSDLscancode( sc );

					if ( !key ) {
						if ( ev.type == SDL_KEYDOWN ) {
							common->Warning( "unmapped SDL key %d (scancode %d)", keycode, ( int )sc );
						}
						continue; // handle next event
					}
				}
			}
#endif
			isDown = IS_SDL_BTN_DOWN( ev.key );

			res.evType = SE_KEY;
			res.evValue = key;
			res.evValue2 = isDown;

			kbd_polls.Append( kbd_poll_t( key, isDown ) );

#if SDL_VERSION_ATLEAST(2, 0, 0)
			if ( key == K_BACKSPACE && isDown ) {
				c = key;
			}
#else
			if ( ev.key.state == SDL_PRESSED && ( ev.key.keysym.unicode & 0xff00 ) == 0 ) {
				c = ev.key.keysym.unicode & 0xff;
			}
#endif
			// Note: c will be sent as SE_CHAR next time this function is called
			return res;

#if SDL_VERSION_ATLEAST(2, 0, 0)
		case SDL_TEXTINPUT:
			if ( ev.text.text[0] ) {
				res.evType = SE_CHAR;

				if ( isAscii( ev.text.text ) ) {
					res.evValue = ev.text.text[0];

					if ( ev.text.text[1] != '\0' ) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
						// yes, it's ok if this is not 0-terminated, the code generating
						// events from s handles that
						strncpy( s, ev.text.text, sizeof( s ) );
#else // SDL2
						memcpy( s, ev.text.text, SDL_TEXTINPUTEVENT_TEXT_SIZE );
						s[SDL_TEXTINPUTEVENT_TEXT_SIZE] = '\0';
#endif
						// pos 0 is returned, the rest of s is returned as SE_CHAR events
						// at the next times this function is called
						s_pos = 1;
					}
					return res;
				} else if ( D3_UTF8toISO8859_1( ev.text.text, s, sizeof( s ) ) && s[0] != '\0' ) {
					res.evValue = ( unsigned char )s[0];
					if ( s[1] == '\0' ) {
						s_pos = 0;
						s[0] = '\0';
					} else {
						// pos 0 is returned, the rest of s is returned as SE_CHAR events
						// at the next times this function is called
						s_pos = 1;
					}
					return res;
				}
			}
			continue; // handle next event
#endif

		case SDL_MOUSEMOTION:
			if ( in_relativeMouseMode ) {
				res.evType = SE_MOUSE;
				res.evValue = ev.motion.xrel;
				res.evValue2 = ev.motion.yrel;

				mouse_polls.Append( mouse_poll_t( M_DELTAX, ev.motion.xrel ) );
				mouse_polls.Append( mouse_poll_t( M_DELTAY, ev.motion.yrel ) );
			} else {
				res.evType = SE_MOUSE_ABS;
				res.evValue = ev.motion.x;
				res.evValue2 = ev.motion.y;
			}
			return res;

#if SDL_VERSION_ATLEAST(2, 0, 0)
		case SDL_MOUSEWHEEL:
			res.evType = SE_KEY;

			if ( ev.wheel.y > 0 ) {
				res.evValue = K_MWHEELUP;
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, 1 ) );
			} else if ( ev.wheel.y < 0 ) {
				res.evValue = K_MWHEELDOWN;
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, -1 ) );
			}
			res.evValue2 = 1;

			return res;
#endif

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			isDown = IS_SDL_BTN_DOWN( ev.button );
			res.evType = SE_KEY;

			switch ( ev.button.button ) {
			case SDL_BUTTON_LEFT:
				res.evValue = K_MOUSE1;
				mouse_polls.Append( mouse_poll_t( M_ACTION1, isDown ) );
				break;
			case SDL_BUTTON_MIDDLE:
				res.evValue = K_MOUSE3;
				mouse_polls.Append( mouse_poll_t( M_ACTION3, isDown ) );
				break;
			case SDL_BUTTON_RIGHT:
				res.evValue = K_MOUSE2;
				mouse_polls.Append( mouse_poll_t( M_ACTION2, isDown ) );
				break;

#if !SDL_VERSION_ATLEAST(2, 0, 0)
			case SDL_BUTTON_WHEELUP:
				res.evValue = K_MWHEELUP;
				if ( ev.button.state == SDL_PRESSED ) {
					mouse_polls.Append( mouse_poll_t( M_DELTAZ, 1 ) );
				}
				break;
			case SDL_BUTTON_WHEELDOWN:
				res.evValue = K_MWHEELDOWN;
				if ( ev.button.state == SDL_PRESSED ) {
					mouse_polls.Append( mouse_poll_t( M_DELTAZ, -1 ) );
				}
				break;
#endif
			default:
#if SDL_VERSION_ATLEAST(2, 0, 0)
				// handle X1 button and above
				if ( ev.button.button < SDL_BUTTON_LEFT + 8 ) { // doesn't support more than 8 mouse buttons
					int buttonIndex = ev.button.button - SDL_BUTTON_LEFT;
					res.evValue = K_MOUSE1 + buttonIndex;
					mouse_polls.Append( mouse_poll_t( M_ACTION1 + buttonIndex, isDown ) );
				} else
#endif
					continue; // handle next event
			}
			res.evValue2 = isDown;

			return res;

#if SDL_VERSION_ATLEAST(2, 0, 0) // gamecontroller/gamepad not supported in SDL1
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP: {
			if ( !in_useGamepad.GetBool() ) {
				common->Warning( "Gamepad support is disabled! Set the in_useGamepad CVar to 1 to enable it!\n" );
				continue;
			}
			isDown = IS_SDL_BTN_DOWN( ev.cbutton );

			res.evType = SE_KEY;
			res.evValue2 = isDown;

			// special case: always treat the start button as escape so it opens/closes the menu
			// (also makes that button non-bindable)
			if ( ev.cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
				res.evValue = K_ESCAPE;
				return res;
			} else if ( ( ev.cbutton.button == SDL_CONTROLLER_BUTTON_A || ev.cbutton.button == SDL_CONTROLLER_BUTTON_Y )
			            && D3_IN_interactiveIngameGuiActive && sessLocal.GetActiveMenu() == NULL ) {
				// ugly hack: currently an interactive ingame GUI (with a cursor) is active/focused
				// so pretend that the gamepads A (south) or Y (north, used by D3BFG to click ingame GUIs) button
				// is the left mouse button so it can be used for "clicking"..
				mouse_polls.Append( mouse_poll_t( M_ACTION1, res.evValue2 ) );
				res.evValue = K_MOUSE1;
				return res;
			}
			sys_jEvents jEvent =  mapjoybutton( ( SDL_GameControllerButton )ev.cbutton.button );

			joystick_polls.Append( joystick_poll_t( jEvent, isDown ) );

			if ( ( jEvent >= J_ACTION_FIRST ) && ( jEvent <= J_ACTION_MAX ) ) {
				res.evValue = K_FIRST_JOY + ( jEvent - J_ACTION_FIRST );
				return res;
			}
			continue; // try to get a decent event.
		}

		case SDL_CONTROLLERAXISMOTION: {
			const int range = 16384;

			if ( !in_useGamepad.GetBool() ) {
				// not printing a message here, I guess we get lots of spurious axis events..
				// TODO: or print a message if value is big enough?
				continue;
			}
			sys_jEvents jEvent = mapjoyaxis( ( SDL_GameControllerAxis )ev.caxis.axis );

			joystick_polls.Append( joystick_poll_t(	jEvent, ev.caxis.value ) );

			if ( jEvent == J_AXIS_LEFT_X ) {
				PushButton( K_JOY_STICK1_LEFT, ( ev.caxis.value < -range ) );
				PushButton( K_JOY_STICK1_RIGHT, ( ev.caxis.value > range ) );
			} else if ( jEvent == J_AXIS_LEFT_Y ) {
				PushButton( K_JOY_STICK1_UP, ( ev.caxis.value < -range ) );
				PushButton( K_JOY_STICK1_DOWN, ( ev.caxis.value > range ) );
			} else if ( jEvent == J_AXIS_RIGHT_X ) {
				PushButton( K_JOY_STICK2_LEFT, ( ev.caxis.value < -range ) );
				PushButton( K_JOY_STICK2_RIGHT, ( ev.caxis.value > range ) );
			} else if ( jEvent == J_AXIS_RIGHT_Y ) {
				PushButton( K_JOY_STICK2_UP, ( ev.caxis.value < -range ) );
				PushButton( K_JOY_STICK2_DOWN, ( ev.caxis.value > range ) );
			} else if ( jEvent == J_AXIS_LEFT_TRIG ) {
				PushButton( K_JOY_TRIGGER1, ( ev.caxis.value > range ) );
			} else if ( jEvent == J_AXIS_RIGHT_TRIG ) {
				PushButton( K_JOY_TRIGGER2, ( ev.caxis.value > range ) );
			}

			if ( jEvent >= J_AXIS_MIN && jEvent <= J_AXIS_MAX ) {
				// NOTE: the stuff set here is only used to move the cursor in menus
				//       ingame movement is done via joystick_polls
				int axis = jEvent - J_AXIS_MIN;
				float dz = joy_deadZone.GetFloat();
				float val = fabsf( ev.caxis.value * ( 1.0f / 32767.0f ) );

				if ( val < dz ) {
					val = 0.0f;
				} else {
					// from deadzone .. 1 to 0 .. 1-deadzone
					val -= dz;
					// and then to 0..1
					val = val * ( 1.0f / ( 1.0f - dz ) );

					if ( ev.caxis.value < 0 ) {
						val = -val;
					}
				}
				joyAxis[axis] = val;
			}

			// handle next event; joy axis events are generated below,
			// when there are no further SDL events
			continue;
		}
		break;

		case SDL_JOYDEVICEADDED: {
			SDL_GameController *gc = SDL_GameControllerOpen( ev.jdevice.which );
			if ( gc != NULL ) {
				setGamepadType( gc );
			}
			// TODO: hot swapping maybe.
			//lbOnControllerPlugIn(event.jdevice.which);
			break;
		}
		case SDL_JOYDEVICEREMOVED:
			// TODO: hot swapping maybe.
			//lbOnControllerUnPlug(event.jdevice.which);
			break;
#endif // SDL2+

		case SDL_QUIT:
			PushConsoleEvent( "quit" );
			return res_none;

		case SDL_USEREVENT:
			switch ( ev.user.code ) {
			case SE_CONSOLE:
				res.evType = SE_CONSOLE;
				res.evPtrLength = ( intptr_t )ev.user.data1;
				res.evPtr = ev.user.data2;
				return res;
			default:
				common->Warning( "unknown user event %u", ev.user.code );
				continue; // handle next event
			}
		default:
			// ok, I don't /really/ care about unknown SDL events. only uncomment this for debugging.
			// common->Warning("unknown SDL event 0x%x", ev.type);
			continue; // handle next event
		}
	}

	// before returning res_none for "these were all events for now",
	// first return joyaxis events, if gamepad is enabled and 16ms are over
	// (or we haven't returned the values for all axis yet)
	if ( in_useGamepad.GetBool() ) {
		static unsigned int lastMS = 0;
		static int joyAxisToSend = 0;
		unsigned int nowMS = Sys_Milliseconds();

		if ( nowMS - lastMS >= 16 ) {
			int val = joyAxis[joyAxisToSend] * 100; // float to percent
			res.evType = SE_JOYSTICK;
			res.evValue = joyAxisToSend;
			res.evValue2 = val;
			++joyAxisToSend;

			if ( joyAxisToSend == MAX_JOYSTICK_AXIS ) {
				// we're done for this frame, so update lastMS and reset joyAxisToSend
				joyAxisToSend = 0;
				lastMS = nowMS;
			}
			return res;
		}
	}
	return res_none;
}

/*
================
Sys_ClearEvents
================
*/
void Sys_ClearEvents() {
	SDL_Event ev;

	while ( SDL_PollEvent( &ev ) );

	kbd_polls.SetNum( 0, false );
	mouse_polls.SetNum( 0, false );
	joystick_polls.SetNum( 0, false );

	memset( buttonStates, 0, sizeof( buttonStates ) );
	memset( joyAxis, 0, sizeof( joyAxis ) );

	event_overflow.SetNum( 0, false );
}

static void handleMouseGrab() {

	// these are the defaults for when the window does *not* have focus
	// (don't grab in any way)
	bool showCursor = true;
	bool grabMouse = false;
	bool relativeMouse = false;
	bool enableTextInput = false;
	const bool imguiHasFocus = D3::ImGuiHooks::ShouldShowCursor();

	// if com_editorActive, release everything, just like when we have no focus
	if ( in_hasFocus && !com_editorActive && !imguiHasFocus ) {
		// Note: this generally handles fullscreen menus, but not the PDA, because the PDA
		//       is an ugly hack in gamecode that doesn't go through sessLocal.guiActive.
		//       It goes through weapon input code or sth? That's also the reason only
		//       leftclick (fire) works there (no mousewheel..)
		//       So the PDA will continue to use relative mouse events to set its cursor position.
		const bool menuActive = ( sessLocal.GetActiveMenu() != NULL );

		if ( menuActive ) {
			showCursor = false;
			relativeMouse = false;
			grabMouse = false; // TODO: or still grab to window? (maybe only if in exclusive fullscreen mode?)
			enableTextInput = true;
		} else if ( console->Active() ) {
			showCursor = true;
			relativeMouse = grabMouse = false;
			enableTextInput = true;
		} else { // in game
			showCursor = false;
			grabMouse = relativeMouse = true;
		}
		in_relativeMouseMode = relativeMouse;

		// if in_nograb is set, in_relativeMouseMode and relativeMouse can disagree
		// (=> don't enable relative mouse mode in SDL, but still use relative mouse events
		//  in the game, unless we'd use absolute mousemode anyway)
		if ( in_nograb.GetBool() ) {
			grabMouse = relativeMouse = false;
		}
	} else {
		in_relativeMouseMode = false;
		// if an ImGui window has focus, enable text input so one can write in there
		// TODO: add explicit GRAB_DISABLETEXTINPUT and don't set it at all here for ImGui?
		//  in theory, ImGui handles that itself, but currently GLimp_GrabInput() seems to override it
		enableTextInput = imguiHasFocus;
	}
	int flags = 0;

	if ( !showCursor ) {
		flags |= GRAB_HIDECURSOR;
	}

	if ( grabMouse ) {
		flags |= GRAB_GRABMOUSE;
	}

	if ( relativeMouse ) {
		flags |= GRAB_RELATIVEMOUSE;
	}

	if ( enableTextInput ) {
		flags |= GRAB_ENABLETEXTINPUT;
	}
	GLimp_GrabInput( flags );
}

/*
================
Sys_GenerateEvents
================
*/
void Sys_GenerateEvents() {

	handleMouseGrab();

	char *s = Sys_ConsoleInput();

	if ( s ) {
		PushConsoleEvent( s );
	}

	// doesn't make sense on dedicated server and SDL3 handles this in GLimp_GrabInput()
#if !defined(ID_DEDICATED) && ! SDL_VERSION_ATLEAST(3, 0, 0)
	if ( in_grabKeyboard.IsModified() ) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_SetHint( SDL_HINT_GRAB_KEYBOARD, in_grabKeyboard.GetString() );
		if ( in_grabKeyboard.GetBool() ) {
			common->Printf( "in_grabKeyboard: Will grab the keyboard if mouse is grabbed, so global keyboard-shortcuts (like Alt-Tab or the Windows key) will *not* work\n" );
		} else {
			common->Printf( "in_grabKeyboard: Will *not* grab the keyboard if mouse is grabbed, so global keyboard-shortcuts (like Alt-Tab) will still work\n" );
		}
#else
		common->Printf( "Note: SDL1.2 doesn't support in_grabKeyboard (it's always grabbed if mouse is grabbed)\n" );
#endif
		in_grabKeyboard.ClearModified();
	}
#endif
	SDL_PumpEvents();
}

/*
================
Sys_PollKeyboardInputEvents
================
*/
int Sys_PollKeyboardInputEvents() {
	return kbd_polls.Num();
}

/*
================
Sys_ReturnKeyboardInputEvent
================
*/
int Sys_ReturnKeyboardInputEvent( const int n, int &key, bool &state ) {
	if ( n >= kbd_polls.Num() ) {
		return 0;
	}
	key = kbd_polls[n].key;
	state = kbd_polls[n].state;
	return 1;
}

/*
================
Sys_EndKeyboardInputEvents
================
*/
void Sys_EndKeyboardInputEvents() {
	kbd_polls.SetNum( 0, false );
}

/*
================
Sys_PollMouseInputEvents
================
*/
int Sys_PollMouseInputEvents() {
	return mouse_polls.Num();
}

/*
================
Sys_ReturnMouseInputEvent
================
*/
int	Sys_ReturnMouseInputEvent( const int n, int &action, int &value ) {
	if ( n >= mouse_polls.Num() ) {
		return 0;
	}
	action = mouse_polls[n].action;
	value = mouse_polls[n].value;
	return 1;
}

/*
================
Sys_EndMouseInputEvents
================
*/
void Sys_EndMouseInputEvents() {
	mouse_polls.SetNum( 0, false );
}

/*
================
Joystick Input Methods
================
*/
void Sys_SetRumble( int device, int low, int hi ) {
	// TODO: support multiple controllers.
	assert( device == 0 );

	// TODO: support rumble maybe.
	assert( 0 );
}

int	Sys_PollJoystickInputEvents( int deviceNum ) {
	// TODO: support multiple controllers.
	assert( deviceNum == 0 );
	return joystick_polls.Num();
}

int	Sys_ReturnJoystickInputEvent( const int n, int &action, int &value ) {
	if ( n >= joystick_polls.Num() ) {
		return 0;
	}
	action = joystick_polls[n].action;
	value = joystick_polls[n].value;
	return 1;
}

void Sys_EndJoystickInputEvents() {
	joystick_polls.SetNum( 0, false );
}

