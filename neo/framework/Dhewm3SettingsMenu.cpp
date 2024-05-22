
#define IMGUI_DEFINE_MATH_OPERATORS

#include "Common.h"
#include "CVarSystem.h"

#include "idlib/LangDict.h"

#include "KeyInput.h"
#include "UsercmdGen.h" // key bindings
//#include "Game.h" // idGameEdit to access entity definitions (player, weapons)
#include "DeclEntityDef.h"

#include "sys/sys_imgui.h"
#include "../libs/imgui/imgui_internal.h"

#ifndef IMGUI_DISABLE

extern const char* D3_GetGamepadStartButtonName();

namespace {

const char* GetLocalizedString( const char* id, const char* fallback )
{
	if ( id == nullptr || id[0] == '\0' ) {
		return fallback;
	}
	const char* ret = common->GetLanguageDict()->GetString( id );
	if ( ret == nullptr || ret[0] == '\0'
	    || ( ret[0] == '#' && idStr::Cmpn( ret, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 ) ) {
		ret = fallback;
	}
	return ret;
}

// TODO: move the following two functions into sys_imgui.cpp ?
static void AddTooltip( const char* text )
{
	if ( ImGui::BeginItemTooltip() )
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted( text );
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

static void AddDescrTooltip( const char* description )
{
	if ( description != nullptr ) {
		ImGui::SameLine();
		ImGui::TextDisabled( "(?)" );
		AddTooltip( description );
	}
}



/*************************
 *                       *
 *    Keybinding Menu    *
 *                       *
 *************************/

struct BindingEntry;
static BindingEntry* FindBindingEntryForKey( int keyNum );
static int numBindingColumns = 4; // TODO: in Doom3, save in CVar (in_maxBindingsPerCommand or sth)

static int rebindKeyNum = -1; // only used for HandleRebindPopup()
static BindingEntry* rebindOtherEntry = nullptr; // ditto

static bool IsKeyPressed( ImGuiKey key ) {
	return ImGui::IsKeyPressed( key, false );
}

// is Enter (or Keypad Enter) or gamepad A (or equivalent on non-xinput-devices) pressed?
// used for confirmation in popup dialogs, and to initiate key binding in the binding tables
static bool IsConfirmKeyPressed() {
	return IsKeyPressed( ImGuiKey_Enter ) || IsKeyPressed( ImGuiKey_KeypadEnter )
	       || IsKeyPressed( ImGuiKey_GamepadFaceDown );
}

// is Delete, Backspace or gamepad Y (or equivalent on non-xinput-devices) pressed?
static bool IsClearKeyPressed() {
	return IsKeyPressed( ImGuiKey_Delete ) || IsKeyPressed( ImGuiKey_Backspace )
	       || IsKeyPressed( ImGuiKey_GamepadFaceUp );
}

// is Escape or gamepad Start or gamepad B (or equivalents on non-xinput-devices) pressed?
static bool IsCancelKeyPressed() {
	// using Escape, gamepad Start and gamepad B for cancel, except in the
	// binding case, there only Esc and Start work (so gamepad B can be bound),
	// but the binding popup doesn't use this function anyway

	// Note: In Doom3, Escape opens/closes the main menu, so in dhewm3 the gamepad Start button
	//       behaves the same, incl. the specialty that it can't be bound by the user
	return IsKeyPressed( ImGuiKey_Escape ) || IsKeyPressed( ImGuiKey_GamepadFaceRight )
	       || IsKeyPressed( ImGuiKey_GamepadStart );
}

static const char* GetGamepadStartName() {
	return D3_GetGamepadStartButtonName();
}

static const char* GetGamepadCancelButtonNames() {
	static char ret[64];
	// on xbox: "B or Start"
	D3_snprintfC99( ret, sizeof(ret), "%s or %s", Sys_GetLocalizedJoyKeyName( K_JOY_BTN_EAST ), GetGamepadStartName() );
	return ret;
}

static const char* GetGamepadBindNowButtonName() { // TODO: rename to confirm or sth
	return Sys_GetLocalizedJoyKeyName( K_JOY_BTN_SOUTH ); // xbox A
}

static const char* GetGamepadUnbindButtonName() {
	return Sys_GetLocalizedJoyKeyName( K_JOY_BTN_NORTH ); // xbox Y
}


const char* GetKeyName( int keyNum, bool localized = true )
{
	if( keyNum <= 0 )
		return "<none>";

	if ( keyNum >= 'a' && keyNum <= 'z' ) {
		static char oneChar[2] = {};
		oneChar[0] = keyNum - 32; // to uppercase
		return oneChar;
	}
	// handle scancodes separately, because ImGui uses UTF-8, while dhewm3 uses ISO8859-1
	if ( keyNum >= K_FIRST_SCANCODE && keyNum <= K_LAST_SCANCODE ) {
		const char* scName = NULL;
		if ( localized ) {
			scName = Sys_GetLocalizedScancodeNameUTF8( keyNum );
		} else {
			scName = Sys_GetScancodeName( keyNum );
		}
		if ( scName != NULL ) {
			return scName;
		}
	}

	return idKeyInput::KeyNumToString( keyNum, localized );
}

// background color for the first column of the binding table, that contains the name of the command
static ImU32 displayNameBGColor = 0;

static const ImVec4 RedButtonColor(1.00f, 0.17f, 0.17f, 0.58f);
static const ImVec4 RedButtonHoveredColor(1.00f, 0.17f, 0.17f, 1.00f);
static const ImVec4 RedButtonActiveColor(1.00f, 0.37f, 0.37f, 1.00f);

static float CalcDialogButtonWidth()
{
	// with the standard font, 120px wide Ok/Cancel buttons look good,
	// this text (+default padding) has that width there
	float testTextWidth = ImGui::CalcTextSize( "Ok or Cancel ???" ).x;
	float framePadWidth = ImGui::GetStyle().FramePadding.x;
	return testTextWidth + 2.0f * framePadWidth;
}


enum BindingEntrySelectionState {
	BESS_NotSelected = 0,
	BESS_Selected,
	BESS_WantBind,
	BESS_WantClear,
	BESS_WantRebind // we were in WantBind, but the key is already bound to another command, so show a confirmation popup
};

struct BoundKey {
	int keyNum = -1;
	idStr keyName;
	idStr internalKeyName; // the one used in bind commands in the D3 console and config

	void Set( int _keyNum )
	{
		keyNum = _keyNum;
		keyName = GetKeyName( _keyNum, true );
		internalKeyName = GetKeyName( _keyNum, false );
	}

	void Clear()
	{
		keyNum = -1;
		keyName = "";
		internalKeyName = "";
	}

	BoundKey() = default;

	BoundKey ( int _keyNum ) {
		Set( _keyNum );
	}

};

struct BindingEntryTemplate {
	const char* command;
	const char* name;
	const char* nameLocStr;
	const char* description;
};

struct BindingEntry {
	idStr command; // "_impulse3" or "_forward" or similar - or "" for heading entry
	idStr displayName;
	idList<BoundKey> bindings;

	const char* description = nullptr;

	enum {
		BIND_NONE   = -1, // no binding currently selected
		// all are selected (clicked command name column): clear all,
		//   or add a new binding in some visible column (idx in bindings < numBindingColumns)
		BIND_ALL    = -2,
		// append new binding at the end, or set it in unused bindings entry, if any (used by AllBindingsWindow)
		BIND_APPEND = -3
	};
	// which binding is currently selected in the UI, if any (and only if this
	//   binding entry is currently active according to Draw()'s oldSelState)
	int selectedBinding = BIND_NONE; // index in bindings or one of the enum values

	BindingEntry() = default;

	BindingEntry( const char* _displayName ) : displayName(_displayName) {}

	BindingEntry( const char* _command, const char* _displayName, const char* descr = nullptr )
	: command( _command ), displayName( _displayName ), description( descr ) {}

	BindingEntry( const idStr& _command, const idStr& _displayName, const char* descr = nullptr )
		: command( _command ), displayName( _displayName ), description( descr ) {}

	BindingEntry( const idStr& _command, const char* _displayName, const char* descr = nullptr )
		: command( _command ), displayName( _displayName ), description( descr ) {}

	BindingEntry( const BindingEntryTemplate& bet )
	: command( bet.command ), description( bet.description ) {
		displayName = GetLocalizedString( bet.nameLocStr, bet.name );
		displayName.StripTrailingWhitespace();
	}

	bool IsHeading() const
	{
		return command.Length() == 0;
	}

	// only removes the entry from bindings, does *not* unbind!
	void RemoveBindingEntry( unsigned idx )
	{
		if ( idx < bindings.Num() ) {
			bindings.RemoveIndex( idx );
		}
	}

	// remove all entries from bindings that don't have a key set
	void CompactBindings()
	{
		for ( int i = bindings.Num() - 1; i >= 0; --i ) {
			if ( bindings[i].keyNum == -1 ) {
				RemoveBindingEntry( i );
			}
		}
	}

	// also updates this->selectedColumn
	void UpdateSelectionState( int bindIdx, /* in+out */ BindingEntrySelectionState& selState )
	{
		// if currently a popup is shown for creating a new binding or clearing one (BESS_WantBind
		// or BESS_WantClear), everything is still rendered, but in a disabled (greyed out) state
		// and shouldn't handle any input => then there's not much to do here,
		//  except for highlighting at the end of the function
		if ( selState < BESS_WantBind ) {
			if ( ImGui::IsItemFocused() ) {
				// Note: even when using the mouse, clicking a selectable will make it focused,
				//  so it's possible to select a command (or specific binding of a command)
				//  with the mouse and then press Enter to (re)bind it or Delete to clear it.
				//  So whether something is selected mostly is equivalent to it being focused.
				//  In the initial development of this code that wasn't the case, so there
				//  *might* be some small inconsistencies due to that; but also intentional
				//  special cases, like a binding entry being drawn as selected while
				//  one of the popups is open to modify it
				//  (=> it doesn't have focus then because the popup has focus)
				//  That's not just cosmetical, selState and selectedBinding are
				//  used to configure the popup.

				selectedBinding = bindIdx;

				if ( IsConfirmKeyPressed() ) {
					selState = BESS_WantBind;
				} else if ( IsClearKeyPressed() ) {
					bool nothingToClear = false;
					if ( bindIdx == BIND_ALL ) {
						if ( bindings.Num() == 0 ) {
							D3::ImGuiHooks::ShowWarningOverlay( "No keys are bound to this command, so there's nothing to unbind" );
							nothingToClear = true;
						}
					} else if ( bindIdx < 0 || bindIdx >= bindings.Num() || bindings[bindIdx].keyNum == -1 ) {
						D3::ImGuiHooks::ShowWarningOverlay( "No bound key selected for unbind" );
						nothingToClear = true;
					}

					selState = nothingToClear ? BESS_Selected : BESS_WantClear;
				} else if ( selState == BESS_NotSelected ) {
					selState = BESS_Selected;
				}
			} else if (selectedBinding == bindIdx && selState != BESS_NotSelected) {
				// apparently this was still selected last frame, but is not focused anymore => unselect it
				selState = BESS_NotSelected;
			}

			if ( ImGui::IsItemHovered() ) { // mouse cursor is on this item
				if ( bindIdx == BIND_ALL ) {
					// if the first column (command name, like "Move Left") is hovered, highlight the whole row
					// A normal Selectable would use ImGuiCol_HeaderHovered, but I use that as the "selected"
					// color (in Draw()), so use the next brighter thing (ImGuiCol_HeaderActive) here.
					ImU32 highlightRowColor = ImGui::GetColorU32( ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive) );
					ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, highlightRowColor );
				}

				if ( ImGui::IsMouseDoubleClicked( 0 ) ) {
					selState = BESS_WantBind;
					selectedBinding = bindIdx;
				}
				// Note: single-clicking an item gives it focus, so that's implictly
				//  handled above in `if ( ImGui::IsItemFocused() ) { ...`
			}
		}

		// this column is selected => highlight it
		if ( selState != BESS_NotSelected && selectedBinding == bindIdx ) {
			// ImGuiCol_Header would be the regular "selected cell/row" color that Selectable would use
			// but ImGuiCol_HeaderHovered is more visible, IMO
			ImU32 highlightRowColor = ImGui::GetColorU32( ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) );
			ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, highlightRowColor );

			if ( bindIdx == BIND_ALL ) {
				// the displayName column is selected => highlight the whole row
				ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, highlightRowColor );
				// (yes, still set the highlight color for ImGuiTableBgTarget_CellBg above for extra
				//  highlighting of the column 0 cell, otherwise it'd look darker due to displayNameBGColor)
			}
		}
	}

	bool DrawAllBindingsWindow( /* in+out */ BindingEntrySelectionState& selState, bool newOpen, const ImVec2& btnMin, const ImVec2& btnMax )
	{
		bool showThisMenu = true;
		idStr menuWinTitle = idStr::Format( "All keys bound to %s###allBindingsWindow", displayName.c_str() );
		int numBindings = bindings.Num();

		ImGuiWindowFlags menuWinFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
		const float fontSize = ImGui::GetFontSize();
		ImVec2 winMinSize = ImGui::CalcTextSize( menuWinTitle, nullptr, true );
		winMinSize.x += fontSize * 2.0f;
		const ImGuiViewport& viewPort = *ImGui::GetMainViewport();
		ImVec2 maxWinSize( viewPort.WorkSize.x, viewPort.WorkSize.y * 0.9f );
		// make sure the window is big enough to show the full title (incl. displayName)
		// and that it fits into the screen (it can scroll if it gets too long)
		ImGui::SetNextWindowSizeConstraints( winMinSize, maxWinSize );

		static ImVec2 winPos;
		if ( newOpen ) {
			// position the window right next to the [++] button that opens/closes it
			winPos = btnMin;
			winPos.x = btnMax.x + ImGui::GetStyle().ItemInnerSpacing.x;
			ImGui::OpenPopup( menuWinTitle );
			ImGui::SetNextWindowPos(winPos);
		}

		if ( ImGui::Begin( menuWinTitle, &showThisMenu, menuWinFlags ) )
		{
			ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;
			if ( numBindings > 0 && ImGui::BeginTable( "AllBindingsForCommand", 2, tableFlags ) ) {
				ImGui::TableSetupColumn("command", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("buttons", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex( 0 );

				// turn the next button (Unbind all) red
				ImGui::PushStyleColor( ImGuiCol_Button, RedButtonColor );
				ImGui::PushStyleColor( ImGuiCol_ButtonHovered, RedButtonHoveredColor );
				ImGui::PushStyleColor( ImGuiCol_ButtonActive,  RedButtonActiveColor );

				ImGui::Indent();
				if ( ImGui::Button( idStr::Format( "Unbind all" ) ) ) {
					selState = BESS_WantClear;
					selectedBinding = BIND_ALL;
				} else {
					ImGui::SetItemTooltip( "Remove all keybindings for %s", displayName.c_str() );
				}

				ImGui::Unindent();
				ImGui::PopStyleColor(3); // return to normal button color

				ImGui::TableSetColumnIndex( 1 );

				float helpHoverWidth = ImGui::CalcTextSize("(?)").x;
				float offset = ImGui::GetContentRegionAvail().x - helpHoverWidth;
				ImGui::SetCursorPosX( ImGui::GetCursorPosX() + offset );
				ImGui::AlignTextToFramePadding();
				ImGui::TextDisabled( "(?)" );
				if ( ImGui::BeginItemTooltip() ) {
					ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
					ImGui::Text( "You can close this window with Escape (or %s) or by clicking the little (x) button or by clicking the [++] button again.",
								 GetGamepadCancelButtonNames() );
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				ImGui::Spacing();

				ImU32 highlightRowColor = 0;
				if ( selectedBinding == BIND_ALL ) {
					highlightRowColor = ImGui::GetColorU32( ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) );
				}

				ImGui::Indent( fontSize * 0.5f );

				for ( int bnd = 0; bnd < numBindings; ++bnd ) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );

					ImGui::PushID( bnd ); // the buttons have the same names in every row, so push the row number as ID

					bool colHasBinding = bindings[bnd].keyNum != -1;
					const char* keyName = "";

					if ( colHasBinding ) {
						keyName = bindings[bnd].keyName.c_str();
						ImGui::AlignTextToFramePadding();
						ImGui::TextUnformatted( keyName );
						AddTooltip( bindings[bnd].internalKeyName.c_str() );
					}

					if ( selectedBinding == BIND_ALL ) {
						// if all bindings are selected from the binding table (for clear all),
						// mark the whole first row here. otherwise, nothing is marked here,
						// as in this window only the buttons are clickble, not the key cells
						ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, highlightRowColor );
					}

					ImGui::TableNextColumn();
					if ( colHasBinding ) {
						if ( ImGui::Button( "Rebind" ) ) {
							selState = BESS_WantBind;
							selectedBinding = bnd;
						} else {
							ImGui::SetItemTooltip( "Unbind '%s' and bind another key to %s", keyName, displayName.c_str() );
						}
						ImGui::SameLine();
						ImGui::SetCursorPosX( ImGui::GetCursorPosX() + fontSize*0.5f );
						if ( ImGui::Button( "Unbind" ) ) {
							selState = BESS_WantClear;
							selectedBinding = bnd;
						} else {
							ImGui::SetItemTooltip( "Unbind key '%s' from %s", keyName, displayName.c_str() );
						}
					} else {
						if ( ImGui::Button( "Bind" ) ) {
							selState = BESS_WantBind;
							selectedBinding = bnd;
						} else {
							ImGui::SetItemTooltip( "Set a keybinding for %s", displayName.c_str() );
						}
					}

					ImGui::PopID(); // bnd
				}

				ImGui::EndTable();
			}

			const char* addBindButtonLabel = (numBindings == 0) ? "Bind a key" : "Bind another key";
			float buttonWidth = ImGui::CalcTextSize(addBindButtonLabel).x + 2.0f * ImGui::GetStyle().FramePadding.x;
			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - buttonWidth);

			if ( ImGui::Button( addBindButtonLabel ) ) {
				selState = BESS_WantBind;
				selectedBinding = BIND_APPEND;
			} else {
				ImGui::SetItemTooltip( "Add %s keybinding for %s",
									   (numBindings == 0) ? "a" : "another",
									   displayName.c_str() );
			}

			ImVec2 winSize = ImGui::GetWindowSize();
			ImRect winRect;
			winRect.Min = ImGui::GetWindowPos();
			winRect.Max = winRect.Min + winSize;
			ImRect workRect(viewPort.WorkPos, viewPort.WorkPos + viewPort.WorkSize);

			if ( !workRect.Contains(winRect) ) {
				// this window is at least partly outside the visible area of the screen (or SDL window)
				// => move it around so it's completely visible, if possible
				ImRect r_avoid( btnMin, btnMax );
				r_avoid.Expand( ImGui::GetStyle().ItemInnerSpacing );

				ImGuiDir dir = ImGuiDir_Right;
				ImVec2 newWinPos = ImGui::FindBestWindowPosForPopupEx( ImVec2(btnMin.x, btnMax.y),
							winSize, &dir, workRect, r_avoid, ImGuiPopupPositionPolicy_Default );
				ImVec2 posDiff = newWinPos - winPos;
				if ( fabsf(posDiff.x) > 2.0f || fabsf(posDiff.y) > 2.0f ) {
					winPos = newWinPos;
					ImGui::SetWindowPos( newWinPos );
				}
			}

			// allow closing this window with escape
			if ( ImGui::IsWindowFocused() && IsCancelKeyPressed() ) {
				showThisMenu = false;
			}
		}
		ImGui::End();

		return showThisMenu;
	}

	BindingEntrySelectionState Draw( int bindRowNum, const BindingEntrySelectionState oldSelState )
	{
		if ( IsHeading() ) {
			ImGui::SeparatorText( displayName );
			if ( description ) {
				AddDescrTooltip( description );
			}
		} else {
			ImGui::PushID( command );

			ImGui::TableNextRow( 0, ImGui::GetFrameHeightWithSpacing() );
			ImGui::TableSetColumnIndex( 0 );
			ImGui::AlignTextToFramePadding();

			// the first column (with the display name in it) gets a different background color
			ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, displayNameBGColor );

			BindingEntrySelectionState newSelState = oldSelState;

			// not really using the selectable feature, mostly making it selectable
			// so keyboard/gamepad navigation works
			ImGui::Selectable( "##cmd", false, 0 );

			UpdateSelectionState( BIND_ALL, newSelState );

			ImGui::SameLine();
			ImGui::TextUnformatted( displayName );

			AddTooltip( command );

			if ( description ) {
				AddDescrTooltip( description );
			}

			int numBindings = bindings.Num();
			for ( int bnd=0; bnd < numBindingColumns ; ++bnd ) {
				ImGui::TableSetColumnIndex( bnd+1 );

				bool colHasBinding = (bnd < numBindings) && bindings[bnd].keyNum != -1;
				char selTxt[128];
				if ( colHasBinding ) {
					D3_snprintfC99( selTxt, sizeof(selTxt), "%s###%d", bindings[bnd].keyName.c_str(), bnd );
				} else {
					D3_snprintfC99( selTxt, sizeof(selTxt), "###%d", bnd );
				}
				ImGui::Selectable( selTxt, false, 0 );
				UpdateSelectionState( bnd, newSelState );

				if ( colHasBinding ) {
					AddTooltip( bindings[bnd].internalKeyName.c_str() );
				}
			}

			ImGui::TableSetColumnIndex( numBindingColumns + 1 );
			// the last column contains a "++" button that opens a window that lists all bindings
			// for this rows command (including ones not listed in the table because of lack of columns)
			// if there actually are more bindings than columns, the button is red, else it has the default color
			// clicking the button again will close the window, and the buttons color depends on whether
			// its window is open or not. only one such window can be open at at time, clicking the
			// button in another row closes the current window and opens a new one
			static int  showAllBindingsMenuRowNum = -1;
			bool allBindWinWasOpen = (showAllBindingsMenuRowNum == bindRowNum);
			int styleColorsToPop = 0;
			if ( numBindings <= numBindingColumns ) {
				if ( allBindWinWasOpen ) {
					// if the all bindings menu/window is showed for this entry,
					// the button is "active" => switch its normal and hovered colors
					ImVec4 btnColor = ImGui::GetStyleColorVec4( ImGuiCol_ButtonHovered );
					ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button) );
					ImGui::PushStyleColor( ImGuiCol_Button, btnColor );
					styleColorsToPop = 2;
				}
			} else { // more bindings than can be shown in the table => make ++ button red
				ImGui::PushStyleColor( ImGuiCol_Button, allBindWinWasOpen ? RedButtonHoveredColor : RedButtonColor );
				ImGui::PushStyleColor( ImGuiCol_ButtonHovered, allBindWinWasOpen ? RedButtonColor : RedButtonHoveredColor );
				ImGui::PushStyleColor( ImGuiCol_ButtonActive, RedButtonActiveColor );
				styleColorsToPop = 3;
			}

			// TODO: close the window if another row has been selected (or used to create/delete a binding or whatever)?
			bool newOpen = false;
			if ( ImGui::Button( "++" ) ) {
				showAllBindingsMenuRowNum = allBindWinWasOpen ? -1 : bindRowNum;
				newOpen = true;
				CompactBindings();
			}
			if ( ImGui::IsItemFocused() && newSelState != BESS_NotSelected ) {
				newSelState = BESS_NotSelected;
			}
			ImVec2 btnMin = ImGui::GetItemRectMin();
			ImVec2 btnMax = ImGui::GetItemRectMax();
			if ( numBindings > numBindingColumns ) {
				ImGui::SetItemTooltip( "There are additional bindings for %s.\nClick here to show all its bindings.", displayName.c_str() );
			} else {
				ImGui::SetItemTooltip( "Show all bindings for %s in a list", displayName.c_str() );
			}

			if ( styleColorsToPop > 0 ) {
				ImGui::PopStyleColor( styleColorsToPop ); // restore button colors
			}

			if ( showAllBindingsMenuRowNum == bindRowNum ) {
				ImGui::SetNextWindowBgAlpha( 1.0f );
				if ( !DrawAllBindingsWindow( newSelState, newOpen, btnMin, btnMax ) ) {
					showAllBindingsMenuRowNum = -1;
					CompactBindings();
				}
			}

			ImGui::PopID();

			if ( newSelState == BESS_NotSelected ) {
				selectedBinding = BIND_NONE;
			}
			return newSelState;
		}
		return BESS_NotSelected;
	}

	void Bind( int keyNum ) {
		if ( keyNum >= 0 ) {
			idKeyInput::SetBinding( keyNum, command );
		}
	}

	void Unbind( int keyNum ) {
		if ( keyNum >= 0 ) {
			idKeyInput::SetBinding( keyNum, "" );
		}
	}

	BindingEntrySelectionState HandleClearPopup( const char* popupName, bool newOpen )
	{
		BindingEntrySelectionState ret = BESS_WantClear;
		const int selectedBinding = this->selectedBinding;

		if ( ImGui::BeginPopupModal( popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if ( selectedBinding == BIND_ALL ) {
				ImGui::Text( "Clear all keybindings for %s ?", displayName.c_str() );
			} else {
				ImGui::Text( "Unbind key '%s' from command %s ?",
							 bindings[selectedBinding].keyName.c_str(), displayName.c_str() );
			}

			ImGui::NewLine();
			ImGui::Text( "Press Enter (or %s) to confirm, or\nEscape (or %s) to cancel.",
						 GetGamepadBindNowButtonName(), GetGamepadCancelButtonNames() );
			ImGui::NewLine();

			// center the Ok and Cancel buttons
			float dialogButtonWidth = CalcDialogButtonWidth();
			float spaceBetweenButtons = ImGui::GetFontSize();
			float buttonOffset = (ImGui::GetWindowWidth() - 2.0f*dialogButtonWidth - spaceBetweenButtons) * 0.5f;
			ImGui::SetCursorPosX( buttonOffset );

			bool confirmedByKey = false;
			if ( !newOpen && !ImGui::IsAnyItemFocused() ) {
				// if no item is focused (=> not using keyboard or gamepad navigation to select
				//  [Ok] or [Cancel] button), check if Enter has been pressed to confirm deletion
				// (otherwise, enter can be used to chose the selected button)
				// also, don't do this when just opened, because then enter might still
				// be pressed from selecting the Unbind button in the AllBindingsWindow
				confirmedByKey = IsConfirmKeyPressed();
			}
			if ( ImGui::Button( "Ok", ImVec2(dialogButtonWidth, 0) ) || confirmedByKey ) {
				if ( selectedBinding == BIND_ALL ) {
					for ( BoundKey& bk : bindings ) {
						Unbind( bk.keyNum );
					}
					bindings.SetNum( 0, false );
					// don't select all columns after they have been cleared,
					// instead only select the first, good point to add a new binding
					this->selectedBinding = 0;
				} else {
					Unbind( bindings[selectedBinding].keyNum );
					if ( selectedBinding == numBindingColumns - 1 ) {
						// when removing the binding of the last column visible
						// in the big binding table, remove that entry so
						// the next one not visible there can take its place
						RemoveBindingEntry( selectedBinding );
					} else {
						bindings[selectedBinding].Clear();
					}
				}

				ImGui::CloseCurrentPopup();
				ret = BESS_Selected;
			}
			ImGui::SetItemDefaultFocus();

			ImGui::SameLine( 0.0f, spaceBetweenButtons );
			if ( ImGui::Button( "Cancel", ImVec2(dialogButtonWidth, 0) ) || IsCancelKeyPressed() ) {
				ImGui::CloseCurrentPopup();
				ret = BESS_Selected;
			}

			ImGui::EndPopup();
		}

		return ret;
	}


	void AddKeyBinding( int keyNum )
	{
		assert( selectedBinding != -1 );
		Bind( keyNum );

		int numBindings = bindings.Num();
		if ( selectedBinding == BIND_ALL || selectedBinding == BIND_APPEND ) {
			for ( int i=0; i < numBindings; ++i ) {
				// if there's an empty column, use that
				if ( bindings[i].keyNum == -1 ) {
					bindings[i].Set( keyNum );
					// select the column this was inserted into
					selectedBinding = i;

					return;
				}
			}
			if ( numBindings < numBindingColumns || selectedBinding == BIND_APPEND ) {
				// just append an entry to bindings
				bindings.Append( BoundKey(keyNum) );
				selectedBinding = numBindings;
			} else {
				// insert in last column of table so it's visible
				// (but don't remove any elements from bindings!)
				bindings.Insert( BoundKey(keyNum), numBindingColumns-1 );
				selectedBinding = numBindingColumns-1;
			}
		} else {
			int selectedBinding = this->selectedBinding;
			assert( selectedBinding >= 0 );
			if ( selectedBinding < numBindings ) {
				Unbind( bindings[selectedBinding].keyNum );
				bindings[selectedBinding].Set( keyNum );
			} else  {
				if ( selectedBinding > numBindings ) {
					// apparently a column with other unset columns before it was selected
					// => add enough empty columns
					bindings.SetNum( selectedBinding, false );
				}
				bindings.Append( BoundKey(keyNum) );
			}
		}
	}

	void RemoveKeyBinding( int keyNum )
	{
		int delPos = -1;
		int numBindings = bindings.Num();
		for ( int i = 0; i < numBindings; ++i ) {
			if ( bindings[i].keyNum == keyNum ) {
				delPos = i;
				break;
			}
		}
		if ( delPos != -1 ) {
			Unbind( keyNum );
			RemoveBindingEntry( delPos );
		}
	}


	BindingEntrySelectionState HandleBindPopup( const char* popupName, bool newOpen )
	{
		BindingEntrySelectionState ret = BESS_WantBind;
		const int selectedBinding = this->selectedBinding;
		assert(selectedBinding == BIND_ALL || selectedBinding == BIND_APPEND || selectedBinding >= 0);

		ImGuiIO& io = ImGui::GetIO();

		if ( newOpen ) {
			D3::ImGuiHooks::SetKeyBindMode( true );

			// disable keyboard and gamepad input while the bind popup is open
			// (the mouse can still be used to click the Cancel button, and Escape
			//  and Gamepad Start will be handled specially as well to allow canceling)
			io.ConfigFlags &= ~(ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NavEnableKeyboard);
		}

		if ( ImGui::BeginPopupModal( popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if ( selectedBinding < 0 || selectedBinding >= bindings.Num()
				|| bindings[selectedBinding].keyNum == -1 ) {
				// add a binding
				ImGui::Text( "Press a key or button to bind to %s", displayName.c_str() );
			} else {
				// overwrite a binding
				ImGui::Text( "Press a key or button to replace '%s' binding to %s",
							  bindings[selectedBinding].keyName.c_str(), displayName.c_str() );
			}

			ImGui::NewLine();
			ImGui::TextUnformatted( "To bind a mouse button, click it in the following field" );

			const float windowWidth = ImGui::GetWindowWidth();
			ImVec2 clickFieldSize( windowWidth * 0.8f, ImGui::GetTextLineHeightWithSpacing() * 4.0f );
			ImGui::SetCursorPosX( windowWidth * 0.1f );

			ImGui::Button( "###clickField", clickFieldSize );
			bool clickFieldHovered = ImGui::IsItemHovered();

			ImGui::NewLine();
			ImGui::Text( "... or press Escape (or %s) to cancel.", GetGamepadStartName() );

			ImGui::NewLine();
			// center the Cancel button
			float dialogButtonWidth = CalcDialogButtonWidth();
			float buttonOffset = (windowWidth - dialogButtonWidth) * 0.5f;
			ImGui::SetCursorPosX( buttonOffset );

			// Note: gamepad Start also generates K_ESCAPE in dhewm3
			if ( ImGui::Button( "Cancel", ImVec2(dialogButtonWidth, 0) ) || idKeyInput::IsDown( K_ESCAPE ) ) {
				ImGui::CloseCurrentPopup();
				ret = BESS_Selected;
				io.ConfigFlags |= (ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NavEnableKeyboard);
				D3::ImGuiHooks::SetKeyBindMode( false );
			} else if ( !newOpen ) {
				// find out if any key is pressed and bind that (except for Esc which can't be
				// bound and is already handled though IsCancelKeyPressed() above)
				// (but don't run this when the popup has just been opened, because then
				//  the key that opened this, likely Enter, is still registered as pressed)

				int pressedKey = -1;

				for ( int k = 1; k < K_LAST_KEY; ++k ) {
					if ( k == K_ESCAPE || k == K_JOY_BTN_START || k == K_CONSOLE )
						continue; // unbindable keys
					if ( !clickFieldHovered && k >= K_MOUSE1 && k <= K_MOUSE3 )
						continue; // mouse buttons must be clicked within the field to bind them

					if ( idKeyInput::IsDown( k ) ) {
						pressedKey = k;
						break;
					}
				}

				if ( pressedKey != -1 ) {
					// idKeyInput::IsDown() has a special case to return true for
					// K_CTRL also when K_RIGHT_CTRL is pressed, same for K_(RIGHT_)SHIFT
					// (but not the other way around). add extra check for that, because
					// when binding keys the distinction is important.
					if ( pressedKey == K_CTRL && idKeyInput::IsDown( K_RIGHT_CTRL ) ) {
						pressedKey = K_RIGHT_CTRL;
					} else if ( pressedKey == K_SHIFT && idKeyInput::IsDown( K_RIGHT_SHIFT ) ) {
						pressedKey = K_RIGHT_SHIFT;
					}

					D3::ImGuiHooks::SetKeyBindMode( false );

					const char* oldBinding = idKeyInput::GetBinding( pressedKey );
					if ( oldBinding[0] == '\0' ) {
						// that key isn't bound yet, hooray!
						AddKeyBinding( pressedKey );
						ret = BESS_Selected;
					} else {
						// Doom3 says: already bound!
						BindingEntry* oldBE = FindBindingEntryForKey( pressedKey );
						if ( oldBE == this ) {
							// that key is already bound to this command, show warning, otherwise do nothing
							const char* keyName = GetKeyName( pressedKey );
							idStr warning = idStr::Format( "Key '%s' is already bound to this command (%s)!",
									keyName, displayName.c_str() );
							D3::ImGuiHooks::ShowWarningOverlay( warning );
							ret = BESS_Selected;
							// TODO: select column with that specific binding?
						} else {
							// that key is already bound to some other command, show confirmation popup
							rebindKeyNum = pressedKey;
							rebindOtherEntry = oldBE; // NULL for commands this menu doesn't know!

							ret = BESS_WantRebind;
						}
					}

					ImGui::CloseCurrentPopup();
					io.ConfigFlags |= (ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NavEnableKeyboard);
				}
			}
			ImGui::EndPopup();
		}

		return ret;
	}

	BindingEntrySelectionState HandleRebindPopup( const char* popupName, bool newOpen )
	{
		BindingEntrySelectionState ret = BESS_WantRebind;

		if ( ImGui::BeginPopupModal( popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			const char* keyName = GetKeyName( rebindKeyNum );

			// TODO: if rebindOtherEntry == NULL, get command for rebindKeyNum from Doom3 directly
			//       and don't use rebindOtherEntry !

			if ( rebindOtherEntry != nullptr ) {
				ImGui::Text( "Key '%s' is already bound to command %s !\nBind to %s instead?",
				             keyName, rebindOtherEntry->displayName.c_str(), displayName.c_str() );
			} else {
				const char* commandName = idKeyInput::GetBinding( rebindKeyNum );
				ImGui::Text( "Key '%s' is already bound to command '%s'\n (not handled by this menu)!\nBind to %s instead?",
				             keyName, commandName, displayName.c_str() );
			}

			ImGui::NewLine();
			ImGui::Text( "Press Enter (or %s) to confirm,\nor Escape (or %s) to cancel.",
						 GetGamepadBindNowButtonName(), GetGamepadCancelButtonNames() );
			ImGui::NewLine();

			// center the Ok and Cancel buttons
			float dialogButtonWidth = CalcDialogButtonWidth();
			float spaceBetweenButtons = ImGui::GetFontSize();
			float buttonOffset = (ImGui::GetWindowWidth() - 2.0f*dialogButtonWidth - spaceBetweenButtons) * 0.5f;
			ImGui::SetCursorPosX( buttonOffset );

			bool confirmedByKey = false;
			if ( !newOpen && !ImGui::IsAnyItemFocused() ) {
				// if no item is focused (=> not using keyboard or gamepad navigation to select
				//  [Ok] or [Cancel] button), check if Enter has been pressed to confirm deletion
				// (otherwise, enter can be used to chose the selected button)
				// but don't do this when just opened, because then enter might still
				// be pressed from trying to bind Enter in the BindPopup
				confirmedByKey = IsConfirmKeyPressed();
			}

			if ( ImGui::Button( "Ok", ImVec2(dialogButtonWidth, 0) ) || confirmedByKey ) {
				if ( rebindOtherEntry != nullptr ) {
					rebindOtherEntry->RemoveKeyBinding( rebindKeyNum );
				}
				// Note: AddKeyBinding() eventually calls idKeyInput::SetBinding()
				//   which implicitly unbinds the old binding when it sets the new one,
				//   so for the rebindOtherEntry == nullptr case there's nothing else to do
				AddKeyBinding( rebindKeyNum );

				rebindOtherEntry = nullptr;
				rebindKeyNum = -1;

				ImGui::CloseCurrentPopup();
				ret = BESS_Selected;
			}
			ImGui::SetItemDefaultFocus();

			ImGui::SameLine( 0.0f, spaceBetweenButtons );
			if ( ImGui::Button( "Cancel", ImVec2(dialogButtonWidth, 0) ) || IsCancelKeyPressed() ) {
				rebindOtherEntry = nullptr;
				rebindKeyNum = -1;

				ImGui::CloseCurrentPopup();
				ret = BESS_Selected;
			}

			ImGui::EndPopup();
		}

		return ret;
	}

	void HandlePopup( BindingEntrySelectionState& selectionState )
	{
		assert(selectedBinding != BIND_NONE);
		const char* popupName = nullptr;

		if ( selectionState == BESS_WantClear ) {
			if ( bindings.Num() == 0 || selectedBinding >= bindings.Num()
				|| ( selectedBinding < 0 && selectedBinding != BIND_ALL )
				|| ( selectedBinding >= 0 && bindings[selectedBinding].keyNum == -1 ) ) {
				// there are no bindings at all for this command, or at least not in the selected column
				// => don't show popup, but keep the cell selected
				selectionState = BESS_Selected;
				return;
			}
			popupName = (selectedBinding == BIND_ALL) ? "Unbind keys" : "Unbind key";
		} else if ( selectionState == BESS_WantBind ) {
			popupName = "Bind key";
		} else {
			assert( selectionState == BESS_WantRebind );
			popupName = "Confirm rebinding key";
		}

		static bool popupOpened = false;
		bool newOpen = false;
		if ( !popupOpened ) {
			ImGui::OpenPopup( popupName );
			popupOpened = true;
			newOpen = true;
		}
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos( center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f) );

		BindingEntrySelectionState newSelState = BESS_Selected;
		if ( selectionState == BESS_WantClear ) {
			newSelState = HandleClearPopup( popupName, newOpen );
		} else if ( selectionState == BESS_WantBind ) {
			newSelState = HandleBindPopup( popupName, newOpen );
		} else {
			newSelState = HandleRebindPopup( popupName, newOpen );
		}

		if ( newSelState != selectionState ) {
			popupOpened = false;
			selectionState = newSelState;
		}
	}

};

const idDict* GetEntityDefDict( const char* name )
{
	const idDecl* decl = declManager->FindType( DECL_ENTITYDEF, name, false );
	const idDeclEntityDef* entDef = static_cast<const idDeclEntityDef*>( decl );
	return (entDef != nullptr) ? &entDef->dict : nullptr;
}

static idList<BindingEntry> bindingEntries;
static int firstObscureEntryIndex = 0;
static bool bindingsMenuAlreadyOpen = false;

// return NULL if not currently bound to anything
static BindingEntry* FindBindingEntryForKey( int keyNum )
{
	for ( BindingEntry& be : bindingEntries ) {
		for ( const BoundKey& bk : be.bindings ) {
			if ( bk.keyNum == keyNum ) {
				return &be;
			}
		}
	}
	return nullptr;
}

static void InitBindingEntries()
{
	bindingsMenuAlreadyOpen = false;

	bindingEntries.Clear();

	const BindingEntryTemplate betsMoveLookAttack[] = {
		{ "_forward",       "Forward"    , "#str_02100" },
		{ "_back",          "Backpedal"  , "#str_02101" },
		{ "_moveLeft",      "Move Left"  , "#str_02102" },
		{ "_moveRight",     "Move Right" , "#str_02103" },
		{ "_moveUp",        "Jump"       , "#str_02104" },
		{ "_moveDown",      "Crouch"     , "#str_02105" },
		{ "_left",          "Turn Left"  , "#str_02106" },
		{ "_right",         "Turn Right" , "#str_02107" },

		{ "_speed",         "Sprint"     , "#str_02109" },

		{ "_strafe",        "Strafe"     , "#str_02108" },

		{ "_lookUp",        "Look Up"    , "#str_02116" },
		{ "_lookDown",      "Look Down"  , "#str_02117" },

		{ "_mlook",         "Mouse Look" , "#str_02118", "only really relevant if in_freeLook = 0" },
		{ "_impulse18",     "Center View", "#str_02119" },

		{ nullptr,          "Attack"     , "#str_02112" },

		{ "_attack",        "Attack"     , "#str_02112" },
		{ "_impulse13",     "Reload"     , "#str_02115" },
		{ "_impulse14",     "Prev. Weapon" , "#str_02113" },
		{ "_impulse15",     "Next Weapon"  , "#str_02114" },
		{ "_zoom",          "Zoom View"    , "#str_02120" },
		{ "clientDropWeapon", "Drop Weapon", "#str_04071" },

		// also the heading for weapons, but the weapons entries are generated below..
		{ nullptr,          "Weapons"    , "#str_01416" },
	};

	const BindingEntryTemplate betsOther[] = {
		{ nullptr,          "Other"          , "#str_04064" }, // TODO: or "#str_02406"	"Misc"

		{ "_impulse19",     "PDA / Score"    , "#str_04066" },
		{ "dhewm3Settings", "dhewm3 settings menu", nullptr },
		{ "savegame quick", "Quick Save"     , "#str_04067" },
		{ "loadgame quick", "Quick Load"     , "#str_04068" },
		{ "screenshot",     "Screenshot"     , "#str_04069" },
		{ "clientMessageMode",   "Chat"      , "#str_02068" },
		{ "clientMessageMode 1", "Team Chat" , "#str_02122" },
		{ "_impulse20",     "Toggle Team"    , "#str_04070" },
		{ "_impulse22",     "Spectate"       , "#str_02125" },
		{ "_impulse17",     "Ready"          , "#str_02126" },
		{ "_impulse28",     "Vote Yes"       , "#str_02127" },
		{ "_impulse29",     "Vote No"        , "#str_02128" },
		{ "_impulse40",     "Use Vehicle"    , nullptr },
	};

	int numReserve = IM_ARRAYSIZE(betsMoveLookAttack) + IM_ARRAYSIZE(betsOther);
	numReserve += 16; // up to 14 weapons + weaponheading + moveLookHeading
	numReserve += 43; // the remaining "obscure" impulses
	if(bindingEntries.NumAllocated() < numReserve) {
		bindingEntries.Resize( numReserve );
	}

	idStr moveLookHeading = GetLocalizedString( "#str_02404", "Move" );
	moveLookHeading += " / ";
	moveLookHeading += GetLocalizedString( "#str_02403", "Look" );

	bindingEntries.Append( BindingEntry( moveLookHeading ) );

	for ( const BindingEntryTemplate& bet : betsMoveLookAttack ) {
		bindingEntries.Append( BindingEntry( bet ) );
	}

	const idDict* playerDict = GetEntityDefDict( "player_doommarine" );
	const idDict* playerDictMP = GetEntityDefDict( "player_doommarine_mp" );
	bool impulse27used = false;
	for ( int i = 0; i <= 13; ++i ) {
		int weapNum = i;
		int impulseNum = i;
		if (i == 13) {
			// Hack: D3XP uses def_weapon18 for (MP-only) weapon_chainsaw
			//  and the corresponding impulse is _impulse27
			// (otherwise def_weaponX corresponds to _impulseX)
			weapNum = 18;
			impulseNum = 27;
		}

		idStr defWeapName = idStr::Format( "def_weapon%d", weapNum );

		const char* weapName = playerDict->GetString( defWeapName, nullptr );
		if ( (weapName == nullptr || weapName[0] == '\0') && playerDictMP != nullptr ) {
			weapName = playerDictMP->GetString( defWeapName, nullptr );
		}

		// TODO: could also handle weapontoggles, in playerDict(MP):
		// "weapontoggle1"		"2,1" // _impulse1 toggles between def_weapon2 and def_weapon1
		// "weapontoggle4"		"5,4" // _impulse4 toggles between def_weapon5 and def_weapon4

		// note: weapon_PDA is skipped, because the generic open PDA action is _impulse19
		if ( weapName != nullptr && weapName[0] != '\0'
			 && idStr::Icmp( weapName, "weapon_pda" ) != 0 ) {
			const idDict* weapDict = GetEntityDefDict( weapName );
			if ( weapDict != nullptr ) {
				const char* displayName = weapDict->GetString( "inv_name", nullptr );
				if ( displayName == nullptr ) {
					displayName = weapName;
				} else if ( idStr::Cmpn( displayName, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 ) {
					displayName = GetLocalizedString( displayName, weapName );
				}
				bindingEntries.Append( BindingEntry( idStr::Format("_impulse%d", impulseNum), displayName ) );
				if ( impulseNum == 27 )
					impulse27used = true;
			}
		}
	}

	for ( const BindingEntryTemplate& bet : betsOther ) {
		bindingEntries.Append( BindingEntry( bet ) );
	}

	firstObscureEntryIndex = bindingEntries.Num();

	bindingEntries.Append( BindingEntry( "_impulse16", "_impulse16" ) );
	bindingEntries.Append( BindingEntry( "_impulse21", "_impulse21" ) );
	// _impulse22 is "spectate", handled in "Other" section
	bindingEntries.Append( BindingEntry( "_impulse23", "_impulse23" ) );
	bindingEntries.Append( BindingEntry( "_impulse24", "_impulse24" ) );
	bindingEntries.Append( BindingEntry( "_impulse25", "_impulse25",
			"In RoE's Capture The Flag with si_midnight = 2, this appears to toggle some kind of light" ) );
	bindingEntries.Append( BindingEntry( "_impulse26", "_impulse26" ) );
	if ( !impulse27used ) {
		bindingEntries.Append( BindingEntry( "_impulse27", "_impulse27" ) );
	}
	for ( int i=28; i <= 63; ++i ) {
		if ( i == 40 ) // _impulse40 is "use vehicle", handled above in "Other" section
			continue;

		idStr impName = idStr::Format( "_impulse%d", i );
		bindingEntries.Append( BindingEntry( impName, impName ) );
	}

	// player.def defines, in player_base, used by player_doommarine and player_doommarine_mp (and player_doommarine_ctf),
	// "def_weapon0"  "weapon_fists", "def_weapon1"  "weapon_pistol" etc
	// => get all those definitions (up to MAX_WEAPONS=16) from Player, and then
	//    get the entities for the corresponding keys ("weapon_fists" etc),
	//    which should have an entry like "inv_name"  "Pistol" (could also be #str_00100207 though!)

	// hardcorps uses: idCVar pm_character("pm_character", "0", CVAR_GAME | CVAR_BOOL, "Change Player character. 1 = Scarlet. 0 = Doom Marine");
	// but I guess (hope) they use the same weapons..
}

// this initialization should be done every time the bindings tab is opened,
// in case the bindings have been changed in the mean time (through console or classic menu)
static void UpdateKeyBindingsInEntries() {

	for ( BindingEntry& be : bindingEntries ) {
		be.bindings.SetNum( 0, false );
	}

	for ( int k = 1; k < K_LAST_KEY; ++k ) {
		if ( k == K_ESCAPE || k == K_JOY_BTN_START || k == K_CONSOLE ) {
			continue; // unbindable keys
		}

		const char* binding = idKeyInput::GetBinding( k );
		if ( binding[0] != '\0' ) {
			for ( BindingEntry& be : bindingEntries ) {
				if ( be.command.Icmp( binding ) == 0 ) {
					be.bindings.Append( BoundKey( k ) );
					break;
				}
			}
		}
	}
}


static void DrawBindingsMenu()
{
	if ( !bindingsMenuAlreadyOpen ) {
		UpdateKeyBindingsInEntries();
		bindingsMenuAlreadyOpen = true;
	}

	{
		// the InputInt will look kinda like this:
		// [10] [-] [+] Number of Binding columns...
		// the [-] and [+] buttons have size GetFrameHeight()^2
		// calculate the width it needs (without the "Number of ..." text)
		// for the text input field not too look too big
		const ImGuiStyle& st = ImGui::GetStyle();
		float w = ImGui::CalcTextSize("10").x;
		w += 2.0f * (ImGui::GetFrameHeight() + st.FramePadding.x + st.ItemInnerSpacing.x);
		ImGui::SetNextItemWidth(w);

		ImGui::InputInt( "Number of Binding columns to show", &numBindingColumns );

		numBindingColumns = idMath::ClampInt( 1, 10, numBindingColumns );

		// calculate the background color for the first column of the key binding tables
		// (it contains the command, while the other columns contain the keys bound to that command)
		displayNameBGColor = ImGui::GetColorU32( ImGuiCol_TableHeaderBg, 0.5f );


		if ( ImGui::TreeNode("Usage Help") ) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

			ImGui::TextWrapped( "Double click a keybinding entry below to bind a (different) key, or select it by clicking it once or navigating to it with cursor keys or gamepad and pressing Enter (or %s) to (re)bind it.",
			                    GetGamepadBindNowButtonName() );
			ImGui::TextWrapped( "Remove a key binding (unbind) by selecting it and pressing Backspace or Delete (or %s).",
			                    GetGamepadUnbindButtonName() );
			ImGui::TextWrapped( "If you select the first column (with the command name), you can unbind all keybindings for that command, or add another keybinding for it without overwriting an existing one." );
			ImGui::TextWrapped( "You can unselect the currently selected binding by clicking it again or by pressing Escape (or %s).",
			                    GetGamepadCancelButtonNames() );
			ImGui::TextWrapped( "The [++] button on the right opens (or closes) a window that shows all keys bound to the corresponding command (even if it's more than the number of binding columns) and has buttons to configure them. It's red when there actually are more key bound than can be shown in the columns of this window." );

			ImGui::PopStyleColor(); // ImGuiCol_Text
			ImGui::TreePop();
		} else {
			AddTooltip( "Click to show help text" );
		}
	}

	static int selectedRow = -1;
	static BindingEntrySelectionState selectionState = BESS_NotSelected;

	// make the key column entries in the bindings table center-aligned instead of left-aligned
	ImGui::PushStyleVar( ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.0f) );

	ImU32 borderCol = ImGui::GetColorU32( ImGuiCol_TableBorderLight, 0.5f );
	ImGui::PushStyleColor( ImGuiCol_TableBorderLight, borderCol );

	ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV;

	// inTable: are we currently adding elements to a table of bindings?
	//  (we're not when adding a heading from bindingEntries: existing tables are ended
	//   before a heading and a new table is started afterwards)
	bool inTable = false;
	// did the last ImGui::BeginTable() call return true (or is it not currently visible)?
	// (init to true so the first heading before any bindings is shown)
	bool lastBeginTable = true;
	int tableNum = 1;

	const ImVec2 defFramePadding = ImGui::GetStyle().FramePadding;
	const float commandColumnWidth = ImGui::CalcTextSize( "dhewm3 settings menu" ).x + defFramePadding.x * 2.0f;
	const float overflowColumnWidth = ImGui::CalcTextSize( "++" ).x + defFramePadding.x * 2.0f;

	// this handles the "regular" binding entries, i.e. everything but the "obscure" impulses list
	for ( int i=0, n=firstObscureEntryIndex; i < n; ++i ) {
		BindingEntry& be = bindingEntries[i];
		bool isHeading = be.IsHeading();
		if ( !isHeading && !inTable ) {
			// there is no WIP table (that has been started but not ended yet)
			// and the current element is a regular bind entry that's supposed
			// to go in a table, so start a new one
			inTable = true;
			char tableID[10];
			D3_snprintfC99( tableID, sizeof(tableID), "bindTab%d", tableNum++ );
			lastBeginTable = ImGui::BeginTable( tableID, numBindingColumns + 2, tableFlags );
			if ( lastBeginTable ) {
				ImGui::TableSetupScrollFreeze(1, 0);
				ImGui::TableSetupColumn( "Command", ImGuiTableColumnFlags_WidthFixed, commandColumnWidth );
				for ( int j=0; j < numBindingColumns; ++j ) {
					char colName[16];
					D3_snprintfC99(colName, sizeof(colName), "binding%d", j);
					ImGui::TableSetupColumn( colName );
				}

				ImGui::TableSetupColumn( "ShowAll", ImGuiTableColumnFlags_WidthFixed, overflowColumnWidth );
			}
		} else if ( isHeading && inTable ) {
			// we've been adding elements to a table (unless lastBeginTable = false) that hasn't
			// been closed with EndTable() yet, but now we're at a heading so the table is done
			if ( lastBeginTable ) {
				ImGui::EndTable();
			}
			inTable = false;
		}

		if ( lastBeginTable ) { // if ImGui::BeginTable() returned false, don't draw its elements
			BindingEntrySelectionState bess = (selectedRow == i) ? selectionState : BESS_NotSelected;
			bess = be.Draw( i, bess );
			if ( bess != BESS_NotSelected ) {
				selectedRow = i;
				selectionState = bess;
			} else if ( selectedRow == i ) {
				// this row was selected, but be.Draw() returned BESS_NotSelected, so unselect it
				selectedRow = -1;
			}
		}
	}
	if ( inTable && lastBeginTable ) {
		// end the last binding table, if any
		ImGui::EndTable();
	}

	const int numBindingEntries = bindingEntries.Num();
	// now the obscure impulses, unless they're hidden
	bool showObscImp = ImGui::CollapsingHeader( "Obscure Impulses" );
	AddDescrTooltip( "_impulseXY commands that are usually unused, but might be used by some mods, e.g. for additional weapons" );
	if (showObscImp) {
		if ( ImGui::BeginTable( "bindTabObsc", numBindingColumns + 2, tableFlags ) ) {
			ImGui::TableSetupScrollFreeze(1, 0);
			ImGui::TableSetupColumn( "Command", ImGuiTableColumnFlags_WidthFixed, commandColumnWidth );
			for ( int j=0; j < numBindingColumns; ++j ) {
				char colName[16];
				D3_snprintfC99(colName, sizeof(colName), "binding%d", j);
				ImGui::TableSetupColumn( colName );
			}

			ImGui::TableSetupColumn( "ShowAll", ImGuiTableColumnFlags_WidthFixed, overflowColumnWidth );

			for ( int i=firstObscureEntryIndex; i < numBindingEntries; ++i ) {
				BindingEntry& be = bindingEntries[i];
				BindingEntrySelectionState bess = (selectedRow == i) ? selectionState : BESS_NotSelected;
				bess = be.Draw( i, bess );
				if ( bess != BESS_NotSelected ) {
					selectedRow = i;
					selectionState = bess;
				} else if ( selectedRow == i ) {
					// this row was selected, but be.Draw() returned BESS_NotSelected, so unselect it
					selectedRow = -1;
				}
			}

			ImGui::EndTable();
		}
	}

	if ( ImGui::IsWindowFocused() && IsCancelKeyPressed() ) {
		// pressing Escape unfocuses whatever entry is focused
		selectedRow = -1;
		// FIXME: the following looks dangerous, but for some reason ImGui doesn't
		//        seem to have a function to unfocus o_O
		ImGui::GetCurrentContext()->NavId = 0;
	}

	ImGui::PopStyleColor(); // ImGuiCol_TableBorderLight
	ImGui::PopStyleVar(); // ImGuiStyleVar_SelectableTextAlign

	// WantBind or WantClear or WantRebind => show a popup
	if ( selectionState > BESS_Selected ) {
		assert(selectedRow >= 0 && selectedRow < numBindingEntries);
		bindingEntries[selectedRow].HandlePopup( selectionState );
	}
}



/****************************
 *                          *
 * Other (CVar-based) Menus *
 *                          *
 ****************************/

static void AddCVarOptionTooltips( const idCVar& cvar, const char* desc = nullptr )
{
	AddTooltip( cvar.GetName() );
	AddDescrTooltip( desc ? desc : cvar.GetDescription() );
}

enum OptionType {
	OT_NONE,
	OT_HEADING, // not an option, just a heading on the page
	OT_BOOL,
	OT_FLOAT,
	OT_INT,
	OT_CUSTOM,  // using a callback in Draw()
};

struct CVarOption {
	typedef void (*DrawCallback)( idCVar& cvar ); // for OT_CUSTOM

	const char* name = nullptr;
	idCVar* cvar = nullptr;
	const char* label = nullptr;
	DrawCallback drawCallback = nullptr;
	OptionType type = OT_NONE;
	// TODO: the following two could be a union, together with drawCallback and possibly others!
	float minVal = 0.0f;
	float maxVal = 0.0f;


	CVarOption() = default;

	CVarOption(const char* _name, const char* _label, OptionType _type, float _minVal = 0.0f, float _maxVal = 0.0f)
	: name(_name), label(_label), type(_type), minVal(_minVal), maxVal(_maxVal)
	{}

	CVarOption(const char* _name, DrawCallback drawCB)
	: name(_name), drawCallback(drawCB), type(OT_CUSTOM)
	{}

	CVarOption(const char* headingLabel) : label(headingLabel), type(OT_HEADING)
	{}

	void Init()
	{
		if (name != NULL) {
			cvar = cvarSystem->Find(name);
			printf("# Init() name = %s cvar = %p\n", name, cvar);
		}
		else printf("# Init() name = NULL label = %s\n", label);
	}

	void Draw()
	{
		if (type == OT_HEADING) {
			if (label != NULL) {
				ImGui::SeparatorText(label);
			}
		} else if (cvar != nullptr) {
			switch(type) {
				case OT_BOOL:
				{
					bool b = cvar->GetBool();
					bool bOrig = b;
					ImGui::Checkbox( label, &b );
					AddCVarOptionTooltips( *cvar );
					if (b != bOrig) {
						cvar->SetBool(b);
					}
					break;
				}
				case OT_FLOAT:
				{
					float f = cvar->GetFloat();
					float fOrig = f;
					// TODO: make format configurable?
					ImGui::SliderFloat(label, &f, minVal, maxVal, "%.2f", 0);
					AddCVarOptionTooltips( *cvar );
					if(f != fOrig) {
						cvar->SetFloat(f);
					}
					break;
				}
				case OT_INT:
				{
					int i = cvar->GetInteger();
					int iOrig = i;
					ImGui::SliderInt(label, &i, minVal, maxVal);
					AddCVarOptionTooltips( *cvar );
					if (i != iOrig) {
						cvar->SetInteger(i);
					}
					break;
				}
				case OT_CUSTOM:
					if (drawCallback != nullptr) {
						drawCallback(*cvar);
					}
					break;
			}
		}
	}
};

static void InitOptions(CVarOption options[], size_t numOptions)
{
	for( int i=0; i < numOptions; ++i ) {
		options[i].Init();
	}
}

static void DrawOptions(CVarOption options[], size_t numOptions)
{
	for( int i=0; i < numOptions; ++i ) {
		options[i].Draw();
	}
}

static CVarOption controlOptions[] = {

	// TODO: always run?

	CVarOption("Mouse Settings"),
	CVarOption("sensitivity", "Sensitivity", OT_FLOAT, 1.0f, 30.0f),
	CVarOption("m_smooth", "Smoothing Samples", OT_INT, 1, 8),
	CVarOption("in_nograb", "Don't grab Mouse Cursor (for debugging/testing)", OT_BOOL),
	// TODO: add in_invertLook or m_invertLook ?

	CVarOption("Keyboard Settings"),
	CVarOption("in_grabKeyboard", "Grab Keyboard", OT_BOOL),
	CVarOption("in_ignoreConsoleKey", "Don't open console with key between Esc, Tab and 1", OT_BOOL),

	CVarOption("Gamepad Settings"),
	CVarOption("in_useGamepad", "Enable Gamepad Support", OT_BOOL),
	CVarOption("joy_gamepadLayout", [](idCVar& cvar) {
			int sel = cvar.GetInteger() + 1; // -1 .. 3 => 0 .. 4
			int selOrig = sel;
			// -1: auto (needs SDL 2.0.12 or newer), 0: XBox-style, 1: Nintendo-style, 2: PS4/5-style, 3: PS2/3-style
			const char* items[] = { "Auto-Detect", "XBox Controller-like",
					"Nintendo-style", "Playstation 4/5 Controller-like",
					"Playstation 2/3 Controller-like" };
			ImGui::Combo("Gamepad Layout", &sel, items, IM_ARRAYSIZE(items));
			AddCVarOptionTooltips( cvar, "Button Layout of Gamepad (esp. for the displayed names of the 4 buttons on the right)" );
			if(sel != selOrig) {
				cvar.SetInteger(sel-1);
			}
		}),
	CVarOption("joy_deadZone", "Axis Deadzone", OT_FLOAT, 0.0f, 0.99f),
	CVarOption("joy_triggerThreshold", "Trigger Threshold/Deadzone", OT_FLOAT, 0.0f, 0.99f),
	CVarOption("joy_pitchSpeed", "Pitch speed (for looking up/down)", OT_INT, 60.0f, 600.0f),
	CVarOption("joy_yawSpeed", "Yaw speed (for looking left/right)", OT_INT, 60.0f, 600.0f),
	// TODO: joy_invertLook? (I don't really see the point, one can just bind move stick up to look down)
	CVarOption("joy_gammaLook", "Use logarithmic gamma curve instead of power curve for axes", OT_BOOL),
	CVarOption("joy_powerScale", "If using power curve, this is the exponent", OT_FLOAT, 0.1f, 10.0f), // TODO: what are sensible min/max values?
	// TODO: joy_dampenlook and joy_deltaPerMSLook ? comment in code says they were "bad idea"
};

// TODO: r_scaleMenusTo43

} //anon namespace

// called from D3::ImGuiHooks::NewFrame() (if this window is enabled)
void Com_DrawDhewm3SettingsMenu()
{
	bool showSettingsWindow = true;
	ImGui::Begin("dhewm3 Settings", &showSettingsWindow);

	float scale = D3::ImGuiHooks::GetScale();

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);

	if ( ImGui::DragFloat("ImGui scale", &scale, 0.005f, 0.25f, 8.0f, "%.3f") ) {
		D3::ImGuiHooks::SetScale( scale );
	}
	ImGui::SameLine();
	if ( ImGui::Button("Reset") ) {
		D3::ImGuiHooks::SetScale( -1.0f );
	}

	if (ImGui::Button("Show ImGui Demo")) {
		D3::ImGuiHooks::OpenWindow( D3::ImGuiHooks::D3_ImGuiWin_Demo );
	}

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("SettingsTabBar", tab_bar_flags))
	{
		if ( ImGui::BeginTabItem("Control Bindings") ) {
			DrawBindingsMenu();
			ImGui::EndTabItem();
		} else {
			bindingsMenuAlreadyOpen = false;
		}
		if ( ImGui::BeginTabItem("Control Options") ) {
			DrawOptions( controlOptions, IM_ARRAYSIZE(controlOptions) );
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Game Options"))
		{
			ImGui::Text("This is the Game Options tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Video Options"))
		{
			ImGui::Text("This is the Video tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Audio Options"))
		{
			ImGui::Text("This is the Audio tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
	if (!showSettingsWindow) {
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_Settings );
	}
}

void Com_InitDhewm3SettingsMenu()
{
	InitOptions( controlOptions, IM_ARRAYSIZE(controlOptions) );
	InitBindingEntries();
}

void Com_Dhewm3Settings_f( const idCmdArgs &args )
{
	bool menuOpen = (D3::ImGuiHooks::GetOpenWindowsMask() & D3::ImGuiHooks::D3_ImGuiWin_Settings) != 0;
	if ( !menuOpen ) {
		// TODO: if in SP game, pause

		Com_InitDhewm3SettingsMenu();
		D3::ImGuiHooks::OpenWindow( D3::ImGuiHooks::D3_ImGuiWin_Settings );
	} else {
		D3::ImGuiHooks::CloseWindow( D3::ImGuiHooks::D3_ImGuiWin_Settings );

		// TODO: if in SP game, unpause
	}
}

#else // IMGUI_DISABLE - just a stub function

void Com_Dhewm3Settings_f( const idCmdArgs &args ) {}

#endif
