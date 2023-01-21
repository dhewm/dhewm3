/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 1999-2011 Raven Software
Copyright (C) 2021 Harrie van Ginneken

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



#if defined( ID_ALLOW_TOOLS )
#include "tools/edit_gui_common.h"
#include "DebuggerServer.h"
#include "DebuggerApp.h"
#else
#include "DebuggerServer.h"
#include "debugger_common.h"
// we need a lot to be able to list all threads in mars_city1
const int MAX_MSGLEN = 8600;
#endif

/*
================
rvDebuggerServer::rvDebuggerServer
================
*/
rvDebuggerServer::rvDebuggerServer ( )
{
	mConnected			= false;
	mBreakNext			= false;
	mBreak				= false;
	mBreakStepOver		= false;
	mBreakStepInto		= false;
	mGameThreadBreakCond = NULL;
	mGameThreadBreakLock = NULL;
	mLastStatementLine	= -1;
	mBreakStepOverFunc1 = NULL;
	mBreakStepOverFunc2 = NULL;
	mBreakInstructionPointer = 0;
	mBreakInterpreter = NULL;
	mBreakProgram = NULL;
	mGameDLLHandle = 0;
	mBreakStepOverDepth = 0;
	mCriticalSection = NULL;
}

/*
================
rvDebuggerServer::~rvDebuggerServer
================
*/
rvDebuggerServer::~rvDebuggerServer ( )
{
}

/*
================
rvDebuggerServer::Initialize

Initialize the debugger server.  This function should be called before the
debugger server is used.
================
*/
bool rvDebuggerServer::Initialize ( void )
{
	// Initialize the network connection
	if ( !mPort.InitForPort ( 27980 ) )
	{
		return false;
	}

	// we're using a condition variable to pause the game thread in rbDebuggerServer::Break()
	// until rvDebuggerServer::Resume() is called (from another thread)
	mGameThreadBreakCond = SDL_CreateCond();
	mGameThreadBreakLock = SDL_CreateMutex();

	// Create a critical section to ensure that the shared thread
	// variables are protected
	mCriticalSection = SDL_CreateMutex();

	// Server must be running on the local host on port 28980
	Sys_StringToNetAdr ( com_dbgClientAdr.GetString( ), &mClientAdr, true );
	mClientAdr.port = 27981;

	// Attempt to let the server know we are here.  The server may not be running so this
	// message will just get ignored.
	SendMessage ( DBMSG_CONNECT );

	return true;
}

void rvDebuggerServer::OSPathToRelativePath( const char *osPath, idStr &qpath )
{
	if ( strchr( osPath, ':' ) ) // XXX: what about linux?
	{
		qpath = fileSystem->OSPathToRelativePath( osPath );
	}
	else
	{
		qpath = osPath;
	}
}

/*
================
rvDebuggerServer::Shutdown

Shutdown the debugger server.
================
*/
void rvDebuggerServer::Shutdown ( void )
{
	// Let the debugger client know we are shutting down
	if ( mConnected )
	{
		SendMessage ( DBMSG_DISCONNECT );
		mConnected = false;
	}

	mPort.Close();

	Resume(); // just in case we're still paused

	// dont need the crit section anymore
	SDL_DestroyMutex( mCriticalSection );
	mCriticalSection = NULL;

	SDL_DestroyCond( mGameThreadBreakCond );
	mGameThreadBreakCond = NULL;
	SDL_DestroyMutex( mGameThreadBreakLock );
	mGameThreadBreakLock = NULL;
}

/*
================
rvDebuggerServer::ProcessMessages

Process all incoming network messages from the debugger client
================
*/
bool rvDebuggerServer::ProcessMessages ( void )
{
	netadr_t adrFrom;
	idBitMsg	 msg;
	byte	 buffer[MAX_MSGLEN];

	// Check for pending udp packets on the debugger port
	int msgSize;
	while ( mPort.GetPacket ( adrFrom, buffer, msgSize, MAX_MSGLEN) )
	{
		short command;
		msg.Init(buffer, sizeof(buffer));
		msg.SetSize(msgSize);
		msg.BeginReading();
		
		if ( adrFrom.type != NA_LOOPBACK ) {
			// Only accept packets from the debugger server for security reasons
			if ( !Sys_CompareNetAdrBase( adrFrom, mClientAdr ) )
				continue;
		}

		command = msg.ReadShort( );

		switch ( command )
		{
			case DBMSG_CONNECT:
				mConnected = true;
				SendMessage ( DBMSG_CONNECTED );
				HandleInspectScripts ( NULL );
				com_editors |= EDITOR_DEBUGGER;
				break;

			case DBMSG_CONNECTED:
				mConnected = true;
				HandleInspectScripts( NULL );
				com_editors |= EDITOR_DEBUGGER;
				break;

			case DBMSG_DISCONNECT:
				ClearBreakpoints ( );
				Resume ( );
				mConnected = false;
				com_editors &= ~EDITOR_DEBUGGER;
				break;

			case DBMSG_ADDBREAKPOINT:
				HandleAddBreakpoint ( &msg );
				break;

			case DBMSG_REMOVEBREAKPOINT:
				HandleRemoveBreakpoint ( &msg );
				break;

			case DBMSG_RESUME:
				HandleResume ( &msg );
				break;

			case DBMSG_BREAK:
				mBreakNext = true;
				break;

			case DBMSG_STEPOVER:
				mBreakStepOver = true;
				mBreakStepOverDepth = ((idGameEditExt*) gameEdit)->GetInterpreterCallStackDepth(mBreakInterpreter);
				mBreakStepOverFunc1 = ((idGameEditExt*) gameEdit)->GetInterpreterCallStackFunction(mBreakInterpreter);
				if (mBreakStepOverDepth)
				{
					mBreakStepOverFunc2 = ((idGameEditExt*) gameEdit)->GetInterpreterCallStackFunction(mBreakInterpreter,mBreakStepOverDepth - 1);
				}
				else
				{
					mBreakStepOverFunc2 = NULL;
				}
				Resume ( );
				break;

			case DBMSG_STEPINTO:
				mBreakStepInto = true;
				Resume ( );
				break;

			case DBMSG_INSPECTVARIABLE:
				HandleInspectVariable ( &msg );
				break;

			case DBMSG_INSPECTCALLSTACK:
				HandleInspectCallstack ( &msg );
				break;

			case DBMSG_INSPECTTHREADS:
				HandleInspectThreads ( &msg );
				break;

			case DBMSG_INSPECTSCRIPTS:
				HandleInspectScripts( &msg );
				break;

			case DBMSG_EXECCOMMAND:
				HandleExecCommand( &msg );
				break;
		}
	}

	return true;
}

/*
================
rvDebuggerServer::SendMessage

Send a message with no data to the debugger server.
================
*/
void rvDebuggerServer::SendMessage ( EDebuggerMessage dbmsg )
{
	idBitMsg	 msg;
	byte	 buffer[MAX_MSGLEN];

	msg.Init( buffer, sizeof( buffer ) );
	msg.BeginWriting();
	msg.WriteShort ( (short)dbmsg );

	SendPacket ( msg.GetData(), msg.GetSize() );
}

/*
================
rvDebuggerServer::HandleAddBreakpoint

Handle the DBMSG_ADDBREAKPOINT message being sent by the debugger client.  This
message is handled by first checking if it is valid
and is added as a new breakpoint to the breakpoint list with the
data supplied in the message.
================
*/
void rvDebuggerServer::HandleAddBreakpoint ( idBitMsg* msg )
{
	bool onceOnly = false;
	long lineNumber;
	long id;
	char filename[2048]; // DG: randomly chose this size

	// Read the breakpoint info
	onceOnly = msg->ReadBits( 1 ) ? true : false;
	lineNumber = msg->ReadInt ( );
	id		   = msg->ReadInt ( );

	msg->ReadString ( filename, sizeof(filename) );

	//check for statement on requested breakpoint location 
	if (!((idGameEditExt*) gameEdit)->IsLineCode(filename, lineNumber))
	{
		idBitMsg	msgOut;
		byte		buffer[MAX_MSGLEN];

		msgOut.Init(buffer, sizeof(buffer));
		msgOut.BeginWriting();
		msgOut.WriteShort((short)DBMSG_REMOVEBREAKPOINT);
		msgOut.WriteInt(lineNumber);
		msgOut.WriteString(filename);
		SendPacket(msgOut.GetData(), msgOut.GetSize());
		return;
	}


	SDL_LockMutex( mCriticalSection );
	mBreakpoints.Append ( new rvDebuggerBreakpoint ( filename, lineNumber, id, onceOnly ) );
	SDL_UnlockMutex( mCriticalSection );
}

/*
================
rvDebuggerServer::HandleRemoveBreakpoint

Handle the DBMSG_REMOVEBREAKPOINT message being sent by the debugger client.  This
message is handled by removing the breakpoint that matches the given id from the
list.
================
*/
void rvDebuggerServer::HandleRemoveBreakpoint ( idBitMsg* msg )
{
	int i;
	int id;

	// ID that we are to remove
	id = msg->ReadInt ( );

	// Since breakpoints are used by both threads we need to
	// protect them with a crit section
	SDL_LockMutex( mCriticalSection );

	// Find the breakpoint that matches the given id and remove it from the list
	for ( i = 0; i < mBreakpoints.Num(); i ++ )
	{
		if ( mBreakpoints[i]->GetID ( ) == id )
		{
			delete mBreakpoints[i];
			mBreakpoints.RemoveIndex ( i );
			break;
		}
	}

	SDL_UnlockMutex( mCriticalSection );
}

/*
================
rvDebuggerServer::HandleResume

Resume the game thread.
================

*/
void rvDebuggerServer::HandleResume(idBitMsg* msg)
{
	//Empty msg
	Resume();
}

/*
================
rvDebuggerServer::HandleInspectCallstack

Handle an incoming inspect callstack message by sending a message
back to the client with the callstack data.
================
*/
void rvDebuggerServer::HandleInspectCallstack ( idBitMsg* msg )
{
	idBitMsg	 msgOut;
	byte		 buffer[MAX_MSGLEN];

	msgOut.Init(buffer, sizeof( buffer ) );
	msgOut.BeginWriting();
	msgOut.WriteShort ( (short)DBMSG_INSPECTCALLSTACK );

	((idGameEditExt*) gameEdit)->MSG_WriteInterpreterInfo(&msgOut, mBreakInterpreter, mBreakProgram, mBreakInstructionPointer);

	SendPacket (msgOut.GetData(), msgOut.GetSize() );
}

/*
================
rvDebuggerServer::HandleInspectThreads

Send the list of the current threads in the interpreter back to the debugger client
================
*/
void rvDebuggerServer::HandleInspectThreads ( idBitMsg* msg )
{
	idBitMsg	msgOut;
	byte		buffer[MAX_MSGLEN];
	int			i;

	// Initialize the message
	msgOut.Init( buffer, sizeof( buffer ) );
	msgOut.SetAllowOverflow(true);
	msgOut.BeginWriting();
	msgOut.WriteShort ( (short)DBMSG_INSPECTTHREADS );

	// Write the number of threads to the message
	msgOut.WriteShort ((short)((idGameEditExt*) gameEdit)->GetTotalScriptThreads() );

	// Loop through all of the threads and write their name and number to the message
	for ( i = 0; i < ((idGameEditExt*) gameEdit)->GetTotalScriptThreads(); i ++ )
	{
		((idGameEditExt*) gameEdit)->MSG_WriteThreadInfo(&msgOut,((idGameEditExt*) gameEdit)->GetThreadByIndex(i), mBreakInterpreter);
	}

	// Send off the inspect threads packet to the debugger client
	SendPacket (msgOut.GetData(), msgOut.GetSize() );
}

/*
================
rvDebuggerServer::HandleExecCommand

Send the list of the current loaded scripts in the interpreter back to the debugger client
================
*/
void rvDebuggerServer::HandleExecCommand( idBitMsg *msg ) {
	char cmdStr[2048]; // HvG: randomly chose this size

	msg->ReadString( cmdStr, sizeof( cmdStr ) );
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, cmdStr );	// valid command
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "\n" );
}


/*
================
rvDebuggerServer::HandleInspectScripts

Send the list of the current loaded scripts in the interpreter back to the debugger client
================
*/
void rvDebuggerServer::HandleInspectScripts( idBitMsg* msg )
{
	idBitMsg	 msgOut;
	byte		 buffer[MAX_MSGLEN];

	// Initialize the message
	msgOut.Init(buffer, sizeof(buffer));
	msgOut.BeginWriting();
	msgOut.WriteShort((short)DBMSG_INSPECTSCRIPTS);

	((idGameEditExt*) gameEdit)->MSG_WriteScriptList( &msgOut );

	SendPacket(msgOut.GetData(), msgOut.GetSize());
}

/*
================
rvDebuggerServer::HandleInspectVariable

Respondes to a request from the debugger client to inspect the value of a given variable
================
*/
void rvDebuggerServer::HandleInspectVariable ( idBitMsg* msg )
{
	char varname[256];
	int  scopeDepth;

	if ( !mBreak )
	{
		return;
	}

	scopeDepth = (short)msg->ReadShort ( );
	msg->ReadString ( varname, 256 );

	idStr varvalue;

	idBitMsg	 msgOut;
	byte		 buffer[MAX_MSGLEN];

	// Initialize the message
	msgOut.Init( buffer, sizeof( buffer ) );
	msgOut.BeginWriting();
	msgOut.WriteShort ( (short)DBMSG_INSPECTVARIABLE );

	if (!((idGameEditExt*) gameEdit)->GetRegisterValue(mBreakInterpreter, varname, varvalue, scopeDepth ) )
	{
		varvalue = "???";
	}

	msgOut.WriteShort ( (short)scopeDepth );
	msgOut.WriteString ( varname );
	msgOut.WriteString ( varvalue );

	SendPacket (msgOut.GetData(), msgOut.GetSize() );
}

/*
================
rvDebuggerServer::CheckBreakpoints

Check to see if any breakpoints have been hit.  This includes "break next",
"step into", and "step over" break points
================
*/
void rvDebuggerServer::CheckBreakpoints	( idInterpreter* interpreter, idProgram* program, int instructionPointer )
{
	const char*			filename;
	int					i;

	if ( !mConnected ) {
		return;
	}

	
	// Grab the current statement and the filename that it came from
	filename = ((idGameEditExt*) gameEdit)->GetFilenameForStatement(program, instructionPointer);
	int linenumber = ((idGameEditExt*) gameEdit)->GetLineNumberForStatement(program, instructionPointer);

	// Operate on lines, not statements
	if ( mLastStatementLine == linenumber && mLastStatementFile == filename)
	{
		return;
	}
	
	// Save the last visited line and file so we can prevent
	// double breaks on lines with more than one statement
	mLastStatementFile = idStr(filename);
	mLastStatementLine = linenumber;

	// Reset stepping when the last function on the callstack is returned from
	if ( ((idGameEditExt*) gameEdit)->ReturnedFromFunction(program, interpreter,instructionPointer))
	{
		mBreakStepOver = false;
		mBreakStepInto = false;
	}

	// See if we are supposed to break on the next script line
	if ( mBreakNext )
	{
		HandleInspectScripts(NULL);
		Break ( interpreter, program, instructionPointer );
		return;
	}

	// Only break on the same callstack depth and thread as the break over
	if ( mBreakStepOver )
	{
		//virtual bool CheckForBreakpointHit(interpreter,function1,function2,depth)
		if (((idGameEditExt*) gameEdit)->CheckForBreakPointHit(interpreter, mBreakStepOverFunc1, mBreakStepOverFunc2, mBreakStepOverDepth))
		{
			Break ( interpreter, program, instructionPointer );
			return;
		}
	}

	// See if we are supposed to break on the next line
	if ( mBreakStepInto )
	{
		HandleInspectScripts(NULL);
		// Break
		Break ( interpreter, program, instructionPointer );
		return;
	}

	idStr qpath;
	OSPathToRelativePath(filename,qpath);
	qpath.BackSlashesToSlashes ( );

	SDL_LockMutex( mCriticalSection );

	// Check all the breakpoints
	for ( i = 0; i < mBreakpoints.Num ( ); i ++ )
	{
		rvDebuggerBreakpoint* bp = mBreakpoints[i];

		// Skip if not match of the line number
		if ( linenumber != bp->GetLineNumber ( ) )
		{
			continue;
		}

		// Skip if no match of the filename
		if ( idStr::Icmp ( bp->GetFilename(), qpath.c_str() ) )
		{
			continue;
		}

		// DG: onceOnly support
		if ( bp->GetOnceOnly() ) {
			// we'll do the one Break() a few lines below; remove it here while mBreakpoints is unmodified
			// (it can be modifed from the client while in Break() below)
			mBreakpoints.RemoveIndex( i );
			delete bp;

			// also tell client to remove the breakpoint
			idBitMsg	msgOut;
			byte		buffer[MAX_MSGLEN];
			msgOut.Init( buffer, sizeof( buffer ) );
			msgOut.BeginWriting();
			msgOut.WriteShort( (short)DBMSG_REMOVEBREAKPOINT );
			msgOut.WriteInt( linenumber );
			msgOut.WriteString( qpath.c_str() );
			SendPacket( msgOut.GetData(), msgOut.GetSize() );
		}
		// DG end

		// Pop out of the critical section so we dont get stuck
		SDL_UnlockMutex( mCriticalSection );

		HandleInspectScripts(NULL);
		// We hit a breakpoint, so break
		Break ( interpreter, program, instructionPointer );

		// Back into the critical section since we are going to have to leave it
		SDL_LockMutex( mCriticalSection );

		break;
	}

	SDL_UnlockMutex( mCriticalSection );
}

/*
================
rvDebuggerServer::Break

Halt execution of the game threads and inform the debugger client that
the game has been halted
================
*/
void rvDebuggerServer::Break ( idInterpreter* interpreter, idProgram* program, int instructionPointer )
{
	idBitMsg			msg;
	byte				buffer[MAX_MSGLEN];
	const char*			filename;

	// Clear all the break types
	mBreakStepOver = false;
	mBreakStepInto = false;
	mBreakNext     = false;

	// Grab the current statement and the filename that it came from
	filename = ((idGameEditExt*) gameEdit)->GetFilenameForStatement(program,instructionPointer);
	int linenumber = ((idGameEditExt*) gameEdit)->GetLineNumberForStatement(program, instructionPointer);
	idStr fileStr = filename;
	fileStr.BackSlashesToSlashes();

	// Give the mouse cursor back to the world
	Sys_GrabMouseCursor( false );

	// Set the break variable so we know the main thread is stopped
	mBreak = true;
	mBreakProgram = program;
	mBreakInterpreter = interpreter;
	mBreakInstructionPointer = instructionPointer;

	// Inform the debugger of the breakpoint hit
	msg.Init( buffer, sizeof( buffer ) );
	msg.BeginWriting();
	msg.WriteShort ( (short)DBMSG_BREAK );
	msg.WriteInt ( linenumber );
	msg.WriteString ( fileStr.c_str() );

	//msg.WriteInt64( (int64_t)mBreakProgram );

	SendPacket ( msg.GetData(), msg.GetSize() );

	// Suspend the game thread.  Since this will be called from within the main game thread
	// execution wont return until after the thread is resumed
	// DG: the original code used Win32 SuspendThread() here, but as there is no equivalent
	//     function in SDL and as this is only called within the main game thread anyway,
	//     just use a condition variable to put this thread to sleep until Resume() has set mBreak
	SDL_LockMutex( mGameThreadBreakLock );
	while ( mBreak ) {
		SDL_CondWait( mGameThreadBreakCond, mGameThreadBreakLock );
	}
	SDL_UnlockMutex( mGameThreadBreakLock );

	// Let the debugger client know that we have started back up again
	SendMessage ( DBMSG_RESUMED );

	// this should be platform specific
	// TODO: maybe replace with SDL code? or does it not matter if debugger client runs on another machine?
#if defined( ID_ALLOW_TOOLS )
	// This is to give some time between the keypress that
	// told us to resume and the setforeground window.  Otherwise the quake window
	// would just flash
	Sleep ( 150 );

	// Bring the window back to the foreground
	SetForegroundWindow ( win32.hWnd );
	SetActiveWindow ( win32.hWnd );
	UpdateWindow ( win32.hWnd );
	SetFocus ( win32.hWnd );
#endif

	// Give the mouse cursor back to the game
	// HVG_Note : there be dragons here. somewhere.
	Sys_GrabMouseCursor( true );

	// Clear all commands that were generated before we went into suspended mode.  This is
	// to ensure we dont have mouse downs with no ups because the context was changed.
	idKeyInput::ClearStates();
}

/*
================
rvDebuggerServer::Resume

Resume execution of the game.
================
*/
void rvDebuggerServer::Resume ( void )
{
	// Cant resume if not paused
	if ( !mBreak )
	{
		return;
	}

	// Start the game thread back up
	SDL_LockMutex( mGameThreadBreakLock );
	mBreak = false;
	SDL_CondSignal( mGameThreadBreakCond);
	SDL_UnlockMutex( mGameThreadBreakLock );
}

/*
================
rvDebuggerServer::ClearBreakpoints

Remove all known breakpoints
================
*/
void rvDebuggerServer::ClearBreakpoints	( void )
{
	int i;

	for ( i = 0; i < mBreakpoints.Num(); i ++ )
	{
		delete mBreakpoints[i];
	}

	mBreakpoints.Clear ( );
}

/*
================
rvDebuggerServer::Print

Sends a console print message over to the debugger client
================
*/
void rvDebuggerServer::Print ( const char* text )
{
	if ( !mConnected )
	{
		return;
	}

	idBitMsg msg;
	byte	 buffer[MAX_MSGLEN];

	msg.Init( buffer, sizeof( buffer ) );
	msg.BeginWriting();
	msg.WriteShort ( (short)DBMSG_PRINT );
	msg.WriteString ( text );

	SendPacket ( msg.GetData(), msg.GetSize() );
}
