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

#include "tools/edit_gui_common.h"


#include "DebuggerApp.h"

/*
================
rvDebuggerClient::rvDebuggerClient
================
*/
rvDebuggerClient::rvDebuggerClient ( )
{
	mConnected = false;
	mWaitFor   = DBMSG_UNKNOWN;
}

/*
================
rvDebuggerClient::~rvDebuggerClient
================
*/
rvDebuggerClient::~rvDebuggerClient ( )
{
	ClearBreakpoints ( );
	ClearCallstack ( );
	ClearThreads ( );
}

/*
================
rvDebuggerClient::Initialize

Initialize the debugger client
================
*/
bool rvDebuggerClient::Initialize ( void )
{
	// Nothing else can run with the debugger
	com_editors = EDITOR_DEBUGGER;

	// Initialize the network connection
	if ( !mPort.InitForPort ( 27981 ) )
	{
		return false;
	}

	// Server must be running on the local host on port 28980
	Sys_StringToNetAdr ( com_dbgServerAdr.GetString( ), &mServerAdr, true );
	mServerAdr.port = 27980;

	// Attempt to let the server know we are here.  The server may not be running so this
	// message will just get ignored.
	SendMessage ( DBMSG_CONNECT );

	return true;
}

/*
================
rvDebuggerClient::Shutdown

Shutdown the debugger client and let the debugger server
know we are shutting down
================
*/
void rvDebuggerClient::Shutdown ( void )
{
	if ( mConnected )
	{
		SendMessage ( DBMSG_DISCONNECT );
		mConnected = false;
	}
}

/*
================
rvDebuggerClient::ProcessMessages

Process all incomding messages from the debugger server
================
*/
bool rvDebuggerClient::ProcessMessages ( void )
{
	netadr_t adrFrom;
	idBitMsg	 msg;
	byte	 buffer[MAX_MSGLEN];

	msg.SetSize(MAX_MSGLEN);
	msg.BeginReading();

	int msgSize;
	// Check for pending udp packets on the debugger port
	while ( mPort.GetPacket ( adrFrom, buffer,msgSize, MAX_MSGLEN) )
	{
		short command;
		msg.Init(buffer, sizeof(buffer));
		msg.SetSize(msgSize);
		msg.BeginReading();

		// Only accept packets from the debugger server for security reasons
		if ( !Sys_CompareNetAdrBase ( adrFrom, mServerAdr ) )
		{
			continue;
		}
		command = msg.ReadShort ( );

		// Is this what we are waiting for? 
		if ( command == mWaitFor )
		{
			mWaitFor = DBMSG_UNKNOWN;
		}

		switch ( command )
		{
			case DBMSG_CONNECT:
				mConnected = true;
				SendMessage ( DBMSG_CONNECTED );
				SendBreakpoints ( );
				break;

			case DBMSG_CONNECTED:
				mConnected = true;
				SendBreakpoints ( );
				break;

			case DBMSG_DISCONNECT:
				mConnected = false;
				break;

			case DBMSG_BREAK:
				HandleBreak ( &msg );
				break;

			// Callstack being send to the client
			case DBMSG_INSPECTCALLSTACK:
				HandleInspectCallstack ( &msg );
				break;

			// Thread list is being sent to the client
			case DBMSG_INSPECTTHREADS:
				HandleInspectThreads ( &msg );
				break;

			case DBMSG_INSPECTVARIABLE:
				HandleInspectVariable ( &msg );
				break;

			case DBMSG_REMOVEBREAKPOINT:
				HandleRemoveBreakpoint( &msg );
				break;
			case DBMSG_INSPECTSCRIPTS:
				HandleInspectScripts( &msg );
				break;
		}

		// Give the window a chance to process the message
		msg.SetReadCount(0);
		msg.SetReadBit(0);
		gDebuggerApp.GetWindow().ProcessNetMessage ( &msg );
	}

	return true;
}

void rvDebuggerClient::HandleRemoveBreakpoint(idBitMsg* msg)
{
	long lineNumber;
	char filename[MAX_PATH];

	// Read the breakpoint info

	lineNumber = msg->ReadInt();
	msg->ReadString(filename, MAX_PATH);

	rvDebuggerBreakpoint* bp = FindBreakpoint(filename, lineNumber);
	if(bp)
		RemoveBreakpoint(bp->GetID());
}

/*
================
rvDebuggerClient::HandleBreak

Handle the DBMSG_BREAK message send from the server.  This message is handled
by caching the file and linenumber where the break occured.
================
*/
void rvDebuggerClient::HandleBreak ( idBitMsg* msg )
{
	char filename[MAX_PATH];

	mBreak = true;

	// Line number
	mBreakLineNumber = msg->ReadInt ( );

	// Filename
	msg->ReadString ( filename, MAX_PATH );
	mBreakFilename   = filename;

	//int64_t ptr64b = msg->ReadInt64();
	//mBreakProgram = (idProgram*)ptr64b;

	// Clear the variables
	mVariables.Clear ( );

	// Request the callstack and threads
	SendMessage ( DBMSG_INSPECTCALLSTACK );
	WaitFor ( DBMSG_INSPECTCALLSTACK, 2000 );

	SendMessage ( DBMSG_INSPECTTHREADS );
	WaitFor ( DBMSG_INSPECTTHREADS, 2000 );
}


/*
================
rvDebuggerClient::InspectScripts

Instructs the client to inspect the loaded scripts
================
*/
void rvDebuggerClient::InspectScripts ( void )
{
	idBitMsg	msg;
	byte		buffer[MAX_MSGLEN];

	msg.Init(buffer, sizeof(buffer));
	msg.BeginWriting();
	msg.WriteShort((short)DBMSG_INSPECTSCRIPTS);
	SendPacket(msg.GetData(), msg.GetSize());
}


/*
================
rvDebuggerClient::InspectVariable

Instructs the client to inspect the given variable at the given callstack depth.  The
variable is inspected by sending a DBMSG_INSPECTVARIABLE message to the server which
will in turn respond back to the client with the variable value
================
*/
void rvDebuggerClient::InspectVariable ( const char* name, int callstackDepth )
{
	idBitMsg	msg;
	byte		buffer[MAX_MSGLEN];

	msg.Init( buffer, sizeof( buffer ) );
	msg.BeginWriting();
	msg.WriteShort ( (short)DBMSG_INSPECTVARIABLE );
	msg.WriteShort ( (short)(mCallstack.Num()-callstackDepth) );
	msg.WriteString ( name );

	SendPacket ( msg.GetData(), msg.GetSize());
}

/*
================
rvDebuggerClient::HandleInspectScripts

Handle the message DBMSG_INSPECTSCRIPTS being sent from the server.  This message
is handled by adding the script entries to a list for later lookup.
================
*/
void rvDebuggerClient::HandleInspectScripts( idBitMsg* msg )
{	
	int totalScripts;

	mServerScripts.Clear();

	// Read all of the callstack entries specfied in the message
	for (totalScripts = msg->ReadInt(); totalScripts > 0; totalScripts--)
	{
		char temp[1024];

		// Script Name
		msg->ReadString(temp, 1024);
		mServerScripts.Append(temp);
	}
}

/*
================
rvDebuggerClient::HandleInspectCallstack

Handle the message DBMSG_INSPECTCALLSTACK being sent from the server.  This message
is handled by adding the callstack entries to a list for later lookup.
================
*/
void rvDebuggerClient::HandleInspectCallstack ( idBitMsg* msg )
{
	int depth;

	ClearCallstack ( );

	// Read all of the callstack entries specfied in the message
	for ( depth = (short)msg->ReadShort ( ) ; depth > 0; depth -- )
	{
		rvDebuggerCallstack* entry = new rvDebuggerCallstack;

		char temp[1024];

		// Function name
		msg->ReadString ( temp, 1024 );
		entry->mFunction = idStr(temp);

		// Filename
		msg->ReadString ( temp, 1024 );
		entry->mFilename = idStr(temp);

		// Line Number
		entry->mLineNumber = msg->ReadInt ( );

		// Add to list
		mCallstack.Append ( entry );
	}
}

/*
================
rvDebuggerClient::HandleInspectThreads

Handle the message DBMSG_INSPECTTHREADS being sent from the server.  This message
is handled by adding the list of threads to a list for later lookup.
================
*/
void rvDebuggerClient::HandleInspectThreads ( idBitMsg* msg )
{
	int	count;

	ClearThreads ( );

	// Loop over the number of threads in the message
	for ( count = (short)msg->ReadShort ( ) ; count > 0; count -- )
	{
		rvDebuggerThread* entry = new rvDebuggerThread;

		char temp[1024];

		// Thread name
		msg->ReadString ( temp, 1024 );
		entry->mName = temp;

		// Thread ID
		entry->mID = msg->ReadInt ( );

		// Thread state
		entry->mCurrent = msg->ReadBits ( 1 ) ? true : false;
		entry->mDoneProcessing = msg->ReadBits ( 1 ) ? true : false;
		entry->mWaiting = msg->ReadBits ( 1 ) ? true : false;
		entry->mDying = msg->ReadBits ( 1 ) ? true : false;

		// Add thread to list
		mThreads.Append ( entry );
	}
}

/*
================
rvDebuggerClient::HandleInspectVariable

Handle the message DBMSG_INSPECTVARIABLE being sent from the server.  This message
is handled by adding the inspected variable to a dictionary for later lookup
================
*/
void rvDebuggerClient::HandleInspectVariable ( idBitMsg* msg )
{
	char	var[1024];
	char	value[1024];
	int		callDepth;

	callDepth = (short)msg->ReadShort ( );
	msg->ReadString ( var, 1024 );
	msg->ReadString ( value, 1024 );

	mVariables.Set ( va("%d:%s", mCallstack.Num()-callDepth, var), value );
}

/*
================
rvDebuggerClient::WaitFor

Waits the given amount of time for the specified message to be received by the
debugger client.
================
*/
bool rvDebuggerClient::WaitFor ( EDebuggerMessage msg, int time )
{
	int start;

	// Cant wait if not connected
	if ( !mConnected )
	{
		return false;
	}

	start    = Sys_Milliseconds ( );
	mWaitFor = msg;

	while ( mWaitFor != DBMSG_UNKNOWN && Sys_Milliseconds()-start < time )
	{
		ProcessMessages ( );
		Sleep ( 0 );
	}

	if ( mWaitFor != DBMSG_UNKNOWN )
	{
		mWaitFor = DBMSG_UNKNOWN;
		return false;
	}

	return true;
}

/*
================
rvDebuggerClient::FindBreakpoint

Searches for a breakpoint that maches the given filename and linenumber
================
*/
rvDebuggerBreakpoint* rvDebuggerClient::FindBreakpoint ( const char* filename, int linenumber )
{
	int i;

	for ( i = 0; i < mBreakpoints.Num(); i ++ )
	{
		rvDebuggerBreakpoint* bp = mBreakpoints[i];

		if ( linenumber == bp->GetLineNumber ( ) && !idStr::Icmp ( bp->GetFilename ( ), filename ) )
		{
			return bp;
		}
	}

	return NULL;
}

/*
================
rvDebuggerClient::ClearBreakpoints

Removes all breakpoints from the client and server
================
*/
void rvDebuggerClient::ClearBreakpoints ( void )
{
	int i;

	for ( i = 0; i < GetBreakpointCount(); i ++ )
	{
		rvDebuggerBreakpoint* bp = mBreakpoints[i];
		assert ( bp );

		SendRemoveBreakpoint ( *bp );
		delete bp;
	}

	mBreakpoints.Clear ( );
}

/*
================
rvDebuggerClient::AddBreakpoint

Adds a breakpoint to the client and server with the give nfilename and linenumber
================
*/
int rvDebuggerClient::AddBreakpoint ( const char* filename, int lineNumber, bool onceOnly )
{
	int index = mBreakpoints.Append ( new rvDebuggerBreakpoint ( filename, lineNumber, -1, onceOnly ) );

	SendAddBreakpoint ( *mBreakpoints[index] );

	return index;
}

/*
================
rvDebuggerClient::RemoveBreakpoint

Removes the breakpoint with the given ID from the client and server
================
*/
bool rvDebuggerClient::RemoveBreakpoint ( int bpID )
{
	int index;

	for ( index = 0; index < GetBreakpointCount(); index ++ )
	{
		if ( mBreakpoints[index]->GetID ( ) == bpID )
		{
			SendRemoveBreakpoint ( *mBreakpoints[index] );
			delete mBreakpoints[index];
			mBreakpoints.RemoveIndex ( index );
			return true;
		}
	}

	return false;
}

/*
================
rvDebuggerClient::SendMessage

Send a message with no data to the debugger server
================
*/
void rvDebuggerClient::SendMessage ( EDebuggerMessage dbmsg )
{
	idBitMsg	 msg;
	byte	 buffer[MAX_MSGLEN];

	msg.Init ( buffer, sizeof( buffer ) );
	msg.BeginWriting ( );
	msg.WriteShort ( (short)dbmsg );

	SendPacket ( msg.GetData(), msg.GetSize() );
}

/*
================
rvDebuggerClient::SendBreakpoints

Send all breakpoints to the debugger server
================
*/
void rvDebuggerClient::SendBreakpoints ( void )
{
	int i;

	if ( !mConnected )
	{
		return;
	}

	// Send all the breakpoints to the server
	for ( i = 0; i < mBreakpoints.Num(); i ++ )
	{
		SendAddBreakpoint ( *mBreakpoints[i] );
	}
}

/*
================
rvDebuggerClient::SendAddBreakpoint

Send an individual breakpoint over to the debugger server
================
*/
void rvDebuggerClient::SendAddBreakpoint ( rvDebuggerBreakpoint& bp )
{
	idBitMsg msg;
	byte	 buffer[MAX_MSGLEN];

	if ( !mConnected )
	{
		return;
	}

	msg.Init( buffer, sizeof( buffer ) );
	msg.BeginWriting();
	msg.WriteShort	( (short)DBMSG_ADDBREAKPOINT );
	msg.WriteBits	( bp.GetOnceOnly() ? 1 : 0, 1 );
	msg.WriteInt	( (unsigned long) bp.GetLineNumber ( ) );
	msg.WriteInt	( bp.GetID ( ) );
	msg.WriteString ( bp.GetFilename() ); // FIXME: this implies make7bit ?!

	SendPacket ( msg.GetData(), msg.GetSize() );
}

/*
================
rvDebuggerClient::SendRemoveBreakpoint

Sends a remove breakpoint message to the debugger server
================
*/
void rvDebuggerClient::SendRemoveBreakpoint ( rvDebuggerBreakpoint& bp )
{
	idBitMsg	 msg;
	byte	 buffer[MAX_MSGLEN];

	if ( !mConnected )
	{
		return;
	}

	msg.Init		( buffer, sizeof( buffer ) );
	msg.BeginWriting( );
	msg.WriteShort	( (short)DBMSG_REMOVEBREAKPOINT );
	msg.WriteInt	( bp.GetID() );

	SendPacket ( msg.GetData(), msg.GetSize() );
}

/*
================
rvDebuggerClient::ClearCallstack

Clear all callstack entries
================
*/
void rvDebuggerClient::ClearCallstack ( void )
{
	int depth;

	for ( depth = 0; depth < mCallstack.Num(); depth ++ )
	{
		delete mCallstack[depth];
	}

	mCallstack.Clear ( );
}

/*
================
rvDebuggerClient::ClearThreads

Clear all thread entries
================
*/
void rvDebuggerClient::ClearThreads ( void )
{
	int i;

	for ( i = 0; i < mThreads.Num(); i ++ )
	{
		delete mThreads[i];
	}

	mThreads.Clear ( );
}
/*
================
rvDebuggerClient::SendCommand
================
*/
void rvDebuggerClient::SendCommand( const char *cmdStr )
{
	idBitMsg msg;
	byte	 buffer[MAX_MSGLEN];

	if ( !mConnected ) 	{
		return;
	}

	msg.Init( buffer, sizeof( buffer ) );
	msg.BeginWriting( );
	msg.WriteShort( ( short ) DBMSG_EXECCOMMAND );
	msg.WriteString( cmdStr ); // FIXME: this implies make7bit ?!

	SendPacket( msg.GetData( ), msg.GetSize( ) );
}

