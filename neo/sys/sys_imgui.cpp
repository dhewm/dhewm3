
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
#include "renderer/qgl.h"
#include "renderer/tr_local.h" // glconfig

extern void Com_DrawDhewm3SettingsMenu(); // in framework/dhewm3SettingsMenu.cpp

static idCVar imgui_scale( "imgui_scale", "-1.0", CVAR_SYSTEM|CVAR_FLOAT|CVAR_ARCHIVE, "factor to scale ImGUI menus by (-1: auto)" ); // TODO: limit values?

namespace D3 {
namespace ImGuiHooks {

#include "proggyvector_font.h"

static SDL_Window* sdlWindow = NULL;
ImGuiContext* imguiCtx = NULL;
static bool haveNewFrame = false;
static int openImguiWindows = 0; // or-ed enum D3ImGuiWindow values

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

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// make it a bit prettier with rounded edges
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 2.0f;
	style.FrameRounding = 3.0f;
	//style.ChildRounding = 6.0f;
	style.ScrollbarRounding = 8.0f;
	style.GrabRounding = 1.0f;
	style.PopupRounding = 2.0f;

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_TitleBg]                = ImVec4(0.28f, 0.36f, 0.48f, 0.88f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.42f, 0.69f, 1.00f, 0.80f);
	colors[ImGuiCol_TabActive]              = ImVec4(0.24f, 0.51f, 0.83f, 1.00f);

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
	if( ImGui_ImplSDL2_ProcessEvent( ev ) ) {
		ImGuiIO& io = ImGui::GetIO();
		if ( io.WantCaptureMouse ) {
			switch( ev->type ) {
				case SDL_MOUSEMOTION:
				case SDL_MOUSEWHEEL:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					return true;
			}
		}

		if ( io.WantCaptureKeyboard ) {
			switch( ev->type ) {
				case SDL_TEXTINPUT:
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					return true;
			}
		}
	}
	return false;
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

}} //namespace D3::ImGuiHooks
