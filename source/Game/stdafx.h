#pragma once

#if defined(_MSC_VER)
#pragma warning(disable:5045)
#pragma warning(push, 0)
#pragma warning(disable:4365)
#endif

#if defined(__EMSCRIPTEN__)
#	include <emscripten/emscripten.h>
#	include <SDL2/SDL.h>
#else
#	include <SDL2/SDL.h>
#endif

#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif