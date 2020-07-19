#pragma once
#ifdef __linux__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include <memory>

struct SDL_Deleter {
    void operator()(SDL_Window* ptr) { if (ptr) SDL_DestroyWindow(ptr); }
    void operator()(SDL_GLContext ptr) { if (ptr) SDL_GL_DeleteContext(ptr); }
};

// RAII
using SDLWindowPtr = std::unique_ptr<SDL_Window, SDL_Deleter>;
using SDLGLContextPtr = std::unique_ptr<void, SDL_Deleter>;

inline void SDL_GL_SwapWindow(SDLWindowPtr const &window) { SDL_GL_SwapWindow(window.get()); }
inline SDL_GLContext SDL_GL_CreateContext(SDLWindowPtr& window) { return SDL_GL_CreateContext(window.get()); };