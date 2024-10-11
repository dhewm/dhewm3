// just a wrapper for #include <SDL.h> to support SDL1.2, SDL2 and SDL3

#ifndef NEO_SYS_SYS_SDL_H_
#define NEO_SYS_SYS_SDL_H_

#ifdef D3_SDL3
  // HACK: I don't want SDL.h to drag in SDL_oldnames.h to avoid all the warnings about redefined definitions
  #define SDL_oldnames_h_
  #include <SDL3/SDL.h>
#else // SDL1.2 or SDL2
  #include <SDL.h>
#endif

#endif /* NEO_SYS_SYS_SDL_H_ */
