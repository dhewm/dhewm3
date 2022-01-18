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

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pwd.h>
#include <dlfcn.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>

#include "sys/platform.h"
#include "idlib/containers/StrList.h"
#include "framework/FileSystem.h"
#include "framework/KeyInput.h"
#include "framework/EditField.h"
#include "framework/Licensee.h"
#include "sys/sys_local.h"

#include "sys/posix/posix_public.h"

#include <SDL.h> // clipboard

#define					COMMAND_HISTORY 64

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

// pid - useful when you attach to gdb..
idCVar com_pid( "com_pid", "0", CVAR_INTEGER | CVAR_INIT | CVAR_SYSTEM, "process id" );

// exit - quit - error --------------------------------------------------------

static int set_exit = 0;
static char exit_spawn[ 1024 ] = { 0 };

static FILE* consoleLog = NULL;
void Sys_VPrintf(const char *msg, va_list arg);

#ifdef snprintf
  // I actually wanna use real snprintf here, not idStr:snPrintf(),
  // so get rid of the use_idStr_snPrintf #define
  #undef snprintf
#endif

/*
================
Posix_Exit
================
*/
void Posix_Exit(int ret) {
	if ( tty_enabled ) {
		Sys_Printf( "shutdown terminal support\n" );
		if ( tcsetattr( 0, TCSADRAIN, &tty_tc ) == -1 ) {
			Sys_Printf( "tcsetattr failed: %s\n", strerror( errno ) );
		}
	}

	// process spawning. it's best when it happens after everything has shut down
	if ( exit_spawn[0] ) {
		Sys_DoStartProcess( exit_spawn, false );
	}

	if(consoleLog != NULL) {
		fclose(consoleLog);
		consoleLog = NULL;
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
Posix_SetExit
================
*/
void Posix_SetExit(int ret) {
	set_exit = 0;
}

/*
===============
Posix_SetExitSpawn
set the process to be spawned when we quit
===============
*/
void Posix_SetExitSpawn( const char *exeName ) {
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
	if ( quit ) {
		common->DPrintf( "Sys_StartProcess %s (delaying until final exit)\n", exeName );
		Posix_SetExitSpawn( exeName );
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
	Posix_Exit( EXIT_SUCCESS );
}

/*
================
Sys_Mkdir
================
*/
void Sys_Mkdir( const char *path ) {
	mkdir(path, 0777);
}

/*
================
Sys_ListFiles
================
*/
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
	struct dirent *d;
	DIR *fdir;
	bool dironly = false;
	char search[MAX_OSPATH];
	struct stat st;
	bool debug;

	list.Clear();

	debug = cvarSystem->GetCVarBool( "fs_debug" );

	if (!extension)
		extension = "";

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0) {
		extension = "";
		dironly = true;
	}

	// search
	// NOTE: case sensitivity of directory path can screw us up here
	if ((fdir = opendir(directory)) == NULL) {
		if (debug) {
			common->Printf("Sys_ListFiles: opendir %s failed\n", directory);
		}
		return -1;
	}

	while ((d = readdir(fdir)) != NULL) {
		idStr::snPrintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if (!dironly) {
			idStr look(search);
			idStr ext;
			look.ExtractFileExtension(ext);
			if (extension[0] != '\0' && ext.Icmp(&extension[1]) != 0) {
				continue;
			}
		}
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		list.Append(d->d_name);
	}

	closedir(fdir);

	if ( debug ) {
		common->Printf( "Sys_ListFiles: %d entries in %s\n", list.Num(), directory );
	}

	return list.Num();
}

/*
================
Posix_Cwd
================
*/
const char *Posix_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	if (getcwd( cwd, sizeof( cwd ) - 1 ))
		cwd[MAX_OSPATH-1] = 0;
	else
		cwd[0] = 0;

	return cwd;
}

/*
=================
Sys_Init
=================
*/
void Sys_Init( void ) {
	if(consoleLog != NULL)
		common->Printf("Logging console output to %s/dhewm3log.txt\n", Posix_GetSavePath());

	Posix_InitConsoleInput();
	com_pid.SetInteger( getpid() );
	common->Printf( "pid: %d\n", com_pid.GetInteger() );
	common->Printf( "%d MB System Memory\n", Sys_GetSystemRam() );
}

/*
=================
Posix_Shutdown
=================
*/
void Posix_Shutdown( void ) {
	for ( int i = 0; i < COMMAND_HISTORY; i++ ) {
		history[ i ].Clear();
	}
}

/*
=================
Sys_DLL_Load
TODO: OSX - use the native API instead? NSModule
=================
*/
uintptr_t Sys_DLL_Load( const char *path ) {
	void* ret = dlopen( path, RTLD_NOW );
	if (ret == NULL) {
		// dlopen() failed - this might be ok (we tried one possible path and the next will work)
		// or it might be worth warning about (the lib existed but still couldn't be loaded,
		// maybe a missing symbol or permission problems)
		// unfortunately we only get a string from dlerror(), not some distinctive error code..
		// so use try to open() the file to get a better idea what went wrong

		int fd = open(path, O_RDONLY);
		if (fd < 0) { // couldn't open file for reading either
			int e = errno;
			if(e != ENOENT) {
				// it didn't fail because the file doesn't exist - log it, might be interesting (=> likely permission problem)
				common->Warning("Failed to load lib '%s'! Reason: %s ( %s )\n", path, dlerror(), strerror(e));
			}
		} else {
			// file could be opened, so it exists => log just dlerror()
			close(fd);
			common->Warning("Failed to load lib '%s' even though it exists and is readable! Reason: %s\n", path, dlerror());
		}
	}
	return (uintptr_t)ret;
}

/*
=================
Sys_DLL_GetProcAddress
=================
*/
void* Sys_DLL_GetProcAddress( uintptr_t handle, const char *sym ) {
	const char *error;
	void *ret = dlsym( (void *)handle, sym );
	if ((error = dlerror()) != NULL)  {
		Sys_Printf( "dlsym '%s' failed: %s\n", sym, error );
	}
	return ret;
}

/*
=================
Sys_DLL_Unload
=================
*/
void Sys_DLL_Unload( uintptr_t handle ) {
	dlclose( (void *)handle );
}

/*
================
Sys_ShowConsole
================
*/
void Sys_ShowConsole( int visLevel, bool quitOnClose ) { }

// ---------------------------------------------------------------------------

ID_TIME_T Sys_FileTimeStamp(FILE * fp) {
	struct stat st;
	fstat(fileno(fp), &st);
	return st.st_mtime;
}

char *Sys_GetClipboardData(void) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return SDL_GetClipboardText();
#else
	Sys_Printf( "TODO: Sys_GetClipboardData\n" );
	return NULL;
#endif
}

void Sys_FreeClipboardData( char* data ) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_free( data );
#else
	assert( 0 && "why is this called, Sys_GetClipboardData() isn't implemented for SDL1.2" );
#endif
}

void Sys_SetClipboardData( const char *string ) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetClipboardText( string );
#else
	Sys_Printf( "TODO: Sys_SetClipboardData\n" );
#endif
}

/*
================
Sys_LockMemory
================
*/
bool Sys_LockMemory( void *ptr, int bytes ) {
	return true;
}

/*
================
Sys_UnlockMemory
================
*/
bool Sys_UnlockMemory( void *ptr, int bytes ) {
	return true;
}

/*
================
Sys_SetPhysicalWorkMemory
================
*/
void Sys_SetPhysicalWorkMemory( int minBytes, int maxBytes ) {
	common->DPrintf( "TODO: Sys_SetPhysicalWorkMemory\n" );
}

/*
===========
Sys_GetDriveFreeSpace
return in MegaBytes
===========
*/
int Sys_GetDriveFreeSpace( const char *path ) {
	common->DPrintf( "TODO: Sys_GetDriveFreeSpace\n" );
	return 1000 * 1024;
}


// ----------- lots of signal handling stuff ------------

static const int   crashSigs[]     = {  SIGILL,   SIGABRT,   SIGFPE,   SIGSEGV };
static const char* crashSigNames[] = { "SIGILL", "SIGABRT", "SIGFPE", "SIGSEGV" };

#if ( defined(__linux__) && defined(__GLIBC__) ) || defined(__FreeBSD__) || (defined(__APPLE__) && !defined(OSX_TIGER))
  #define D3_HAVE_BACKTRACE
  #include <execinfo.h>
#endif

// unlike Sys_Printf() this doesn't call tty_Hide(); and tty_Show();
// to minimize interaction with broken dhewm3 state
// (but unlike regular printf() it'll also write to dhewm3log.txt)
static void CrashPrintf(const char* msg, ...)
{
	va_list argptr;
	va_start( argptr, msg );
	Sys_VPrintf( msg, argptr );
	va_end( argptr );
}

#ifdef D3_HAVE_LIBBACKTRACE
// non-ancient versions of GCC and clang include libbacktrace
// for ancient versions it can be built from https://github.com/ianlancetaylor/libbacktrace
#include <backtrace.h>
#include <cxxabi.h> // for demangling C++ symbols

static struct backtrace_state *bt_state = NULL;

static void bt_error_callback( void *data, const char *msg, int errnum )
{
	CrashPrintf("libbacktrace ERROR: %d - %s\n", errnum, msg);
}

static void bt_syminfo_callback( void *data, uintptr_t pc, const char *symname,
								 uintptr_t symval, uintptr_t symsize )
{
	if (symname != NULL) {
		int status;
		// FIXME: sucks that __cxa_demangle() insists on using malloc().. but so does printf()
		char* name = abi::__cxa_demangle(symname, NULL, NULL, &status);
		if (name != NULL) {
			symname = name;
		}
		CrashPrintf("  %zu %s\n", pc, symname);
		free(name);
	} else {
		CrashPrintf("  %zu (unknown symbol)\n", pc);
	}
}

static int bt_pcinfo_callback( void *data, uintptr_t pc, const char *filename, int lineno, const char *function )
{
	if (data != NULL) {
		int* hadInfo = (int*)data;
		*hadInfo = (function != NULL);
	}

	if (function != NULL) {
		int status;
		// FIXME: sucks that __cxa_demangle() insists on using malloc()..
		char* name = abi::__cxa_demangle(function, NULL, NULL, &status);
		if (name != NULL) {
			function = name;
		}

		const char* fileNameNeo = strstr(filename, "/neo/");
		if (fileNameNeo != NULL) {
			filename = fileNameNeo+1; // I want "neo/bla/blub.cpp:42"
		}
		CrashPrintf("  %zu %s:%d %s\n", pc, filename, lineno, function);
		free(name);
	}

	return 0;
}

static void bt_error_dummy( void *data, const char *msg, int errnum )
{
	//CrashPrintf("ERROR-DUMMY: %d - %s\n", errnum, msg);
}

static int bt_simple_callback(void *data, uintptr_t pc)
{
	int pcInfoWorked = 0;
	// if this fails, the executable doesn't have debug info, that's ok (=> use bt_error_dummy())
	backtrace_pcinfo(bt_state, pc, bt_pcinfo_callback, bt_error_dummy, &pcInfoWorked);
	if (!pcInfoWorked) { // no debug info? use normal symbols instead
		// yes, it would be easier to call backtrace_syminfo() in bt_pcinfo_callback() if function == NULL,
		// but some libbacktrace versions (e.g. in Ubuntu 18.04's g++-7) don't call bt_pcinfo_callback
		// at all if no debug info was available - which is also the reason backtrace_full() can't be used..
		backtrace_syminfo(bt_state, pc, bt_syminfo_callback, bt_error_callback, NULL);
	}

	return 0;
}

#endif

static void signalhandlerCrash(int sig)
{
	const char* name = "";
	for(int i=0; i<sizeof(crashSigs)/sizeof(crashSigs[0]); ++i) {
		if(crashSigs[i] == sig)
			name = crashSigNames[i];
	}

	// TODO: should probably use a custom print function around write(STDERR_FILENO, ...)
	//       because printf() could allocate which is not good if processes state is fscked
	//       (could use backtrace_symbols_fd() then)
	CrashPrintf("\n\nLooks like %s crashed with signal %s (%d) - sorry!\n", ENGINE_VERSION, name, sig);

#ifdef D3_HAVE_LIBBACKTRACE
	if (bt_state != NULL) {
		int skip = 1; // skip this function in backtrace
		backtrace_simple(bt_state, skip, bt_simple_callback, bt_error_callback, NULL);
	} else {
		CrashPrintf("(No backtrace because libbacktrace state is NULL)\n");
	}
#elif defined(D3_HAVE_BACKTRACE)
	// this is partly based on Yamagi Quake II code
	void* array[128];
	int size = backtrace(array, sizeof(array)/sizeof(array[0]));
	char** strings = backtrace_symbols(array, size);

	CrashPrintf("\nBacktrace:\n");

	for(int i = 0; i < size; i++) {
		CrashPrintf("  %s\n", strings[i]);
	}

	CrashPrintf("\n(Sorry it's not overly useful, build with libbacktrace support to get function names)\n");

	free(strings);

#else
	CrashPrintf("(No Backtrace on this platform)\n");
#endif

	fflush(stdout);
	if(consoleLog != NULL) {
		fflush(consoleLog);
		// TODO: fclose(consoleLog); ?
		//       consoleLog = NULL;
	}

	raise(sig); // pass it on to system
}

static bool disableTTYinput = false;

static void signalhandlerConsoleStuff(int sig)
{
	if(sig == SIGTTIN) {
		// we get this if dhewm3 was started in foreground, then put to sleep with ctrl-z
		// and afterwards set to background..
		// as it's in background now, disable console input
		// (if someone uses fg afterwards that's their problem, this is already obscure enough)
		if(tty_enabled) {
			Sys_Printf( "Sent to background, disabling terminal support.\n" );
			in_tty.SetBool( false );
			tty_enabled = false;

			tcsetattr(0, TCSADRAIN, &tty_tc);

			// Note: this is only about TTY input, we'll still print to stdout
			// (which, I think, is normal for processes running in the background)
		}
	}

	// apparently we get SIGTTOU from tcsetattr() in Posix_InitConsoleInput()
	// so we'll handle the disabling console there (it checks for disableTTYinput)

	disableTTYinput = true;
}

static void installSigHandler(int sig, int flags, void (*handler)(int))
{
	struct sigaction sigact = {0};
	sigact.sa_handler = handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = flags;
	sigaction(sig, &sigact, NULL);
}

static bool dirExists(const char* dirPath)
{
	struct stat buf = {};
	if(stat(dirPath, &buf) == 0) {
		return (buf.st_mode & S_IFMT) == S_IFDIR;
	}
	return false;
}

static bool createPathRecursive(char* path)
{
	if(!dirExists(path)) {
		char* lastDirSep = strrchr(path, '/');
		if(lastDirSep != NULL) {
			*lastDirSep = '\0'; // cut off last part of the path and try first with parent directory
			bool ok = createPathRecursive(path);
			*lastDirSep = '/'; // restore path
			// if parent dir was successfully created (or already existed), create this dir
			if(ok && mkdir(path, 0755) == 0) {
				return true;
			}
		}
		return false;
	}
	return true;
}

void Posix_InitSignalHandlers( void )
{
#ifdef D3_HAVE_LIBBACKTRACE
	// can't use idStr here and thus can't use Sys_GetPath(PATH_EXE) => added Posix_GetExePath()
	const char* exePath = Posix_GetExePath();
	bt_state = backtrace_create_state(exePath[0] ? exePath : NULL, 0, bt_error_callback, NULL);
#endif

	for(int i=0; i<sizeof(crashSigs)/sizeof(crashSigs[0]); ++i)
	{
		installSigHandler(crashSigs[i], SA_RESTART|SA_RESETHAND, signalhandlerCrash);
	}

	installSigHandler(SIGTTIN, 0, signalhandlerConsoleStuff);
	installSigHandler(SIGTTOU, 0, signalhandlerConsoleStuff);

	// this is also a good place to open dhewm3log.txt for Sys_VPrintf()

	const char* savePath = Posix_GetSavePath();
	size_t savePathLen = strlen(savePath);
	if(savePathLen > 0 && savePathLen < PATH_MAX) {
		char logPath[PATH_MAX] = {};
		if(savePath[savePathLen-1] == '/') {
			--savePathLen;
		}
		memcpy(logPath, savePath, savePathLen);
		logPath[savePathLen] = '\0';
		if(!createPathRecursive(logPath)) {
			printf("WARNING: Couldn't create save path '%s'!\n", logPath);
			return;
		}
		char logFileName[PATH_MAX] = {};
		int fullLogLen = snprintf(logFileName, sizeof(logFileName), "%s/dhewm3log.txt", logPath);
		// cast to size_t which is unsigned and would get really big if fullLogLen < 0 (=> error in snprintf())
		if((size_t)fullLogLen >= sizeof(logFileName)) {
			printf("WARNING: Couldn't create dhewm3log.txt at '%s' because its length would be '%d' which is > PATH_MAX (%zd) or < 0!\n",
			       logPath, fullLogLen, (size_t)PATH_MAX);
			return;
		}
		struct stat buf;
		if(stat(logFileName, &buf) == 0) {
			// logfile exists, rename to dhewm3log-old.txt
			char oldLogFileName[PATH_MAX] = {};
			if((size_t)snprintf(oldLogFileName, sizeof(oldLogFileName), "%s/dhewm3log-old.txt", logPath) < sizeof(logFileName))
			{
				rename(logFileName, oldLogFileName);
			}
		}
		consoleLog = fopen(logFileName, "w");
		if(consoleLog == NULL) {
			printf("WARNING: Couldn't open/create '%s', error was: %d (%s)\n", logFileName, errno, strerror(errno));
		} else {
			time_t tt = time(NULL);
			const struct tm* tms = localtime(&tt);
			char timeStr[64] = {};
			strftime(timeStr, sizeof(timeStr), "%F %H:%M:%S", tms);
			fprintf(consoleLog, "Opened this log at %s\n", timeStr);
		}

	} else {
		printf("WARNING: Posix_GetSavePath() returned path with invalid length '%zd'!\n", savePathLen);
	}
}

// ----------- signal handling stuff done ------------

/*
===============
Posix_InitConsoleInput
===============
*/
void Posix_InitConsoleInput( void ) {
	struct termios tc;

	common->StartupVariable( "in_tty", false );

	if ( in_tty.GetBool() ) {
		if ( isatty( STDIN_FILENO ) != 1 ) {
			Sys_Printf( "terminal support disabled: stdin is not a tty\n" );
			in_tty.SetBool( false );
			return;
		}
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
		if ( tcsetattr( 0, TCSADRAIN, &tc ) == -1 ) {
			if(disableTTYinput) {
				// got SIGTTOU => running in the background (started with `./dhewm3 &` or similar)
				// so we shouldn't take any console input
				Sys_Printf( "Running in background, disabling terminal support.\n" );
				in_tty.SetBool( false );
				return;
			} else {
				Sys_Printf( "tcsetattr failed: %s (%d)\n", strerror( errno ), errno );
				Sys_Printf( "terminal support may not work correctly. Use +set in_tty 0 to disable it\n" );
			}
		}
#if 0
		// make the output non blocking
		if ( fcntl( STDOUT_FILENO, F_SETFL, fcntl( STDOUT_FILENO, F_GETFL, 0 ) | O_NONBLOCK ) == -1 ) {
			Sys_Printf( "fcntl STDOUT non blocking failed: %s\n", strerror( errno ) );
		}
#endif
		tty_enabled = true;
		// check the terminal type for the supported ones
		char *term = getenv( "TERM" );
		if ( term ) {
			if ( strcmp( term, "linux" ) != 0 && strcmp( term, "xterm" ) != 0
			     && idStr::Cmpn( term, "xterm-", 6 ) != 0 && strcmp( term, "screen" ) != 0) {
				Sys_Printf( "WARNING: terminal type '%s' is unknown. terminal support may not work correctly\n", term );
			}
		}
		Sys_Printf( "terminal support enabled ( use +set in_tty 0 to disable )\n" );
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
  int key;
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
	if ( tty_enabled ) {
		int	key;
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
				if ( history_count < COMMAND_HISTORY ) {
					history[ history_count ] = input_ret;
					history_count++;
				} else {
					history[ history_start ] = input_ret;
					history_start++;
					history_start %= COMMAND_HISTORY;
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
						// 1 .. Min( COMMAND_HISTORY, history_count ): back in history
						if ( history_current < 0 ) {
							history_current = Min( COMMAND_HISTORY, history_count );
						} else {
							history_current %= Min( COMMAND_HISTORY, history_count ) + 1;
						}
						int index = -1;
						if ( history_current == 0 ) {
							input_field = history_backup;
						} else {
							index = history_start + Min( COMMAND_HISTORY, history_count ) - history_current;
							index %= COMMAND_HISTORY;
							assert( index >= 0 && index < COMMAND_HISTORY );
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
		// disabled on OSX. works fine from a terminal, but launching from Finder is causing trouble
		// I'm pretty sure it could be re-enabled if needed, and just handling the Finder failure case right (TTimo)
#ifndef MACOS_X
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
#endif
	}
	return NULL;
}

/*
===============
low level output
===============
*/

void Sys_VPrintf( const char *msg, va_list arg ) {
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

	Posix_Exit( EXIT_FAILURE );
}
