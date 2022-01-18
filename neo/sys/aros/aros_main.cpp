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

#define DEBUG 1

#include <aros/debug.h>
#undef ASSERT

#include <proto/exec.h>
#include <proto/dos.h>

// undefine - conflict with ID functions
#undef Remove
#undef Insert
#undef Read
#undef Write
#undef Seek
#undef Flush
#undef Close
#undef Allocate
#undef Printf
#undef VPrintf

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <pwd.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>

#include <SDL_main.h>

#include "dll/dll.h"

#include "sys/platform.h"
#include "idlib/containers/StrList.h"
#include "framework/FileSystem.h"
#include "framework/KeyInput.h"
#include "framework/EditField.h"
#include "sys/sys_local.h"

#include "sys/aros/aros_public.h"

#define					COMMAND_HISTORY 64

extern idStr	adoom3_basepath;
extern idStr	adoom3_savepath;

static int				input_hide = 0;

idEditField				input_field;
static char				input_ret[256];

static idStr			history[ COMMAND_HISTORY ];	// cycle buffer
static int				history_count = 0;			// buffer fill up
static int				history_start = 0;			// current history start
static int				history_current = 0;			// goes back in history
idEditField				history_backup;				// the base edit line

// terminal support
idCVar in_tty( "in_tty", "1", CVAR_BOOL | CVAR_INIT | CVAR_SYSTEM, "terminal tab-completion and history" );

static bool				tty_enabled = false;
static struct termios	tty_tc;

static FILE* consoleLog = NULL;

// pid - useful when you attach to gdb..
idCVar com_pid( "com_pid", "0", CVAR_INTEGER | CVAR_INIT | CVAR_SYSTEM, "process id" );

// exit - quit - error --------------------------------------------------------

static int set_exit = 0;
static char exit_spawn[ 1024 ];

/*
================
AROS_Exit
================
*/
void AROS_Exit(int ret) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    if ( tty_enabled ) {
        Sys_Printf( "shutdown terminal support\n" );
#if (0)
        if ( tcsetattr( 0, TCSADRAIN, &tty_tc ) == -1 ) {
                Sys_Printf( "tcsetattr failed: %s\n", strerror( errno ) );
        }
#endif
    }
    // at this point, too late to catch signals
    AROS_ClearSigs();

    if( consoleLog != NULL ) {
        fclose(consoleLog);
        consoleLog = NULL;
    }

    // process spawning. it's best when it happens after everything has shut down
    if ( exit_spawn[0] ) {
        Sys_DoStartProcess( exit_spawn, false );
    }
    // in case of signal, handler tries a common->Quit
    // we use set_exit to maintain a correct exit code
    if ( set_exit ) {
        exit( set_exit );
    }
    exit( ret );
}

/*
================
AROS_SetExit
================
*/
void AROS_SetExit(int ret) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    set_exit = 0;
}

/*
===============
AROS_SetExitSpawn
set the process to be spawned when we quit
===============
*/
void AROS_SetExitSpawn( const char *exeName ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    idStr::Copynz( exit_spawn, exeName, 1024 );
}

/*
==================
idSysLocal::StartProcess
if !quit, start the process asap
otherwise, push it for execution at exit
(i.e. let complete shutdown of the game and freeing of resources happen)
NOTE: might even want to add a small delay?
==================
*/
void idSysLocal::StartProcess( const char *exeName, bool quit ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    if ( quit ) {
        common->DPrintf( "Sys_StartProcess %s (delaying until final exit)\n", exeName );
        AROS_SetExitSpawn( exeName );
        cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
        return;
    }

    common->DPrintf( "Sys_StartProcess %s\n", exeName );
    Sys_DoStartProcess( exeName );
}

/*
================
Sys_Quit
================
*/
void Sys_Quit(void) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    AROS_Exit( EXIT_SUCCESS );
}

/*
=================
Sys_Init
=================
*/
void Sys_Init( void ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    AROS_InitConsoleInput();
    com_pid.SetString( (char *)FindTask(NULL) );
    common->Printf( "pid: %p\n", com_pid.GetString() );
    common->Printf( "%d MB System Memory\n", Sys_GetSystemRam() );
#if (0)
#ifndef ID_DEDICATED
    common->Printf( "%d MB Video Memory\n", Sys_GetVideoRam() );
#endif
#endif
}

/*
=================
AROS_Shutdown
=================
*/
void AROS_Shutdown( void ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    for ( int i = 0; i < COMMAND_HISTORY; i++ ) {
            history[ i ].Clear();
    }
}

/*
=================
Sys_DLL_Load
=================
*/
uintptr_t Sys_DLL_Load( const char *path ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    void *handle = dllLoadLibrary( (char *)path, FilePart(path) );
    if ( !handle ) {
            Sys_Printf( "dllLoadLibrary '%s' failed\n", path );
    }
    return (uintptr_t)handle;
}

/*
=================
Sys_DLL_GetProcAddress
=================
*/
void* Sys_DLL_GetProcAddress( uintptr_t handle, const char *sym ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    //const char *error;
    void *ret = dllGetProcAddress( (void *)handle, (char *)sym);
    if (ret == NULL)  {
            Sys_Printf( "dllGetProcAddress '%s' failed\n", sym );
    }
    return ret;
}

/*
=================
Sys_DLL_Unload
=================
*/
void Sys_DLL_Unload( uintptr_t handle ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    dllFreeLibrary( (void *)handle);		
}

/*
================
Sys_ShowConsole
================
*/
void Sys_ShowConsole( int visLevel, bool quitOnClose ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);
}

// ---------------------------------------------------------------------------

ID_TIME_T Sys_FileTimeStamp(FILE * fp) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    struct stat st;
    fstat(fileno(fp), &st);
    return st.st_mtime;
}

bool Sys_FPU_StackIsEmpty( void ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    return true;
}

const char *Sys_FPU_GetState( void ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    return "";
}

void Sys_FPU_SetPrecision( int precision ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

}

/*
================
Sys_SetPhysicalWorkMemory
================
*/
void Sys_SetPhysicalWorkMemory( int minBytes, int maxBytes ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    common->DPrintf( "TODO: Sys_SetPhysicalWorkMemory\n" );
}

extern bool AROS_GetSavePath(char buf[1024]);

static void initLog()
{
	char logPath[1024];
	if(!AROS_GetSavePath(logPath))
		return;

	// TODO: create savePath directory if it doesn't exist..

	char logBkPath[1024];
	strcpy(logBkPath, logPath);
	idStr::Append(logBkPath, sizeof(logBkPath), PATHSEPERATOR_STR "dhewm3log-old.txt");
	idStr::Append(logPath, sizeof(logPath), PATHSEPERATOR_STR "dhewm3log.txt");

	rename(logPath, logBkPath); // I hope AROS supports this, but it's standard C89 so it should

	consoleLog = fopen(logPath, "w");
	if(consoleLog == NULL) {
		printf("WARNING: Couldn't open/create '%s', error was: %d (%s)\n", logPath, errno, strerror(errno));
	} else {
		time_t tt = time(NULL);
		const struct tm* tms = localtime(&tt);
		char timeStr[64] = {};
		strftime(timeStr, sizeof(timeStr), "%F %H:%M:%S", tms);
		fprintf(consoleLog, "Opened this log at %s\n", timeStr);
	}
}

/*
===============
AROS_EarlyInit
===============
*/
void AROS_EarlyInit( void ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    initLog();

    exit_spawn[0] = '\0';
    AROS_InitLibs();
    AROS_InitSigs();

    // TODO: logfile
}

/*
===============
AROS_InitConsoleInput
===============
*/
void AROS_InitConsoleInput( void ) {
    struct termios tc;

    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);


    if ( in_tty.GetBool() ) {
        if ( isatty( STDIN_FILENO ) != 1 ) {
            Sys_Printf( "terminal support disabled: stdin is not a tty\n" );
            in_tty.SetBool( false );
            return;
        }
#if !defined(__AROS__)
        if ( tcgetattr( 0, &tty_tc ) == -1 ) {
            Sys_Printf( "tcgetattr failed. disabling terminal support: %s\n", strerror( errno ) );
            in_tty.SetBool( false );
            return;
        }
        // make the input non blocking
        if ( fcntl( STDIN_FILENO, F_SETFL, fcntl( STDIN_FILENO, F_GETFL, 0 ) | O_NONBLOCK ) == -1 ) {
            Sys_Printf( "fcntl STDIN non blocking failed.  disabling terminal support: %s\n", strerror( errno ) );
            in_tty.SetBool( false );
            return;
        }
#endif
        tc = tty_tc;
        /*
          ECHO: don't echo input characters
          ICANON: enable canonical mode.  This  enables  the  special
                characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
                STATUS, and WERASE, and buffers by lines.
          ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
                DSUSP are received, generate the corresponding signal
        */
        tc.c_lflag &= ~(ECHO | ICANON);
        /*
          ISTRIP strip off bit 8
          INPCK enable input parity checking
        */
        tc.c_iflag &= ~(ISTRIP | INPCK);
        tc.c_cc[VMIN] = 1;
        tc.c_cc[VTIME] = 0;
        
#if !defined(__AROS__)
        if ( tcsetattr( 0, TCSADRAIN, &tc ) == -1 ) {
            Sys_Printf( "tcsetattr failed: %s\n", strerror( errno ) );
            Sys_Printf( "terminal support may not work correctly. Use +set in_tty 0 to disable it\n" );
        }
        // make the output non blocking
        if ( fcntl( STDOUT_FILENO, F_SETFL, fcntl( STDOUT_FILENO, F_GETFL, 0 ) | O_NONBLOCK ) == -1 ) {
            Sys_Printf( "fcntl STDOUT non blocking failed: %s\n", strerror( errno ) );
        }
#endif
        tty_enabled = true;
        // check the terminal type for the supported ones
        char *term = getenv( "TERM" );
        if ( term ) {
            if ( strcmp( term, "linux" ) && strcmp( term, "xterm" ) && strcmp( term, "xterm-color" ) && strcmp( term, "screen" ) ) {
                Sys_Printf( "WARNING: terminal type '%s' is unknown. terminal support may not work correctly\n", term );
            }
        }
        Sys_Printf( "terminal support enabled ( use +set in_tty 0 to disabled )\n" );
    } else {
        Sys_Printf( "terminal support disabled\n" );
    }
}

/*
================
terminal support utilities
================
*/

void tty_Del() {
    putchar('\b');
    putchar(' ');
    putchar('\b');
}

void tty_Left() {
    putchar('\b');
}

void tty_Right() {
    putchar(27);
    putchar('[');
    putchar('C');
}

// clear the display of the line currently edited
// bring cursor back to beginning of line
void tty_Hide() {
    int len, buf_len;

    if ( !tty_enabled ) {
        return;
    }
    if ( input_hide ) {
        input_hide++;
        return;
    }
    // clear after cursor
    len = strlen( input_field.GetBuffer() ) - input_field.GetCursor();
    while ( len > 0 ) {
        tty_Right();
        len--;
    }
    buf_len = strlen( input_field.GetBuffer() );
    while ( buf_len > 0 ) {
        tty_Del();
        buf_len--;
    }
    input_hide++;
}

// show the current line
void tty_Show() {
    //	int i;
    if ( !tty_enabled ) {
        return;
    }
    assert( input_hide > 0 );
    input_hide--;
    if ( input_hide == 0 ) {
        char *buf = input_field.GetBuffer();
        size_t len = strlen(buf);
        if ( len < 1 )
            return;

        len = write( STDOUT_FILENO, buf, len );
        if ( len < 1 )
            return;

        len -= input_field.GetCursor();
        while ( len > 0 ) {
            tty_Left();
            len--;
        }
    }
}

void tty_FlushIn() {
  char key;
  while ( ( key = getchar() ) != EOF ) {
      Sys_Printf( "'%d' ", key );
  }
  Sys_Printf( "\n" );
}

/*
================
Sys_ConsoleInput
Checks for a complete line of text typed in at the console.
Return NULL if a complete line is not ready.
================
*/
char *Sys_ConsoleInput( void ) {
    /*
	if ( tty_enabled ) {
		char	key;
		bool	hidden = false;
		while ( ( key = getchar() ) != EOF ) {
			if ( !hidden ) {
				tty_Hide();
				hidden = true;
			}
			switch ( key ) {
			case 1:
				input_field.SetCursor( 0 );
				break;
			case 5:
				input_field.SetCursor( strlen( input_field.GetBuffer() ) );
				break;
			case 127:
			case 8:
				input_field.CharEvent( K_BACKSPACE );
				break;
			case '\n':
				idStr::Copynz( input_ret, input_field.GetBuffer(), sizeof( input_ret ) );
				assert( hidden );
				tty_Show();
				putchar(key);
				input_field.Clear();
				if ( history_count < CMD_HIST ) {
					history[ history_count ] = input_ret;
					history_count++;
				} else {
					history[ history_start ] = input_ret;
					history_start++;
					history_start %= CMD_HIST;
				}
				history_current = 0;
				return input_ret;
			case '\t':
				input_field.AutoComplete();
				break;
			case 27: {
				// enter escape sequence mode
				if ( ( key = getchar() ) == EOF ) {
					Sys_Printf( "dropping sequence: '27' " );
					tty_FlushIn();
					assert( hidden );
					tty_Show();
					return NULL;
				}
				switch ( key ) {
				case 79:
					if ( ( key = getchar() ) == EOF ) {
						Sys_Printf( "dropping sequence: '27' '79' " );
						tty_FlushIn();
						assert( hidden );
						tty_Show();
						return NULL;
					}
					switch ( key ) {
					case 72:
						// xterm only
						input_field.SetCursor( 0 );
						break;
					case 70:
						// xterm only
						input_field.SetCursor( strlen( input_field.GetBuffer() ) );
						break;
					default:
						Sys_Printf( "dropping sequence: '27' '79' '%d' ", key );
						tty_FlushIn();
						assert( hidden );
						tty_Show();
						return NULL;
					}
					break;
				case 91: {
					if ( ( key = getchar() ) == EOF ) {
						Sys_Printf( "dropping sequence: '27' '91' " );
						tty_FlushIn();
						assert( hidden );
						tty_Show();
						return NULL;
					}
					switch ( key ) {
					case 49: {
						if ( ( key = getchar() ) == EOF  || key != 126 ) {
							Sys_Printf( "dropping sequence: '27' '91' '49' '%d' ", key );
							tty_FlushIn();
							assert( hidden );
							tty_Show();
							return NULL;
						}
						// only screen and linux terms
						input_field.SetCursor( 0 );
						break;
					}
					case 50: {
						if ( ( key = getchar() ) == EOF || key != 126 ) {
							Sys_Printf( "dropping sequence: '27' '91' '50' '%d' ", key );
							tty_FlushIn();
							assert( hidden );
							tty_Show();
							return NULL;
						}
						// all terms
						input_field.KeyDownEvent( K_INS );
						break;
					}
					case 52: {
						if ( ( key = getchar() ) == EOF || key != 126 ) {
							Sys_Printf( "dropping sequence: '27' '91' '52' '%d' ", key );
							tty_FlushIn();
							assert( hidden );
							tty_Show();
							return NULL;
						}
						// only screen and linux terms
						input_field.SetCursor( strlen( input_field.GetBuffer() ) );
						break;
					}
					case 51: {
						if ( ( key = getchar() ) == EOF ) {
							Sys_Printf( "dropping sequence: '27' '91' '51' " );
							tty_FlushIn();
							assert( hidden );
							tty_Show();
							return NULL;
						}
						if ( key == 126 ) {
							input_field.KeyDownEvent( K_DEL );
							break;
						}
						Sys_Printf( "dropping sequence: '27' '91' '51' '%d'", key );
						tty_FlushIn();
						assert( hidden );
						tty_Show();
						return NULL;
					}
					case 65:
					case 66: {
						// history
						if ( history_current == 0 ) {
							history_backup = input_field;
						}
						if ( key == 65 ) {
							// up
							history_current++;
						} else {
							// down
							history_current--;
						}
						// history_current cycle:
						// 0: current edit
						// 1 .. Min( CMD_HIST, history_count ): back in history
						if ( history_current < 0 ) {
							history_current = Min( CMD_HIST, history_count );
						} else {
							history_current %= Min( CMD_HIST, history_count ) + 1;
						}
						int index = -1;
						if ( history_current == 0 ) {
							input_field = history_backup;
						} else {
							index = history_start + Min( CMD_HIST, history_count ) - history_current;
							index %= CMD_HIST;
							assert( index >= 0 && index < CMD_HIST );
							input_field.SetBuffer( history[ index ] );
						}
						assert( hidden );
						tty_Show();
						return NULL;
					}
					case 67:
						input_field.KeyDownEvent( K_RIGHTARROW );
						break;
					case 68:
						input_field.KeyDownEvent( K_LEFTARROW );
						break;
					default:
						Sys_Printf( "dropping sequence: '27' '91' '%d' ", key );
						tty_FlushIn();
						assert( hidden );
						tty_Show();
						return NULL;
					}
					break;
				}
				default:
					Sys_Printf( "dropping sequence: '27' '%d' ", key );
					tty_FlushIn();
					assert( hidden );
					tty_Show();
					return NULL;
				}
				break;
			}
			default:
				if ( key >= ' ' ) {
					input_field.CharEvent( key );
					break;
				}
				Sys_Printf( "dropping sequence: '%d' ", key );
				tty_FlushIn();
				assert( hidden );
				tty_Show();
				return NULL;
			}
		}
		if ( hidden ) {
			tty_Show();
		}
		return NULL;
	} else {
		// no terminal support - read only complete lines
		int				len;
		fd_set			fdset;
		struct timeval	timeout;

		FD_ZERO( &fdset );
		FD_SET( STDIN_FILENO, &fdset );
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if ( select( 1, &fdset, NULL, NULL, &timeout ) == -1 || !FD_ISSET( 0, &fdset ) ) {
			return NULL;
		}

		len = read( 0, input_ret, sizeof( input_ret ) );
		if ( len == 0 ) {
			// EOF
			return NULL;
		}

		if ( len < 1 ) {
			Sys_Printf( "read failed: %s\n", strerror( errno ) );	// something bad happened, cancel this line and print an error
			return NULL;
		}

		if ( len == sizeof( input_ret ) ) {
			Sys_Printf( "read overflow\n" );	// things are likely to break, as input will be cut into pieces
		}

		input_ret[ len-1 ] = '\0';		// rip off the \n and terminate
		return input_ret;
	}*/
	return NULL;
}

/*
===============
low level output
===============
*/

void Sys_VPrintf(const char *msg, va_list arg) {
	// gonna use arg twice, so copy it
	va_list arg2;
	va_copy(arg2, arg);

	// first print to stdout()
	vprintf(msg, arg2);

	va_end(arg2); // arg2 is not needed anymore

	// then print to the log, if any
	if(consoleLog != NULL)
	{
		vfprintf(consoleLog, msg, arg);
	}
}

void Sys_DebugPrintf( const char *fmt, ... ) {
    va_list argptr;

    tty_Hide();
    va_start( argptr, fmt );
    Sys_VPrintf( fmt, argptr );
    va_end( argptr );
    tty_Show();
}

void Sys_DebugVPrintf( const char *fmt, va_list arg ) {
    tty_Hide();
    Sys_VPrintf( fmt, arg );
    tty_Show();
}

void Sys_Printf(const char *msg, ...) {
    va_list argptr;

    tty_Hide();
    va_start( argptr, msg );
    Sys_VPrintf( msg, argptr );
    va_end( argptr );
    tty_Show();
}

/*
================
Sys_Error
================
*/
void Sys_Error(const char *error, ...) {
    va_list argptr;

    Sys_Printf( "Sys_Error: " );
    va_start( argptr, error );
    Sys_DebugVPrintf( error, argptr );
    va_end( argptr );
    Sys_Printf( "\n" );

    AROS_Exit( EXIT_FAILURE );
}

/*
===============
Sys_Shutdown
===============
*/
void Sys_Shutdown( void ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    adoom3_basepath.Clear();
    adoom3_savepath.Clear();
    AROS_Shutdown();
}

/*
===============
Sys_FPU_EnableExceptions
===============
*/
void Sys_FPU_EnableExceptions( int exceptions ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);
}

/*
===============
Sys_FPE_handler
===============
*/
void Sys_FPE_handler( int signum, siginfo_t *info, void *context ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    assert( signum == SIGFPE );
    Sys_Printf( "FPE\n" );
}

/*
==================
Sys_DoStartProcess
if we don't fork, this function never returns
the no-fork lets you keep the terminal when you're about to spawn an installer

if the command contains spaces, system() is used. Otherwise the more straightforward execl ( system() blows though )
==================
*/
void Sys_DoStartProcess( const char *exeName, bool dofork ) {
    bool use_system = false;

    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    if ( strchr( exeName, ' ' ) ) {
            use_system = true;
    } else {
            // set exec rights when it's about a single file to execute
            struct stat buf;
            if ( stat( exeName, &buf ) == -1 ) {
                    printf( "stat %s failed: %s\n", exeName, strerror( errno ) );
            } else {
                    if ( chmod( exeName, buf.st_mode | S_IXUSR ) == -1 ) {
                            printf( "cmod +x %s failed: %s\n", exeName, strerror( errno ) );
                    }
            }
    }
    if ( use_system ) {
            printf( "system %s\n", exeName );
            if (system( exeName ) == -1)
                    printf( "system failed: %s\n", strerror( errno ) );
            else
                    sleep( 1 );	// on some systems I've seen that starting the new process and exiting this one should not be too close
    } else {
            printf( "execl %s\n", exeName );
            execl( exeName, exeName, 0 );
            printf( "execl failed: %s\n", strerror( errno ) );
    }
    // terminate
    _exit( 0 );
}

/*
=================
Sys_OpenURL
=================
*/
void idSysLocal::OpenURL( const char *url, bool quit ) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    AROS_OpenURL( url );
}

/*
===============
main
===============
*/
int main(int argc, char **argv) {
    bug("[ADoom3] %s()\n", __PRETTY_FUNCTION__);

    AROS_EarlyInit( );

    if ( argc > 1 ) {
            common->Init( argc-1, &argv[1] );
    } else {
            common->Init( 0, NULL );
    }

    Sys_Init( );

    while (1) {
            common->Frame();
    }
    return 0;
}
