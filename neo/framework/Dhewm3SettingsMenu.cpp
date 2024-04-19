
#include "Common.h"
#include "CVarSystem.h"

#include "sys/sys_imgui.h"

#ifndef IMGUI_DISABLE

namespace {

static void AddCVarOptionTooltips( const idCVar& cvar, const char* desc = nullptr )
{
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted( cvar.GetName() );
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
		ImGui::TextUnformatted( desc ? desc : cvar.GetDescription() );
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
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
	for( int i=0; i<numOptions; ++i ) {
		options[i].Init();
	}
}

static void DrawOptions(CVarOption options[], size_t numOptions)
{
	for( int i=0; i<numOptions; ++i ) {
		options[i].Draw();
	}
}

static CVarOption controlOptions[] = {

	CVarOption("Mouse Settings"),
	CVarOption("sensitivity", "Sensitivity", OT_FLOAT, 1.0f, 30.0f),
	CVarOption("m_smooth", "Smoothing Samples", OT_INT, 1, 8),
	CVarOption("in_nograb", "Don't grab Mouse Cursor (for debugging/testing)", OT_BOOL),

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
		/*if (ImGui::BeginTabItem("Control Bindings"))
		{
			ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}*/
		if (ImGui::BeginTabItem("Control Options"))
		{
			DrawOptions( controlOptions, IM_ARRAYSIZE(controlOptions) );
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Game Options"))
		{
			ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Video Options"))
		{
			ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Audio Options"))
		{
			ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
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
