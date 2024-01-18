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

#ifndef __SYS_PUBLIC__
#define __SYS_PUBLIC__

class idStr;

typedef enum {
	CPUID_NONE							= 0x00000,
	CPUID_UNSUPPORTED					= 0x00001,	// unsupported (386/486)
	CPUID_GENERIC						= 0x00002,	// unrecognized processor
	CPUID_MMX							= 0x00010,	// Multi Media Extensions
	CPUID_3DNOW							= 0x00020,	// 3DNow!
	CPUID_SSE							= 0x00040,	// Streaming SIMD Extensions
	CPUID_SSE2							= 0x00080,	// Streaming SIMD Extensions 2
	CPUID_SSE3							= 0x00100,	// Streaming SIMD Extentions 3 aka Prescott's New Instructions
	CPUID_ALTIVEC						= 0x00200,	// AltiVec
} cpuidSimd_t;

typedef enum {
	AXIS_LEFT_X,
	AXIS_LEFT_Y,
	AXIS_RIGHT_X,
	AXIS_RIGHT_Y,
	AXIS_LEFT_TRIG,
	AXIS_RIGHT_TRIG,
	MAX_JOYSTICK_AXIS
} joystickAxis_t;

typedef enum {
	SE_NONE,				// evTime is still valid
	SE_KEY,					// evValue is a key code, evValue2 is the down flag
	SE_CHAR,				// evValue is an ascii char
	SE_MOUSE,				// evValue and evValue2 are relative signed x / y moves
	SE_MOUSE_ABS,			// evValue and evValue2 are absolute x / y coordinates in the window
	SE_JOYSTICK,			// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE				// evPtr is a char*, from typing something at a non-game console
} sysEventType_t;

typedef enum {
	M_ACTION1,
	M_ACTION2,
	M_ACTION3,
	M_ACTION4,
	M_ACTION5,
	M_ACTION6,
	M_ACTION7,
	M_ACTION8,
	M_DELTAX,
	M_DELTAY,
	M_DELTAZ
} sys_mEvents;

typedef enum {
	J_ACTION_FIRST,
	// these names are similar to the SDL3 SDL_GamepadButton names
	J_BTN_SOUTH = J_ACTION_FIRST, // bottom face button, like Xbox A
	J_BTN_EAST,  // right face button, like Xbox B
	J_BTN_WEST,  // left face button, like Xbox X
	J_BTN_NORTH, // top face button, like Xbox Y
	J_BTN_BACK,
	J_BTN_GUIDE, // Note: this one should probably not be used?
	J_BTN_START,
	J_BTN_LSTICK, // press left stick
	J_BTN_RSTICK, // press right stick
	J_BTN_LSHOULDER,
	J_BTN_RSHOULDER,

	J_DPAD_UP,
	J_DPAD_DOWN,
	J_DPAD_LEFT,
	J_DPAD_RIGHT,

	J_BTN_MISC1, // Additional button (e.g. Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button)
	J_BTN_RPADDLE1, // Upper or primary paddle, under your right hand (e.g. Xbox Elite paddle P1)
	J_BTN_LPADDLE1, // Upper or primary paddle, under your left hand (e.g. Xbox Elite paddle P3)
	J_BTN_RPADDLE2, // Lower or secondary paddle, under your right hand (e.g. Xbox Elite paddle P2)
	J_BTN_LPADDLE2, //  Lower or secondary paddle, under your left hand (e.g. Xbox Elite paddle P4)

	J_ACTION_MAX = J_BTN_LPADDLE2,
	// leaving some space here for about 12 additional J_ACTIONs, if needed

	J_AXIS_MIN = 32,
	J_AXIS_LEFT_X = J_AXIS_MIN + AXIS_LEFT_X,
	J_AXIS_LEFT_Y = J_AXIS_MIN + AXIS_LEFT_Y,
	J_AXIS_RIGHT_X = J_AXIS_MIN + AXIS_RIGHT_X,
	J_AXIS_RIGHT_Y = J_AXIS_MIN + AXIS_RIGHT_Y,
	J_AXIS_LEFT_TRIG = J_AXIS_MIN + AXIS_LEFT_TRIG,
	J_AXIS_RIGHT_TRIG = J_AXIS_MIN + AXIS_RIGHT_TRIG,

	J_AXIS_MAX = J_AXIS_MIN + MAX_JOYSTICK_AXIS - 1,

	MAX_JOY_EVENT
} sys_jEvents;

struct sysEvent_t {
	sysEventType_t	evType;
	int				evValue;	// for keys: K_* or ASCII code; for joystick: axis; for mouse: mouseX
	int				evValue2;	// for keys: 0/1 for up/down; for axis: value; for mouse: mouseY
	int				evPtrLength;		// bytes of data pointed to by evPtr, for journaling
	void *			evPtr;				// this must be manually freed if not NULL
};

enum sysPath_t {
	PATH_BASE,
	PATH_CONFIG,
	PATH_SAVE,
	PATH_EXE
};

template<class type> class idList;		// for Sys_ListFiles


void			Sys_Init( void );
void			Sys_Shutdown( void );
void			Sys_Error( const char *error, ...);
void			Sys_Quit( void );

// note that this isn't journaled...
char *			Sys_GetClipboardData( void );
void			Sys_FreeClipboardData( char* data );
void			Sys_SetClipboardData( const char *string );

// will go to the various text consoles
// NOT thread safe - never use in the async paths
void			Sys_Printf( const char *msg, ... )id_attribute((format(printf,1,2)));

// guaranteed to be thread-safe
void			Sys_DebugPrintf( const char *fmt, ... )id_attribute((format(printf,1,2)));
void			Sys_DebugVPrintf( const char *fmt, va_list arg );

// allow game to yield CPU time
// NOTE: due to SDL_TIMESLICE this is very bad portability karma, and should be completely removed
void			Sys_Sleep( int msec );

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
unsigned int	Sys_Milliseconds( void );

// returns a selection of the CPUID_* flags
int				Sys_GetProcessorId( void );

// sets the FPU precision
void			Sys_FPU_SetPrecision();

// sets Flush-To-Zero mode
void			Sys_FPU_SetFTZ( bool enable );

// sets Denormals-Are-Zero mode
void			Sys_FPU_SetDAZ( bool enable );

// returns amount of system ram
int				Sys_GetSystemRam( void );

// returns amount of drive space in path
int				Sys_GetDriveFreeSpace( const char *path );

// lock and unlock memory
bool			Sys_LockMemory( void *ptr, int bytes );
bool			Sys_UnlockMemory( void *ptr, int bytes );

// set amount of physical work memory
void			Sys_SetPhysicalWorkMemory( int minBytes, int maxBytes );

// DLL loading, the path should be a fully qualified OS path to the DLL file to be loaded
uintptr_t		Sys_DLL_Load( const char *dllName );
void *			Sys_DLL_GetProcAddress( uintptr_t dllHandle, const char *procName );
void			Sys_DLL_Unload( uintptr_t dllHandle );

// event generation
void			Sys_GenerateEvents( void );
sysEvent_t		Sys_GetEvent( void );
void			Sys_ClearEvents( void );
char			*Sys_ConsoleInput( void );

// input is tied to windows, so it needs to be started up and shut down whenever
// the main window is recreated
void			Sys_InitInput( void );
void			Sys_ShutdownInput( void );
void			Sys_InitScanTable( void );
unsigned char	Sys_GetConsoleKey( bool shifted );
// map a scancode key to a char
// does nothing on win32, as SE_KEY == SE_CHAR there
// on other OSes, consider the keyboard mapping
unsigned char	Sys_MapCharForKey( int key );
// for keynums between K_FIRST_SCANCODE and K_LAST_SCANCODE
// returns e.g. "SC_A" for K_SC_A
const char* Sys_GetScancodeName( int key );
// returns localized name of the key (between K_FIRST_SCANCODE and K_LAST_SCANCODE),
// regarding the current keyboard layout - if that name is in ASCII or corresponds
// to a "High-ASCII" char supported by Doom3.
// Otherwise return same name as Sys_GetScancodeName()
// !! Returned string is only valid until next call to this function !!
const char* Sys_GetLocalizedScancodeName( int key );
// returns keyNum_t (K_SC_* constant) for given scancode name (like "SC_A")
int Sys_GetKeynumForScancodeName( const char* name );

// returns display name of the key (between K_FIRST_JOY and K_LAST_JOY)
// With SDL2 it'll return the name in the SDL_GameController standard layout
// (which is based on XBox/XInput => on Nintendo gamepads, A/B and X/Y will be flipped),
// with SDL3 it will return the "real" button name
const char* Sys_GetLocalizedJoyKeyName( int key );

// keyboard input polling
int				Sys_PollKeyboardInputEvents( void );
int				Sys_ReturnKeyboardInputEvent( const int n, int &ch, bool &state );
void			Sys_EndKeyboardInputEvents( void );

// mouse input polling
int				Sys_PollMouseInputEvents( void );
int				Sys_ReturnMouseInputEvent( const int n, int &action, int &value );
void			Sys_EndMouseInputEvents( void );

// joystick input polling
void			Sys_SetRumble( int device, int low, int hi );
int				Sys_PollJoystickInputEvents( int deviceNum );
int				Sys_ReturnJoystickInputEvent( const int n, int &action, int &value );
void			Sys_EndJoystickInputEvents();

// when the console is down, or the game is about to perform a lengthy
// operation like map loading, the system can release the mouse cursor
// when in windowed mode
void			Sys_GrabMouseCursor( bool grabIt );

// DG: added this for an ungodly hack for gamepad support
// active = true means "currently a GUI with a cursor is active/focused"
// active = false means "that GUI is not active anymore"
class idUserInterface;
void			Sys_SetInteractiveIngameGuiActive( bool active, idUserInterface* ui );

void			Sys_ShowWindow( bool show );
bool			Sys_IsWindowVisible( void );
void			Sys_ShowConsole( int visLevel, bool quitOnClose );


void			Sys_Mkdir( const char *path );
ID_TIME_T			Sys_FileTimeStamp( FILE *fp );
// NOTE: do we need to guarantee the same output on all platforms?
const char *	Sys_TimeStampToStr( ID_TIME_T timeStamp );

bool			Sys_GetPath(sysPath_t type, idStr &path);

// use fs_debug to verbose Sys_ListFiles
// returns -1 if directory was not found (the list is cleared)
int				Sys_ListFiles( const char *directory, const char *extension, idList<class idStr> &list );

/*
==============================================================

	Networking

==============================================================
*/

typedef enum {
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP
} netadrtype_t;

typedef struct {
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
} netadr_t;

#define	PORT_ANY			-1

class idPort {
public:
				idPort();				// this just zeros netSocket and port
	virtual		~idPort();

	// if the InitForPort fails, the idPort.port field will remain 0
	bool		InitForPort( int portNumber );
	int			GetPort( void ) const { return bound_to.port; }
	netadr_t	GetAdr( void ) const { return bound_to; }
	void		Close();

	bool		GetPacket( netadr_t &from, void *data, int &size, int maxSize );
	bool		GetPacketBlocking( netadr_t &from, void *data, int &size, int maxSize, int timeout );
	void		SendPacket( const netadr_t to, const void *data, int size );

	int			packetsRead;
	int			bytesRead;

	int			packetsWritten;
	int			bytesWritten;

private:
	netadr_t	bound_to;		// interface and port
	int			netSocket;		// OS specific socket
};

class idTCP {
public:
				idTCP();
	virtual		~idTCP();

	// if host is host:port, the value of port is ignored
	bool		Init( const char *host, short port );
	void		Close();

	// returns -1 on failure (and closes socket)
	// those are non blocking, can be used for polling
	// there is no buffering, you are not guaranteed to Read or Write everything in a single call
	// (specially on win32, see recv and send documentation)
	int			Read( void *data, int size );
	int			Write( void *data, int size );

private:
	netadr_t	address;		// remote address
	int			fd;				// OS specific socket
};

				// parses the port number
				// can also do DNS resolve if you ask for it.
				// NOTE: DNS resolve is a slow/blocking call, think before you use
				// ( could be exploited for server DoS )
bool			Sys_StringToNetAdr( const char *s, netadr_t *a, bool doDNSResolve );
const char *	Sys_NetAdrToString( const netadr_t a );
bool			Sys_IsLANAddress( const netadr_t a );
bool			Sys_CompareNetAdrBase( const netadr_t a, const netadr_t b );

void			Sys_InitNetworking( void );
void			Sys_ShutdownNetworking( void );


/*
==============================================================

	Multi-threading

==============================================================
*/

struct SDL_Thread;

typedef int (*xthread_t)( void * );

typedef struct {
	const char		*name;
	SDL_Thread		*threadHandle;
	unsigned long	threadId;
} xthreadInfo;

void				Sys_CreateThread( xthread_t function, void *parms, xthreadInfo &info, const char *name );
void				Sys_DestroyThread( xthreadInfo& info ); // sets threadHandle back to 0

// find the name of the calling thread
// if index != NULL, set the index in threads array (use -1 for "main" thread)
const char *		Sys_GetThreadName( int *index = 0 );

extern void Sys_InitThreads();
extern void Sys_ShutdownThreads();

bool Sys_IsMainThread();

const int MAX_CRITICAL_SECTIONS		= 5;

enum {
	CRITICAL_SECTION_ZERO = 0,
	CRITICAL_SECTION_ONE,
	CRITICAL_SECTION_TWO,
	CRITICAL_SECTION_THREE,
	CRITICAL_SECTION_SYS
};

void				Sys_EnterCriticalSection( int index = CRITICAL_SECTION_ZERO );
void				Sys_LeaveCriticalSection( int index = CRITICAL_SECTION_ZERO );

const int MAX_TRIGGER_EVENTS		= 4;

enum {
	TRIGGER_EVENT_ZERO = 0,
	TRIGGER_EVENT_ONE,
	TRIGGER_EVENT_TWO,
	TRIGGER_EVENT_THREE
};

void				Sys_WaitForEvent( int index = TRIGGER_EVENT_ZERO );
void				Sys_TriggerEvent( int index = TRIGGER_EVENT_ZERO );

/*
==============================================================

	idSys

==============================================================
*/

class idSys {
public:
	virtual void			DebugPrintf( const char *fmt, ... )id_attribute((format(printf,2,3))) = 0;
	virtual void			DebugVPrintf( const char *fmt, va_list arg ) = 0;

	virtual unsigned int	GetMilliseconds( void ) = 0;
	virtual int				GetProcessorId( void ) = 0;
	virtual void			FPU_SetFTZ( bool enable ) = 0;
	virtual void			FPU_SetDAZ( bool enable ) = 0;

	virtual bool			LockMemory( void *ptr, int bytes ) = 0;
	virtual bool			UnlockMemory( void *ptr, int bytes ) = 0;

	virtual uintptr_t		DLL_Load( const char *dllName ) = 0;
	virtual void *			DLL_GetProcAddress( uintptr_t dllHandle, const char *procName ) = 0;
	virtual void			DLL_Unload( uintptr_t dllHandle ) = 0;
	virtual void			DLL_GetFileName( const char *baseName, char *dllName, int maxLength ) = 0;

	virtual sysEvent_t		GenerateMouseButtonEvent( int button, bool down ) = 0;
	virtual sysEvent_t		GenerateMouseMoveEvent( int deltax, int deltay ) = 0;

	virtual void			OpenURL( const char *url, bool quit ) = 0;
	virtual void			StartProcess( const char *exePath, bool quit ) = 0;
};

extern idSys *				sys;

#endif /* !__SYS_PUBLIC__ */
