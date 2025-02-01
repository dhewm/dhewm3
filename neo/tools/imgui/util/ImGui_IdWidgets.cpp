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
