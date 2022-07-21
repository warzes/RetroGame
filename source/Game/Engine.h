#pragma once

#define FLIP_PIXEL_BUFFER_Y 1

#define RESOLUTION_X 1024
#define RESOLUTION_Y 768
#define VIEWPORT_X 160
#define VIEWPORT_Y 120

inline void PrintLog(const std::string& text)
{
#if defined(__EMSCRIPTEN__)
	puts(text.c_str());
#else
	SDL_Log(text.c_str());
#endif
}

inline void ErrorLog(const std::string& text)
{
	PrintLog("ERROR: " + text);
}

template<size_t W, size_t H>
class PixelBuffer
{
public:
	PixelBuffer(SDL_Renderer* renderer)
	{
		m_textureSDL = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STATIC, W, H);
	}

	~PixelBuffer()
	{
		SDL_DestroyTexture(m_textureSDL);
	}

	void Clear()
	{
		memset(m_pixels, 200, W * H * sizeof(uint32_t));
	}

	void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
	{
		if (x < 0 || y < 0 || x >= W || y >= H) return;
#if FLIP_PIXEL_BUFFER_Y
		y = H - y - 1;
#endif
		uint8_t* p = ((uint8_t*)m_pixels) + (y * W + x) * 4 + 1;

		*p = blue;
		++p;
		*p = green;
		++p;
		*p = red;

		// проверить что лучше
		//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		//SDL_RenderDrawPoint(renderer, 640 / 2, 480 / 2);
	}

	void Draw(SDL_Renderer* renderer)
	{
		SDL_UpdateTexture(m_textureSDL, nullptr, m_pixels, W * sizeof(uint32_t));
		SDL_RenderCopy(renderer, m_textureSDL, nullptr, nullptr);
	}

private:
	SDL_Texture* m_textureSDL = nullptr;
	uint32_t m_pixels[W * H] = { 200 };
};

extern PixelBuffer<VIEWPORT_X, VIEWPORT_Y>* gPixelBuffer;

enum class GameKey
{
	W,
	S,
	A,
	D,
	M,
	Q,
	E,
};
constexpr size_t MaxGameKey = 7;

class Input
{
public:
	Input()
	{
		m_keyMapping[SDLK_w] = GameKey::W;
		m_keyMapping[SDLK_a] = GameKey::A;
		m_keyMapping[SDLK_s] = GameKey::S;
		m_keyMapping[SDLK_d] = GameKey::D;
		m_keyMapping[SDLK_m] = GameKey::M;
		m_keyMapping[SDLK_q] = GameKey::Q;
		m_keyMapping[SDLK_e] = GameKey::E;
	}
	void SetKey(SDL_Event sdlEvent, bool state)
	{
		auto it = m_keyMapping.find(sdlEvent.key.keysym.sym);
		if (it != m_keyMapping.end())
		{
			const size_t id = static_cast<size_t>(it->second);

			m_keyDown[id] = state;
			if (!state) m_keyPress[id] = false;
		}
	}

	bool IsKeyDown(GameKey key)
	{
		return m_keyDown[static_cast<size_t>(key)];
	}
	bool IsKeyPress(GameKey key)
	{
		if (!IsKeyDown(key) || m_keyPress[static_cast<size_t>(key)]) return false;
		m_keyPress[static_cast<size_t>(key)] = true;
		return true;
	}
private:
	std::map<SDL_Keycode, GameKey> m_keyMapping;
	bool m_keyDown[MaxGameKey] = { false };
	bool m_keyPress[MaxGameKey] = { false };
};

extern Input gInput;