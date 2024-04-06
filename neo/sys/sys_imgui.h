

#ifndef NEO_SYS_SYS_IMGUI_H_
#define NEO_SYS_SYS_IMGUI_H_

#include "../libs/imgui/imgui.h"

namespace D3 {
namespace ImGuiHooks {

enum D3ImGuiWindow {
	D3_ImGuiWin_None        = 0,
	D3_ImGuiWin_Settings    = 1, // advanced dhewm3 settings menu
	D3_ImGuiWin_Demo        = 2, // ImGui demo window
	// next should be 4, then 8, etc so a bitmask can be used
};

#ifndef IMGUI_DISABLE

extern ImGuiContext* imguiCtx; // this is only here so IsImguiEnabled() can use it inline

inline bool IsImguiEnabled()
{
	return imguiCtx != NULL;
}

// using void* instead of SDL_Window and SDL_GLContext to avoid dragging SDL headers into sys_imgui.h
extern bool Init(void* sdlWindow, void* sdlGlContext);

extern void Shutdown();

extern void OpenWindow( D3ImGuiWindow win );

extern void CloseWindow( D3ImGuiWindow win );

// called with every SDL event by Sys_GetEvent()
// returns true if ImGui has handled the event (so it shouldn't be handled by D3)
extern bool ProcessEvent(const void* sdlEvent);

// NewFrame() is called once per D3 frame, after all events have been gotten
// => ProcessEvent() has already been called (probably multiple times)
extern void NewFrame();

// called at the end of the D3 frame, when all other D3 rendering is done
// renders ImGui menus then
extern void EndFrame();

extern float GetDefaultDPI();

#else // IMGUI_DISABLE - just stub out everything

inline bool IsImguiEnabled()
{
	return false;
}

// using void* instead of SDL_Window and SDL_GLContext to avoid dragging SDL headers into sys_imgui.h
inline bool Init(void* sdlWindow, void* sdlGlContext)
{
	return false;
}

inline void Shutdown() {}

inline bool ProcessEvent(const void* sdlEvent) { return false; }

inline void NewFrame() {}

inline void EndFrame() {}

inline void OpenWindow( D3ImGuiWindow win ) {}

inline void CloseWindow( D3ImGuiWindow win ) {}

inline float GetDefaultDPI() { return 96.0f; }

#endif

}} //namespace D3::ImGuiHooks


#endif /* NEO_SYS_SYS_IMGUI_H_ */
