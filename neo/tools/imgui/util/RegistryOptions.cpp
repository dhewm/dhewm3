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

#include "sys/sys_imgui.h"
#include "framework/FileSystem.h"
#include "idlib/Token.h"

#include "RegistryOptions.h"

namespace ImGuiTools {

/*
================
rvRegistryOptions::rvRegistryOptions

Constructor
================
*/
rvRegistryOptions::rvRegistryOptions( void ) {
}

/*
================
rvRegistryOptions::Init
================
*/
void rvRegistryOptions::Init( const char *key ) {
	mBaseKey = key;
}

void OutputString( idFile *file, const char *string ) {
	char *out;
	int i, c;

	while ( 1 ) {
		c = *string++;
		switch( c ) {
			case '\0': return;
			case '\\': file->Printf( "\\\\" ); break;
			case '\n': file->Printf( "\\n" ); break;
			case '\r': file->Printf( "\\r" ); break;
			case '\t': file->Printf( "\\t" ); break;
			case '\v': file->Printf( "\\v" ); break;
			case '"': file->Printf( "\\\"" ); break;
			default: file->Printf( "%c", c ); break;
		}
	}
}

/*
================
rvRegistryOptions::Save

Write the options to the registry
================
*/
bool rvRegistryOptions::Save ( void )
{
	int		i;

	idFile	*file = fileSystem->OpenFileWrite( mBaseKey );
	if ( !file ) {
		return false;
	}
	
	file->Printf( "{\n" );

	// Write out the values
	for ( i = 0; i < mValues.GetNumKeyVals(); i ++ )
	{
		const idKeyValue* key = mValues.GetKeyVal ( i );
		assert ( key );
		
		file->Printf( "%s ", key->GetKey().c_str() );
		file->Printf( "\"" );
		OutputString( file, key->GetValue().c_str() );
		file->Printf( "\"\n" );
	}

	file->Printf( "}\n" );

	// Write Recent Files
	file->Printf( "{\n" );
	for ( i = 0; i < mRecentFiles.Num(); i ++ )
	{
		file->Printf( "mru%d ", i );
		file->Printf( "\"%s\"", mRecentFiles[i].c_str() );
		file->Printf( "\n" );
	}
	file->Printf( "}\n" );

	file->Flush();
	fileSystem->CloseFile( file );

	return true;
}

/*
================
rvRegistryOptions::Load

Read the options from the registry
================
*/
bool rvRegistryOptions::Load ( void )
{
	const char *buffer = NULL;

	mValues.Clear ( );
	mRecentFiles.Clear ( );

	idLexer src( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );

	int len = fileSystem->ReadFile( mBaseKey.c_str(), (void**)&buffer );
	if ( len <= 0 ) {
		return false;
	}
	src.LoadMemory( buffer, strlen( buffer ), mBaseKey.c_str() );
	if ( !src.IsLoaded() ) {
		return false;
	}

	idToken tok, tok2;
	src.ExpectTokenString( "{" );
	while ( src.ReadToken( &tok ) ) {
		if ( tok == "}" ) {
			break;
		}
		if ( src.ReadToken( &tok2 ) ) {
			if ( tok2 == "}" ) {
				break;
			}
			mValues.Set( tok.c_str(), tok2.c_str() );
		}
	}

	if ( tok != "}" ) {
		idLib::fileSystem->FreeFile( (void*)buffer );
		return false;
	}

	while ( src.ReadToken( &tok ) ) {
		if ( tok == "}" ) {
			break;
		}
		if ( src.ReadToken( &tok2 ) ) {
			if ( tok2 == "}" ) {
				break;
			}
			AddRecentFile( tok2.c_str() );
		}
	}

	idLib::fileSystem->FreeFile( (void*)buffer );

	return true;
}

/*
================
rvRegistryOptions::SetWindowPlacement

Set a window placement in the options
================
*/
void rvRegistryOptions::SetWindowPlacement ( const char* name, int hwnd )
{
	/*WINDOWPLACEMENT wp;

	wp.length = sizeof(wp);
	::GetWindowPlacement ( hwnd, &wp );

	idStr out;

	out = va("%d %d %d %d %d %d %d %d %d %d",
			 wp.flags,
			 wp.ptMaxPosition.x,
			 wp.ptMaxPosition.y,
			 wp.ptMinPosition.x,
			 wp.ptMinPosition.y,
			 wp.rcNormalPosition.left,
			 wp.rcNormalPosition.top,
			 wp.rcNormalPosition.right,
			 wp.rcNormalPosition.bottom,
			 wp.showCmd );

	mValues.Set ( name, out );*/
}

/*
================
rvRegistryOptions::GetWindowPlacement

Retrieve a window placement from the options
================
*/
bool rvRegistryOptions::GetWindowPlacement ( const char* name, int hwnd )
{/*
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);

	const idKeyValue* key = mValues.FindKey ( name );
	if ( !key )
	{
		return false;
	}

	sscanf ( key->GetValue().c_str(), "%d %d %d %d %d %d %d %d %d %d",
			 &wp.flags,
			 &wp.ptMaxPosition.x,
			 &wp.ptMaxPosition.y,
			 &wp.ptMinPosition.x,
			 &wp.ptMinPosition.y,
			 &wp.rcNormalPosition.left,
			 &wp.rcNormalPosition.top,
			 &wp.rcNormalPosition.right,
			 &wp.rcNormalPosition.bottom,
			 &wp.showCmd );

	::SetWindowPlacement ( hwnd, &wp );
	
	return true;*/
	return false;
}

/*
================
rvRegistryOptions::AddRecentFile

Adds the given filename to the MRU list
================
*/
void rvRegistryOptions::AddRecentFile ( const char* filename )
{
	int i;

	idStr path = filename;

	// Remove duplicates first
	for ( i = mRecentFiles.Num() - 1; i >= 0; i -- )
	{
		if ( !mRecentFiles[i].Icmp ( filename ) )
		{
			mRecentFiles.RemoveIndex ( i );
			break;
		}
	}

	// Alwasy trip to the max MRU size
	while ( mRecentFiles.Num ( ) >= MAX_MRU_SIZE )
	{
		mRecentFiles.RemoveIndex ( 0 );
	}

	mRecentFiles.Append ( path );
}

/*
================
rvRegistryOptions::SetColumnWidths

Set a group of column widths in the options
================
*/
void rvRegistryOptions::SetColumnWidths ( const char* name, int list )
{
	/*LVCOLUMN col;
	int		 index;
	idStr	 widths;

	col.mask = LVCF_WIDTH;

	for ( index = 0; ListView_GetColumn ( list, index, &col ); index ++ )
	{
		widths += va("%d ", col.cx );
	}

	mValues.Set ( name, widths );*/
}

/*
================
rvRegistryOptions::GetColumnWidths

Retrieve a group of column widths from the options
================
*/
void rvRegistryOptions::GetColumnWidths ( const char* name, int list )
{/*
	idStr		widths;
	const char* parse;
	const char* next;
	int			index;

	widths = mValues.GetString ( name );
	parse = widths;
	index = 0;

	while ( NULL != (next = strchr ( parse, ' ' ) ) )
	{
		int width;

		sscanf ( parse, "%d", &width );
		parse = next + 1;

		ListView_SetColumnWidth ( list, index++, width );
	}*/
}

/*
================
rvRegistryOptions::SetBinary

Set binary data for the given key
================
*/
void rvRegistryOptions::SetBinary ( const char* name, const unsigned char* data, int size )
{
	idStr binary;
	for ( size --; size >= 0; size --, data++ )
	{
		binary += va("%02x", *data );
	}

	mValues.Set ( name, binary );
}

/*
================
rvRegistryOptions::GetBinary

Get the binary data for a given key
================
*/
void rvRegistryOptions::GetBinary ( const char* name, unsigned char* data, int size )
{
	const char* parse;
	parse = mValues.GetString ( name );
	for ( size --; size >= 0 && *parse && *(parse+1); size --, parse += 2, data ++  )
	{
		int value;
		sscanf ( parse, "%02x", &value );
		*data = (unsigned char)value;
	}
}

}
