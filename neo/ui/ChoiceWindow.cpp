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

#include "sys/platform.h"
#include "idlib/LangDict.h"
#include "framework/KeyInput.h"
#include "ui/DeviceContext.h"
#include "ui/Window.h"
#include "ui/UserInterfaceLocal.h"

#include "ui/ChoiceWindow.h"

/*
============
idChoiceWindow::InitVars
============
*/
void idChoiceWindow::InitVars( ) {
	if ( cvarStr.Length() ) {
		cvar = cvarSystem->Find( cvarStr );
		if ( !cvar ) {
			if (strcmp(cvarStr.c_str(), "s_driver") && strcmp(cvarStr.c_str(), "net_serverAllowServerMod"))
				common->Warning( "idChoiceWindow::InitVars: gui '%s' window '%s' references undefined cvar '%s'", gui->GetSourceFile(), name.c_str(), cvarStr.c_str() );
			return;
		}
		updateStr.Append( &cvarStr );
	}
	if ( guiStr.Length() ) {
		updateStr.Append( &guiStr );
	}
	updateStr.SetGuiInfo( gui->GetStateDict() );
	updateStr.Update();
}

/*
============
idChoiceWindow::CommonInit
============
*/
void idChoiceWindow::CommonInit() {
	currentChoice = 0;
	choiceType = 0;
	cvar = NULL;
	liveUpdate = true;
	choices.Clear();
}

idChoiceWindow::idChoiceWindow(idDeviceContext *d, idUserInterfaceLocal *g) : idWindow(d, g) {
	dc = d;
	gui = g;
	CommonInit();
}

idChoiceWindow::idChoiceWindow(idUserInterfaceLocal *g) : idWindow(g) {
	gui = g;
	CommonInit();
}

idChoiceWindow::~idChoiceWindow() {

}

void idChoiceWindow::RunNamedEvent( const char* eventName ) {
	idStr event, group;

	if ( !idStr::Cmpn( eventName, "cvar read ", 10 ) ) {
		event = eventName;
		group = event.Mid( 10, event.Length() - 10 );
		if ( !group.Cmp( updateGroup ) ) {
			UpdateVars( true, true );
		}
	} else if ( !idStr::Cmpn( eventName, "cvar write ", 11 ) ) {
		event = eventName;
		group = event.Mid( 11, event.Length() - 11 );
		if ( !group.Cmp( updateGroup ) ) {
			UpdateVars( false, true );
		}
	}
}

void idChoiceWindow::UpdateVars( bool read, bool force ) {
	if ( force || liveUpdate ) {
		if ( cvar && cvarStr.NeedsUpdate() ) {
			if ( read ) {
				cvarStr.Set( cvar->GetString() );
			} else {
				cvar->SetString( cvarStr.c_str() );
			}
		}
		if ( !read && guiStr.NeedsUpdate() ) {
			guiStr.Set( va( "%i", currentChoice ) );
		}
	}
}

const char *idChoiceWindow::HandleEvent(const sysEvent_t *event, bool *updateVisuals) {
	int key;
	bool runAction = false;
	bool runAction2 = false;

	if ( event->evType == SE_KEY ) {
		key = event->evValue;

		if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW || key == K_MOUSE1)  {
			// never affects the state, but we want to execute script handlers anyway
			if ( !event->evValue2 ) {
				RunScript( ON_ACTIONRELEASE );
				return cmd;
			}
			currentChoice++;
			if (currentChoice >= choices.Num()) {
				currentChoice = 0;
			}
			runAction = true;
		}

		if ( key == K_LEFTARROW || key == K_KP_LEFTARROW || key == K_MOUSE2) {
			// never affects the state, but we want to execute script handlers anyway
			if ( !event->evValue2 ) {
				RunScript( ON_ACTIONRELEASE );
				return cmd;
			}
			currentChoice--;
			if (currentChoice < 0) {
				currentChoice = choices.Num() - 1;
			}
			runAction = true;
		}

		if ( !event->evValue2 ) {
			// is a key release with no action catch
			return "";
		}

	} else if ( event->evType == SE_CHAR ) {

		key = event->evValue;

		int potentialChoice = -1;
		for ( int i = 0; i < choices.Num(); i++ ) {
			if ( toupper(key) == toupper(choices[i][0]) ) {
				if ( i < currentChoice && potentialChoice < 0 ) {
					potentialChoice = i;
				} else if ( i > currentChoice ) {
					potentialChoice = -1;
					currentChoice = i;
					break;
				}
			}
		}
		if ( potentialChoice >= 0 ) {
			currentChoice = potentialChoice;
		}

		runAction = true;
		runAction2 = true;

	} else {
		return "";
	}

	if ( runAction ) {
		RunScript( ON_ACTION );
	}

	if ( choiceType == 0 ) {
		cvarStr.Set( va( "%i", currentChoice ) );
	} else if ( values.Num() ) {
		cvarStr.Set( values[ currentChoice ] );
	} else {
		cvarStr.Set( choices[ currentChoice ] );
	}

	UpdateVars( false );

	if ( runAction2 ) {
		RunScript( ON_ACTIONRELEASE );
	}

	return cmd;
}

void idChoiceWindow::ValidateChoice() {
	if ( currentChoice < 0 || currentChoice >= choices.Num() ) {
		currentChoice = 0;
	}
	if ( choices.Num() == 0 ) {
		choices.Append( "No Choices Defined" );
	}
}

void idChoiceWindow::UpdateChoice() {
	if ( !updateStr.Num() ) {
		return;
	}
	UpdateVars( true );
	updateStr.Update();
	if ( choiceType == 0 ) {
		// ChoiceType 0 stores current as an integer in either cvar or gui
		// If both cvar and gui are defined then cvar wins, but they are both updated
		if ( updateStr[ 0 ]->NeedsUpdate() ) {
			currentChoice = atoi( updateStr[ 0 ]->c_str() );
		}
		ValidateChoice();
	} else {
		// ChoiceType 1 stores current as a cvar string
		int c = ( values.Num() ) ? values.Num() : choices.Num();
		int i;
		for ( i = 0; i < c; i++ ) {
			if ( idStr::Icmp( cvarStr.c_str(), ( values.Num() ) ? values[i] : choices[i] ) == 0 ) {
				break;
			}
		}
		if (i == c) {
			i = 0;
		}
		currentChoice = i;
		ValidateChoice();
	}
}

bool idChoiceWindow::ParseInternalVar(const char *_name, idParser *src) {
	if (idStr::Icmp(_name, "choicetype") == 0) {
		choiceType = src->ParseInt();
		return true;
	}
	if (idStr::Icmp(_name, "currentchoice") == 0) {
		currentChoice = src->ParseInt();
		return true;
	}
	return idWindow::ParseInternalVar(_name, src);
}


idWinVar *idChoiceWindow::GetWinVarByName(const char *_name, bool fixup, drawWin_t** owner) {
	if ( idStr::Icmp( _name, "choices" ) == 0 ) {
		return &choicesStr;
	}
	if ( idStr::Icmp( _name, "values" ) == 0 ) {
		return &choiceVals;
	}
	if ( idStr::Icmp( _name, "cvar" ) == 0 ) {
		return &cvarStr;
	}
	if ( idStr::Icmp( _name, "gui" ) == 0 ) {
		return &guiStr;
	}
	if ( idStr::Icmp( _name, "liveUpdate" ) == 0 ) {
		return &liveUpdate;
	}
	if ( idStr::Icmp( _name, "updateGroup" ) == 0 ) {
		return &updateGroup;
	}

	return idWindow::GetWinVarByName(_name, fixup, owner);
}

// update the lists whenever the WinVar have changed
void idChoiceWindow::UpdateChoicesAndVals( void ) {
	idToken token;
	idStr str2, str3;
	idLexer src;

	if ( latchedChoices.Icmp( choicesStr ) ) {
		choices.Clear();
		src.FreeSource();
		src.SetFlags( LEXFL_NOFATALERRORS | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );
		src.LoadMemory( choicesStr, choicesStr.Length(), "<ChoiceList>" );
		if ( src.IsLoaded() ) {
			while( src.ReadToken( &token ) ) {
				if ( token == ";" ) {
					if ( str2.Length() ) {
						str2.StripTrailingWhitespace();
						str2 = common->GetLanguageDict()->GetString( str2 );
						choices.Append(str2);
						str2 = "";
					}
					continue;
				}
				str2 += token;
				str2 += " ";
			}
			if ( str2.Length() ) {
				str2.StripTrailingWhitespace();
				choices.Append( str2 );
			}
		}
		latchedChoices = choicesStr.c_str();
	}
	if ( choiceVals.Length() && latchedVals.Icmp( choiceVals ) ) {
		values.Clear();
		src.FreeSource();
		src.SetFlags( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );
		src.LoadMemory( choiceVals, choiceVals.Length(), "<ChoiceVals>" );
		str2 = "";
		bool negNum = false;
		if ( src.IsLoaded() ) {
			while( src.ReadToken( &token ) ) {
				if (token == "-") {
					negNum = true;
					continue;
				}
				if (token == ";") {
					if (str2.Length()) {
						str2.StripTrailingWhitespace();
						values.Append( str2 );
						str2 = "";
					}
					continue;
				}
				if ( negNum ) {
					str2 += "-";
					negNum = false;
				}
				str2 += token;
				str2 += " ";
			}
			if ( str2.Length() ) {
				str2.StripTrailingWhitespace();
				values.Append( str2 );
			}
		}
		if ( choices.Num() != values.Num() ) {
			common->Warning( "idChoiceWindow:: gui '%s' window '%s' has value count unequal to choices count", gui->GetSourceFile(), name.c_str());
		}
		latchedVals = choiceVals.c_str();
	}
}

idStr R_GetVidModeListString(bool addCustom);
idStr R_GetVidModeValsString(bool addCustom);

void idChoiceWindow::PostParse() {
	idWindow::PostParse();

	// DG: HACKHACKFUCKINGUGLYHACK: overwrite resolution list
	//     to support more resolutions and widescreen and stuff.
	bool injectResolutions = false;
	bool injectCustomMode = true;

	/*
	 * Mods that have their own video settings menu can tell dhewm3 to replace the
	 * "choices" and "values" entries in their choiceDef with the resolutions supported by
	 * dhewm3 (and corresponding modes). So if we add new video modes to dhewm3,
	 * they'll automatically appear in the menu without changing the .gui
	 * To enable this, the mod authors only need to add an "injectResolutions 1" entry
	 * to their resolution choiceDef. By default, the first entry will be "r_custom*"
	 * for r_mode -1, which means "custom resolution, use r_customWidth and r_customHeight".
	 * If that entry shoud be disabled for the mod, just add another entry:
	 * "injectCustomResolutionMode 0"
	 */
	idWinVar* wv = GetWinVarByName("injectResolutions");
	if(wv != NULL) {
		const char* val = wv->c_str();
		if(val != NULL && *val != '\0' && idStr::Cmp(val, "0") != 0) {
			injectResolutions = true;
			wv = GetWinVarByName("injectCustomResolutionMode");
			if(wv != NULL) {
				const char* val = wv->c_str();
				if(val != NULL && *val != '\0' && idStr::Cmp(val, "0") == 0) {
					injectCustomMode = false;
				}
			}
		}
	}
	else if( idStr::Cmp(GetName(), "OS2Primary") == 0 && cvarStr == "r_mode"
	         && ( idStr::Icmp(GetGui()->GetSourceFile(), "guis/demo_mainmenu.gui") == 0 ||
		          idStr::Icmp(GetGui()->GetSourceFile(), "guis/mainmenu.gui") == 0 ) )
	{
		// always enable this for base/ and d3xp/ mainmenu.gui (like we did before)
		injectResolutions = true;
	}

	if(injectResolutions) {
		choicesStr.Set( R_GetVidModeListString(injectCustomMode) );
		choiceVals.Set( R_GetVidModeValsString(injectCustomMode) );
	}
	// DG end

	UpdateChoicesAndVals();

	InitVars();
	UpdateChoice();
	UpdateVars(false);

	flags |= WIN_CANFOCUS;
}

void idChoiceWindow::Draw(int time, float x, float y) {
	idVec4 color = foreColor;

	UpdateChoicesAndVals();
	UpdateChoice();

	// FIXME: It'd be really cool if textAlign worked, but a lot of the guis have it set wrong because it used to not work
	textAlign = 0;

	if ( textShadow ) {
		idStr shadowText = choices[currentChoice];
		idRectangle shadowRect = textRect;

		shadowText.RemoveColors();
		shadowRect.x += textShadow;
		shadowRect.y += textShadow;

		dc->DrawText( shadowText, textScale, textAlign, colorBlack, shadowRect, false, -1 );
	}

	if ( hover && !noEvents && Contains(gui->CursorX(), gui->CursorY()) ) {
		color = hoverColor;
	} else {
		hover = false;
	}
	if ( flags & WIN_FOCUS ) {
		color = hoverColor;
	}

	if(choices.Num() > 0) {
		dc->DrawText( choices[currentChoice], textScale, textAlign, color, textRect, false, -1 );
	}
}

void idChoiceWindow::Activate( bool activate, idStr &act ) {
	idWindow::Activate( activate, act );
	if ( activate ) {
		// sets the gui state based on the current choice the window contains
		UpdateChoice();
	}
}
