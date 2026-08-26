#ifndef __COMPAT_131_H__
#define __COMPAT_131_H__

#if defined(_WIN32) && !defined(_WIN64)
#define D3_SDK_131_COMPAT 1
#else
#define D3_SDK_131_COMPAT 0
#endif

#if D3_SDK_131_COMPAT

#include "sys/platform.h"
#include "sys/sys_public.h"
#include "framework/Common.h"
#include "idlib/MapFile.h"
#include "renderer/Cinematic.h"
#include "renderer/RenderSystem.h"

class idSys;
class idFileSystem;
class idCollisionModelManager;
class idSoundSystem;
class idRenderSystem;

namespace Shim131 {

    const int SDK_131_API_VERSION = 8;

    class idSysShim131;
    class idFileSystemShim131;
    class idCollisionModelManagerShim131;
    class idSoundSystemShim131;
    class idRenderSystemShim131;

    extern idSysShim131 *sys131;
    extern idFileSystemShim131 *fileSystem131;
    extern idCollisionModelManagerShim131 *collisionModelManager131;
    extern idSoundSystemShim131 *soundSystem131;
    extern idRenderSystemShim131 *renderSystem131;

    /***********************************************************************

      idSys (v1.3.1)

    ***********************************************************************/

    typedef enum {
        SE_NONE,				// evTime is still valid
        SE_KEY,					// evValue is a key code, evValue2 is the down flag
        SE_CHAR,				// evValue is an ascii char
        SE_MOUSE,				// evValue and evValue2 are reletive signed x / y moves
        SE_JOYSTICK_AXIS,		// evValue is an axis number and evValue2 is the current state (-127 to 127)
        SE_CONSOLE				// evPtr is a char*, from typing something at a non-game console
    } sysEventType_t;

    typedef struct sysEvent_s {
        sysEventType_t  	evType;
        int			    	evValue;
        int			    	evValue2;
        int			    	evPtrLength;		// bytes of data pointed to by evPtr, for journaling
        void *		    	evPtr;				// this must be manually freed if not NULL
    } sysEvent_t;

    typedef enum {
        CPUID_NONE							= 0x00000,
        CPUID_UNSUPPORTED					= 0x00001,	// unsupported (386/486)
        CPUID_GENERIC						= 0x00002,	// unrecognized processor
        CPUID_INTEL							= 0x00004,	// Intel
        CPUID_AMD							= 0x00008,	// AMD
        CPUID_MMX							= 0x00010,	// Multi Media Extensions
        CPUID_3DNOW							= 0x00020,	// 3DNow!
        CPUID_SSE							= 0x00040,	// Streaming SIMD Extensions
        CPUID_SSE2							= 0x00080,	// Streaming SIMD Extensions 2
        CPUID_SSE3							= 0x00100,	// Streaming SIMD Extentions 3 aka Prescott's New Instructions
        CPUID_ALTIVEC						= 0x00200,	// AltiVec
        CPUID_HTT							= 0x01000,	// Hyper-Threading Technology
        CPUID_CMOV							= 0x02000,	// Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
        CPUID_FTZ							= 0x04000,	// Flush-To-Zero mode (denormal results are flushed to zero)
        CPUID_DAZ							= 0x08000	// Denormals-Are-Zero mode (denormal source operands are set to zero)
    } cpuid_t;

    typedef unsigned long address_t;

    class idSys {
    public:
        virtual void			DebugPrintf( const char *fmt, ... )id_attribute((format(printf,2,3))) = 0;
        virtual void			DebugVPrintf( const char *fmt, va_list arg ) = 0;

        virtual double			GetClockTicks( void ) = 0;
        virtual double			ClockTicksPerSecond( void ) = 0;
        virtual cpuid_t 		GetProcessorId( void ) = 0;
        virtual const char *	GetProcessorString( void ) = 0;
        virtual const char *	FPU_GetState( void ) = 0;
        virtual bool			FPU_StackIsEmpty( void ) = 0;
        virtual void			FPU_SetFTZ( bool enable ) = 0;
        virtual void			FPU_SetDAZ( bool enable ) = 0;

        virtual void			FPU_EnableExceptions( int exceptions ) = 0;

        virtual bool			LockMemory( void *ptr, int bytes ) = 0;
        virtual bool			UnlockMemory( void *ptr, int bytes ) = 0;

        virtual void			GetCallStack( address_t *callStack, const int callStackSize ) = 0;
        virtual const char *	GetCallStackStr( const address_t *callStack, const int callStackSize ) = 0;
        virtual const char *	GetCallStackCurStr( int depth ) = 0;
        virtual void			ShutdownSymbols( void ) = 0;

        virtual int				DLL_Load( const char *dllName ) = 0;
        virtual void *			DLL_GetProcAddress( int dllHandle, const char *procName ) = 0;
        virtual void			DLL_Unload( int dllHandle ) = 0;
        virtual void			DLL_GetFileName( const char *baseName, char *dllName, int maxLength ) = 0;

        virtual sysEvent_t  	GenerateMouseButtonEvent( int button, bool down ) = 0;
        virtual sysEvent_t  	GenerateMouseMoveEvent( int deltax, int deltay ) = 0;

        virtual void			OpenURL( const char *url, bool quit ) = 0;
        virtual void			StartProcess( const char *exePath, bool quit ) = 0;

    };

    class idSysShim131 : public idSys {
    public:
        idSysShim131() : sys( NULL ) {}

        void Shim(::idSys *sys) { this->sys = sys; }

        virtual void			DebugPrintf( const char *fmt, ... )id_attribute((format(printf,2,3)));
        virtual void			DebugVPrintf( const char *fmt, va_list arg );

        virtual double			GetClockTicks( void );
        virtual double			ClockTicksPerSecond( void );
        virtual cpuid_t 		GetProcessorId( void );
        virtual const char *	GetProcessorString( void );
        virtual const char *	FPU_GetState( void );
        virtual bool			FPU_StackIsEmpty( void );
        virtual void			FPU_SetFTZ( bool enable );
        virtual void			FPU_SetDAZ( bool enable );

        virtual void			FPU_EnableExceptions( int exceptions );

        virtual bool			LockMemory( void *ptr, int bytes );
        virtual bool			UnlockMemory( void *ptr, int bytes );

        virtual void			GetCallStack( address_t *callStack, const int callStackSize );
        virtual const char *	GetCallStackStr( const address_t *callStack, const int callStackSize );
        virtual const char *	GetCallStackCurStr( int depth );
        virtual void			ShutdownSymbols( void );

        virtual int				DLL_Load( const char *dllName );
        virtual void *			DLL_GetProcAddress( int dllHandle, const char *procName );
        virtual void			DLL_Unload( int dllHandle );
        virtual void			DLL_GetFileName( const char *baseName, char *dllName, int maxLength );

        virtual sysEvent_t  	GenerateMouseButtonEvent( int button, bool down );
        virtual sysEvent_t  	GenerateMouseMoveEvent( int deltax, int deltay );

        virtual void			OpenURL( const char *url, bool quit );
        virtual void			StartProcess( const char *exePath, bool quit );
    private:
        ::idSys *sys;
    };

    /***********************************************************************

      idFileSystem (v1.3.1)

    ***********************************************************************/

    class idModList;
    class idFileList;
    class idFile;
    class idDict;
    class backgroundDownload_t;

    typedef int findFile_t;
    typedef int fsPureReply_t;
    typedef int fsMode_t;

    class idFileSystem {
    public:
        virtual					~idFileSystem() {}
                                // Initializes the file system.
        virtual void			Init( void ) = 0;
                                // Restarts the file system.
        virtual void			Restart( void ) = 0;
                                // Shutdown the file system.
        virtual void			Shutdown( bool reloading ) = 0;
                                // Returns true if the file system is initialized.
        virtual bool			IsInitialized( void ) const = 0;
                                // Returns true if we are doing an fs_copyfiles.
        virtual bool			PerformingCopyFiles( void ) const = 0;
                                // Returns a list of mods found along with descriptions
                                // 'mods' contains the directory names to be passed to fs_game
                                // 'descriptions' contains a free form string to be used in the UI
        virtual idModList *		ListMods( void ) = 0;
                                // Frees the given mod list
        virtual void			FreeModList( idModList *modList ) = 0;
                                // Lists files with the given extension in the given directory.
                                // Directory should not have either a leading or trailing '/'
                                // The returned files will not include any directories or '/' unless fullRelativePath is set.
                                // The extension must include a leading dot and may not contain wildcards.
                                // If extension is "/", only subdirectories will be returned.
        virtual idFileList *	ListFiles( const char *relativePath, const char *extension, bool sort = false, bool fullRelativePath = false, const char* gamedir = NULL ) = 0;
                                // Lists files in the given directory and all subdirectories with the given extension.
                                // Directory should not have either a leading or trailing '/'
                                // The returned files include a full relative path.
                                // The extension must include a leading dot and may not contain wildcards.
        virtual idFileList *	ListFilesTree( const char *relativePath, const char *extension, bool sort = false, const char* gamedir = NULL ) = 0;
                                // Frees the given file list.
        virtual void			FreeFileList( idFileList *fileList ) = 0;
                                // Converts a relative path to a full OS path.
        virtual const char *	OSPathToRelativePath( const char *OSPath ) = 0;
                                // Converts a full OS path to a relative path.
        virtual const char *	RelativePathToOSPath( const char *relativePath, const char *basePath = "fs_devpath" ) = 0;
                                // Builds a full OS path from the given components.
        virtual const char *	BuildOSPath( const char *base, const char *game, const char *relativePath ) = 0;
                                // Creates the given OS path for as far as it doesn't exist already.
        virtual void			CreateOSPath( const char *OSPath ) = 0;
                                // Returns true if a file is in a pak file.
        virtual bool			FileIsInPAK( const char *relativePath ) = 0;
                                // Returns a space separated string containing the checksums of all referenced pak files.
                                // will call SetPureServerChecksums internally to restrict itself
        virtual void			UpdatePureServerChecksums( void ) = 0;
                                // setup the mapping of OS -> game pak checksum
        virtual bool			UpdateGamePakChecksums( void ) = 0;
                                // 0-terminated list of pak checksums
                                // if pureChecksums[ 0 ] == 0, all data sources will be allowed
                                // otherwise, only pak files that match one of the checksums will be checked for files
                                // with the sole exception of .cfg files.
                                // the function tries to configure pure mode from the paks already referenced and this new list
                                // it returns wether the switch was successfull, and sets the missing checksums
                                // the process is verbosive when fs_debug 1
        virtual fsPureReply_t	SetPureServerChecksums( const int pureChecksums[], int gamePakChecksum, int missingChecksums[], int *missingGamePakChecksum ) = 0;
                                // fills a 0-terminated list of pak checksums for a client
                                // if OS is -1, give the current game pak checksum. if >= 0, lookup the game pak table (server only)
        virtual void			GetPureServerChecksums( int checksums[], int OS, int *gamePakChecksum ) = 0;
                                // before doing a restart, force the pure list and the search order
                                // if the given checksum list can't be completely processed and set, will error out
        virtual void			SetRestartChecksums( const int pureChecksums[], int gamePakChecksum ) = 0;
                                // equivalent to calling SetPureServerChecksums with an empty list
        virtual	void			ClearPureChecksums( void ) = 0;
                                // get a mask of supported OSes. if not pure, returns -1
        virtual int				GetOSMask( void ) = 0;
                                // Reads a complete file.
                                // Returns the length of the file, or -1 on failure.
                                // A null buffer will just return the file length without loading.
                                // A null timestamp will be ignored.
                                // As a quick check for existance. -1 length == not present.
                                // A 0 byte will always be appended at the end, so string ops are safe.
                                // The buffer should be considered read-only, because it may be cached for other uses.
        virtual int				ReadFile( const char *relativePath, void **buffer, ID_TIME_T *timestamp = NULL ) = 0;
                                // Frees the memory allocated by ReadFile.
        virtual void			FreeFile( void *buffer ) = 0;
                                // Writes a complete file, will create any needed subdirectories.
                                // Returns the length of the file, or -1 on failure.
        virtual int				WriteFile( const char *relativePath, const void *buffer, int size, const char *basePath = "fs_savepath" ) = 0;
                                // Removes the given file.
        virtual void			RemoveFile( const char *relativePath ) = 0;
                                // Opens a file for reading.
        virtual idFile *		OpenFileRead( const char *relativePath, bool allowCopyFiles = true, const char* gamedir = NULL ) = 0;
                                // Opens a file for writing, will create any needed subdirectories.
        virtual idFile *		OpenFileWrite( const char *relativePath, const char *basePath = "fs_savepath" ) = 0;
                                // Opens a file for writing at the end.
        virtual idFile *		OpenFileAppend( const char *filename, bool sync = false, const char *basePath = "fs_basepath" ) = 0;
                                // Opens a file for reading, writing, or appending depending on the value of mode.
        virtual idFile *		OpenFileByMode( const char *relativePath, fsMode_t mode ) = 0;
                                // Opens a file for reading from a full OS path.
        virtual idFile *		OpenExplicitFileRead( const char *OSPath ) = 0;
                                // Opens a file for writing to a full OS path.
        virtual idFile *		OpenExplicitFileWrite( const char *OSPath ) = 0;
                                // Closes a file.
        virtual void			CloseFile( idFile *f ) = 0;
                                // Returns immediately, performing the read from a background thread.
        virtual void			BackgroundDownload( backgroundDownload_t *bgl ) = 0;
                                // resets the bytes read counter
        virtual void			ResetReadCount( void ) = 0;
                                // retrieves the current read count
        virtual int				GetReadCount( void ) = 0;
                                // adds to the read count
        virtual void			AddToReadCount( int c ) = 0;
                                // look for a dynamic module
        virtual void			FindDLL( const char *basename, char dllPath[], bool updateChecksum ) = 0;
                                // case sensitive filesystems use an internal directory cache
                                // the cache is cleared when calling OpenFileWrite and RemoveFile
                                // in some cases you may need to use this directly
        virtual void			ClearDirCache( void ) = 0;

                                // is D3XP installed? even if not running it atm
        virtual bool			HasD3XP( void ) = 0;
                                // are we using D3XP content ( through a real d3xp run or through a double mod )
        virtual bool			RunningD3XP( void ) = 0;

                                // don't use for large copies - allocates a single memory block for the copy
        virtual void			CopyFile( const char *fromOSPath, const char *toOSPath ) = 0;

                                // lookup a relative path, return the size or 0 if not found
        virtual int				ValidateDownloadPakForChecksum( int checksum, char path[ MAX_STRING_CHARS ], bool isGamePak ) = 0;

        virtual idFile *		MakeTemporaryFile( void ) = 0;

                                // make downloaded pak files known so pure negociation works next time
        virtual int				AddZipFile( const char *path ) = 0;

                                // look for a file in the loaded paks or the addon paks
                                // if the file is found in addons, FS's internal structures are ready for a reloadEngine
        virtual findFile_t		FindFile( const char *path, bool scheduleAddons ) = 0;

                                // get map/addon decls and take into account addon paks that are not on the search list
                                // the decl 'name' is in the "path" entry of the dict
        virtual int				GetNumMaps() = 0;
        virtual const idDict *	GetMapDecl( int i ) = 0;
        virtual void			FindMapScreenshot( const char *path, char *buf, int len ) = 0;

                                // ignore case and seperator char distinctions
        virtual bool			FilenameCompare( const char *s1, const char *s2 ) const = 0;
    };

    class idFileSystemShim131 : public idFileSystem {
    public:
        idFileSystemShim131() : fileSystem( NULL ) {}
        void Shim(::idFileSystem *fileSystem) { this->fileSystem = fileSystem; }

        virtual void			Init( void );
        virtual void			Restart( void );
        virtual void			Shutdown( bool reloading );
        virtual bool			IsInitialized( void ) const;
        virtual bool			PerformingCopyFiles( void ) const;
        virtual idModList *		ListMods( void );
        virtual void			FreeModList( idModList *modList );
        virtual idFileList *	ListFiles( const char *relativePath, const char *extension, bool sort = false, bool fullRelativePath = false, const char* gamedir = NULL );
        virtual idFileList *	ListFilesTree( const char *relativePath, const char *extension, bool sort = false, const char* gamedir = NULL );
        virtual void			FreeFileList( idFileList *fileList );
        virtual const char *	OSPathToRelativePath( const char *OSPath );
        virtual const char *	RelativePathToOSPath( const char *relativePath, const char *basePath = "fs_devpath" );
        virtual const char *	BuildOSPath( const char *base, const char *game, const char *relativePath );
        virtual void			CreateOSPath( const char *OSPath );
        virtual bool			FileIsInPAK( const char *relativePath );
        virtual void			UpdatePureServerChecksums( void );
        virtual bool			UpdateGamePakChecksums( void );
        virtual fsPureReply_t	SetPureServerChecksums( const int pureChecksums[], int gamePakChecksum, int missingChecksums[], int *missingGamePakChecksum );
        virtual void			GetPureServerChecksums( int checksums[], int OS, int *gamePakChecksum );
        virtual void			SetRestartChecksums( const int pureChecksums[], int gamePakChecksum );
        virtual	void			ClearPureChecksums( void );
        virtual int				GetOSMask( void );
        virtual int				ReadFile( const char *relativePath, void **buffer, ID_TIME_T *timestamp = NULL );
        virtual void			FreeFile( void *buffer );
        virtual int				WriteFile( const char *relativePath, const void *buffer, int size, const char *basePath = "fs_savepath" );
        virtual void			RemoveFile( const char *relativePath );
        virtual idFile *		OpenFileRead( const char *relativePath, bool allowCopyFiles = true, const char* gamedir = NULL );
        virtual idFile *		OpenFileWrite( const char *relativePath, const char *basePath = "fs_savepath" );
        virtual idFile *		OpenFileAppend( const char *filename, bool sync = false, const char *basePath = "fs_basepath" );
        virtual idFile *		OpenFileByMode( const char *relativePath, fsMode_t mode );
        virtual idFile *		OpenExplicitFileRead( const char *OSPath );
        virtual idFile *		OpenExplicitFileWrite( const char *OSPath );
        virtual void			CloseFile( idFile *f );
        virtual void			BackgroundDownload( backgroundDownload_t *bgl );
        virtual void			ResetReadCount( void );
        virtual int				GetReadCount( void );
        virtual void			AddToReadCount( int c );
        virtual void			FindDLL( const char *basename, char dllPath[], bool updateChecksum );
        virtual void			ClearDirCache( void );

        virtual bool			HasD3XP( void );
        virtual bool			RunningD3XP( void );

        virtual void			CopyFile( const char *fromOSPath, const char *toOSPath );

        virtual int				ValidateDownloadPakForChecksum( int checksum, char path[ MAX_STRING_CHARS ], bool isGamePak );

        virtual idFile *		MakeTemporaryFile( void );

        virtual int				AddZipFile( const char *path );

        virtual findFile_t		FindFile( const char *path, bool scheduleAddons = false );

        virtual int				GetNumMaps();
        virtual const idDict *	GetMapDecl( int i );
        virtual void			FindMapScreenshot( const char *path, char *buf, int len );

        virtual bool			FilenameCompare( const char *s1, const char *s2 ) const;
    private:
        ::idFileSystem *fileSystem;
    };

    /***********************************************************************

      idCollisionModelManager (v1.3.1)

    ***********************************************************************/

    class idTraceModel;
    class idBounds;
    class idFixedWinding;
    class contactInfo_t;
    class idMapEntity;
    class idMaterial;
    class trace_t;

    typedef int cmHandle_t;

    struct idMapFileShim131 {
        float					version;
        unsigned int			fileTime;
        unsigned int			geometryCRC;
        idList<idMapEntity *>	entities;
        idStr					name;
        bool					hasPrimitiveData;
    };

    class idCollisionModelManager {
    public:
        virtual					~idCollisionModelManager( void ) {}

        // Loads collision models from a map file.
        virtual void			LoadMap( const idMapFileShim131 *mapFile ) = 0;
        // Frees all the collision models.
        virtual void			FreeMap( void ) = 0;

        // Gets the clip handle for a model.
        virtual cmHandle_t		LoadModel( const char *modelName, const bool precache ) = 0;
        // Sets up a trace model for collision with other trace models.
        virtual cmHandle_t		SetupTrmModel( const idTraceModel &trm, const idMaterial *material ) = 0;
        // Creates a trace model from a collision model, returns true if succesfull.
        virtual bool			TrmFromModel( const char *modelName, idTraceModel &trm ) = 0;

        // Gets the name of a model.
        virtual const char *	GetModelName( cmHandle_t model ) const = 0;
        // Gets the bounds of a model.
        virtual bool			GetModelBounds( cmHandle_t model, idBounds &bounds ) const = 0;
        // Gets all contents flags of brushes and polygons of a model ored together.
        virtual bool			GetModelContents( cmHandle_t model, int &contents ) const = 0;
        // Gets a vertex of a model.
        virtual bool			GetModelVertex( cmHandle_t model, int vertexNum, idVec3 &vertex ) const = 0;
        // Gets an edge of a model.
        virtual bool			GetModelEdge( cmHandle_t model, int edgeNum, idVec3 &start, idVec3 &end ) const = 0;
        // Gets a polygon of a model.
        virtual bool			GetModelPolygon( cmHandle_t model, int polygonNum, idFixedWinding &winding ) const = 0;

        // Translates a trace model and reports the first collision if any.
        virtual void			Translation( trace_t *results, const idVec3 &start, const idVec3 &end,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) = 0;
        // Rotates a trace model and reports the first collision if any.
        virtual void			Rotation( trace_t *results, const idVec3 &start, const idRotation &rotation,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) = 0;
        // Returns the contents touched by the trace model or 0 if the trace model is in free space.
        virtual int				Contents( const idVec3 &start,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) = 0;
        // Stores all contact points of the trace model with the model, returns the number of contacts.
        virtual int				Contacts( contactInfo_t *contacts, const int maxContacts, const idVec3 &start, const idVec6 &dir, const float depth,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) = 0;

        // Tests collision detection.
        virtual void			DebugOutput( const idVec3 &origin ) = 0;
        // Draws a model.
        virtual void			DrawModel( cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis,
                                                    const idVec3 &viewOrigin, const float radius ) = 0;
        // Prints model information, use -1 handle for accumulated model info.
        virtual void			ModelInfo( cmHandle_t model ) = 0;
        // Lists all loaded models.
        virtual void			ListModels( void ) = 0;
        // Writes a collision model file for the given map entity.
        virtual bool			WriteCollisionModelForMapEntity( const idMapEntity *mapEnt, const char *filename, const bool testTraceModel = true ) = 0;
    };

    class idCollisionModelManagerShim131 : public idCollisionModelManager {
    public:
        idCollisionModelManagerShim131() : collisionModelManager( NULL ) {}
        void Shim(::idCollisionModelManager *collisionModelManager) { this->collisionModelManager = collisionModelManager; }

        // Loads collision models from a map file.
        virtual void			LoadMap( const idMapFileShim131 *mapFile );
        // Frees all the collision models.
        virtual void			FreeMap( void );

        // Gets the clip handle for a model.
        virtual cmHandle_t		LoadModel( const char *modelName, const bool precache );
        // Sets up a trace model for collision with other trace models.
        virtual cmHandle_t		SetupTrmModel( const idTraceModel &trm, const idMaterial *material );
        // Creates a trace model from a collision model, returns true if succesfull.
        virtual bool			TrmFromModel( const char *modelName, idTraceModel &trm );

        // Gets the name of a model.
        virtual const char *	GetModelName( cmHandle_t model ) const;
        // Gets the bounds of a model.
        virtual bool			GetModelBounds( cmHandle_t model, idBounds &bounds ) const;
        // Gets all contents flags of brushes and polygons of a model ored together.
        virtual bool			GetModelContents( cmHandle_t model, int &contents ) const;
        // Gets a vertex of a model.
        virtual bool			GetModelVertex( cmHandle_t model, int vertexNum, idVec3 &vertex ) const;
        // Gets an edge of a model.
        virtual bool			GetModelEdge( cmHandle_t model, int edgeNum, idVec3 &start, idVec3 &end ) const;
        // Gets a polygon of a model.
        virtual bool			GetModelPolygon( cmHandle_t model, int polygonNum, idFixedWinding &winding ) const;

        // Translates a trace model and reports the first collision if any.
        virtual void			Translation( trace_t *results, const idVec3 &start, const idVec3 &end,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis );
        // Rotates a trace model and reports the first collision if any.
        virtual void			Rotation( trace_t *results, const idVec3 &start, const idRotation &rotation,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis );
        // Returns the contents touched by the trace model or 0 if the trace model is in free space.
        virtual int				Contents( const idVec3 &start,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis );
        // Stores all contact points of the trace model with the model, returns the number of contacts.
        virtual int				Contacts( contactInfo_t *contacts, const int maxContacts, const idVec3 &start, const idVec6 &dir, const float depth,
                                    const idTraceModel *trm, const idMat3 &trmAxis, int contentMask,
                                    cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis );

        // Tests collision detection.
        virtual void			DebugOutput( const idVec3 &origin );
        // Draws a model.
        virtual void			DrawModel( cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis,
                                                    const idVec3 &viewOrigin, const float radius );
        // Prints model information, use -1 handle for accumulated model info.
        virtual void			ModelInfo( cmHandle_t model );
        // Lists all loaded models.
        virtual void			ListModels( void );
        // Writes a collision model file for the given map entity.
        virtual bool			WriteCollisionModelForMapEntity( const idMapEntity *mapEnt, const char *filename, const bool testTraceModel = true );
    private:
        ::idCollisionModelManager *collisionModelManager;
    };

    /***********************************************************************

      idSoundSystem (v1.3.1)

    ***********************************************************************/

    class idRenderWorld;
    class idSoundWorld;
    class soundDecoderInfo_t;

    class idSoundSystem {
    public:
        virtual					~idSoundSystem( void ) {}

        // all non-hardware initialization
        virtual void			Init( void ) = 0;

        // shutdown routine
        virtual	void			Shutdown( void ) = 0;

        // call ClearBuffer if there is a chance that the AsyncUpdate won't get called
        // for 20+ msec, which would cause a stuttering repeat of the current
        // buffer contents
        virtual void			ClearBuffer( void ) = 0;

        // sound is attached to the window, and must be recreated when the window is changed
        virtual bool			InitHW( void ) = 0;
        virtual bool			ShutdownHW( void ) = 0;

        // asyn loop, called at 60Hz
        virtual int				AsyncUpdate( int time ) = 0;

        // async loop, when the sound driver uses a write strategy
        virtual int				AsyncUpdateWrite( int time ) = 0;

        // it is a good idea to mute everything when starting a new level,
        // because sounds may be started before a valid listener origin
        // is specified
        virtual void			SetMute( bool mute ) = 0;

        // for the sound level meter window
        virtual cinData_t		ImageForTime( const int milliseconds, const bool waveform ) = 0;

        // get sound decoder info
        virtual int				GetSoundDecoderInfo( int index, soundDecoderInfo_t &decoderInfo ) = 0;

        // if rw == NULL, no portal occlusion or rendered debugging is available
        virtual idSoundWorld *	AllocSoundWorld( idRenderWorld *rw ) = 0;

        // specifying NULL will cause silence to be played
        virtual void			SetPlayingSoundWorld( idSoundWorld *soundWorld ) = 0;

        // some tools, like the sound dialog, may be used in both the game and the editor
        // This can return NULL, so check!
        virtual idSoundWorld *	GetPlayingSoundWorld( void ) = 0;

        // Mark all soundSamples as currently unused,
        // but don't free anything.
        virtual	void			BeginLevelLoad( void ) = 0;

        // Free all soundSamples marked as unused
        // We might want to defer the loading of new sounds to this point,
        // as we do with images, to avoid having a union in memory at one time.
        virtual	void			EndLevelLoad( const char *mapString ) = 0;

        // direct mixing for OSes that support it
        virtual int				AsyncMix( int soundTime, float *mixBuffer ) = 0;

        // prints memory info
        virtual void			PrintMemInfo( MemInfo_t *mi ) = 0;

        // is EAX support present - -1: disabled at compile time, 0: no suitable hardware, 1: ok, 2: failed to load OpenAL DLL
        virtual int				IsEAXAvailable( void ) = 0;
    };

    class idSoundSystemShim131 : public idSoundSystem {
    public:
        idSoundSystemShim131() : soundSystem( NULL ) {}
        void Shim(::idSoundSystem *soundSystem) { this->soundSystem = soundSystem; }

        // all non-hardware initialization
        virtual void			Init( void );

        // shutdown routine
        virtual	void			Shutdown( void );

        // call ClearBuffer if there is a chance that the AsyncUpdate won't get called
        // for 20+ msec, which would cause a stuttering repeat of the current
        // buffer contents
        virtual void			ClearBuffer( void );

        // sound is attached to the window, and must be recreated when the window is changed
        virtual bool			InitHW( void );
        virtual bool			ShutdownHW( void );

        // asyn loop, called at 60Hz
        virtual int				AsyncUpdate( int time );

        // async loop, when the sound driver uses a write strategy
        virtual int				AsyncUpdateWrite( int time );

        // it is a good idea to mute everything when starting a new level,
        // because sounds may be started before a valid listener origin
        // is specified
        virtual void			SetMute( bool mute );

        // for the sound level meter window
        virtual cinData_t		ImageForTime( const int milliseconds, const bool waveform );

        // get sound decoder info
        virtual int				GetSoundDecoderInfo( int index, soundDecoderInfo_t &decoderInfo );

        // if rw == NULL, no portal occlusion or rendered debugging is available
        virtual idSoundWorld *	AllocSoundWorld( idRenderWorld *rw );

        // specifying NULL will cause silence to be played
        virtual void			SetPlayingSoundWorld( idSoundWorld *soundWorld );

        // some tools, like the sound dialog, may be used in both the game and the editor
        // This can return NULL, so check!
        virtual idSoundWorld *	GetPlayingSoundWorld( void );

        // Mark all soundSamples as currently unused,
        // but don't free anything.
        virtual	void			BeginLevelLoad( void );

        // Free all soundSamples marked as unused
        // We might want to defer the loading of new sounds to this point,
        // as we do with images, to avoid having a union in memory at one time.
        virtual	void			EndLevelLoad( const char *mapString );

        // direct mixing for OSes that support it
        virtual int				AsyncMix( int soundTime, float *mixBuffer );

        // prints memory info
        virtual void			PrintMemInfo( MemInfo_t *mi );

        // is EAX support present - -1: disabled at compile time, 0: no suitable hardware, 1: ok, 2: failed to load OpenAL DLL
        virtual int				IsEAXAvailable( void );
    private:
        ::idSoundSystem *soundSystem;
    };

    /***********************************************************************

      idRenderSystem (v1.3.1)

    ***********************************************************************/

    class idRenderSystem {
    public:
        virtual					~idRenderSystem() {}

        // set up cvars and basic data structures, but don't
        // init OpenGL, so it can also be used for dedicated servers
        virtual void			Init( void ) = 0;

        // only called before quitting
        virtual void			Shutdown( void ) = 0;

        virtual void			InitOpenGL( void ) = 0;

        virtual void			ShutdownOpenGL( void ) = 0;

        virtual bool			IsOpenGLRunning( void ) const = 0;

        virtual bool			IsFullScreen( void ) const = 0;
        virtual int				GetScreenWidth( void ) const = 0;
        virtual int				GetScreenHeight( void ) const = 0;

        // allocate a renderWorld to be used for drawing
        virtual idRenderWorld *	AllocRenderWorld( void ) = 0;
        virtual	void			FreeRenderWorld( idRenderWorld * rw ) = 0;

        // All data that will be used in a level should be
        // registered before rendering any frames to prevent disk hits,
        // but they can still be registered at a later time
        // if necessary.
        virtual void			BeginLevelLoad( void ) = 0;
        virtual void			EndLevelLoad( void ) = 0;

        // font support
        virtual bool			RegisterFont( const char *fontName, fontInfoEx_t &font ) = 0;

        // GUI drawing just involves shader parameter setting and axial image subsections
        virtual void			SetColor( const idVec4 &rgba ) = 0;
        virtual void			SetColor4( float r, float g, float b, float a ) = 0;

        virtual void			DrawStretchPic( const idDrawVert *verts, const glIndex_t *indexes, int vertCount, int indexCount, const idMaterial *material,
                                                bool clip = true, float min_x = 0.0f, float min_y = 0.0f, float max_x = 640.0f, float max_y = 480.0f ) = 0;
        virtual void			DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const idMaterial *material ) = 0;

        virtual void			DrawStretchTri ( idVec2 p1, idVec2 p2, idVec2 p3, idVec2 t1, idVec2 t2, idVec2 t3, const idMaterial *material ) = 0;
        virtual void			GlobalToNormalizedDeviceCoordinates( const idVec3 &global, idVec3 &ndc ) = 0;
        virtual void			GetGLSettings( int& width, int& height ) = 0;
        virtual void			PrintMemInfo( MemInfo_t *mi ) = 0;

        virtual void			DrawSmallChar( int x, int y, int ch, const idMaterial *material ) = 0;
        virtual void			DrawSmallStringExt( int x, int y, const char *string, const idVec4 &setColor, bool forceColor, const idMaterial *material ) = 0;
        virtual void			DrawBigChar( int x, int y, int ch, const idMaterial *material ) = 0;
        virtual void			DrawBigStringExt( int x, int y, const char *string, const idVec4 &setColor, bool forceColor, const idMaterial *material ) = 0;

        // dump all 2D drawing so far this frame to the demo file
        virtual void			WriteDemoPics() = 0;

        // draw the 2D pics that were saved out with the current demo frame
        virtual void			DrawDemoPics() = 0;

        // a frame cam consist of 2D drawing and potentially multiple 3D scenes
        // window sizes are needed to convert SCREEN_WIDTH / SCREEN_HEIGHT values
        virtual void			BeginFrame( int windowWidth, int windowHeight ) = 0;

        // if the pointers are not NULL, timing info will be returned
        virtual void			EndFrame( int *frontEndMsec, int *backEndMsec ) = 0;

        // aviDemo uses this.
        // Will automatically tile render large screen shots if necessary
        // Samples is the number of jittered frames for anti-aliasing
        // If ref == NULL, session->updateScreen will be used
        // This will perform swapbuffers, so it is NOT an approppriate way to
        // generate image files that happen during gameplay, as for savegame
        // markers.  Use WriteRender() instead.
        virtual void			TakeScreenshot( int width, int height, const char *fileName, int samples, struct renderView_s *ref ) = 0;

        // the render output can be cropped down to a subset of the real screen, as
        // for save-game reviews and split-screen multiplayer.  Users of the renderer
        // will not know the actual pixel size of the area they are rendering to

        // the x,y,width,height values are in virtual SCREEN_WIDTH / SCREEN_HEIGHT coordinates

        // to render to a texture, first set the crop size with makePowerOfTwo = true,
        // then perform all desired rendering, then capture to an image
        // if the specified physical dimensions are larger than the current cropped region, they will be cut down to fit
        virtual void			CropRenderSize( int width, int height, bool makePowerOfTwo = false, bool forceDimensions = false ) = 0;
        virtual void			CaptureRenderToImage( const char *imageName ) = 0;
        // fixAlpha will set all the alpha channel values to 0xff, which allows screen captures
        // to use the default tga loading code without having dimmed down areas in many places
        virtual void			CaptureRenderToFile( const char *fileName, bool fixAlpha = false ) = 0;
        virtual void			UnCrop() = 0;
        virtual void			GetCardCaps( bool &oldCard, bool &nv10or20 ) = 0;

        // the image has to be already loaded ( most straightforward way would be through a FindMaterial )
        // texture filter / mipmapping / repeat won't be modified by the upload
        // returns false if the image wasn't found
        virtual bool			UploadImage( const char *imageName, const byte *data, int width, int height ) = 0;
    };

    class idRenderSystemShim131 : public idRenderSystem {
    public:
        idRenderSystemShim131() : renderSystem( NULL ) {}
        void Shim(::idRenderSystem *renderSystem) { this->renderSystem = renderSystem; }

        // set up cvars and basic data structures, but don't
        // init OpenGL, so it can also be used for dedicated servers
        virtual void			Init( void );

        // only called before quitting
        virtual void			Shutdown( void );

        virtual void			InitOpenGL( void );

        virtual void			ShutdownOpenGL( void );

        virtual bool			IsOpenGLRunning( void ) const;

        virtual bool			IsFullScreen( void ) const;
        virtual int				GetScreenWidth( void ) const;
        virtual int				GetScreenHeight( void ) const;

        // allocate a renderWorld to be used for drawing
        virtual idRenderWorld *	AllocRenderWorld( void );
        virtual	void			FreeRenderWorld( idRenderWorld * rw );

        // All data that will be used in a level should be
        // registered before rendering any frames to prevent disk hits,
        // but they can still be registered at a later time
        // if necessary.
        virtual void			BeginLevelLoad( void );
        virtual void			EndLevelLoad( void );

        // font support
        virtual bool			RegisterFont( const char *fontName, fontInfoEx_t &font );

        // GUI drawing just involves shader parameter setting and axial image subsections
        virtual void			SetColor( const idVec4 &rgba );
        virtual void			SetColor4( float r, float g, float b, float a );

        virtual void			DrawStretchPic( const idDrawVert *verts, const glIndex_t *indexes, int vertCount, int indexCount, const idMaterial *material,
                                                bool clip = true, float min_x = 0.0f, float min_y = 0.0f, float max_x = 640.0f, float max_y = 480.0f );
        virtual void			DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const idMaterial *material );

        virtual void			DrawStretchTri ( idVec2 p1, idVec2 p2, idVec2 p3, idVec2 t1, idVec2 t2, idVec2 t3, const idMaterial *material );
        virtual void			GlobalToNormalizedDeviceCoordinates( const idVec3 &global, idVec3 &ndc );
        virtual void			GetGLSettings( int& width, int& height );
        virtual void			PrintMemInfo( MemInfo_t *mi );

        virtual void			DrawSmallChar( int x, int y, int ch, const idMaterial *material );
        virtual void			DrawSmallStringExt( int x, int y, const char *string, const idVec4 &setColor, bool forceColor, const idMaterial *material );
        virtual void			DrawBigChar( int x, int y, int ch, const idMaterial *material );
        virtual void			DrawBigStringExt( int x, int y, const char *string, const idVec4 &setColor, bool forceColor, const idMaterial *material );

        // dump all 2D drawing so far this frame to the demo file
        virtual void			WriteDemoPics();

        // draw the 2D pics that were saved out with the current demo frame
        virtual void			DrawDemoPics();

        // a frame cam consist of 2D drawing and potentially multiple 3D scenes
        // window sizes are needed to convert SCREEN_WIDTH / SCREEN_HEIGHT values
        virtual void			BeginFrame( int windowWidth, int windowHeight );

        // if the pointers are not NULL, timing info will be returned
        virtual void			EndFrame( int *frontEndMsec, int *backEndMsec );

        // aviDemo uses this.
        // Will automatically tile render large screen shots if necessary
        // Samples is the number of jittered frames for anti-aliasing
        // If ref == NULL, session->updateScreen will be used
        // This will perform swapbuffers, so it is NOT an approppriate way to
        // generate image files that happen during gameplay, as for savegame
        // markers.  Use WriteRender() instead.
        virtual void			TakeScreenshot( int width, int height, const char *fileName, int samples, struct renderView_s *ref );

        // the render output can be cropped down to a subset of the real screen, as
        // for save-game reviews and split-screen multiplayer.  Users of the renderer
        // will not know the actual pixel size of the area they are rendering to

        // the x,y,width,height values are in virtual SCREEN_WIDTH / SCREEN_HEIGHT coordinates

        // to render to a texture, first set the crop size with makePowerOfTwo = true,
        // then perform all desired rendering, then capture to an image
        // if the specified physical dimensions are larger than the current cropped region, they will be cut down to fit
        virtual void			CropRenderSize( int width, int height, bool makePowerOfTwo = false, bool forceDimensions = false );
        virtual void			CaptureRenderToImage( const char *imageName );
        // fixAlpha will set all the alpha channel values to 0xff, which allows screen captures
        // to use the default tga loading code without having dimmed down areas in many places
        virtual void			CaptureRenderToFile( const char *fileName, bool fixAlpha = false );
        virtual void			UnCrop();
        virtual void			GetCardCaps( bool &oldCard, bool &nv10or20 );

        // the image has to be already loaded ( most straightforward way would be through a FindMaterial )
        // texture filter / mipmapping / repeat won't be modified by the upload
        // returns false if the image wasn't found
        virtual bool			UploadImage( const char *imageName, const byte *data, int width, int height );
    private:
        ::idRenderSystem *renderSystem;
    };

} /* namespace Shim131 */

#endif /* !D3_SDK_131_COMPAT */

#endif /* !__COMPAT_131_H__ */
