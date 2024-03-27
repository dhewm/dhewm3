

#ifndef NEO_SYS_SYS_IMGUI_H_
#define NEO_SYS_SYS_IMGUI_H_

#include "../libs/imgui/imgui.h"

namespace D3 {
namespace ImGuiHooks {

#ifndef IMGUI_DISABLE

extern ImGuiContext* imguiCtx; // this is only here so IsImguiEnabled() can use it inline

inline bool IsImguiEnabled()
{
	return imguiCtx != NULL;
}

// using void* instead of SDL_Window and SDL_GLContext to avoid dragging SDL headers into sys_imgui.h
extern bool Init(void* sdlWindow, void* sdlGlContext);

extern void Shutdown();

extern void NewFrame();

extern bool ProcessEvent(const void* sdlEvent);

extern void EndFrame();

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

inline void NewFrame() {}

inline bool ProcessEvent(const void* sdlEvent) { return false; }

inline void EndFrame() {}

#endif

}} //namespace D3::ImGuiHooks


#endif /* NEO_SYS_SYS_IMGUI_H_ */
