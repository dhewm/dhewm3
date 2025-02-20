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

#include "sys/sys_imgui.h"
#include "sys/platform.h"

#include "idlib/math/Vector.h"

#include "idlib/Heap.h"
#include "idlib/containers/StrList.h"
#include "ImGui_IdWidgets.h"

#include "framework/FileSystem.h"

#include "renderer/Material.h"

static const char* bodyContentsNames[5] =
{
	"solid",
	"body",
	"corpse",
	"playerclip",
	"monsterclip"
};

static int contentMappingFlags[5] =
{
	CONTENTS_SOLID,
	CONTENTS_BODY,
	CONTENTS_CORPSE,
	CONTENTS_PLAYERCLIP,
	CONTENTS_MONSTERCLIP
};

namespace ImGuiTools
{

MultiSelectWidget::MultiSelectWidget( const char** aNames, int* contentMapping, int aNumEntries )
	: names( aNames )
	, contentMapping( contentMapping )
	, numEntries( aNumEntries )
	, selectables( nullptr )
{
	selectables = ( bool* )Mem_Alloc( numEntries * sizeof( bool ) );
	memset( selectables, 0, numEntries * sizeof( bool ) );
}

MultiSelectWidget::~MultiSelectWidget()
{
	Mem_Free( selectables );
}

void MultiSelectWidget::Update( int index, bool value )
{
	assert( index < numEntries );
	selectables[index] = value;
}

void MultiSelectWidget::UpdateWithBitFlags( int bitFlags )
{
	for( int i = 0; i < numEntries; i++ )
	{
		Update( i, bitFlags & contentMapping[i] );
	}
}

bool DoMultiSelect( MultiSelectWidget* widget, int* contents )
{
	bool pressed = false;
	for( int i = 0; i < 5; i++ )
	{
		if( ImGui::Selectable( widget->names[i], &widget->selectables[i] ) )
		{
			pressed = true;
			if( widget->selectables[i] )
			{
				*contents |= widget->contentMapping[i];
			}
			else
			{
				*contents &= ~widget->contentMapping[i];
			}
		}
	}

	return pressed;
}

void HelpMarker( const char* desc )
{
	ImGui::TextDisabled( "(?)" );
	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
		ImGui::TextUnformatted( desc );
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool StringListItemGetter( void* data, int index, const char** outText )
{
	idStrList* list = reinterpret_cast<idStrList*>( data );
	assert( index < list->Num() );

	*outText = ( *list )[index];
	return true;
}

MultiSelectWidget MakePhysicsContentsSelector()
{
	return MultiSelectWidget( bodyContentsNames, contentMappingFlags, 5 );
}

ColorPicker::ColorPicker( const char *_label ) {
	label = _label;
	color.Set( 0, 0, 0, 1.0f );
}

bool ColorPicker::Button( const idVec4 &_color ) {
	ImVec4 col = ImVec4( _color.x, _color.y, _color.z, _color.w );

	if ( ImGui::ColorButton( label, col ) ) {
		oldColor = _color;
		ImGui::OpenPopup( label );
		return true;
	}

	return false;
}

bool ColorPicker::Draw() {
	idStr realLabel;
	bool isAccepted = false;

	if ( ImGui::BeginPopupModal( label, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		realLabel = label;
		realLabel += "Picker";

		bool changed = ImGui::ColorPicker4( realLabel.c_str(), color.ToFloatPtr(), ImGuiColorEditFlags_AlphaBar, oldColor.ToFloatPtr());

		if ( ImGui::Button( "OK" ) ) {
			isAccepted = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			isAccepted = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return isAccepted;
}

DeclNewSelect::DeclNewSelect( declType_t _declType, const char *_directory, const char *_extension, const char *_label )
	: declType(_declType)
	, directory(_directory)
	, extension(_extension)
	, label(_label)
	, fileSelection(-1)
	, files()
	, fileName( "" )
	, name( "" )
	, errorText( "" )
	, dp( NULL )
	, state(DONE)
{
}

void DeclNewSelect::Start() {
	files.Clear();

	idFileList* names = fileSystem->ListFiles( directory, extension, true, true );
	for( int i = 0; i < names->GetNumFiles(); i++ )
	{
		idStr file = names->GetFile( i );

		file.StripPath();
		file.StripFileExtension();

		files.Append( file );
	}
	fileSystem->FreeFileList( names );

	fileSelection = -1;
	fileName.Clear();
	name.Clear();
	errorText.Clear();
	dp = NULL;
	state = NAME;

	ImGui::OpenPopup( label );
}

bool DeclNewSelect::Draw() {
	if ( state == DONE ) {
		return false;
	}

	bool accepted = false;
	bool canceled = false;

	if ( ImGui::BeginPopupModal( label, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "%s", errorText.c_str() );

		if ( ImGui::InputTextStr( "File Name", &fileName ) ) {
			// nop
		}

		if ( ImGui::BeginListBox( "Files##prtFileSelect" ) ) {
			for( int i = 0; i < files.Num(); i++ )
			{
				if ( fileName.Length() && files[i].Find( fileName.c_str(), false ) == -1 ) {
					continue;
				}

				bool selected = ( i == fileSelection );

				ImGui::PushID( i );
				if ( ImGui::Selectable( files[i].c_str(), selected ) ) {
					fileSelection = i;
					fileName = files[fileSelection];
				}
				if ( selected ) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
				
			ImGui::EndListBox();
		}

		if ( ImGui::InputTextStr( "Name", &name ) ) {
			// nop
		}

		if ( ImGui::Button( "OK" ) ) {
			errorText.Clear();

			if ( name.IsEmpty() ) {
				errorText += "Please enter a name\n";
				accepted = false;
			}

			idDecl *newDecl = const_cast<idDecl*>( declManager->FindType( declType, name.c_str(), false ) );
			if( newDecl ) {
				errorText += idStr::Format( "Decl %s already exists in %s. Please select a different name\n", name.c_str(), newDecl->GetFileName() );
				accepted = false;
			}

			if ( errorText.IsEmpty() ) {
				idStr fullName;

				fullName = directory;
				fullName += fileName;
				fullName += extension;

				// create it
				dp = declManager->CreateNewDecl( declType, name.c_str(), fullName.c_str() );
				state = DONE;

				accepted = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			accepted = false;
			state = DONE;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return accepted;
}

DeclSelect::DeclSelect( declType_t _declType, const char *_label )
	: declType(_declType)
	, label(_label)
	, listSel(-1)
	, list()
	, name( "" )
	, errorText( "" )
	, dp( NULL )
	, state(DONE)
{
}

void DeclSelect::Start( const char *_name ) {
	list.Clear();
	for ( int i = 0; i < declManager->GetNumDecls( declType ); i++ ) {
		const idDecl *idp = declManager->DeclByIndex( declType, i, false );
		list.Append( idp->GetName() );
	}
	if ( _name ) {
		name = _name;
		listSel = list.FindIndex( name );
	} else {
		name.Clear();
		listSel = -1;
	}
	
	errorText.Clear();
	dp = NULL;
	state = NAME;

	ImGui::OpenPopup( label );
}

bool DeclSelect::Draw() {
	if ( state == DONE ) {
		return false;
	}

	bool accepted = false;
	bool canceled = false;

	if ( ImGui::BeginPopupModal( label, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		ImGui::TextColored( ImVec4( 1, 0, 0, 1 ), "%s", errorText.c_str() );

		if ( ImGui::InputTextStr( "Name", &name ) ) {
			// nop
		}

		if ( ImGui::BeginListBox( "Decls##prtSystemSelect" ) ) {
			for( int i = 0; i < list.Num(); i++ )
			{
				if ( name.Length() && list[i].Find( name.c_str(), false ) == -1 ) {
					continue;
				}

				bool selected = ( i == listSel );

				ImGui::PushID( i );
				if ( ImGui::Selectable( list[i].c_str(), selected ) ) {
					listSel = i;
					name = list[listSel];
				}
				if ( selected ) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
				
			ImGui::EndListBox();
		}

		if ( ImGui::Button( "OK" ) ) {
			errorText.Clear();

			if ( name.IsEmpty() ) {
				errorText += "Please enter a name or select a decl from the list\n";
				accepted = false;
			}

			idDecl *decl = const_cast<idDecl*>( declManager->FindType( declType, name.c_str(), false ) );
			if( !decl ) {
				errorText += idStr::Format( "Decl %s does not exist. Please select a different decl\n", name.c_str() );
				accepted = false;
			}

			if ( errorText.IsEmpty() ) {
				dp = decl;
				state = DONE;

				accepted = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			accepted = false;
			state = DONE;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return accepted;
}

GoToLineDialog::GoToLineDialog()
	: numberEdit(0)
	, firstLine(0)
	, lastLine(0)
	, waiting(false)
	, valid(false)
	, focus(false)
	, caption()
{
}

void GoToLineDialog::Start( int _firstLine, int _lastLine, int _line ) {
	firstLine = _firstLine;
	lastLine = _lastLine;
	numberEdit = _line;
	valid = ( idMath::ClampInt( firstLine, lastLine, numberEdit ) == numberEdit );
	waiting = true;
	focus = true;
	caption = va( "Line number (%d - %d)", firstLine, lastLine );
}

bool GoToLineDialog::Draw( const ImVec2 &pos, const ImVec2 &size ) {
	bool accepted = false;
	
	if ( !waiting ) {
		return accepted;
	}

	ImGuiStyle& style = ImGui::GetStyle();
	float fieldWidth = 250.0f;

	float captionWidth = ImGui::CalcTextSize( caption.c_str() ).x;

	float windowHeight =
		style.ChildBorderSize * 2.0f +
		style.WindowPadding.y * 2.0f +
		ImGui::GetFrameHeight() * 2.0f +
		style.ItemSpacing.y;

	float windowWidth =
		style.ChildBorderSize * 2.0f +
		style.WindowPadding.x * 2.0f +
		fieldWidth + style.ItemSpacing.x +
		captionWidth;

	ImVec2 oldCursorPos = ImGui::GetCursorPos();

	// TODO: this seems off, the dialog should be centered
	ImGui::SetCursorPos(ImVec2(
		pos.x + (size.x - windowWidth)*0.5f,
		pos.y + (size.y - windowHeight)*0.5f));

	if ( ImGui::BeginChild( "Go To Line", ImVec2( windowWidth, windowHeight ), ImGuiChildFlags_Borders ) ) {
		ImGui::SetNextItemWidth( fieldWidth );
		if ( ImGui::InputInt( caption.c_str(), &numberEdit, 0, 0 ) ) {
			valid = ( idMath::ClampInt( firstLine, lastLine, numberEdit ) == numberEdit );
		}
		if ( focus ) {
			ImGui::SetKeyboardFocusHere( -1 );
			focus = false;
		}

		ImGui::BeginDisabled( !valid );
		if ( ImGui::Button( "OK" ) ) {
			waiting = false;
			accepted = true;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if ( ImGui::Button( "Cancel" ) ) {
			waiting = false;
			accepted = false;
		}
	}
	ImGui::EndChild();
	ImGui::SetCursorPos( oldCursorPos );

	return accepted;
}

FindReplaceDialog::FindReplaceDialog()
	: replace()
	, find()
	, matchCase(false)
	, matchWhole(false)
	, replacement(false)
	, valid(false)
	, visible(false)
	, focus(false)
{
}

void FindReplaceDialog::Start( idStr &selection, bool _replacement ) {
	if ( selection.Length() ) {
		find = selection;
	}
	replace.Clear();
	replacement = _replacement;
	valid = ( find.Length() > 0 );
	visible = true;
	focus = true;
}

FindReplaceDialog::command_t FindReplaceDialog::Draw( const ImVec2 &pos, const ImVec2 &size ) {
	command_t command = command_t::NONE;

	if ( !visible ) {
		return command;
	}

	ImGuiStyle &style = ImGui::GetStyle();
	float fieldWidth = 250.0f;

	float replaceWidth = ImGui::CalcTextSize(" Next ").x + style.FramePadding.x * 2.0f;
	float replaceAllWidth = ImGui::CalcTextSize(" All ").x + style.FramePadding.x * 2.0f;
	float optionWidth = ImGui::CalcTextSize("Aa").x + style.FramePadding.x * 2.0f;

	float windowHeight =
		style.ChildBorderSize * 3.0f +
		style.WindowPadding.y * 3.0f +
		ImGui::GetFrameHeight() * 3.0f +
		style.ItemSpacing.y;

	float windowWidth =
		style.ChildBorderSize * 2.0f +
		style.WindowPadding.x * 2.0f +
		fieldWidth + style.ItemSpacing.x +
		replaceWidth + style.ItemSpacing.x +
		replaceAllWidth + style.ItemSpacing.x;

	ImVec2 oldCursorPos = ImGui::GetCursorPos();

	ImGui::SetCursorPos(ImVec2(
		pos.x + size.x - windowWidth - style.ScrollbarSize - style.ItemSpacing.x,
		pos.y + style.ItemSpacing.y * 2.0f));

	if ( ImGui::BeginChild( "Find/Replace", ImVec2( windowWidth, windowHeight ), ImGuiChildFlags_Borders ) ) {

		ImGui::SetNextItemWidth( fieldWidth );

		if ( ImGui::InputTextStr( "###Find", &find ) ) {
			valid = ( find.Length() > 0 );
		}
		if ( focus ) {
			ImGui::SetKeyboardFocusHere( -1 );
			focus = false;
		}
		ImGui::SetItemTooltip( "Search term" );
		ImGui::SameLine();

		ImGui::BeginDisabled( !valid );
		if ( ImGui::ArrowButton( "Next", ImGuiDir_Down ) ) {
			command = command_t::FIND_NEXT;
		}
		ImGui::SetItemTooltip( "Find next occurrence" );
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "Prev", ImGuiDir_Up ) ) {
			command = command_t::FIND_PREV;
		}
		ImGui::SetItemTooltip( "Find previous occurrence" );
		ImGui::EndDisabled();

		ImGui::SameLine();

		if ( ImGui::ToggleButton( "R", &replacement, ImVec2( optionWidth, 0.0f ) ) ) {

		}
		ImGui::SetItemTooltip( "Toggle to switch between find and replace modes" );

		ImGui::SameLine();

		if ( ImGui::Button( "x", ImVec2( optionWidth, 0.0f ) ) ) {
			visible = false;
			command = DONE;
		}

		ImGui::SetNextItemWidth( fieldWidth );
		ImGui::BeginDisabled( !replacement );
		if ( ImGui::InputTextStr( "###Replace with", &replace ) ) {
		}
		ImGui::SetItemTooltip( "Replacement term" );
		ImGui::SameLine();
		if ( ImGui::Button( "Next###ReplaceNext" ) ) {
			command = command_t::REPLACE_NEXT;
		}
		ImGui::SetItemTooltip( "Replace Next" );
		ImGui::SameLine();
		if ( ImGui::Button( "All" ) ) {
			command = command_t::REPLACE_ALL;
		}
		ImGui::SetItemTooltip("Replace All");
		ImGui::EndDisabled();

		if ( ImGui::ToggleButton( "Aa", &matchCase, ImVec2( optionWidth, 0.0f ) ) ) {
		}
		ImGui::SetItemTooltip( "Match case" );

		ImGui::SameLine();

		if ( ImGui::ToggleButton( "[]", &matchWhole, ImVec2( optionWidth, 0.0f ) ) ) {
		}
		ImGui::SetItemTooltip( "Match whole word" );
	}
	ImGui::EndChild();

	ImGui::SetCursorPos( oldCursorPos );

	return command;
}

MessageBoxDialog::MessageBoxDialog()
	: message()
	, choice(false)
	, error(false)
	, visible(false)
	, acked(false)
	, focus(false)
{
}

void MessageBoxDialog::Start( const char *_message, bool _choice, bool _error ) {
	message = _message;
	choice = _choice;
	error = _error;
	visible = true;
	acked = false;
	focus = true;
}

bool MessageBoxDialog::Draw( const ImVec2 &pos, const ImVec2 &size ) {
	if ( !visible ) {
		return false;
	}

	ImGuiStyle &style = ImGui::GetStyle();

	ImVec2 textSize = ImGui::CalcTextSize( message.c_str() );

	float windowHeight =
		style.ChildBorderSize * 2.0f +
		style.WindowPadding.y * 2.0f +
		ImGui::GetFrameHeight() * 2.0f +
		textSize.y;

	float windowWidth =
		style.ChildBorderSize * 2.0f +
		style.WindowPadding.x * 2.0f +
		textSize.x;

	ImVec2 oldCursorPos = ImGui::GetCursorPos();

	bool interacted = false;

	ImGui::SetCursorPos(ImVec2(
		pos.x + size.x * 0.5f - windowWidth * 0.5f,
		pos.y + size.y * 0.5f - windowHeight * 0.5f));

	if ( ImGui::BeginChild( "Message", ImVec2( windowWidth, windowHeight ), ImGuiChildFlags_Borders ) ) {
		if ( error ) {
			ImGui::TextColored( ImVec4(1, 0, 0, 1), "%s", message.c_str() );
		} else {
			ImGui::TextUnformatted( message.c_str() );
		}
		
		if ( focus ) {
			ImGui::SetKeyboardFocusHere( -1 );
			focus = false;
		}

		if ( choice ) {
			if ( ImGui::Button( "Yes" ) ) {
				acked = true;
				interacted = true;
				visible = false;
			}
			ImGui::SameLine();
			if ( ImGui::Button( "No" ) ) {
				acked = false;
				interacted = true;
				visible = false;
			}
		} else {
			if ( ImGui::Button( "OK" ) ) {
				visible = false;
				interacted = true;
				acked = true;
			}
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorPos( oldCursorPos );

	return interacted;
}


} //namespace ImGuiTools



// our custom ImGui functions

// like DragFloat3(), but with "X: ", "Y: " or "Z: " prepended to each display_format, for vectors
// if !ignoreLabelWidth, it makes sure the label also fits into the current item width.
//    note that this screws up alignment with consecutive "value+label widgets" (like Drag* or ColorEdit*)
bool ImGui::DragVec3( const char* label, idVec3& v, float v_speed,
					  float v_min, float v_max, const char* display_format, float power, bool ignoreLabelWidth )
{
	bool value_changed = false;
	ImGui::BeginGroup();
	ImGui::PushID( label );

	ImGuiStyle& style = ImGui::GetStyle();
	float wholeWidth = ImGui::CalcItemWidth() - 2.0f * style.ItemSpacing.x;
	float spacing = style.ItemInnerSpacing.x;
	float labelWidth = ignoreLabelWidth ? 0.0f : ( ImGui::CalcTextSize( label, NULL, true ).x + spacing );
	float coordWidth = ( wholeWidth - labelWidth - 2.0f * spacing ) * ( 1.0f / 3.0f ); // width of one x/y/z dragfloat

	ImGui::PushItemWidth( coordWidth );
	for( int i = 0; i < 3; i++ )
	{
		ImGui::PushID( i );
		char format[64];
		idStr::snPrintf( format, sizeof( format ), "%c: %s", "XYZ"[i], display_format );
		value_changed |= ImGui::DragFloat( "##v", &v[i], v_speed, v_min, v_max, format, power );

		ImGui::PopID();
		ImGui::SameLine( 0.0f, spacing );
	}
	ImGui::PopItemWidth();
	ImGui::PopID();

	const char* labelEnd = strstr( label, "##" );
	ImGui::TextUnformatted( label, labelEnd );

	ImGui::EndGroup();

	return value_changed;
}

// shortcut for DragXYZ with ignorLabelWidth = false
// very similar, but adjusts width to width of label to make sure it's not cut off
// sometimes useful, but might not align with consecutive "value+label widgets" (like Drag* or ColorEdit*)
bool ImGui::DragVec3fitLabel( const char* label, idVec3& v, float v_speed,
							  float v_min, float v_max, const char* display_format, float power )
{
	return ImGui::DragVec3( label, v, v_speed, v_min, v_max, display_format, power, false );
}

struct InputTextCallback_UserData
{
	idStr *					Str;
	ImGuiInputTextCallback	ChainCallback;
	void *					ChainCallbackUserData;
};

static int InputTextCallback( ImGuiInputTextCallbackData *data )
{
	InputTextCallback_UserData *user_data = ( InputTextCallback_UserData * )data->UserData;

	if ( data->EventFlag == ImGuiInputTextFlags_CallbackResize )
	{
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
		idStr *str = user_data->Str;
		IM_ASSERT( data->Buf == str->c_str() );
		int length = data->BufTextLen > 0 ? data->BufTextLen : 1;
		str->ReAllocate( length, true );
		data->Buf = ( char * )str->c_str();
	}
	else if ( user_data->ChainCallback )
	{
		// Forward to user callback, if any
		data->UserData = user_data->ChainCallbackUserData;
		return user_data->ChainCallback( data );
	}
	return 0;
}

bool ImGui::InputTextStr( const char *label, idStr *str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data ) {
	IM_ASSERT( ( flags & ImGuiInputTextFlags_CallbackResize ) == 0 );
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;

	bool result = ImGui::InputText( label, ( char * )str->c_str(), str->Length() + 1, flags, InputTextCallback, &cb_user_data );

	// fix the length (characters get appended directly into the buffer allocated by idStr)
	idStr tmp = str->c_str();
	*str = tmp;

	return result;
}

bool ImGui::InputTextMultilineStr( const char *label, idStr *str, const ImVec2 &size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data ) {
	IM_ASSERT( ( flags & ImGuiInputTextFlags_CallbackResize ) == 0 );
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;

	bool result = ImGui::InputTextMultiline( label, ( char * )str->c_str(), str->Length() + 1, size, flags, InputTextCallback, &cb_user_data );

	// fix the length (characters get appended directly into the buffer allocated by idStr)
	idStr tmp = str->c_str();
	*str = tmp;

	return result;
}

bool ImGui::InputDialogName( const char *text, const char *label, idStr *str ) {
	bool accepted = false;

	if ( ImGui::BeginPopupModal( label, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		ImGui::TextUnformatted( text );
		if ( ImGui::InputTextStr( "Name", str ) ) {
			// nop
		}

		if ( ImGui::Button( "OK" ) ) {
			accepted = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			accepted = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return accepted;
}

bool ImGui::InputMessageBox( const char *text, const char* label, bool allowCancel ) {
	bool accepted = false;

	ImGui::OpenPopup( label );

	if ( ImGui::BeginPopupModal( label, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
		ImGui::TextUnformatted( text );

		if ( ImGui::Button( "OK" ) ) {
			accepted = true;
			ImGui::CloseCurrentPopup();
		}
		if ( allowCancel ) {
			ImGui::SameLine();
			if ( ImGui::Button( "Cancel" ) ) {
				accepted = false;
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}

	return accepted;
}

// NOTE: this code is adapted from https://github.com/goossens/ObjectTalk/blob/62977f72389a2bbdde4d2535faadad46ab2920a1/gfx/framework/OtUi.cpp#L202
bool ImGui::ToggleButton( const char *label, bool *value, const ImVec2 &size ) {
	bool		changed = false;
	ImVec4		*colors = ImGui::GetStyle().Colors;

	if ( *value ) {
		ImGui::PushStyleColor( ImGuiCol_Button, colors[ImGuiCol_ButtonActive] );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, colors[ImGuiCol_ButtonActive] );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, colors[ImGuiCol_TableBorderLight] );
	} else {
		ImGui::PushStyleColor( ImGuiCol_Button, colors[ImGuiCol_TableBorderLight] );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, colors[ImGuiCol_TableBorderLight] );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, colors[ImGuiCol_ButtonActive] );
	}

	ImGui::Button( label, size );

	if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
		*value = !*value;
		changed = true;
	}

	ImGui::PopStyleColor( 3 );

	return changed;
}
