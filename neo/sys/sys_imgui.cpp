
#define IMGUI_DEFINE_MATH_OPERATORS

#include "sys_sdl.h"

#if SDL_VERSION_ATLEAST(3, 0, 0)
  // compat with SDL2
  #define SDL_TEXTINPUT SDL_EVENT_TEXT_INPUT
  #define SDL_CONTROLLERAXISMOTION SDL_EVENT_GAMEPAD_AXIS_MOTION
  #define SDL_CONTROLLERBUTTONDOWN SDL_EVENT_GAMEPAD_BUTTON_DOWN
  #define SDL_MOUSEBUTTONDOWN SDL_EVENT_MOUSE_BUTTON_DOWN
  #define SDL_MOUSEMOTION SDL_EVENT_MOUSE_MOTION
  #define SDL_MOUSEWHEEL SDL_EVENT_MOUSE_WHEEL
  #define SDL_KEYDOWN SDL_EVENT_KEY_DOWN
#endif


#include "sys_imgui.h"

#include "../libs/imgui/backends/imgui_impl_opengl2.h"

#if SDL_VERSION_ATLEAST(3, 0, 0)
  #include "../libs/imgui/backends/imgui_impl_sdl3.h"
  #define ImGui_ImplSDLx_InitForOpenGL ImGui_ImplSDL3_InitForOpenGL
  #define ImGui_ImplSDLx_Shutdown ImGui_ImplSDL3_Shutdown
  #define ImGui_ImplSDLx_NewFrame ImGui_ImplSDL3_NewFrame
  #define ImGui_ImplSDLx_ProcessEvent ImGui_ImplSDL3_ProcessEvent
#else
  #include "../libs/imgui/backends/imgui_impl_sdl2.h"
  #define ImGui_ImplSDLx_InitForOpenGL ImGui_ImplSDL2_InitForOpenGL
  #define ImGui_ImplSDLx_Shutdown ImGui_ImplSDL2_Shutdown
  #define ImGui_ImplSDLx_NewFrame ImGui_ImplSDL2_NewFrame
  #define ImGui_ImplSDLx_ProcessEvent ImGui_ImplSDL2_ProcessEvent
#endif

#include "framework/Common.h"
#include "framework/KeyInput.h"
#include "framework/Session_local.h" // sessLocal.GetActiveMenu()
#include "renderer/qgl.h"
#include "renderer/tr_local.h" // glconfig
#include "ui/DeviceContext.h"
#include "ui/UserInterface.h"

extern void Com_DrawDhewm3SettingsMenu(); // in framework/dhewm3SettingsMenu.cpp
extern void Com_OpenCloseDhewm3SettingsMenu( bool open ); // ditto

static idCVar imgui_scale( "imgui_scale", "-1.0", CVAR_SYSTEM|CVAR_FLOAT|CVAR_ARCHIVE, "factor to scale ImGUI menus by (-1: auto)" ); // TODO: limit values?

idCVar imgui_style( "imgui_style", "0", CVAR_SYSTEM|CVAR_INTEGER|CVAR_ARCHIVE, "Which ImGui style to use. 0: Dhewm3 theme, 1: Default ImGui theme, 2: User theme", 0.0f, 2.0f );

extern idCVar r_scaleMenusTo43;

// implemented in imgui_savestyle.cpp
namespace DG {
// writes the given ImGuiStyle to the given filename (opened with fopen())
// returns true on success, false if opening the file failed
extern bool WriteImGuiStyle( const ImGuiStyle& style, const char* filename );

// reads the the given filename (opened with fopen())
// and sets the given ImGuiStyle accordingly.
// if any attributes/colors/behaviors are missing the the file,
// they are not modified in style, so it probably makes sense to initialize
// style to a sane default before calling this function.
// returns true on success, false if opening the file failed
extern bool ReadImGuiStyle( ImGuiStyle& style, const char* filename );

// generate C++ code that replicates the given style into a text buffer
// (that you can write to a file or set the clipboard from or whatever)
// if refStyle is set, only differences in style compared to refStyle are written
extern ImGuiTextBuffer WriteImGuiStyleToCode( const ImGuiStyle& style, const ImGuiStyle* refStyle = nullptr );
} //namespace DG

namespace D3 {
namespace ImGuiHooks {

#ifdef _MSC_VER
  // Visual C++ (at least up to some 2019 version) doesn't support string literals
  // with more than 65535 bytes, so the base85-encoded version won't work here..
  // this alternative doesn't work with Big Endian, but that's not overly relevant for Windows.
  #include "proggyvector_font.h"
#else // proper compilers that support longer string literals
  #include "proggyvector_font_base85.h"
#endif

static SDL_Window* sdlWindow = NULL;
ImGuiContext* imguiCtx = NULL;
static bool haveNewFrame = false;
static int openImguiWindows = 0; // or-ed enum D3ImGuiWindow values

static ImGuiStyle userStyle;

// was there a key down or button (mouse/gamepad) down event this frame?
// used to make the warning overlay disappear
static bool hadKeyDownEvent = false;

static idStr warningOverlayText;
static double warningOverlayStartTime = -100.0;
static ImVec2 warningOverlayStartPos;

idStr GetUserStyleFilename()
{
	idStr ret( cvarSystem->GetCVarString( "fs_configpath" ) );
	ret += "/user.imstyle";
	return ret;
}

static void UpdateWarningOverlay()
{
	double timeNow = ImGui::GetTime();
	if ( timeNow - warningOverlayStartTime > 4.0f ) {
		warningOverlayStartTime = -100.0f;
		return;
	}

	// also hide if a key was pressed or maybe even if the mouse was moved (too much)
	ImVec2 mdv = ImGui::GetMousePos() - warningOverlayStartPos; // Mouse Delta Vector
	float mouseDelta = sqrtf( mdv.x * mdv.x + mdv.y * mdv.y );
	const float fontSize = ImGui::GetFontSize();
	if ( mouseDelta > fontSize * 4.0f || hadKeyDownEvent ) {
		warningOverlayStartTime = -100.0f;
		return;
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4(1.0f, 0.4f, 0.4f, 0.6f) );
	float padSize = fontSize * 2.0f;
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(padSize, padSize) );

	int winFlags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin("WarningOverlay", NULL, winFlags);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 points[] = {
		{0, 40}, {40, 40}, {20, 0}, // triangle
		{20, 12}, {20, 28}, // line
		{20, 33} // dot
	};

	float iconScale = 1.0f; // TODO: global scale also used for fontsize

	ImVec2 offset = ImGui::GetWindowPos() + ImVec2(fontSize, fontSize);
	for ( ImVec2& v : points ) {
		v.x = roundf( v.x * iconScale );
		v.y = roundf( v.y * iconScale );
		v += offset;
	}

	ImU32 color = ImGui::GetColorU32( ImVec4(0.1f, 0.1f, 0.1f, 1.0f) );

	drawList->AddTriangle( points[0], points[1], points[2], color, roundf( iconScale * 4.0f ) );

	drawList->AddPolyline( points+3, 2, color, 0, roundf( iconScale * 3.0f ) );

	float dotRadius = 2.0f * iconScale;
	drawList->AddEllipseFilled( points[5], ImVec2(dotRadius, dotRadius), color, 0, 6 );

	ImGui::Indent( 40.0f * iconScale );
	ImGui::TextUnformatted( warningOverlayText.c_str() );

	ImGui::End();

	ImGui::PopStyleVar(); // WindowPadding
	ImGui::PopStyleColor(); // WindowBg
}

void ShowWarningOverlay( const char* text )
{
	warningOverlayText = text;
	warningOverlayStartTime = ImGui::GetTime();
	warningOverlayStartPos = ImGui::GetMousePos();
}

static float GetDefaultScale()
{
	if ( glConfig.winWidth != glConfig.vidWidth ) {
		// in HighDPI mode, the font sizes are already scaled (to window coordinates), apparently
		return 1.0f;
	}

#if SDL_VERSION_ATLEAST(3, 0, 0)
	float ret = SDL_GetWindowDisplayScale( sdlWindow );
#else
	float ret = ImGui_ImplSDL2_GetContentScaleForWindow( sdlWindow );
#endif

	// Validate that the reported scale is a reasonable size
	// For example: if xrandr fails to read the EDID of the display,
	// a default value 1mm x 1mm will be reported, resulting in an
	// absurdly high DPI
	if ( ret <= 0.0f || ret > 10.0f ) {
		return 1.0f;
	}

	ret = round(ret*2.0)*0.5; // round to .0 or .5
	return ret;
}

float GetScale()
{
	float ret = imgui_scale.GetFloat();
	if (ret < 0.0f) {
		ret = GetDefaultScale();
	}
	return ret;
}

void SetScale( float scale )
{
	imgui_scale.SetFloat( scale );
}

static bool imgui_initialized = false;

// using void* instead of SDL_Window and SDL_GLContext to avoid dragging SDL headers into sys_imgui.h
bool Init(void* _sdlWindow, void* sdlGlContext)
{
	common->Printf( "Initializing ImGui\n" );

	sdlWindow = (SDL_Window*)_sdlWindow;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	imguiCtx = ImGui::CreateContext();
	if ( imguiCtx == NULL ) {
		common->Warning( "Failed to create ImGui Context!\n" );
		return false;
	}
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	static idStr iniPath;
	iniPath = cvarSystem->GetCVarString( "fs_configpath" );
	iniPath += "/imgui.ini";
	io.IniFilename = iniPath.c_str();

  // Setup styles
	SetImGuiStyle( Style::Dhewm3 );
	userStyle = ImGui::GetStyle(); // set dhewm3 style as default, in case the user style is missing values
	if ( DG::ReadImGuiStyle( userStyle, GetUserStyleFilename() ) && imgui_style.GetInteger() == 2 ) {
		ImGui::GetStyle() = userStyle;
	} else if ( imgui_style.GetInteger() == 1 ) {
		ImGui::GetStyle() = ImGuiStyle();
		ImGui::StyleColorsDark();
	}

	imgui_scale.SetModified();

	// Setup fonts, size will come from style.FontSizeBase
#ifdef _MSC_VER
	io.Fonts->AddFontFromMemoryCompressedTTF(ProggyVector_compressed_data, ProggyVector_compressed_size);
#else
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(ProggyVector_compressed_data_base85);
#endif

	// Setup Platform/Renderer backends
	if ( ! ImGui_ImplSDLx_InitForOpenGL( sdlWindow, sdlGlContext ) ) {
		ImGui::DestroyContext( imguiCtx );
		imguiCtx = NULL;
		common->Warning( "Failed to initialize ImGui SDL platform backend!\n" );
		return false;
	}

	if ( ! ImGui_ImplOpenGL2_Init() ) {
		ImGui_ImplSDLx_Shutdown();
		ImGui::DestroyContext( imguiCtx );
		imguiCtx = NULL;
		common->Warning( "Failed to initialize ImGui OpenGL renderer backend!\n" );
		return false;
	}

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

	const char* f10bind = idKeyInput::GetBinding( K_F10 );
	if ( f10bind && f10bind[0] != '\0' ) {
		if ( idStr::Icmp( f10bind, "dhewm3Settings" ) != 0 ) {
			// if F10 is already bound, but not to dhewm3Settings, show a message
			common->Printf( "... the F10 key is already bound to '%s', otherwise it could be used to open the dhewm3 Settings Menu\n" , f10bind );
		}
	} else {
		idKeyInput::SetBinding( K_F10, "dhewm3Settings" );
	}

	imgui_initialized = true;
	return true;
}

void Shutdown()
{
	if ( imgui_initialized ) {
		common->Printf( "Shutting down ImGui\n" );

		// TODO: only if init was successful!
		ImGui_ImplOpenGL2_Shutdown();
		ImGui_ImplSDLx_Shutdown();
		ImGui::DestroyContext( imguiCtx );
		imgui_initialized = false;
	}
}

// NewFrame() is called once per D3 frame, after all events have been gotten
// => ProcessEvent() has already been called (probably multiple times)
void NewFrame()
{
	D3P_ScopedCPUSample(Imgui_NewFrame);
	// it can happen that NewFrame() is called without EndFrame() having been called
	// after the last NewFrame() call, for example when D3Radiant is active and in
	// idSessionLocal::UpdateScreen() Sys_IsWindowVisible() returns false.
	// In that case, end the previous frame here so it's ended at all.
	if ( haveNewFrame ) {
		EndFrame();
	}

	// even if all windows are closed, still run a few frames
	// so ImGui also recognizes internally that all windows are closed
	// and e.g. ImGuiCond_Appearing works as intended
	static int framesAfterAllWindowsClosed = 0;
	if ( openImguiWindows == 0 ) {
		if ( framesAfterAllWindowsClosed > 1 )
			return;
		else
			++framesAfterAllWindowsClosed;
	} else {
		framesAfterAllWindowsClosed = 0;
	}

	if ( imgui_scale.IsModified() ) {
		imgui_scale.ClearModified();
		float scale = imgui_scale.GetFloat();
		if (scale < 0.0f) {
			scale = GetDefaultScale();
		}

  	ImGuiStyle& style = ImGui::GetStyle();
		style.FontScaleDpi = scale;
	}

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();

	if ( ShouldShowCursor() )
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
	else
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ImGui_ImplSDLx_NewFrame();
	ImGui::NewFrame();
	haveNewFrame = true;

	UpdateWarningOverlay();

	if (openImguiWindows & D3_ImGuiWin_Settings) {
		Com_DrawDhewm3SettingsMenu();
	}

	if (openImguiWindows & D3_ImGuiWin_Demo) {
		bool show_demo_window = true;
		ImGui::ShowDemoWindow(&show_demo_window);
		if(!show_demo_window)
			CloseWindow(D3_ImGuiWin_Demo);
	}
}

bool keybindModeEnabled = false;

// called with every SDL event by Sys_GetEvent()
// returns true if ImGui has handled the event (so it shouldn't be handled by D3)
bool ProcessEvent(const void* sdlEvent)
{
	if (openImguiWindows == 0)
		return false;

	const SDL_Event* ev = (const SDL_Event*)sdlEvent;
	// ImGui_ImplSDL2_ProcessEvent() doc says:
	//   You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	//   - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
	//   - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
	//   Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

	bool imguiUsedEvent = ImGui_ImplSDLx_ProcessEvent( ev );
	if ( keybindModeEnabled ) {
		// in keybind mode, all input events are passed to Doom3 so it can translate them
		// to internal events and we can access and use them to create a new binding
		return false;
	}

	if ( ImGui::IsWindowFocused( ImGuiFocusedFlags_AnyWindow )
	    && (ev->type == SDL_CONTROLLERAXISMOTION || ev->type == SDL_CONTROLLERBUTTONDOWN) ) {
		// don't pass on controller axis events to avoid moving the mouse cursor
		// and controller button events to avoid emulating mouse clicks
		return true;
	}

	switch( ev->type ) {
		// Hack: detect if any key was pressed to close the warning overlay
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_MOUSEWHEEL:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_KEYDOWN:
			// TODO: controller trigger?
			hadKeyDownEvent = true;
	}
	if( imguiUsedEvent ) {
		ImGuiIO& io = ImGui::GetIO();

		if ( io.WantCaptureMouse ) {
			switch( ev->type ) {
				case SDL_MOUSEMOTION:
				case SDL_MOUSEWHEEL:
				case SDL_MOUSEBUTTONDOWN:

				// Note: still pass button up events to the engine, so if they were pressed down
				//   before an imgui window got focus they don't remain pressed indefinitely
				//case SDL_MOUSEBUTTONUP:
					return true;
			}
		}

		if ( io.WantCaptureKeyboard ) {
			switch( ev->type ) {
				case SDL_TEXTINPUT:
					return true;
				case SDL_KEYDOWN:
				//case SDL_KEYUP: NOTE: see above why key up events are passed to the engine
#if SDL_VERSION_ATLEAST(3, 0, 0)
					if ( ev->key.key < SDLK_F1 || ev->key.key > SDLK_F12) {
#else
					if ( ev->key.keysym.sym < SDLK_F1 || ev->key.keysym.sym > SDLK_F12) {
#endif
						// F1 - F12 are passed to the engine so its shortcuts
						// (like quickload or screenshot) still work
						// Doom3's menu does the same
						return true;
					}
			}
		}

	}

	return false;
}

void SetKeyBindMode( bool enable )
{
	keybindModeEnabled = enable;
	// make sure no keys are registered as down, neither when entering nor when exiting keybind mode
	idKeyInput::ClearStates();
}

bool ShouldShowCursor()
{
	if ( sessLocal.GetActiveMenu() == nullptr ) {
		// when ingame, render the ImGui/SDL/system cursor if an ImGui window is open
		// because dhewm3 does *not* render its own cursor outside ImGui windows.
		// additionally, only show it if an ImGui window has focus - this allows you
		// to click outside the ImGui window to give Doom3 focus and look around.
		// You can get focus on the ImGui window again by clicking while the invisible
		//  cursor is over the window (things in it still get highlighted), or by
		// opening the main (Esc) or by opening the Dhewm3 Settings window (F10, usually),
		// which will either open it focused or give an ImGui window focus if it
		// was open but unfocused.
		// TODO: Might be nice to have a keyboard shortcut to give focus to any open
		//       ImGui window, maybe Pause?
		return openImguiWindows != 0 && ImGui::IsWindowFocused( ImGuiFocusedFlags_AnyWindow );
	} else {
		// if we're in a menu (probably main menu), dhewm3 renders a cursor for it,
		// so only show the ImGui cursor when the mouse cursor is over an ImGui window
		// or in one of the black bars where Doom3's cursor isn't rendered in
		// non 4:3 resolutions
		if ( openImguiWindows == 0 ) {
			return false; // no open ImGui window => no ImGui cursor
		}
		if ( ImGui::GetIO().WantCaptureMouse ) {
			return true; // over an ImGui window => definitely want ImGui cursor
		}
		// if scaling Doom3 menus to 4:3 is enabled and the cursor is currently
		// in a black bar (Doom3 cursor is not drawn there), show the ImGui cursor
		if ( idUserInterface::IsUserInterfaceScaledTo43( sessLocal.GetActiveMenu() ) ) {
			ImVec2 mousePos = ImGui::GetMousePos();
			float w = glConfig.winWidth;
			float h = glConfig.winHeight;
			float aspectRatio = w/h;
			static const float virtualAspectRatio = float(VIRTUAL_WIDTH)/float(VIRTUAL_HEIGHT); // 4:3 = 1.333
			if(aspectRatio > 1.4f) {
				// widescreen (4:3 is 1.333 3:2 is 1.5, 16:10 is 1.6, 16:9 is 1.7778)
				// => we need to scale and offset w to get the width of the black bars
				float scaleX = virtualAspectRatio/aspectRatio;
				float offsetX = (1.0f - scaleX) * w * 0.5f; // (w - scale*w)/2
				if ( mousePos.x <= offsetX || mousePos.x >= w - offsetX ) {
					return true;
				}
			} else if(aspectRatio < 1.24f) {
				// portrait-mode, "thinner" than 5:4 (which is 1.25)
				// => we need to scale and offset h to get the height of the black bars
				// it's analogue to the other case, but inverted and with height and Y
				float scaleY = aspectRatio/virtualAspectRatio;
				float offsetY = (1.0f - scaleY)* h * 0.5f; // (h - scale*h)/2
				if ( mousePos.y <= offsetY || mousePos.y >= h - offsetY ) {
					return true;
				}
			}
		}
		return false;
	}
}

void EndFrame()
{
	if (openImguiWindows == 0 && !haveNewFrame)
		return;

	// I think this can happen if we're not coming from idCommon::Frame() but screenshot or sth
	if ( !haveNewFrame ) {
		NewFrame();
	}
	haveNewFrame = false;
	ImGui::Render();

	// Doom3 uses the OpenGL ARB shader extensions, for most things it renders.
	// disable those shaders, the OpenGL classic integration of ImGui doesn't use shaders
	qglDisable( GL_VERTEX_PROGRAM_ARB );
	qglDisable( GL_FRAGMENT_PROGRAM_ARB );

	// Doom3 uses OpenGL's ARB_vertex_buffer_object extension to use VBOs on the GPU
	// as buffers for glDrawElements() (instead of passing userspace buffers to that function)
	// ImGui however uses userspace buffers, so remember the currently bound VBO
	// and unbind it (after drawing, bind it again)
	GLint curArrayBuffer = 0;
	if ( glConfig.ARBVertexBufferObjectAvailable ) {
		qglGetIntegerv( GL_ARRAY_BUFFER_BINDING_ARB, &curArrayBuffer );
		qglBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
	}

	// disable all texture units, ImGui_ImplOpenGL2_RenderDrawData() will enable texture 0
	// and bind its own textures to it as needed
	for ( int i = glConfig.maxTextureUnits - 1 ; i >= 0 ; i-- ) {
		GL_SelectTexture( i );
		qglDisable( GL_TEXTURE_2D );
		if ( glConfig.texture3DAvailable ) {
			qglDisable( GL_TEXTURE_3D );
		}
		if ( glConfig.cubeMapAvailable ) {
			qglDisable( GL_TEXTURE_CUBE_MAP_EXT );
		}
	}

	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );

	if ( curArrayBuffer != 0 ) {
		qglBindBufferARB( GL_ARRAY_BUFFER_ARB, curArrayBuffer );
	}

	// reset this at the end of each frame, will be set again by ProcessEvent()
	if ( hadKeyDownEvent ) {
		hadKeyDownEvent = false;
	}
}


void OpenWindow( D3ImGuiWindow win )
{
	if ( openImguiWindows & win )
		return; // already open

	ImGui::SetNextWindowFocus();

	switch ( win ) {
		case D3_ImGuiWin_Settings:
			Com_OpenCloseDhewm3SettingsMenu( true );
			break;
		// TODO: other windows that need explicit opening
	}

	openImguiWindows |= win;
}

void CloseWindow( D3ImGuiWindow win )
{
	if ( (openImguiWindows & win) == 0 )
		return; // already closed

	switch ( win ) {
		case D3_ImGuiWin_Settings:
			Com_OpenCloseDhewm3SettingsMenu( false );
			break;
		// TODO: other windows that need explicit closing
	}

	openImguiWindows &= ~win;
}

int GetOpenWindowsMask()
{
	return openImguiWindows;
}

ImGuiStyle GetImGuiStyle( Style d3style )
{
	ImGuiStyle style; // default style
	if ( d3style == Style::Dhewm3 ) {
		// make it look a bit nicer with rounded edges
		style.WindowRounding = 2.0f;
		style.FrameRounding = 3.0f;
		style.FramePadding = ImVec2( 6.0f, 3.0f );
		//style.ChildRounding = 6.0f;
		style.ScrollbarRounding = 8.0f;
		style.GrabRounding = 3.0f;
		style.PopupRounding = 2.0f;
		SetDhewm3StyleColors( &style );
	} else if ( d3style == Style::User ) {
		style = userStyle;
	} else {
		assert( d3style == Style::ImGui_Default && "invalid/unknown style" );
		ImGui::StyleColorsDark( &style );
	}
	return style;
}

void SetImGuiStyle( Style d3style )
{
	ImGui::GetStyle() = GetImGuiStyle( d3style );
}

void SetDhewm3StyleColors( ImGuiStyle* dst )
{
	if ( dst == nullptr )
		dst = &ImGui::GetStyle();
	ImGui::StyleColorsDark( dst );
	ImVec4* colors = dst->Colors;
	//colors[ImGuiCol_TitleBg]       = ImVec4(0.28f, 0.36f, 0.48f, 0.88f);
	//colors[ImGuiCol_TitleBg]       = ImVec4(0.09f, 0.23f, 0.22f, 0.90f);
	//colors[ImGuiCol_TitleBgActive] = ImVec4(0.02f, 0.52f, 0.53f, 1.00f);
	colors[ImGuiCol_TitleBg]       = ImVec4(0.09f, 0.13f, 0.12f, 0.90f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.03f, 0.33f, 0.33f, 1.00f);
	//colors[ImGuiCol_TitleBg]       = ImVec4(0.12f, 0.17f, 0.16f, 0.90f);
	colors[ImGuiCol_TabHovered]    = ImVec4(0.42f, 0.69f, 1.00f, 0.80f);
	colors[ImGuiCol_TabSelected]     = ImVec4(0.24f, 0.51f, 0.83f, 1.00f);
}

void SetUserStyleColors()
{
	ImGuiStyle& style = ImGui::GetStyle();
	for ( int i=0; i < ImGuiCol_COUNT; ++i ) {
		style.Colors[i] = userStyle.Colors[i];
	}
}

bool WriteUserStyle()
{
	userStyle = ImGui::GetStyle();
	if ( !DG::WriteImGuiStyle( ImGui::GetStyle(), GetUserStyleFilename() ) ) {
		common->Warning( "Couldn't write ImGui userstyle!\n" );
		return false;
	}
	return true;
}

void CopyCurrentStyle( bool onlyChanges )
{
	ImGuiStyle refStyle = GetImGuiStyle( Style::ImGui_Default );
	ImGuiTextBuffer buf = DG::WriteImGuiStyleToCode( ImGui::GetStyle(), onlyChanges ? &refStyle : nullptr );
	Sys_SetClipboardData( buf.c_str() );
}

}} //namespace D3::ImGuiHooks
