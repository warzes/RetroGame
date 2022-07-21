#include "stdafx.h"
#include "Engine.h"
#include "Game.h"
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2main.lib" )
#endif
//-----------------------------------------------------------------------------
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
PixelBuffer<VIEWPORT_X, VIEWPORT_Y>* gPixelBuffer = nullptr;
Input gInput;
constexpr float MicrosecondsToSeconds = 1.0f / 1000000.0f;
std::chrono::steady_clock::time_point startTime;
int64_t frameTimeCurrent = 0;
int64_t frameTimeLast = 0;
int64_t frameTimeDelta = 0;
float deltaTime = 0.0f;
SDL_Event sdlEvent = {};
bool running = true;
//-----------------------------------------------------------------------------
void UpdateFrame()
{
#if defined(__EMSCRIPTEN__)
	if (running == false) emscripten_cancel_main_loop();
#endif
	const auto curTime = std::chrono::high_resolution_clock::now();
	frameTimeCurrent = std::chrono::duration_cast<std::chrono::microseconds>(curTime - startTime).count();
	frameTimeDelta = frameTimeCurrent - frameTimeLast;
	frameTimeLast = frameTimeCurrent;
	deltaTime = static_cast<float>(frameTimeDelta) * MicrosecondsToSeconds;

	GameFrame();

	SDL_RenderClear(renderer);
	gPixelBuffer->Draw(renderer);
	SDL_RenderPresent(renderer);

	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_KEYDOWN:
			gInput.SetKey(sdlEvent, true);
			break;
		case SDL_KEYUP:
			gInput.SetKey(sdlEvent, false);
			break;
		}
	}
}
//-----------------------------------------------------------------------------
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	PrintLog("SDL Init");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		return 0;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	//SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");

	PrintLog("SDL create window");
	window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RESOLUTION_X, RESOLUTION_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if (!window) return 0;

	PrintLog("SDL create renderer");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) return 0;

	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // TODO: ????
	SDL_RenderSetLogicalSize(renderer, VIEWPORT_X, VIEWPORT_Y);

	gPixelBuffer = new PixelBuffer<VIEWPORT_X, VIEWPORT_Y>(renderer);

	if (!GameInit()) return 0;

	startTime = std::chrono::high_resolution_clock::now();
#if defined(__EMSCRIPTEN__)
	emscripten_set_main_loop(UpdateFrame, 0, 1);
#else
	while (running) UpdateFrame();
#endif

	GameClose();
	delete gPixelBuffer;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
//-----------------------------------------------------------------------------