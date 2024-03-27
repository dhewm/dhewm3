
#if 1 // FIXME: introduce D3_ENABLE_IMGUI or similar from cmake? or use IMGUI_DISABLE? either way, no imgui for SDL1.2

#include "sys_imgui.h"

#include <SDL.h>

#include "../libs/imgui/backends/imgui_impl_opengl2.h"
#include "../libs/imgui/backends/imgui_impl_sdl2.h"

#include "framework/Common.h"
#include "renderer/qgl.h"


namespace D3 {
namespace ImGuiHooks {

ImGuiContext* imguiCtx = NULL;
static bool haveNewFrame = false;

// using void* instead of SDL_Window and SDL_GLContext to avoid dragging SDL headers into sys_imgui.h
bool Init(void* sdlWindow, void* sdlGlContext)
{
	SDL_Window* win = (SDL_Window*)sdlWindow;
	SDL_GL_MakeCurrent(win, (SDL_GLContext)sdlGlContext); // XXX: really?
	//SDL_GLContext glctx = (SDL_GLContext)sdlGlContext;
	SDL_GL_SetSwapInterval(1); // XXX: really?

	common->Printf( "Initializing ImGui\n" );

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	imguiCtx = ImGui::CreateContext();
	if (imguiCtx == NULL) {
		common->Warning("Failed to create ImGui Context!\n");
		return false;
	}
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	if (!ImGui_ImplSDL2_InitForOpenGL(win, sdlGlContext)) {
		ImGui::DestroyContext(imguiCtx);
		imguiCtx = NULL;
		common->Warning("Failed to initialize ImGui SDL platform backend!\n");
		return false;
	}

	if (!ImGui_ImplOpenGL2_Init()) {
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext(imguiCtx);
		imguiCtx = NULL;
		common->Warning("Failed to initialize ImGui OpenGL renderer backend!\n");
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

	return true;
}

void Shutdown()
{
	common->Printf( "Shutting down ImGui\n" );

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext(imguiCtx);
}

void NewFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	haveNewFrame = true;

	// XXX hack for testing:
	static bool show_another_window = false;
	static bool show_demo_window = true;

	if(show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);


	{
		static float f = 0.0f;
		static int counter = 0;

		ImGuiIO& io = ImGui::GetIO();

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	if(show_another_window) {
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
}

bool ProcessEvent(const void* sdlEvent)
{
	return ImGui_ImplSDL2_ProcessEvent( (const SDL_Event*)sdlEvent );
}

void EndFrame()
{
	// I think this can happen if we're not coming from idCommon::Frame() but screenshot or sth
	if (!haveNewFrame) {
		NewFrame();
	}
	haveNewFrame = false;
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	//qglViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	//qglClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	//qglClear(GL_COLOR_BUFFER_BIT);

	qglDisable( GL_VERTEX_PROGRAM_ARB );
	qglDisable( GL_FRAGMENT_PROGRAM_ARB );

	GLint curArrayBuffer = 0, curElemBuffer = 0;
	qglGetIntegerv( GL_ARRAY_BUFFER_BINDING_ARB, &curArrayBuffer );
	qglGetIntegerv( GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &curElemBuffer );

	qglBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
	qglBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );


	ImDrawData* drawData = ImGui::GetDrawData();
	ImGui_ImplOpenGL2_RenderDrawData( drawData );

	if( curArrayBuffer != 0 ) {
		qglBindBufferARB( GL_ARRAY_BUFFER_ARB, curArrayBuffer );
	}
	if( curElemBuffer != 0 ) {
		qglBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, curElemBuffer );
	}
}

}} //namespace D3::ImGuiHooks

#endif // D3_ENABLE_IMGUI
