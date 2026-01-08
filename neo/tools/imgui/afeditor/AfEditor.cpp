/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

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

#include "AfEditor.h"

#include "../util/ImGui_IdWidgets.h"

#include "framework/FileSystem.h"
#include "framework/Game.h"

namespace ImGuiTools
{

static bool BodyItemGetter( void* data, int index, const char** items );
static bool ConstraintItemGetter( void* data, int index, const char** items );

static bool CVarCheckBox( const char* label, const char* cvarname );

AfEditor::AfEditor()
	: isShown( false )
	, fileSelection( 0 )
	, currentAf( 0 )
	, currentConstraint( 0 )
	, currentBodySelection( 0 )
	, currentEntity( 0 )
	, decl( nullptr )
	, body( nullptr )
	, constraint( nullptr )
	, propertyEditor( nullptr )
{
}

AfEditor::~AfEditor()
{
	if( propertyEditor )
	{
		delete propertyEditor;
	}
	bodyEditors.DeleteContents( true );
	constraintEditors.DeleteContents( true );
}

void AfEditor::Init()
{
}

void AfEditor::Draw()
{
	bool showTool = isShown;

	if( ImGui::Begin( "AF Editor", &showTool, ImGuiWindowFlags_MenuBar ) )
	{
		impl::SetReleaseToolMouse( true );

		bool changedAf = false;
		bool openedAfBrowser = false;
		bool clickedNew = false;

		if( ImGui::BeginMenuBar() )
		{
			if( ImGui::BeginMenu( "File" ) )
			{
				if( ImGui::MenuItem( "New", "Ctrl+N" ) )
				{
					clickedNew = true;
				}

				if( ImGui::MenuItem( "Open..", "Ctrl+O" ) )
				{
					afList.shouldPopulate = true;
					openedAfBrowser = true;
				}

				if( ImGui::MenuItem( "Save", "Ctrl+S" ) )
				{
					if( decl )
					{
						decl->Save();
					}
				}

				// When the editor is closed. it should also set g_editEntities with ~= EDITOR_AF.
				if( ImGui::MenuItem( "Close", "Ctrl+W" ) )
				{
					showTool = false;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if( clickedNew )
		{
			ImGui::OpenPopup( "New Articulated Figure" );
		}

		if( ImGui::BeginPopupModal( "New Articulated Figure" ) )
		{
			if( afFiles.Num() == 0 )
			{
				idFileList* files = fileSystem->ListFiles( "af", ".af", true, true );
				for( int i = 0; i < files->GetNumFiles(); i++ )
				{
					afFiles.Append( files->GetFile( i ) );
				}
				fileSystem->FreeFileList( files );
			}

			ImGui::BeginListBox( "##afFileSelect" );
			for( int i = 0; i < afFiles.Num(); i++ )
			{
				if( ImGui::ListBox( "Files", &fileSelection, StringListItemGetter, &afFiles, afFiles.Num() ) )
				{
					fileName = afFiles[fileSelection];
				}
			}
			ImGui::EndListBox();

			ImGui::SameLine();
			ImGui::SmallButton( "New File" );

			ImGui::InputTextStr( "AF Name", &fileName );
			if( ImGui::Button( "Create" ) )
			{
				idStr afName = fileName;
				afName.StripPath();
				afName.StripFileExtension();

				if( !afName.IsEmpty() )
				{
					idDeclAF* newDecl = static_cast<idDeclAF*>( const_cast<idDecl*>( declManager->FindType( DECL_AF, afName.c_str(), false ) ) );
					if( !newDecl )
					{
						// create it
						newDecl = static_cast<idDeclAF*>( declManager->CreateNewDecl( DECL_AF, afName.c_str(), fileName.c_str() ) );
					}

					afList.names.Append( afName );
					currentAf = afList.names.Num() - 1;

					OnNewDecl( newDecl );

					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::SameLine();
			if( ImGui::Button( "Close" ) )
			{
				afFiles.Clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if( openedAfBrowser )
		{
			ImGui::OpenPopup( "Articulated Figure Browser" );
		}

		if( ImGui::BeginPopup( "Articulated Figure Browser" ) )
		{
			afList.populate();
			afList.shouldPopulate = false;
			if( afList.names.Num() > 0 )
			{
				ImGui::Combo( "Articulated Figures", &currentAf, &StringListItemGetter, &afList, afList.names.Num() );
				if( ImGui::Button( "Select" ) )
				{
					idDeclAF* newDecl = static_cast<idDeclAF*>( const_cast<idDecl*>( declManager->FindType( DECL_AF, afList.names[currentAf], false ) ) );
					if( newDecl )
					{
						OnNewDecl( newDecl );
					}
					ImGui::CloseCurrentPopup();
				}
			}
			else
			{
				ImGui::Text( "There are no articulated figures!" );
			}

			ImGui::SameLine();

			if( ImGui::Button( "Close" ) )
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if( decl )
		{
			if( ImGui::BeginTabBar( "Tab Bar" ) )
			{
				if( ImGui::BeginTabItem( "View" ) )
				{
					// the CVars must be fetched by name because they're from the Game DLL
					// TODO: alternatively, the cvars could be fetched in Init() and kept around
					CVarCheckBox( "Show Bodies", "af_showBodies" );
					CVarCheckBox( "Show Body Names", "af_showBodyNames" );
					CVarCheckBox( "Show Constraints", "af_showConstraints" );
					CVarCheckBox( "Show Constraint Names", "af_showConstraintNames" );
					CVarCheckBox( "Show Limits", "af_showLimits" );
					CVarCheckBox( "Show Active ", "af_showActive" );
					CVarCheckBox( "Show Trees", "af_showTrees" );
					CVarCheckBox( "Show Mass", "af_showMass" );
					ImGui::EndTabItem();
				}

				if( ImGui::BeginTabItem( "Properties" ) )
				{
					if( propertyEditor )
					{
						changedAf = changedAf || propertyEditor->Do();
					}
					ImGui::EndTabItem();
				}

				if( ImGui::BeginTabItem( "Bodies##Tab" ) )
				{
					if( decl->bodies.Num() > 0 )
					{
						ImGui::Combo( "Body", &currentBodySelection, BodyItemGetter, decl, decl->bodies.Num() );
					}

					ImGui::SameLine();

					static char bodyName[256] = { '\0' };

					if( ImGui::Button( "New Body" ) )
					{
						ImGui::OpenPopup( "Create a New Body" );
					}
					if( ImGui::BeginPopup( "Create a New Body" ) )
					{
						ImGui::Text( "Create a new body for the articulated figure" );
						ImGui::InputText( "Body Name", &bodyName[0], 256 );
						if( ImGui::Button( "Create" ) )
						{
							// added a new body
							if( strlen( bodyName ) > 0 )
							{
								// TODO: Should do some data validation on the body name.
								decl->NewBody( bodyName );
								bodyName[0] = '\0';
								currentBodySelection = decl->bodies.Num() - 1;
								bodyEditors.Append( new AfBodyEditor( decl, decl->bodies[currentBodySelection] ) );
								ImGui::CloseCurrentPopup();
							}
						}
						ImGui::SameLine();
						if( ImGui::Button( "Cancel" ) )
						{
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					ImGui::Separator();

					if( bodyEditors.Num() > currentBodySelection )
					{
						changedAf = changedAf || bodyEditors[currentBodySelection]->Do();

						if( ImGui::Button( "Delete Body" ) )
						{
							ImGui::OpenPopup( "Delete Body##2" );
						}

						if( ImGui::BeginPopupModal( "Delete Body##2" ) )
						{
							ImGui::Text( "Are you sure you want to delete body %s?", decl->bodies[currentBodySelection]->name.c_str() );

							if( ImGui::Button( "Yes" ) )
							{
								delete bodyEditors[currentBodySelection];
								bodyEditors.RemoveIndex( currentBodySelection );
								decl->DeleteBody( decl->bodies[currentBodySelection]->name.c_str() );
								currentBodySelection -= 1;
								if( currentBodySelection < 0 )
								{
									currentBodySelection = 0;
								}
								ImGui::CloseCurrentPopup();
							}
							ImGui::SameLine();
							if( ImGui::Button( "No" ) )
							{
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}
					}

					ImGui::EndTabItem();
				}

				if( ImGui::BeginTabItem( "Constraints" ) )
				{
					if( decl->constraints.Num() > 0 )
					{
						ImGui::Combo( "Constraint", &currentConstraint, ConstraintItemGetter, decl, decl->constraints.Num() );
					}

					static char constraintName[256] = { '\0' };

					if( ImGui::Button( "New" ) )
					{
						ImGui::OpenPopup( "New Constraint" );
					}

					if( ImGui::BeginPopupModal( "New Constraint" ) )
					{
						ImGui::Text( "Create a new body for the articulated figure" );
						ImGui::InputText( "Constraint Name", &constraintName[0], 256 );

						if( ImGui::Button( "Create" ) )
						{
							// added a new body
							if( strlen( constraintName ) > 0 )
							{
								decl->NewConstraint( constraintName );
								constraintEditors.Append( new AfConstraintEditor( decl, decl->constraints[decl->constraints.Num() - 1] ) );
								currentConstraint = constraintEditors.Num() - 1;
							}
							constraintName[0] = '\0';

							ImGui::CloseCurrentPopup();
						}

						ImGui::SameLine();

						if( ImGui::Button( "Cancel" ) )
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					ImGui::SameLine();

					if( ImGui::Button( "Delete" ) )
					{
						ImGui::OpenPopup( "Delete Constraint##2" );
					}

					if( ImGui::BeginPopupModal( "Delete Constraint##2" ) )
					{
						ImGui::Text( "Are you sure you want to delete constraint %s?", decl->constraints[currentConstraint]->name.c_str() );

						if( ImGui::Button( "Yes" ) )
						{
							delete constraintEditors[currentConstraint];
							constraintEditors.RemoveIndex( currentConstraint );
							decl->DeleteConstraint( decl->constraints[currentConstraint]->name.c_str() );
							currentConstraint -= 1;
							if( currentConstraint < 0 )
							{
								currentConstraint = 0;
							}
							ImGui::CloseCurrentPopup();
						}

						ImGui::SameLine();

						if( ImGui::Button( "No" ) )
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					if( decl->constraints.Num() > currentConstraint )
					{
						changedAf = constraintEditors[currentConstraint]->Do();
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			try
			{
				if( ImGui::Button( "Spawn" ) )
				{
					if( afList.names.Num() > currentAf )
					{
						gameEdit->AF_SpawnEntity( afList.names[currentAf].c_str() );
					}
				}

				ImGui::SameLine();

				if( ImGui::Button( "Kill" ) )
				{
					cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "deleteSelected\n" );
				}

				if( changedAf )
				{
					if( afList.names.Num() > currentAf )
					{
						gameEdit->AF_UpdateEntities( afList.names[currentAf].c_str() );
					}
				}
			}
			catch( idException& exception )
			{
				common->DWarning( "AfEditor exception %s", exception.error );
			}
		}
	}
	ImGui::End();

	if( isShown && !showTool )
	{
		// TODO: do the same as when pressing cancel?
		isShown = showTool;
		impl::SetReleaseToolMouse( false );
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_AfEditor );
	}
}

void AfEditor::OnNewDecl( idDeclAF* newDecl )
{
	decl = newDecl;
	if( propertyEditor )
	{
		delete propertyEditor;
	}
	bodyEditors.DeleteContents( true );
	constraintEditors.DeleteContents( true );

	propertyEditor = new AfPropertyEditor( newDecl );

	for( int i = 0; i < decl->bodies.Num(); i++ )
	{
		bodyEditors.Append( new AfBodyEditor( decl, decl->bodies[i] ) );
	}

	for( int i = 0; i < decl->constraints.Num(); i++ )
	{
		constraintEditors.Append( new AfConstraintEditor( decl, decl->constraints[i] ) );
	}
}

AfEditor& AfEditor::Instance()
{
	static AfEditor instance;
	return instance;
}

void AfEditor::Enable( const idCmdArgs& args )
{
	AfEditor::Instance().ShowIt( true );
}

void AfEditor::AfList::populate()
{
	if( !shouldPopulate )
	{
		return;
	}

	names.Clear();

	int count = declManager->GetNumDecls( DECL_AF );
	for( int i = 0; i < count; i++ )
	{
		names.Append( static_cast<const idDeclAF*>( declManager->DeclByIndex( DECL_AF, i, false ) )->GetName() );
	}
}

bool BodyItemGetter( void* data, int index, const char** itemName )
{
	idDeclAF* decl = reinterpret_cast<idDeclAF*>( data );
	if( !decl )
	{
		return false;
	}

	if( index > decl->bodies.Num() )
	{
		return false;
	}

	idDeclAF_Body* body = decl->bodies[index];

	*itemName = body->name.c_str();

	return true;
}

bool ConstraintItemGetter( void* data, int index, const char** items )
{
	idDeclAF* decl = reinterpret_cast<idDeclAF*>( data );
	if( !decl )
	{
		return false;
	}

	if( index > decl->constraints.Num() )
	{
		return false;
	}

	idDeclAF_Constraint* constraint = decl->constraints[index];

	*items = constraint->name.c_str();

	return true;
}

static bool CVarCheckBox( const char* label, const char* cvarName )
{
	idCVar* cvar = cvarSystem->Find( cvarName );
	if( cvar != NULL )
	{
		bool value = cvar->GetBool();
		if( ImGui::Checkbox( label, &value ) )
		{
			cvar->SetBool( value );
			return true;
		}
	}
	return false;
}

}
