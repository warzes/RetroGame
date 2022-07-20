#pragma once

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

enum class GameKey
{
	A,
	B,
	C,
	X,
	Y,

	UP,
	DOWN,
	LEFT,
	RIGHT
};
constexpr size_t MaxGameKey = 9;

static std::map<SDL_Keycode, GameKey> KeyMapping;
static bool KeyDown[MaxGameKey] = { false };
static bool KeyPress[MaxGameKey] = { false };
inline void SetKey(SDL_Event sdlEvent, bool state)
{
	auto it = KeyMapping.find(sdlEvent.key.keysym.sym);
	if (it != KeyMapping.end())
	{
		const size_t id = static_cast<size_t>(it->second);

		KeyDown[id] = state;
		if (!state) KeyPress[id] = false;
	}
}

inline bool IsKeyDown(GameKey key)
{
	return KeyDown[static_cast<size_t>(key)];
}
inline bool IsKeyPress(GameKey key)
{
	if (!IsKeyDown(key) || KeyPress[static_cast<size_t>(key)]) return false;
	KeyPress[static_cast<size_t>(key)] = true;
	return true;
}