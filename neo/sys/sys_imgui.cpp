
#define IMGUI_DEFINE_MATH_OPERATORS

#include <SDL.h>

#include "sys_imgui.h"

#ifdef D3_SDL_X11
#include <dlfcn.h>
#include <SDL_syswm.h>
//char *XGetDefault(Display* display, const char*	program, const char* option)
typedef char* (*MY_XGETDEFAULTFUN)(Display*, const char*, const char*);
#endif

#include "../libs/imgui/backends/imgui_impl_opengl2.h"
#include "../libs/imgui/backends/imgui_impl_sdl2.h"

#include "framework/Common.h"
#include "framework/KeyInput.h"
#include "renderer/qgl.h"
#include "renderer/tr_local.h" // glconfig

extern void Com_DrawDhewm3SettingsMenu(); // in framework/dhewm3SettingsMenu.cpp

static idCVar imgui_scale( "imgui_scale", "-1.0", CVAR_SYSTEM|CVAR_FLOAT|CVAR_ARCHIVE, "factor to scale ImGUI menus by (-1: auto)" ); // TODO: limit values?

idCVar imgui_style( "imgui_style", "0", CVAR_SYSTEM|CVAR_INTEGER|CVAR_ARCHIVE, "Which ImGui style to use. 0: Dhewm3 theme, 1: Default ImGui theme, 2: User theme", 0.0f, 2.0f );

namespace D3 {
namespace ImGuiHooks {

#include "proggyvector_font.h"

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

bool WriteStyle( const ImGuiStyle& s, const char* filename );
bool ReadStyle( ImGuiStyle& s, const char* filename );

idStr GetUserStyleFilename()
{
	// TODO: put this into the config dir
	return idStr( "user.imstyle" );
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
	ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4(1.0f, 0.4f, 0.4f, 0.4f) );
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


static float GetDefaultDPI()
{
	SDL_Window* win = sdlWindow;
	float dpi = -1.0f;
#ifdef D3_SDL_X11
	SDL_SysWMinfo wmInfo = {};
	SDL_VERSION(&wmInfo.version)
	if(SDL_GetWindowWMInfo(win, &wmInfo) && wmInfo.subsystem == SDL_SYSWM_X11) {
		Display* display = wmInfo.info.x11.display;

		static void* libX11 = NULL;
		if(libX11 == NULL) {
			libX11 = dlopen("libX11.so.6", RTLD_LAZY);
		}
		if(libX11 == NULL) {
			libX11 = dlopen("libX11.so", RTLD_LAZY);
		}
		if(libX11 != NULL) {
			MY_XGETDEFAULTFUN my_xgetdefault = (MY_XGETDEFAULTFUN)dlsym(libX11, "XGetDefault");
			if(my_xgetdefault != NULL) {
				//char *XGetDefault(Display* display, const char*	program, const char* option)
				const char* dpiStr = my_xgetdefault(display, "Xft", "dpi");
				printf("XX dpistr = '%s'\n", dpiStr);
				if(dpiStr != NULL) {
					dpi = atof(dpiStr);
				}
			}
		}
	}
	if (dpi == -1.0f)
#endif
	{
		int winIdx = SDL_GetWindowDisplayIndex( win );
		if (winIdx >= 0) {
			SDL_GetDisplayDPI(winIdx, NULL, &dpi, NULL);
		}
	}
	return dpi;
}

static float GetDefaultScale()
{
	float ret = GetDefaultDPI() / 96.0f;
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
	ImGuiIO& io = ImGui::GetIO();
	float realScale = (scale < 0.0f) ? GetDefaultScale() : scale;
	io.FontGlobalScale = realScale;
	// TODO: could instead set fontsize to 18.0f * scale
	//  (io.Fonts->ClearFonts() and then add again with new size - but must be done before NewFrame() / after EndFrame())
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
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	SetImGuiStyle( Style::Dhewm3 );
	userStyle = ImGui::GetStyle(); // set dhewm3 style as default, in case the user style is missing values
	if ( ReadStyle( userStyle, GetUserStyleFilename() ) && imgui_style.GetInteger() == 2 ) {
		ImGui::GetStyle() = userStyle;
	} else if ( imgui_style.GetInteger() == 1 ) {
		ImGui::GetStyle() = ImGuiStyle();
		ImGui::StyleColorsDark();
	}

	ImFontConfig fontCfg;
	strcpy(fontCfg.Name, "ProggyVector");
	ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(ProggyVector_compressed_data, ProggyVector_compressed_size, 18.0f, nullptr);

	SetScale( GetScale() );

	// Setup Platform/Renderer backends
	if ( ! ImGui_ImplSDL2_InitForOpenGL( sdlWindow, sdlGlContext ) ) {
		ImGui::DestroyContext( imguiCtx );
		imguiCtx = NULL;
		common->Warning( "Failed to initialize ImGui SDL platform backend!\n" );
		return false;
	}

	if ( ! ImGui_ImplOpenGL2_Init() ) {
		ImGui_ImplSDL2_Shutdown();
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
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext( imguiCtx );
		imgui_initialized = false;
	}
}

// NewFrame() is called once per D3 frame, after all events have been gotten
// => ProcessEvent() has already been called (probably multiple times)
void NewFrame()
{
	if (openImguiWindows == 0)
		return;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
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

	bool imguiUsedEvent = ImGui_ImplSDL2_ProcessEvent( ev );
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
					if ( ev->key.keysym.sym < SDLK_F1 || ev->key.keysym.sym > SDLK_F12) {
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
	openImguiWindows |= win;
}

void CloseWindow( D3ImGuiWindow win )
{
	openImguiWindows &= ~win;
}

int GetOpenWindowsMask()
{
	return openImguiWindows;
}

void SetImGuiStyle( Style d3style )
{
	ImGuiStyle& style = ImGui::GetStyle();
	if ( d3style == Style::Dhewm3 ) {
		style = ImGuiStyle(); // start with default style
		// make it look a bit nicer with rounded edges
		style.WindowRounding = 2.0f;
		style.FrameRounding = 3.0f;
		//style.ChildRounding = 6.0f;
		style.ScrollbarRounding = 8.0f;
		style.GrabRounding = 1.0f;
		style.PopupRounding = 2.0f;
		SetDhewm3StyleColors();
	} else if ( d3style == Style::User ) {
		style = userStyle;
	} else {
		assert( d3style == Style::ImGui_Default && "invalid/unknown style" );
		style = ImGuiStyle();
		ImGui::StyleColorsDark();
	}
}

void SetDhewm3StyleColors()
{
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_TitleBg]    = ImVec4(0.28f, 0.36f, 0.48f, 0.88f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.69f, 1.00f, 0.80f);
	colors[ImGuiCol_TabActive]  = ImVec4(0.24f, 0.51f, 0.83f, 1.00f);
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
	if( !WriteStyle( ImGui::GetStyle(), GetUserStyleFilename() ) ) {
		common->Warning( "Couldn't write ImGui userstyle!\n" );
		return false;
	}
	return true;
}

// Note: The trick I'm using with these #defines below is called "X Macro"
//       see https://en.wikipedia.org/wiki/X_macro (except I'm not calling the "entries" X)

#define D3_IMSTYLE_ATTRS  \
	D3_IMATTR_FLOAT( Alpha                       ) \
	D3_IMATTR_FLOAT( DisabledAlpha               ) \
	D3_IMATTR_VEC2(  WindowPadding               ) \
	D3_IMATTR_FLOAT( WindowRounding              ) \
	D3_IMATTR_FLOAT( WindowBorderSize            ) \
	D3_IMATTR_VEC2(  WindowMinSize               ) \
	D3_IMATTR_VEC2(  WindowTitleAlign            ) \
	D3_IMATTR_INT(   WindowMenuButtonPosition    ) \
	D3_IMATTR_FLOAT( ChildRounding               ) \
	D3_IMATTR_FLOAT( ChildBorderSize             ) \
	D3_IMATTR_FLOAT( PopupRounding               ) \
	D3_IMATTR_FLOAT( PopupBorderSize             ) \
	D3_IMATTR_VEC2(  FramePadding                ) \
	D3_IMATTR_FLOAT( FrameRounding               ) \
	D3_IMATTR_FLOAT( FrameBorderSize             ) \
	D3_IMATTR_VEC2(  ItemSpacing                 ) \
	D3_IMATTR_VEC2(  ItemInnerSpacing            ) \
	D3_IMATTR_VEC2(  CellPadding                 ) \
	D3_IMATTR_VEC2(  TouchExtraPadding           ) \
	D3_IMATTR_FLOAT( IndentSpacing               ) \
	D3_IMATTR_FLOAT( ColumnsMinSpacing           ) \
	D3_IMATTR_FLOAT( ScrollbarSize               ) \
	D3_IMATTR_FLOAT( ScrollbarRounding           ) \
	D3_IMATTR_FLOAT( GrabMinSize                 ) \
	D3_IMATTR_FLOAT( GrabRounding                ) \
	D3_IMATTR_FLOAT( LogSliderDeadzone           ) \
	D3_IMATTR_FLOAT( TabRounding                 ) \
	D3_IMATTR_FLOAT( TabBorderSize               ) \
	D3_IMATTR_FLOAT( TabMinWidthForCloseButton   ) \
	D3_IMATTR_FLOAT( TabBarBorderSize            ) \
	D3_IMATTR_FLOAT( TableAngledHeadersAngle     ) \
	D3_IMATTR_VEC2(  TableAngledHeadersTextAlign ) \
	D3_IMATTR_INT(   ColorButtonPosition         ) \
	D3_IMATTR_VEC2(  ButtonTextAlign             ) \
	D3_IMATTR_VEC2(  SelectableTextAlign         ) \
	D3_IMATTR_FLOAT( SeparatorTextBorderSize     ) \
	D3_IMATTR_VEC2(  SeparatorTextAlign          ) \
	D3_IMATTR_VEC2(  SeparatorTextPadding        ) \
	D3_IMATTR_VEC2(  DisplayWindowPadding        ) \
	D3_IMATTR_VEC2(  DisplaySafeAreaPadding      ) \
	D3_IMATTR_FLOAT( MouseCursorScale            ) \
	D3_IMATTR_BOOL(  AntiAliasedLines            ) \
	D3_IMATTR_BOOL(  AntiAliasedLinesUseTex      ) \
	D3_IMATTR_BOOL(  AntiAliasedFill             ) \
	D3_IMATTR_FLOAT( CurveTessellationTol        ) \
	D3_IMATTR_FLOAT( CircleTessellationMaxError  )

#define D3_IMSTYLE_BEHAVIORS \
	D3_IMATTR_FLOAT( HoverStationaryDelay        ) \
	D3_IMATTR_FLOAT( HoverDelayShort             ) \
	D3_IMATTR_FLOAT( HoverDelayNormal            ) \
	D3_IMATTR_INT(   HoverFlagsForTooltipMouse   ) \
	D3_IMATTR_INT(   HoverFlagsForTooltipNav     )

#define D3_IMSTYLE_COLORS \
	D3_IMSTYLE_COLOR( Text                  ) \
	D3_IMSTYLE_COLOR( TextDisabled          ) \
	D3_IMSTYLE_COLOR( WindowBg              ) \
	D3_IMSTYLE_COLOR( ChildBg               ) \
	D3_IMSTYLE_COLOR( PopupBg               ) \
	D3_IMSTYLE_COLOR( Border                ) \
	D3_IMSTYLE_COLOR( BorderShadow          ) \
	D3_IMSTYLE_COLOR( FrameBg               ) \
	D3_IMSTYLE_COLOR( FrameBgHovered        ) \
	D3_IMSTYLE_COLOR( FrameBgActive         ) \
	D3_IMSTYLE_COLOR( TitleBg               ) \
	D3_IMSTYLE_COLOR( TitleBgActive         ) \
	D3_IMSTYLE_COLOR( TitleBgCollapsed      ) \
	D3_IMSTYLE_COLOR( MenuBarBg             ) \
	D3_IMSTYLE_COLOR( ScrollbarBg           ) \
	D3_IMSTYLE_COLOR( ScrollbarGrab         ) \
	D3_IMSTYLE_COLOR( ScrollbarGrabHovered  ) \
	D3_IMSTYLE_COLOR( ScrollbarGrabActive   ) \
	D3_IMSTYLE_COLOR( CheckMark             ) \
	D3_IMSTYLE_COLOR( SliderGrab            ) \
	D3_IMSTYLE_COLOR( SliderGrabActive      ) \
	D3_IMSTYLE_COLOR( Button                ) \
	D3_IMSTYLE_COLOR( ButtonHovered         ) \
	D3_IMSTYLE_COLOR( ButtonActive          ) \
	D3_IMSTYLE_COLOR( Header                ) \
	D3_IMSTYLE_COLOR( HeaderHovered         ) \
	D3_IMSTYLE_COLOR( HeaderActive          ) \
	D3_IMSTYLE_COLOR( Separator             ) \
	D3_IMSTYLE_COLOR( SeparatorHovered      ) \
	D3_IMSTYLE_COLOR( SeparatorActive       ) \
	D3_IMSTYLE_COLOR( ResizeGrip            ) \
	D3_IMSTYLE_COLOR( ResizeGripHovered     ) \
	D3_IMSTYLE_COLOR( ResizeGripActive      ) \
	D3_IMSTYLE_COLOR( Tab                   ) \
	D3_IMSTYLE_COLOR( TabHovered            ) \
	D3_IMSTYLE_COLOR( TabActive             ) \
	D3_IMSTYLE_COLOR( TabUnfocused          ) \
	D3_IMSTYLE_COLOR( TabUnfocusedActive    ) \
	D3_IMSTYLE_COLOR( PlotLines             ) \
	D3_IMSTYLE_COLOR( PlotLinesHovered      ) \
	D3_IMSTYLE_COLOR( PlotHistogram         ) \
	D3_IMSTYLE_COLOR( PlotHistogramHovered  ) \
	D3_IMSTYLE_COLOR( TableHeaderBg         ) \
	D3_IMSTYLE_COLOR( TableBorderStrong     ) \
	D3_IMSTYLE_COLOR( TableBorderLight      ) \
	D3_IMSTYLE_COLOR( TableRowBg            ) \
	D3_IMSTYLE_COLOR( TableRowBgAlt         ) \
	D3_IMSTYLE_COLOR( TextSelectedBg        ) \
	D3_IMSTYLE_COLOR( DragDropTarget        ) \
	D3_IMSTYLE_COLOR( NavHighlight          ) \
	D3_IMSTYLE_COLOR( NavWindowingHighlight ) \
	D3_IMSTYLE_COLOR( NavWindowingDimBg     ) \
	D3_IMSTYLE_COLOR( ModalWindowDimBg      )


bool WriteStyle( const ImGuiStyle& s, const char* filename ) {
	FILE* f = fopen( filename, "w" ); // TODO: "wt" on Windows?
	if ( f == nullptr ) {

		return false;
	}

	fprintf( f, "[style]\n" );

#define D3_IMATTR_FLOAT( NAME ) \
	fprintf( f, #NAME " = %g\n", s . NAME );
#define D3_IMATTR_VEC2( NAME ) \
	fprintf( f, #NAME " = %g, %g\n", s . NAME . x, s . NAME . y );
#define D3_IMATTR_INT( NAME ) \
	fprintf( f, #NAME " = %d\n", s . NAME );
#define D3_IMATTR_BOOL( NAME ) \
	fprintf( f, #NAME " = %d\n", (int) ( s . NAME ) );

	// this (together with the D3_IMATTR_* defines in the previous lines)
	// extends the D3_IMSTYLE_ATTRS table to
	//  fprintf( f, "Alpha = %f\n", s . Alpha );
	//  fprintf( f, "DisabledAlpha = %f\n", s . DisabledAlpha );
	//  fprintf( f, "WindowPadding = %f %f\n", s . WindowPadding . x, s . WindowPadding . y );
	// etc
	D3_IMSTYLE_ATTRS

	fprintf( f, "\n[behaviors]\n" );

	// same for behaviors
	D3_IMSTYLE_BEHAVIORS

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_BOOL

	fprintf( f, "\n[colors]\n" );

#define D3_IMSTYLE_COLOR( NAME ) { \
		const ImVec4& c = s.Colors[ ImGuiCol_ ## NAME  ]; \
		fprintf( f, #NAME " = %g, %g, %g, %g\n", c.x, c.y, c.z, c.w ); \
	}

	// this turns into
	//  { const ImVec4& c = s.Colors[ ImGuiCol_Text ]; fprintf( f, "Text = %f %f %f %f\n", c.x, c.y, c.z, c.w ); }
	//  { const ImVec4& c = s.Colors[ ImGuiCol_TextDisabled ]; fprintf( f, "TextDisabled = %f %f %f %f\n", c.x, c.y, c.z, c.w ); }
	// etc
	D3_IMSTYLE_COLORS

#undef D3_IMSTYLE_COLOR

	fprintf( f, "\n" );
	fflush( f );
	fclose( f );

	return true;
}

static inline char* skipWhitespace( const char* s ) {
	while( *s == ' ' || *s == '\t' )
		++s;
	return (char*)s;
}

#define D3_IMATTR_FLOAT( NAME ) \
	if ( sscanf( line, #NAME " = %f", &f1 ) == 1 ) { \
		s . NAME = f1; \
		return; \
	}

#define D3_IMATTR_VEC2( NAME )  \
	if ( sscanf( line, #NAME " = %f , %f", &f1, &f2 ) == 2 ) { \
		s . NAME .x = f1; s . NAME .y = f2; \
		return; \
	}

#define D3_IMATTR_INT( NAME )   \
	if ( sscanf( line, #NAME " = %d", &i ) == 1 ) { \
		s . NAME = i; \
		return; \
	}

#define D3_IMATTR_BOOL( NAME )  \
	if ( sscanf( line, #NAME " = %d", &i ) == 1 ) { \
		s . NAME = ( i != 0 ); \
		return; \
	}


static void parseStyleLine( ImGuiStyle& s, const char* line )
{
	float f1=0, f2=0;
	int i=0;

	D3_IMSTYLE_ATTRS

	common->Warning( "Invalid line in ImGui style under [style] section: '%s'\n", line );
}

static void parseBehaviorLine( ImGuiStyle& s, const char* line )
{
	float f1=0, f2=0;
	(void)f2; // currently unused in behavior section
	int i=0;

	D3_IMSTYLE_BEHAVIORS

	common->Warning( "Invalid line in ImGui style under [behaviors] section: '%s'\n", line );
}

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_BOOL

static void parseColorLine( ImGuiStyle& s, const char* line )
{
	ImVec4 c;

#define D3_IMSTYLE_COLOR( NAME ) \
	if ( sscanf( line, #NAME " = %f , %f , %f , %f", &c.x, &c.y, &c.z, &c.w) == 4 ) { \
		s.Colors[ ImGuiCol_ ## NAME ] = c; \
		return; \
	}

	D3_IMSTYLE_COLORS

#undef D3_IMSTYLE_COLOR

	common->Warning( "Invalid line in ImGui style under [colors] section: '%s'\n", line );

}

bool ReadStyle( ImGuiStyle& s, const char* filename )
{
	FILE* f = fopen( filename, "r" ); // TODO: "rt" on Windows?
	if ( f == nullptr ) {
		return false;
	}

	char lineBuf[256];

	int section = -1; // 0: style, 1: behaviors, 2: colors

	for ( char* line = fgets( lineBuf, sizeof(lineBuf), f );
		line != nullptr;
		line = fgets( lineBuf, sizeof(lineBuf), f ) )
	{
		// skip whitespace, if any
		line = skipWhitespace(line);
		if ( *line == '#' ) // skip comment lines
			continue;
		if ( *line == '[' ) { // start of a section
			const char* secStr = line+1; // skip '['
			secStr = skipWhitespace(secStr);
			// "[style]" "[behaviors]" "[colors]"
			if ( idStr::Icmpn(secStr, "style", 5) == 0 ) {
				section = 0;
			} else if ( idStr::Icmpn(secStr, "behaviors", 9) == 0 ) {
				section = 1;
			} else if ( idStr::Icmpn(secStr, "colors", 6) == 0 ) {
				section = 2;
			} else {
				common->Warning( "Invalid line that looks like a section in ImGui style: '%s'\n", line );
			}
			continue;
		}
		if ( *line == '\r' || *line == '\n' ) {
			continue; // empty line
		}
		if ( section == 0 ) {
			parseStyleLine( s, line );
		} else if ( section == 1 ) {
			parseBehaviorLine( s, line );
		} else if ( section == 2 ) {
			parseColorLine( s, line );
		} else {
			common->Warning( "Invalid line in ImGui before start of any section: '%s'\n", line );
		}
	}

	return true;
}

// check correctness of the X macros above (detect when something is added to ImGuiStyle)
namespace {

#define D3_IMSTYLE_COLOR(C)   + 1
// "0 D3_IMSTYLE_COLORS" is expanded to 0 + 1 + 1 ... for each D3_IMSTYLE_COLOR entry
// => it should have the same value as the ImGuiCol_COUNT constant
static_assert( ImGuiCol_COUNT == 0 D3_IMSTYLE_COLORS,
		"something was added or removed in enum ImGuiCol_ => adjust D3_IMSTYLE_COLORS table above" );
#undef D3_IMSTYLE_COLOR

// recreate struct ImGuiStyle from the tables above and see if they're identical
// (this struct is only used for the static assertions below)
struct D3_ImGuiStyle_Check {

#define D3_IMATTR_FLOAT( NAME ) float NAME ;
#define D3_IMATTR_VEC2( NAME )  ImVec2 NAME ;
#define D3_IMATTR_INT( NAME )   int NAME ;
#define D3_IMATTR_BOOL( NAME )  bool NAME ;

	// this expands to all the ImGuiStyle members, up to (excluding) Colors
	// exactly like in ImGuiStyle (except the pseudo-enums like ImGuiDir are plain ints here)
	D3_IMSTYLE_ATTRS

	ImVec4 Colors[ImGuiCol_COUNT];

	// just like the other members/attributes, expand the behaviors
	D3_IMSTYLE_BEHAVIORS

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_BOOL

};

template<class T, class U>
struct is_same {
	static constexpr bool value = false;
};

template<class T>
struct is_same<T, T> {
	static constexpr bool value = true;
};

#define D3_IMATTR_FLOAT( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), float >::value, "expected member " #NAME "to be a float - adjust the list!" );

#define D3_IMATTR_VEC2( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), ImVec2 >::value, "expected member " #NAME " to be an ImVec2 - adjust the list!" );

#define D3_IMATTR_INT( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), int >::value, "expected member " #NAME " to be an int - adjust the list!" );

#define D3_IMATTR_BOOL( NAME ) \
	static_assert( offsetof( ImGuiStyle, NAME ) == offsetof( D3_ImGuiStyle_Check, NAME ), \
			"member " #NAME " not at expected offset - is the member before it missing from the list, or moved to another position?" ); \
	static_assert( is_same< decltype( ImGuiStyle :: NAME ), bool >::value, "expected member " #NAME " to be a bool - adjust the list!" );

// expanding those static assertions for offset and type for all attributes and behaviors

D3_IMSTYLE_ATTRS

D3_IMSTYLE_BEHAVIORS

#undef D3_IMATTR_FLOAT
#undef D3_IMATTR_VEC2
#undef D3_IMATTR_INT
#undef D3_IMATTR_BOOL

static_assert( offsetof( ImGuiStyle, Colors ) == offsetof( D3_ImGuiStyle_Check, Colors ), "member Colors not at expected offset" );

// if all other static assertions passed and the following failed, probably a member was added at the end of the ImGuiStyle struct
static_assert( sizeof(D3_ImGuiStyle_Check) == sizeof(ImGuiStyle),
		"something seems to be missing or wrong type in D3_IMSTYLE_ATTRS or D3_IMSTYLE_BEHAVIORS" );

} //anon namespace

}} //namespace D3::ImGuiHooks
