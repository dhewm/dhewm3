#include "framework/compat/Compat_131.h"

#if D3_SDK_131_COMPAT

#include "framework/FileSystem.h"
#include "cm/CollisionModel.h"
#include "idlib/MapFile.h"
#include "sound/sound.h"
#include "renderer/Cinematic.h"

namespace Shim131 {

    static idSysShim131 sys131Local;
    idSysShim131 *sys131 = &sys131Local;

    static idFileSystemShim131 fileSystem131Local;
    idFileSystemShim131 *fileSystem131 = &fileSystem131Local;

    static idCollisionModelManagerShim131 collisionModelManager131Local;
    idCollisionModelManagerShim131 *collisionModelManager131 = &collisionModelManager131Local;

    static idSoundSystemShim131 soundSystem131Local;
    idSoundSystemShim131 *soundSystem131 = &soundSystem131Local;

    static idRenderSystemShim131 renderSystem131Local;
    idRenderSystemShim131 *renderSystem131 = &renderSystem131Local;

    /***********************************************************************

      idSys (v1.3.1)

    ***********************************************************************/

    void idSysShim131::DebugPrintf( const char *fmt, ... ) {
        va_list argptr;

        va_start( argptr, fmt );
        Sys_DebugVPrintf( fmt, argptr );
        va_end( argptr );
    }

    void idSysShim131::DebugVPrintf( const char *fmt, va_list arg ) {
        sys->DebugVPrintf( fmt, arg );
    }

    double idSysShim131::GetClockTicks( void ) {
        return Sys_MillisecondsPrecise();
    }

    double idSysShim131::ClockTicksPerSecond( void ) {
        return 1000.0;
    }

    cpuid_t idSysShim131::GetProcessorId( void ) {
        return (cpuid_t)sys->GetProcessorId();
    }

    const char* idSysShim131::GetProcessorString( void ) {
        return "unavailable";
    }

    const char* idSysShim131::FPU_GetState( void ) {
        return "unavailable";
    }

    bool idSysShim131::FPU_StackIsEmpty( void ) {
        return true;
    }

    void idSysShim131::FPU_SetFTZ( bool enable ) {
        Sys_FPU_SetFTZ( enable );
    }

    void idSysShim131::FPU_SetDAZ( bool enable ) {
        Sys_FPU_SetDAZ( enable );
    }

    void idSysShim131::FPU_EnableExceptions( int exceptions ) {
        return;
    }

    bool idSysShim131::LockMemory( void *ptr, int bytes ) {
        return sys->LockMemory( ptr, bytes );
    }

    bool idSysShim131::UnlockMemory( void *ptr, int bytes ) {
        return sys->UnlockMemory( ptr, bytes );
    }

    void idSysShim131::GetCallStack( address_t *callStack, const int callStackSize ) {
        int i;

        for (i = 0; i < callStackSize; i++)
        {
            callStack[i] = 0;
        }
    }

    const char *idSysShim131::GetCallStackStr( const address_t *callStack, const int callStackSize ) {
        return "unavailable";
    }

    const char *idSysShim131::GetCallStackCurStr( int depth ) {
        return "unavailable";
    }

    void idSysShim131::ShutdownSymbols( void ) {}

    int idSysShim131::DLL_Load( const char *dllName ) {
        return (int)sys->DLL_Load( dllName );
    }

    void *idSysShim131::DLL_GetProcAddress( int dllHandle, const char *procName ) {
        return sys->DLL_GetProcAddress( dllHandle, procName );
    }

    void idSysShim131::DLL_Unload( int dllHandle ) {
        sys->DLL_Unload( dllHandle );
    }

    void idSysShim131::DLL_GetFileName( const char *baseName, char *dllName, int maxLength ) {
        sys->DLL_GetFileName( baseName, dllName, maxLength );
    }

    sysEvent_t ToSysEvent131(::sysEvent_t evt) {
        sysEvent_t res;

        switch (evt.evType) {
            case ::sysEventType_t::SE_NONE:
                res.evType = sysEventType_t::SE_NONE;
                break;

            case ::sysEventType_t::SE_KEY:
                res.evType = sysEventType_t::SE_KEY;
                break;

            case ::sysEventType_t::SE_CHAR:
                res.evType = sysEventType_t::SE_CHAR;
                break;

            case ::sysEventType_t::SE_MOUSE:
                res.evType = sysEventType_t::SE_MOUSE;
                break;

            case ::sysEventType_t::SE_MOUSE_ABS:
                res.evType = sysEventType_t::SE_NONE;
                break;

            case ::sysEventType_t::SE_JOYSTICK:
                res.evType = sysEventType_t::SE_JOYSTICK_AXIS;
                break;

            case ::sysEventType_t::SE_CONSOLE:
                res.evType = sysEventType_t::SE_CONSOLE;
                break;

            default:
                common->FatalError( "unimplemented evt.evType" );
                break;
        }
        res.evValue = evt.evValue;
        res.evValue2 = evt.evValue2;
        res.evPtrLength = evt.evPtrLength;
        res.evPtr = evt.evPtr;

        return res;
    }

    sysEvent_t idSysShim131::GenerateMouseButtonEvent( int button, bool down ) {
        return ToSysEvent131( sys->GenerateMouseButtonEvent( button, down ) );
    }

    sysEvent_t idSysShim131::GenerateMouseMoveEvent( int deltax, int deltay ) {
        return ToSysEvent131( sys->GenerateMouseMoveEvent( deltax, deltay ) );
    }

    void idSysShim131::OpenURL( const char *url, bool quit ) {
        sys->OpenURL( url, quit );
    }

    void idSysShim131::StartProcess( const char *exePath, bool quit ) {
        sys->StartProcess( exePath, quit );
    }

    /***********************************************************************

      idFileSystem (v1.3.1)

    ***********************************************************************/

    void idFileSystemShim131::Init( void ) {
        fileSystem->Init();
    }

    void idFileSystemShim131::Restart( void ) { 
        fileSystem->Restart();
    }

    void idFileSystemShim131::Shutdown( bool reloading ) {
        fileSystem->Shutdown( reloading );
    }

    bool idFileSystemShim131::IsInitialized( void ) const {
        return fileSystem->IsInitialized();
    }

    bool idFileSystemShim131::PerformingCopyFiles( void ) const {
        return fileSystem->PerformingCopyFiles();
    }

    idModList *idFileSystemShim131::ListMods( void ) {
        return (idModList *)fileSystem->ListMods();
    }

    void idFileSystemShim131::FreeModList( idModList *modList ) {
        fileSystem->FreeModList( (::idModList*)modList );
    }

    idFileList *idFileSystemShim131::ListFiles( const char *relativePath, const char *extension, bool sort, bool fullRelativePath, const char* gamedir ) {
        return (idFileList *)fileSystem->ListFiles( relativePath, extension, sort, fullRelativePath, gamedir );
    }

    idFileList *idFileSystemShim131::ListFilesTree( const char *relativePath, const char *extension, bool sort, const char* gamedir ) {
        return (idFileList *)fileSystem->ListFilesTree( relativePath, extension, sort, gamedir );
    }

    void idFileSystemShim131::FreeFileList( idFileList *fileList ) {
        fileSystem->FreeFileList( (::idFileList *)fileList );
    }

    const char *idFileSystemShim131::OSPathToRelativePath( const char *OSPath ) {
        return fileSystem->OSPathToRelativePath( OSPath );
    }

    const char *idFileSystemShim131::RelativePathToOSPath( const char *relativePath, const char *basePath ) {
        return fileSystem->RelativePathToOSPath( relativePath, basePath );
    }

    const char *idFileSystemShim131::BuildOSPath( const char *base, const char *game, const char *relativePath ) {
        return fileSystem->BuildOSPath( base, game, relativePath );
    }

    void idFileSystemShim131::CreateOSPath( const char *OSPath ) {
        fileSystem->CreateOSPath( OSPath );
    }

    bool idFileSystemShim131::FileIsInPAK( const char *relativePath ) {
        return fileSystem->FileIsInPAK( relativePath );
    }

    void idFileSystemShim131::UpdatePureServerChecksums( void ) {
        fileSystem->UpdatePureServerChecksums();
    }

    bool idFileSystemShim131::UpdateGamePakChecksums( void ) {
        return true;
    }

    fsPureReply_t idFileSystemShim131::SetPureServerChecksums( const int pureChecksums[], int gamePakChecksum, int missingChecksums[], int *missingGamePakChecksum ) {
        missingGamePakChecksum = 0;
        return fileSystem->SetPureServerChecksums( pureChecksums, missingChecksums );
    }

    void idFileSystemShim131::GetPureServerChecksums( int checksums[], int OS, int *gamePakChecksum ) {
        fileSystem->GetPureServerChecksums( checksums );
    }

    void idFileSystemShim131::SetRestartChecksums( const int pureChecksums[], int gamePakChecksum ) {
        fileSystem->SetRestartChecksums( pureChecksums );
    }

    void idFileSystemShim131::ClearPureChecksums( void ) {
        fileSystem->ClearPureChecksums();
    }

    int idFileSystemShim131::GetOSMask( void ) {
        return -1;
    }

    int idFileSystemShim131::ReadFile( const char *relativePath, void **buffer, ID_TIME_T *timestamp ) {
        return fileSystem->ReadFile( relativePath, buffer, timestamp );
    }

    void idFileSystemShim131::FreeFile( void *buffer ) {
        fileSystem->FreeFile( buffer );
    }

    int idFileSystemShim131::WriteFile( const char *relativePath, const void *buffer, int size, const char *basePath ) {
        return fileSystem->WriteFile( relativePath, buffer, size, basePath );
    }

    void idFileSystemShim131::RemoveFile( const char *relativePath ) {
        fileSystem->RemoveFile( relativePath );
    }

    idFile *idFileSystemShim131::OpenFileRead( const char *relativePath, bool allowCopyFiles, const char *gamedir ) {
        return (idFile *)fileSystem->OpenFileRead( relativePath, allowCopyFiles, gamedir );
    }

    idFile *idFileSystemShim131::OpenFileWrite( const char *relativePath, const char *basePath ) {
        return (idFile *)fileSystem->OpenFileWrite( relativePath, basePath );
    }

    idFile *idFileSystemShim131::OpenFileAppend( const char *filename, bool sync, const char *basePath ) {
        return (idFile *)fileSystem->OpenFileAppend( filename, sync, basePath );
    }

    idFile *idFileSystemShim131::OpenFileByMode( const char *relativePath, fsMode_t mode ) {
        return (idFile *)fileSystem->OpenFileByMode( relativePath, (::fsMode_t)mode );
    }

    idFile *idFileSystemShim131::OpenExplicitFileRead( const char *OSPath ) {
        return (idFile *)fileSystem->OpenExplicitFileRead( OSPath );
    }

    idFile *idFileSystemShim131::OpenExplicitFileWrite( const char *OSPath ) {
        return (idFile *)fileSystem->OpenExplicitFileWrite( OSPath );
    }

    void idFileSystemShim131::CloseFile( idFile *f ) {
        fileSystem->CloseFile( (::idFile *)f );
    }

    void idFileSystemShim131::BackgroundDownload( backgroundDownload_t *bgl ) {
        fileSystem->BackgroundDownload( (::backgroundDownload_t *)bgl );
    }

    void idFileSystemShim131::ResetReadCount( void ) {
        fileSystem->ResetReadCount();
    }

    int idFileSystemShim131::GetReadCount( void ) {
        return fileSystem->GetReadCount();
    }

    void idFileSystemShim131::AddToReadCount( int c ) {
        fileSystem->AddToReadCount( c );
    }

    void idFileSystemShim131::FindDLL( const char *basename, char dllPath[], bool updateChecksum ) {
        fileSystem->FindDLL( basename, dllPath );
    }

    void idFileSystemShim131::ClearDirCache( void ) {
        fileSystem->ClearDirCache();
    }

    bool idFileSystemShim131::HasD3XP( void ) {
        return fileSystem->HasD3XP();
    }

    bool idFileSystemShim131::RunningD3XP( void ) {
        return fileSystem->RunningD3XP();
    }

    void idFileSystemShim131::CopyFile( const char *fromOSPath, const char *toOSPath ) {
        fileSystem->CopyFile( fromOSPath, toOSPath );
    }

    int idFileSystemShim131::ValidateDownloadPakForChecksum( int checksum, char path[], bool isGamePak ) {
        return fileSystem->ValidateDownloadPakForChecksum( checksum, path );
    }

    idFile *idFileSystemShim131::MakeTemporaryFile( void ) {
        return (idFile *)fileSystem->MakeTemporaryFile();
    }

    int idFileSystemShim131::AddZipFile( const char *path ) {
        return fileSystem->AddZipFile( path );
    }

    findFile_t idFileSystemShim131::FindFile( const char *path, bool scheduleAddons ) {
        return (findFile_t)fileSystem->FindFile( path, scheduleAddons );
    }

    int idFileSystemShim131::GetNumMaps( void ) {
        return fileSystem->GetNumMaps();
    }

    const idDict *idFileSystemShim131::GetMapDecl( int i ) {
        return (const idDict *)fileSystem->GetMapDecl( i );
    }

    void idFileSystemShim131::FindMapScreenshot( const char *path, char *buf, int len ) {
        fileSystem->FindMapScreenshot( path, buf, len );
    }

    bool idFileSystemShim131::FilenameCompare( const char *s1, const char *s2 ) const {
        return fileSystem->FilenameCompare( s1, s2 );
    }

    /***********************************************************************
    
      idCollisionModelManager (v1.3.1)

    ***********************************************************************/

    void idCollisionModelManagerShim131::LoadMap( const idMapFileShim131 *mapFile ) {
        ::idMapFile shimmedMapFile;

        if ( mapFile == NULL ) {
            collisionModelManager->LoadMap( NULL );
            return;
        }

        if ( !shimmedMapFile.Parse( mapFile->name.c_str() ) ) {
            collisionModelManager->LoadMap( NULL );
            return;
        }

        collisionModelManager->LoadMap( &shimmedMapFile );
    }

    void idCollisionModelManagerShim131::FreeMap( void ) {
        collisionModelManager->FreeMap();
    }

    cmHandle_t idCollisionModelManagerShim131::LoadModel( const char *modelName, const bool precache ) {
        return (cmHandle_t)collisionModelManager->LoadModel( modelName, precache );
    }

    cmHandle_t idCollisionModelManagerShim131::SetupTrmModel( const idTraceModel &trm, const idMaterial *material ) {
        return (cmHandle_t)collisionModelManager->SetupTrmModel( (const ::idTraceModel &)trm, (const ::idMaterial *)material );
    }

    bool idCollisionModelManagerShim131::TrmFromModel( const char *modelName, idTraceModel &trm ) {
        return collisionModelManager->TrmFromModel( modelName, (::idTraceModel &)trm );
    }

    const char *idCollisionModelManagerShim131::GetModelName( cmHandle_t model ) const {
        return collisionModelManager->GetModelName( (::cmHandle_t)model );
    }

    bool idCollisionModelManagerShim131::GetModelBounds( cmHandle_t model, idBounds &bounds ) const {
        return collisionModelManager->GetModelBounds( (::cmHandle_t)model, (::idBounds &)bounds );
    }

    bool idCollisionModelManagerShim131::GetModelContents( cmHandle_t model, int &contents ) const {
        return collisionModelManager->GetModelContents( (::cmHandle_t)model, contents );
    }

    bool idCollisionModelManagerShim131::GetModelVertex( cmHandle_t model, int vertexNum, idVec3 &vertex ) const {
        return collisionModelManager->GetModelVertex( (::cmHandle_t)model, vertexNum, (::idVec3 &)vertex );
    }

    bool idCollisionModelManagerShim131::GetModelEdge( cmHandle_t model, int edgeNum, idVec3 &start, idVec3 &end ) const {
        return collisionModelManager->GetModelEdge( (::cmHandle_t)model, edgeNum, (::idVec3 &)start, (::idVec3 &)end );
    }

    bool idCollisionModelManagerShim131::GetModelPolygon( cmHandle_t model, int polygonNum, idFixedWinding &winding ) const {
        return collisionModelManager->GetModelPolygon( (::cmHandle_t)model, polygonNum, (::idFixedWinding &)winding );
    }

    void idCollisionModelManagerShim131::Translation( trace_t *results, const idVec3 &start, const idVec3 &end, const idTraceModel *trm, const idMat3 &trmAxis, int contentMask, cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) {
        collisionModelManager->Translation( (::trace_t *)results, (const ::idVec3 &)start, (const ::idVec3 &)end, (const ::idTraceModel *)trm, (const ::idMat3 &)trmAxis, contentMask, (::cmHandle_t)model, (const ::idVec3 &)modelOrigin, (const ::idMat3 &)modelAxis );
    }

    void idCollisionModelManagerShim131::Rotation( trace_t *results, const idVec3 &start, const idRotation &rotation, const idTraceModel *trm, const idMat3 &trmAxis, int contentMask, cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) {
        collisionModelManager->Rotation( (::trace_t *)results, (const ::idVec3 &)start, (const ::idRotation &)rotation, (const ::idTraceModel *)trm, (const ::idMat3 &)trmAxis, contentMask, (::cmHandle_t)model, (const ::idVec3 &)modelOrigin, (const ::idMat3 &)modelAxis );
    }

    int idCollisionModelManagerShim131::Contents( const idVec3 &start, const idTraceModel *trm, const idMat3 &trmAxis, int contentMask, cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) {
        return collisionModelManager->Contents( (const ::idVec3 &)start, (const ::idTraceModel *)trm, (const ::idMat3 &)trmAxis, contentMask, (::cmHandle_t)model, (const ::idVec3 &)modelOrigin, (const ::idMat3 &)modelAxis );
    }

    int idCollisionModelManagerShim131::Contacts( contactInfo_t *contacts, const int maxContacts, const idVec3 &start, const idVec6 &dir, const float depth, const idTraceModel *trm, const idMat3 &trmAxis, int contentMask, cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis ) {
        return collisionModelManager->Contacts( (::contactInfo_t *)contacts, maxContacts, (const ::idVec3 &)start, (const ::idVec6 &)dir, depth, (const ::idTraceModel *)trm, (const ::idMat3 &)trmAxis, contentMask, (::cmHandle_t)model, (const ::idVec3 &)modelOrigin, (const ::idMat3 &)modelAxis );
    }

    void idCollisionModelManagerShim131::DebugOutput( const idVec3 &origin ) {
        collisionModelManager->DebugOutput( (const ::idVec3 &)origin );
    }

    void idCollisionModelManagerShim131::DrawModel( cmHandle_t model, const idVec3 &modelOrigin, const idMat3 &modelAxis, const idVec3 &viewOrigin, const float radius ) {
        collisionModelManager->DrawModel( (::cmHandle_t)model, (const ::idVec3 &)modelOrigin, (const ::idMat3 &)modelAxis, (const ::idVec3 &)viewOrigin, radius );
    }

    void idCollisionModelManagerShim131::ModelInfo( cmHandle_t model ) {
        collisionModelManager->ModelInfo( (::cmHandle_t)model );
    }

    void idCollisionModelManagerShim131::ListModels( void ) {
        collisionModelManager->ListModels();
    }

    bool idCollisionModelManagerShim131::WriteCollisionModelForMapEntity( const idMapEntity *mapEnt, const char *filename, const bool testTraceModel ) {
        return collisionModelManager->WriteCollisionModelForMapEntity( (const ::idMapEntity *)mapEnt, filename, testTraceModel );
    }

    /***********************************************************************

      idSoundSystem (v1.3.1)

    ***********************************************************************/

    void idSoundSystemShim131::Init() {
        soundSystem->Init();
    }

    void idSoundSystemShim131::Shutdown() {
        soundSystem->Shutdown();
    }

    void idSoundSystemShim131::ClearBuffer() {}

    bool idSoundSystemShim131::InitHW() {
        return soundSystem->InitHW();
    }

    bool idSoundSystemShim131::ShutdownHW() {
        return soundSystem->ShutdownHW();
    }

    int idSoundSystemShim131::AsyncUpdate( int time ) {
        return soundSystem->AsyncUpdate( time );
    }

    int idSoundSystemShim131::AsyncUpdateWrite( int time ) {
        return soundSystem->AsyncUpdateWrite( time );
    }

    void idSoundSystemShim131::SetMute( bool mute ) {
        return soundSystem->SetMute( mute );
    }

    cinData_t idSoundSystemShim131::ImageForTime( const int milliseconds, const bool waveform ) {
        return soundSystem->ImageForTime( milliseconds, waveform );
    }

    int idSoundSystemShim131::GetSoundDecoderInfo( int index, soundDecoderInfo_t &decoderInfo ) {
        return soundSystem->GetSoundDecoderInfo( index, (::soundDecoderInfo_t &)decoderInfo );
    }

    idSoundWorld *idSoundSystemShim131::AllocSoundWorld( idRenderWorld *rw ) {
        return (idSoundWorld *)soundSystem->AllocSoundWorld( (::idRenderWorld *)rw );
    }

    void idSoundSystemShim131::SetPlayingSoundWorld( idSoundWorld *soundWorld ) {
        soundSystem->SetPlayingSoundWorld( (::idSoundWorld *)soundWorld );
    }

    idSoundWorld *idSoundSystemShim131::GetPlayingSoundWorld() {
        return (idSoundWorld *)soundSystem->GetPlayingSoundWorld();
    }

    void idSoundSystemShim131::BeginLevelLoad() {
        soundSystem->BeginLevelLoad();
    }

    void idSoundSystemShim131::EndLevelLoad( const char *mapString ) {
        soundSystem->EndLevelLoad( mapString );
    }

    int idSoundSystemShim131::AsyncMix( int soundTime, float *mixBuffer ) {
        return soundSystem->AsyncMix( soundTime, mixBuffer );
    }

    void idSoundSystemShim131::PrintMemInfo( MemInfo_t *mi ) {
        soundSystem->PrintMemInfo( (::MemInfo_t *)mi );
    }

    int idSoundSystemShim131::IsEAXAvailable( void ) {
        return soundSystem->IsEFXAvailable();
    }

    /***********************************************************************

      idRenderSystem (v1.3.1)

    ***********************************************************************/

    void idRenderSystemShim131::Init() {
        renderSystem->Init();
    }

    void idRenderSystemShim131::Shutdown() {
        renderSystem->Shutdown();
    }

    void idRenderSystemShim131::InitOpenGL() {
        renderSystem->InitOpenGL();
    }

    void idRenderSystemShim131::ShutdownOpenGL() {
        renderSystem->ShutdownOpenGL();
    }

    bool idRenderSystemShim131::IsOpenGLRunning() const {
        return renderSystem->IsOpenGLRunning();
    }

    bool idRenderSystemShim131::IsFullScreen() const {
        return renderSystem->IsFullScreen();
    }

    int idRenderSystemShim131::GetScreenWidth() const {
        return renderSystem->GetScreenWidth();
    }

    int idRenderSystemShim131::GetScreenHeight() const {
        return renderSystem->GetScreenHeight();
    }

    idRenderWorld *idRenderSystemShim131::AllocRenderWorld() {
        return (idRenderWorld *)renderSystem->AllocRenderWorld();
    }

    void idRenderSystemShim131::FreeRenderWorld( idRenderWorld *rw ) {
        renderSystem->FreeRenderWorld( (::idRenderWorld *)rw );
    }

    void idRenderSystemShim131::BeginLevelLoad() {
        renderSystem->BeginLevelLoad();
    }

    void idRenderSystemShim131::EndLevelLoad() {
        renderSystem->EndLevelLoad();
    }

    bool idRenderSystemShim131::RegisterFont( const char *fontName, fontInfoEx_t &font ) {
        return renderSystem->RegisterFont( fontName, font );
    }

    void idRenderSystemShim131::SetColor( const idVec4 &rgba ) {
        renderSystem->SetColor( rgba );
    }

    void idRenderSystemShim131::SetColor4( float r, float g, float b, float a ) {
        renderSystem->SetColor4( r, g, b, a );
    }

    void idRenderSystemShim131::DrawStretchPic( const idDrawVert *verts, const glIndex_t *indexes, int vertCount, int indexCount, const idMaterial *material, bool clip, float min_x, float min_y, float max_x, float max_y ) {
        renderSystem->DrawStretchPic( verts, indexes, vertCount, indexCount, (const ::idMaterial *)material, clip, min_x, min_y, max_x, max_y );
    }

    void idRenderSystemShim131::DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const idMaterial *material ) {
        renderSystem->DrawStretchPic( x, y, w, h, s1, t1, s2, t2, (const ::idMaterial *)material );
    }

    void idRenderSystemShim131::DrawStretchTri( idVec2 p1, idVec2 p2, idVec2 p3, idVec2 t1, idVec2 t2, idVec2 t3, const idMaterial *material ) {
        renderSystem->DrawStretchTri( p1, p2, p3, t1, t2, t3, (const ::idMaterial *)material );
    }

    void idRenderSystemShim131::GlobalToNormalizedDeviceCoordinates( const idVec3 &global, idVec3 &ndc ) {
        renderSystem->GlobalToNormalizedDeviceCoordinates( global, ndc );
    }

    void idRenderSystemShim131::GetGLSettings( int &width, int &height ) {
        renderSystem->GetGLSettings( width, height );
    }

    void idRenderSystemShim131::PrintMemInfo( MemInfo_t *mi ) {
        renderSystem->PrintMemInfo( (::MemInfo_t *)mi );
    }

    void idRenderSystemShim131::DrawSmallChar( int x, int y, int ch, const idMaterial *material ) {
        renderSystem->DrawSmallChar( x, y, ch, (const ::idMaterial *)material );
    }

    void idRenderSystemShim131::DrawSmallStringExt( int x, int y, const char *string, const idVec4 &setColor, bool forceColor, const idMaterial *material ) {
        renderSystem->DrawSmallStringExt( x, y, string, setColor, forceColor, (const ::idMaterial *)material );
    }

    void idRenderSystemShim131::DrawBigChar( int x, int y, int ch, const idMaterial *material ) {
        renderSystem->DrawBigChar( x, y, ch, (const ::idMaterial *)material );
    }

    void idRenderSystemShim131::DrawBigStringExt( int x, int y, const char *string, const idVec4 &setColor, bool forceColor, const idMaterial *material ) {
        renderSystem->DrawBigStringExt( x, y, string, setColor, forceColor, (const ::idMaterial *)material );
    }

    void idRenderSystemShim131::WriteDemoPics() {
        renderSystem->WriteDemoPics();
    }

    void idRenderSystemShim131::DrawDemoPics() {
        renderSystem->DrawDemoPics();
    }

    void idRenderSystemShim131::BeginFrame( int windowWidth, int windowHeight ) {
        renderSystem->BeginFrame( windowWidth, windowHeight );
    }

    void idRenderSystemShim131::EndFrame( int *frontEndMsec, int *backEndMsec ) {
        renderSystem->EndFrame( frontEndMsec, backEndMsec );
    }

    void idRenderSystemShim131::TakeScreenshot( int width, int height, const char *fileName, int samples, renderView_s *ref ) {
        renderSystem->TakeScreenshot( width, height, fileName, samples, ref );
    }

    void idRenderSystemShim131::CropRenderSize( int width, int height, bool makePowerOfTwo, bool forceDimensions ) {
        renderSystem->CropRenderSize( width, height, makePowerOfTwo, forceDimensions );
    }

    void idRenderSystemShim131::CaptureRenderToImage( const char *imageName ) {
        renderSystem->CaptureRenderToImage( imageName );
    }

    void idRenderSystemShim131::CaptureRenderToFile( const char *fileName, bool fixAlpha ) {
        renderSystem->CaptureRenderToFile( fileName, fixAlpha );
    }

    void idRenderSystemShim131::UnCrop() {
        renderSystem->UnCrop();
    }

    void idRenderSystemShim131::GetCardCaps( bool &oldCard, bool &nv10or20 ) {
        oldCard = false;
        nv10or20 = false;
    }

    bool idRenderSystemShim131::UploadImage( const char *imageName, const byte *data, int width, int height ) {
        return renderSystem->UploadImage( imageName, data, width, height );
    }

} // namespace Shim131

#endif /* !D3_SDK_131_COMPAT */
