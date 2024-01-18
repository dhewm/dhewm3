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

#ifndef __KEYINPUT_H__
#define __KEYINPUT_H__

#include "idlib/CmdArgs.h"

class idFile;

/*
===============================================================================

	Key Input

===============================================================================
*/

// these are the key numbers that are used by the key system
// normal keys should be passed as lowercased ascii
// Some high ascii (> 127) characters that are mapped directly to keys on
// western european keyboards are inserted in this table so that those keys
// are bindable (otherwise they get bound as one of the special keys in this
// table)
typedef enum {

	// DG: please don't change any existing constants for keyboard keys below (or recreate the tables in win_input.cpp)!

	K_TAB = 9,
	K_ENTER = 13,
	K_ESCAPE = 27,
	K_SPACE = 32,

	K_BACKSPACE = 127,

	K_COMMAND = 128,
	K_CAPSLOCK,
	K_SCROLL,
	K_POWER,
	K_PAUSE,

	K_UPARROW = 133,
	K_DOWNARROW,
	K_LEFTARROW,
	K_RIGHTARROW,

	// The 3 windows keys
	K_LWIN = 137,
	K_RWIN,
	K_MENU,

	K_ALT = 140,
	K_CTRL,
	K_SHIFT,
	K_INS,
	K_DEL,
	K_PGDN,
	K_PGUP,
	K_HOME,
	K_END,

	K_F1 = 149,
	K_F2,
	K_F3,
	K_F4,
	K_F5,
	K_F6,
	K_F7,
	K_F8,
	K_F9,
	K_F10,
	K_F11,
	K_F12,
	K_INVERTED_EXCLAMATION = 161,	// upside down !
	K_F13,
	K_F14,
	K_F15,

	K_KP_HOME = 165,
	K_KP_UPARROW,
	K_KP_PGUP,
	K_KP_LEFTARROW,
	K_KP_5,
	K_KP_RIGHTARROW,
	K_KP_END,
	K_KP_DOWNARROW,
	K_KP_PGDN,
	K_KP_ENTER,
	K_KP_INS,
	K_KP_DEL,
	K_KP_SLASH,
	K_SUPERSCRIPT_TWO = 178,		// superscript 2
	K_KP_MINUS,
	K_ACUTE_ACCENT = 180,			// accute accent
	K_KP_PLUS,
	K_KP_NUMLOCK,
	K_KP_STAR,
	K_KP_EQUALS,

	// DG: please don't change any existing constants above this one (or recreate the tables in win_input.cpp)!

	K_MASCULINE_ORDINATOR = 186,
	// K_MOUSE enums must be contiguous (no char codes in the middle)
	K_MOUSE1 = 187,
	K_MOUSE2,
	K_MOUSE3,
	K_MOUSE4,
	K_MOUSE5,
	K_MOUSE6,
	K_MOUSE7,
	K_MOUSE8,

	K_MWHEELDOWN = 195,
	K_MWHEELUP,

	//------------------------
	// K_JOY codes must be contiguous, too, and K_JOY_BTN_* should be kept in sync with J_BTN_* of sys_jEvents
	//------------------------

	K_FIRST_JOY = 197,
	K_JOY_BTN_SOUTH = K_FIRST_JOY, // bottom face button, like Xbox A
	K_JOY_BTN_EAST,  // right face button, like Xbox B
	K_JOY_BTN_WEST,  // left face button, like Xbox X
	K_JOY_BTN_NORTH, // top face button, like Xbox Y

	K_JOY_BTN_BACK,
	K_JOY_BTN_GUIDE, // Note: this one should probably not be used?
	K_JOY_BTN_START, // hardcoded to generate Esc to open/close menu
	K_JOY_BTN_LSTICK, // press left stick
	K_JOY_BTN_RSTICK, // press right stick
	K_JOY_BTN_LSHOULDER,
	K_JOY_BTN_RSHOULDER,

	K_JOY_DPAD_UP,
	K_JOY_DPAD_DOWN,
	K_JOY_DPAD_LEFT,
	K_JOY_DPAD_RIGHT,

	K_JOY_BTN_MISC1, // Additional button (e.g. Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button)
	K_JOY_BTN_RPADDLE1, // Upper or primary paddle, under your right hand (e.g. Xbox Elite paddle P1)
	K_JOY_BTN_LPADDLE1, // Upper or primary paddle, under your left hand (e.g. Xbox Elite paddle P3)
	K_JOY_BTN_RPADDLE2, // Lower or secondary paddle, under your right hand (e.g. Xbox Elite paddle P2)
	K_JOY_BTN_LPADDLE2, //  Lower or secondary paddle, under your left hand (e.g. Xbox Elite paddle P4)

	K_JOY_STICK1_UP,
	K_JOY_STICK1_DOWN,
	K_JOY_STICK1_LEFT,
	K_JOY_STICK1_RIGHT,

	K_JOY_STICK2_UP,
	K_JOY_STICK2_DOWN,
	K_JOY_STICK2_LEFT,
	K_JOY_STICK2_RIGHT,

	K_JOY_TRIGGER1,
	K_JOY_TRIGGER2,

	K_LAST_JOY = K_JOY_TRIGGER2,

	K_GRAVE_A = 229,	// lowercase a with grave accent FIXME: used to be 224; this probably isn't used anyway

	K_AUX1 = 230,
	K_CEDILLA_C = 231,	// lowercase c with Cedilla
	K_GRAVE_E = 232,	// lowercase e with grave accent
	K_AUX2,
	K_AUX3,
	K_AUX4,
	K_GRAVE_I = 236,	// lowercase i with grave accent
	K_AUX5,
	K_AUX6,
	K_AUX7,
	K_AUX8,
	K_TILDE_N = 241,	// lowercase n with tilde
	K_GRAVE_O = 242,	// lowercase o with grave accent
	K_AUX9,
	K_AUX10,
	K_AUX11,
	K_AUX12,
	K_AUX13,
	K_AUX14,
	K_GRAVE_U = 249,	// lowercase u with grave accent
	K_AUX15,
	K_AUX16,

	K_PRINT_SCR	= 252,	// SysRq / PrintScr
	K_RIGHT_ALT = 253,	// used by some languages as "Alt-Gr"

	// DG: added the following two
	K_RIGHT_CTRL = 254,
	K_RIGHT_SHIFT = 255,

	// DG: map all relevant scancodes from SDL to K_SC_* (taken from Yamagi Quake II)
	// (relevant are ones that are likely to be keyboardlayout-dependent,
	//  i.e. printable characters of sorts, *not* Ctrl, Alt, F1, Del, ...)
	K_FIRST_SCANCODE = 256,

	// !!! NOTE: if you add a scancode here, make sure to also add it to   !!!
	// !!!       scancodemappings[] in sys/events.cpp (and preserve order) !!!
	K_SC_A = K_FIRST_SCANCODE,
	K_SC_B,
	K_SC_C,
	K_SC_D,
	K_SC_E,
	K_SC_F,
	K_SC_G,
	K_SC_H,
	K_SC_I,
	K_SC_J,
	K_SC_K,
	K_SC_L,
	K_SC_M,
	K_SC_N,
	K_SC_O,
	K_SC_P,
	K_SC_Q,
	K_SC_R,
	K_SC_S,
	K_SC_T,
	K_SC_U,
	K_SC_V,
	K_SC_W,
	K_SC_X,
	K_SC_Y,
	K_SC_Z,
	// leaving out SDL_SCANCODE_1 ... _0, we handle them separately already
	// also return, escape, backspace, tab, space, already handled as keycodes
	K_SC_MINUS,
	K_SC_EQUALS,
	K_SC_LEFTBRACKET,
	K_SC_RIGHTBRACKET,
	K_SC_BACKSLASH,
	K_SC_NONUSHASH,
	K_SC_SEMICOLON,
	K_SC_APOSTROPHE,
	K_SC_GRAVE,
	K_SC_COMMA,
	K_SC_PERIOD,
	K_SC_SLASH,
	// leaving out lots of keys incl. from keypad, we already handle them as normal keys
	K_SC_NONUSBACKSLASH,
	K_SC_INTERNATIONAL1, /**< used on Asian keyboards, see footnotes in USB doc */
	K_SC_INTERNATIONAL2,
	K_SC_INTERNATIONAL3, /**< Yen */
	K_SC_INTERNATIONAL4,
	K_SC_INTERNATIONAL5,
	K_SC_INTERNATIONAL6,
	K_SC_INTERNATIONAL7,
	K_SC_INTERNATIONAL8,
	K_SC_INTERNATIONAL9,
	K_SC_THOUSANDSSEPARATOR,
	K_SC_DECIMALSEPARATOR,
	K_SC_CURRENCYUNIT,
	K_SC_CURRENCYSUBUNIT,

	K_LAST_SCANCODE = K_SC_CURRENCYSUBUNIT, // TODO: keep up to date!

	K_CONSOLE, // special keycode used for the "console key" and only to open/close the console (not bindable)

	// FIXME: maybe move everything joystick related here

	K_LAST_KEY // DG: this said "this better be < 256!"; I hope I fixed all places in code assuming this..
} keyNum_t;

enum { K_NUM_SCANCODES = K_LAST_SCANCODE - K_FIRST_SCANCODE + 1 };

class idKeyInput {
public:
	static void			Init( void );
	static void			Shutdown( void );

	static void			ArgCompletion_KeyName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			PreliminaryKeyEvent( int keyNum, bool down );
	static bool			IsDown( int keyNum );
	static int			GetUsercmdAction( int keyNum );
	static bool			GetOverstrikeMode( void );
	static void			SetOverstrikeMode( bool state );
	static void			ClearStates( void );
	static int			StringToKeyNum( const char *str );
	static const char *	KeyNumToString( int keyNum, bool localized );

	static void			SetBinding( int keyNum, const char *binding );
	static const char *	GetBinding( int keyNum );
	static bool			UnbindBinding( const char *bind );
	static int			NumBinds( const char *binding );
	static bool			ExecKeyBinding( int keyNum );
	static const char *	KeysFromBinding( const char *bind );
	static const char *	BindingFromKey( const char *key );
	static bool			KeyIsBoundTo( int keyNum, const char *binding );
	static void			WriteBindings( idFile *f );
};

#endif /* !__KEYINPUT_H__ */
