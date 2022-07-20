#include "stdafx.h"
#include "Engine.h"
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2main.lib" )
#endif
//-----------------------------------------------------------------------------
#if defined(__EMSCRIPTEN__)
void UpdateDrawFrame()
{
	if (!gEngine->Update()) emscripten_cancel_main_loop();
	gGame->Update(gEngine->GetDeltaTime());
	gGame->Render();
}
#endif
//-----------------------------------------------------------------------------
#define res        4                        //0=160x120 1=360x240 4=640x480
#define SW         160*res                  //screen width
#define SH         120*res                  //screen height
#define SW2        (SW/2)                   //half of screen width
#define SH2        (SH/2)                   //half of screen height
#define pixelScale 4/res                    //OpenGL pixel scale
#define GLSW       (SW*pixelScale)          //OpenGL window width // TODO: rename S3L_RESOLUTION_X
#define GLSH       (SH*pixelScale)          //OpenGL window height // S3L_RESOLUTION_Y
#define PIXELS_SIZE (GLSW * GLSH * 4)

//-----------------------------------------------------------------------------
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	PrintLog("SDL Init");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		return 0;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	//SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");

	PrintLog("SDL create window");
	SDL_Window* window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GLSW, GLSH, /*SDL_WINDOW_RESIZABLE | */SDL_WINDOW_SHOWN);
	if (!window) return 0;

	PrintLog("SDL create renderer");
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) return 0;

	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // TODO: ????
	//SDL_RenderSetLogicalSize(renderer, renderSize.x, renderSize.y);
	//SDL_RenderSetScale(renderer, pixelScale, pixelScale);

	auto startTime = std::chrono::high_resolution_clock::now();

	KeyMapping[SDLK_z] = GameKey::A;
	KeyMapping[SDLK_x] = GameKey::B;
	KeyMapping[SDLK_c] = GameKey::C;
	KeyMapping[SDLK_a] = GameKey::X;
	KeyMapping[SDLK_s] = GameKey::Y;
	KeyMapping[SDLK_UP] = GameKey::UP;
	KeyMapping[SDLK_DOWN] = GameKey::DOWN;
	KeyMapping[SDLK_LEFT] = GameKey::LEFT;
	KeyMapping[SDLK_RIGHT] = GameKey::RIGHT;

	auto pixelBuffer = new PixelBuffer<GLSW, GLSH>(renderer);
	for (int x = 0; x < 10; x++)
	{
		for (int y = 0; y < 10; y++)
		{
			pixelBuffer->SetPixel(0 + x, 0 + x + y, 255, 100, 255);
		}
	}

	int64_t frameTimeCurrent = 0;
	int64_t frameTimeLast = 0;
	int64_t frameTimeDelta = 0;
	float deltaTime = 0.0f;
	SDL_Event sdlEvent = {};
	bool running = true;
#if defined(__EMSCRIPTEN__)
	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
	while (running)
	{
		constexpr float MicrosecondsToSeconds = 1.0f / 1000000.0f;
		const auto curTime = std::chrono::high_resolution_clock::now();
		frameTimeCurrent = std::chrono::duration_cast<std::chrono::microseconds>(curTime - startTime).count();
		frameTimeDelta = frameTimeCurrent - frameTimeLast;
		frameTimeLast = frameTimeCurrent;
		deltaTime = static_cast<float>(frameTimeDelta) * MicrosecondsToSeconds;

		SDL_RenderClear(renderer);
		pixelBuffer->Draw(renderer);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
			case SDL_QUIT:
				running = false;
			case SDL_KEYDOWN:
				SetKey(sdlEvent, true);
				break;
			case SDL_KEYUP:
				SetKey(sdlEvent, false);
				break;
			}
		}
	}
#endif

	delete pixelBuffer;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
//-----------------------------------------------------------------------------