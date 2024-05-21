
#include "Common.h"
#include "CVarSystem.h"

#include "idlib/LangDict.h"

#include "UsercmdGen.h" // key bindings
//#include "Game.h" // idGameEdit to access entity definitions (player, weapons)
#include "DeclEntityDef.h"

#include "sys/sys_imgui.h"

#ifndef IMGUI_DISABLE

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

static void AddCVarOptionTooltips( const idCVar& cvar, const char* desc = nullptr )
{
#if 0
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted( cvar.GetName() );
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
#endif
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

struct BindingEntryTemplate {
	const char* command;
	const char* name;
	const char* nameLocStr;
	const char* description;
};

struct BindingEntry {
	idStr command; // "_impulse3" or "_forward" or similar - or "" for heading entry
	idStr displayName;
	const char* description = nullptr;
	// TODO: actual bindings?

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

	void Init()
	{
		// TODO: get bindings
	}

	void Draw()
	{
		if( command.Length() == 0 ) {
			ImGui::SeparatorText( displayName );
			if(description) {
				AddDescrTooltip(description);
			}
		} else {
			ImGui::Text( "%s => %s", command.c_str(), displayName.c_str() );
			if ( description ) {
				AddDescrTooltip( description );
			}
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

static void InitBindingEntries()
{
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
			}
		}
	}

	for ( const BindingEntryTemplate& bet : betsOther ) {
		bindingEntries.Append( BindingEntry( bet ) );
	}

	firstObscureEntryIndex = bindingEntries.Num();
	// TODO: could instead save bindingEntries.Num() as "firstObscureIndex"
	//  and put all the obscure bindings into the same list
	// => makes handling the code ("what action are we currently trying to bind?")
	//    more uniform and thus simpler

	bindingEntries.Append( BindingEntry( "_impulse16", "_impulse16" ) );
	bindingEntries.Append( BindingEntry( "_impulse21", "_impulse21" ) );
	// _impulse22 is "spectate", handled in "Other" section
	bindingEntries.Append( BindingEntry( "_impulse23", "_impulse23" ) );
	bindingEntries.Append( BindingEntry( "_impulse24", "_impulse24" ) );
	bindingEntries.Append( BindingEntry( "_impulse25", "_impulse25 / midnight CTF light",
			"In RoE's Capture The Flag with si_midnight = 2, this appears to toggle some kind of light" ) );
	for ( int i=26; i <= 63; ++i ) {
		if ( i==40 ) // _impulse40 is "use vehicle", handled above in "Other" section
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

static void DrawBindingsMenu()
{
	for ( int i=0, n=firstObscureEntryIndex; i < n; ++i ) {
		bindingEntries[i].Draw();
	}
	bool showObscImp = ImGui::CollapsingHeader( "Obscure Impulses" );
	AddDescrTooltip( "_impulseXY commands that are usually unused, but might be used by some mods, e.g. for additional weapons" );
	if (showObscImp) {
		for ( int i=firstObscureEntryIndex, n=bindingEntries.Num(); i < n; ++i ) {
			bindingEntries[i].Draw();
		}
	}
}

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
		if (ImGui::BeginTabItem("Control Bindings"))
		{
			DrawBindingsMenu();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Control Options"))
		{
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
