/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2016 Daniel Gibson
Copyright (C) 2022 Stephen Pridham

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/


//#include "../imgui/BFGimgui.h"

#include "framework/Common.h"
#include "framework/Game.h"

#include "idlib/CmdArgs.h"

#include "sys/sys_imgui.h"

// TODO #include "afeditor/AfEditor.h"
#include "lighteditor/LightEditor.h"


static bool releaseMouse = false;

#if 0 // currently this doesn't make too much sense
void ShowEditors_f( const idCmdArgs& args )
{
	showToolWindows = true;
}
#endif // 0

namespace ImGuiTools
{

// things in impl need to be used in at least one other file, but should generally not be touched
namespace impl
{

void SetReleaseToolMouse( bool doRelease )
{
	releaseMouse = doRelease;
}

} //namespace impl

static idEntity* GetSelectedEntity( const idDict* dict, const char* spawnclass )
{
	idEntity* selEntities[16] = {};
	int numEnts = gameEdit->GetSelectedEntities(selEntities, 16);
	const char* name = NULL;
	if ( dict != NULL ) {
		name = dict->GetString( "name", NULL );
		if ( spawnclass == NULL ) {
			spawnclass = dict->GetString( "spawnclass", NULL );
		}
	} else if ( spawnclass == NULL ) { // both NULL
		assert( 0 && "GetSelectedEntity() needs either a spawnclass or an entity dict!" );
		return NULL;
	}

	if ( spawnclass == NULL && name == NULL ) {
		// we may have had a dict, but it had no name, so what can you do..
		// TODO: log warning?
		return NULL;
	}

	for ( int i=0; i<numEnts; ++i ) {
		idEntity* ent = selEntities[i];
		const idDict* spawnArgs = gameEdit->EntityGetSpawnArgs( ent );
		if ( spawnArgs != NULL ) {
			// TODO: theoretically spawnArgs == dict could work, as the dict passed here *is*
			//       the entity's spawnargs - they call common->InitTool( EDITOR_FOO, &spawnArgs );
			if ( spawnclass != NULL ) {
				const char* entClass = spawnArgs->GetString( "spawnclass", NULL );
				if ( entClass != NULL && idStr::Icmp(spawnclass, entClass) == 0 ) {
					const char* entName = spawnArgs->GetString( "name", NULL );
					if ( name == NULL
					    || (entName != NULL && idStr::Icmp(name, entName) == 0) )
					{
						return ent;
					}
				}
			} else { // match just by name
				const char* entName = spawnArgs->GetString( "name", NULL );
				if ( entName != NULL && idStr::Icmp(name, entName) == 0 ) {
					return ent;
				}
			}
		}
	}

	return NULL;
}

// TODO: replace with sys_imgui's openImguiWindows ?
bool AreEditorsActive()
{
	return cvarSystem->GetCVarInteger( "g_editEntityMode" ) > 0 || com_editors != 0;
}

bool ReleaseMouseForTools()
{
	return AreEditorsActive() && releaseMouse;
}

void DrawToolWindows()
{
	if( LightEditor::Instance().IsShown() )
	{
		LightEditor::Instance().Draw();
	}
	/* TODO
	else if( AfEditor::Instance().IsShown() )
	{
		AfEditor::Instance().Draw();
	}*/
}

void LightEditorInit( const idDict* dict )
{
	if( dict == NULL ) {
		return;
	}

	idEntity* ent = GetSelectedEntity(dict, "idLight");
	if( ent == NULL ) {
		return;
	}

	// NOTE: we can't access idEntity (it's just a declaration), because it should
	// be game/mod specific. but we can at least check the spawnclass from the dict.
	assert( idStr::Icmp( dict->GetString( "spawnclass" ), "idLight" ) == 0
			  && "LightEditorInit() must only be called with light entities or NULL!" );

	LightEditor::Instance().ShowIt( true );
	impl::SetReleaseToolMouse( true );

	LightEditor::ReInit( dict, ent );

	D3::ImGuiHooks::OpenWindow( D3::ImGuiHooks::D3_ImGuiWin_LightEditor );
}

void AfEditorInit() // TODO: why no passed spawnargs?
{
	/* TODO
	D3::ImGuiHooks::OpenWindow( D3::ImGuiHooks::D3_ImGuiWin_AfEditor );
	AfEditor::Instance().ShowIt( true );
	impl::SetReleaseToolMouse( true );
	*/
}

} //namespace ImGuiTools
