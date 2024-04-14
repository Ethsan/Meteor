#include <SDL.h>
#include <SDL_image.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>

namespace SDL
{

using Point = SDL_Point;
using FPoint = SDL_FPoint;
using Rect = SDL_Rect;
using FRect = SDL_FRect;
using Color = SDL_Color;
using Vertex = SDL_Vertex;

using DisplayEvent = SDL_DisplayEvent;
using WindowEvent = SDL_WindowEvent;
using KeyboardEvent = SDL_KeyboardEvent;
using TextEditingEvent = SDL_TextEditingEvent;
using TextInputEvent = SDL_TextInputEvent;
using MouseMotionEvent = SDL_MouseMotionEvent;
using MouseButtonEvent = SDL_MouseButtonEvent;
using MouseWheelEvent = SDL_MouseWheelEvent;
using JoyAxisEvent = SDL_JoyAxisEvent;
using JoyBallEvent = SDL_JoyBallEvent;
using JoyHatEvent = SDL_JoyHatEvent;
using JoyButtonEvent = SDL_JoyButtonEvent;
using JoyDeviceEvent = SDL_JoyDeviceEvent;
using ControllerAxisEvent = SDL_ControllerAxisEvent;
using ControllerButtonEvent = SDL_ControllerButtonEvent;
using ControllerDeviceEvent = SDL_ControllerDeviceEvent;
using ControllerTouchpadEvent = SDL_ControllerTouchpadEvent;
using AudioDeviceEvent = SDL_AudioDeviceEvent;
using TouchFingerEvent = SDL_TouchFingerEvent;
using MultiGestureEvent = SDL_MultiGestureEvent;
using DollarGestureEvent = SDL_DollarGestureEvent;
using DropEvent = SDL_DropEvent;
using QuitEvent = SDL_QuitEvent;
using UserEvent = SDL_UserEvent;
using SysWMEvent = SDL_SysWMEvent;
using CommonEvent = SDL_CommonEvent;
using Event = SDL_Event;

using Keycode = SDL_Keycode;
using RendererFlip = SDL_RendererFlip;
using PixelFormatEnum = SDL_PixelFormatEnum;

inline std::string getError()
{
	return SDL_GetError();
}

[[noreturn]] inline void fail(const std::string &msg)
{
	SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", msg.c_str(), getError().c_str());
	throw std::runtime_error(msg + ": " + getError());
}

inline void warn(const std::string &msg)
{
	SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", msg.c_str(), getError().c_str());
}

inline void delay(Uint32 ms)
{
	SDL_Delay(ms);
}

inline void quit()
{
	SDL_Quit();
}

inline std::optional<Event> pollEvent()
{
	Event e;
	if (SDL_PollEvent(&e) == 0)
		return std::nullopt;
	return e;
}

inline Event waitEvent()
{
	Event e;
	if (SDL_WaitEvent(&e) == 0)
		fail("SDL_WaitEvent");
	return e;
}

inline std::optional<Event> waitEventTimeout(int timeout)
{
	Event e;
	if (SDL_WaitEventTimeout(&e, timeout) == 0)
		return std::nullopt;
	return e;
}

inline void pumpEvents()
{
	SDL_PumpEvents();
}

inline bool setHint(const std::string &name, const std::string &value)
{
	return SDL_SetHint(name.c_str(), value.c_str()) == SDL_TRUE;
}

template <std::integral T> inline bool setHint(const std::string &name, T value)
{
	return setHint(name, std::to_string(value));
}

template <> inline bool setHint(const std::string &name, bool value)
{
	return setHint(name, value ? "1" : "0");
}

inline void free(char *filename)
{
	SDL_free(filename);
}

class SDL {
    private:
	static inline Uint32 count = 0;

    public:
	SDL()
	{
		if (count == 0) {
			if (SDL_Init(SDL_INIT_VIDEO) != 0)
				fail("SDL_Init");
			if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
				fail("IMG_Init");
		}
		count++;
	}
	~SDL()
	{
		count--;
		if (count == 0) {
			IMG_Quit();
			SDL_Quit();
		}
	}
};

class Window {
    private:
	SDL sdl_{};
	std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_;

	friend class Renderer;

	SDL_Window *get() const
	{
		return window_.get();
	}

    public:
	Window(Window &&) = default;
	Window &operator=(const Window &) = delete;
	Window &operator=(Window &&) = default;

	Window(const Window &other)
		: window_(SDL_CreateWindowFrom(other.get()), SDL_DestroyWindow)
	{
		if (window_ == nullptr)
			fail("SDL_CreateWindowFrom");
	}

	Window(const std::string title, int x, int y, int w, int h, Uint32 flags = 0)
		: window_(SDL_CreateWindow(title.c_str(), x, y, w, h, flags), SDL_DestroyWindow)
	{
		if (window_ == nullptr)
			fail("SDL_CreateWindow");
	}

	Window(const std::string title, int w, int h, Uint32 flags = 0)
		: window_(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
					   flags),
			  SDL_DestroyWindow)
	{
		if (window_ == nullptr)
			fail("SDL_CreateWindow");
	}

	Uint32 getId() const
	{
		return SDL_GetWindowID(get());
	}
};

using Color = SDL_Color;

struct Pixel {
	inline static constexpr PixelFormatEnum format = SDL_BYTEORDER == SDL_BIG_ENDIAN ? SDL_PIXELFORMAT_RGBA8888 :
											   SDL_PIXELFORMAT_ABGR8888;
	Uint8 r, g, b, a;
};

class Texture;
class Surface;

class Renderer {
    private:
	std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer_;

	friend class Texture;

	SDL_Renderer *get() const
	{
		return renderer_.get();
	}

    public:
	Renderer(Renderer &&) = default;
	Renderer &operator=(const Renderer &) = delete;
	Renderer &operator=(Renderer &&) = default;
	Renderer(Window &window, int index = -1, Uint32 flags = 0)
		: renderer_(SDL_CreateRenderer(window.get(), index, flags), SDL_DestroyRenderer)
	{
		if (renderer_ == nullptr)
			fail("SDL_CreateRenderer");
	}
	Renderer(Surface &surface);
	void clear()
	{
		if (SDL_RenderClear(get()) != 0)
			fail("SDL_RenderClear");
	}
	void present()
	{
		SDL_RenderPresent(get());
	}

	void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		if (SDL_SetRenderDrawColor(get(), r, g, b, a) != 0)
			fail("SDL_SetRenderDrawColor");
	}

	void setColor(const Color &color)
	{
		setColor(color.r, color.g, color.b, color.a);
	}

	void setTarget(Texture &texture);
	void resetTarget()
	{
		if (SDL_SetRenderTarget(get(), nullptr) != 0)
			fail("SDL_SetRenderTarget");
	}
	void copy(Texture &texture, const Rect &src, const Rect &dst);
	void copy(Texture &texture, const Rect &src, const FRect &dst);

	void copy(Texture &texture, const Rect &src, const Rect &dst, double angle, const Point &center,
		  RendererFlip flip);
	void copy(Texture &texture, const Rect &src, const FRect &dst, double angle, const FPoint &center,
		  RendererFlip flip);

	void drawLine(int x1, int y1, int x2, int y2)
	{
		if (SDL_RenderDrawLine(get(), x1, y1, x2, y2) != 0)
			fail("SDL_RenderDrawLine");
	}
	void drawLine(float x1, float y1, float x2, float y2)
	{
		if (SDL_RenderDrawLineF(get(), x1, y1, x2, y2) != 0)
			fail("SDL_RenderDrawLineF");
	}
	void drawLines(std::span<const Point> points)
	{
		if (SDL_RenderDrawLines(get(), points.data(), points.size()) != 0)
			fail("SDL_RenderDrawLines");
	}
	void drawLines(std::span<const FPoint> points)
	{
		if (SDL_RenderDrawLinesF(get(), points.data(), points.size()) != 0)
			fail("SDL_RenderDrawLinesF");
	}

	void drawPoint(int x, int y)
	{
		if (SDL_RenderDrawPoint(get(), x, y) != 0)
			fail("SDL_RenderDrawPoint");
	}

	void drawPoint(float x, float y)
	{
		if (SDL_RenderDrawPointF(get(), x, y) != 0)
			fail("SDL_RenderDrawPointF");
	}
	void drawPoints(std::span<const Point> points)
	{
		if (SDL_RenderDrawPoints(get(), points.data(), points.size()) != 0)
			fail("SDL_RenderDrawPoints");
	}

	void drawPoints(std::span<const FPoint> points)
	{
		if (SDL_RenderDrawPointsF(get(), points.data(), points.size()) != 0)
			fail("SDL_RenderDrawPointsF");
	}

	void drawRect(const Rect &rect)
	{
		if (SDL_RenderDrawRect(get(), &rect) != 0)
			fail("SDL_RenderDrawRect");
	}
	void drawRect(const FRect &rect)
	{
		if (SDL_RenderDrawRectF(get(), &rect) != 0)
			fail("SDL_RenderDrawRectF");
	}

	void drawRects(std::span<const Rect> rects)
	{
		if (SDL_RenderDrawRects(get(), rects.data(), rects.size()) != 0)
			fail("SDL_RenderDrawRects");
	}

	void drawRects(std::span<const FRect> rects)
	{
		if (SDL_RenderDrawRectsF(get(), rects.data(), rects.size()) != 0)
			fail("SDL_RenderDrawRectsF");
	}

	void fillRect(const Rect &rect)
	{
		if (SDL_RenderFillRect(get(), &rect) != 0)
			fail("SDL_RenderFillRect");
	}

	void fillRect(const FRect &rect)
	{
		if (SDL_RenderFillRectF(get(), &rect) != 0)
			fail("SDL_RenderFillRectF");
	}

	void fillRects(std::span<const Rect> rects)
	{
		if (SDL_RenderFillRects(get(), rects.data(), rects.size()) != 0)
			fail("SDL_RenderFillRects");
	}

	void fillRects(std::span<const FRect> rects)
	{
		if (SDL_RenderFillRectsF(get(), rects.data(), rects.size()) != 0)
			fail("SDL_RenderFillRectsF");
	}

	void geometry(std::span<const Vertex> vertices, std::span<const int> indices)
	{
		if (SDL_RenderGeometry(get(), nullptr, vertices.data(), vertices.size(), indices.data(),
				       indices.size()) != 0)
			fail("SDL_RenderGeometry");
	}

	void getClipRect(Rect &rect)
	{
		SDL_RenderGetClipRect(get(), &rect);
	}
};

class Surface {
    private:
	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface_;
	friend class Texture;
	friend class Renderer;

	SDL_Surface *get() const
	{
		return surface_.get();
	}

    public:
	Surface(Surface &&) = default;
	Surface &operator=(const Surface &) = delete;
	Surface &operator=(Surface &&) = default;
	Surface(const std::string &file)
		: surface_(IMG_Load(file.c_str()), SDL_FreeSurface)
	{
		if (surface_ == nullptr)
			fail("IMG_Load");

		if (surface_->format->format != Pixel::format) {
			surface_.reset(SDL_ConvertSurfaceFormat(surface_.get(), Pixel::format, 0));
			if (surface_ == nullptr)
				fail("SDL_ConvertSurfaceFormat");
		}
	}

	Surface(int w, int h)
		: surface_(SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, Pixel::format), SDL_FreeSurface)
	{
		if (surface_ == nullptr)
			fail("SDL_CreateRGBSurface");
	}

	void fillRect(const Rect &rect, Uint32 color)
	{
		if (SDL_FillRect(get(), &rect, color) != 0)
			fail("SDL_FillRect");
	}

	void fillRects(std::span<const Rect> rects, Uint32 color)
	{
		if (SDL_FillRects(get(), rects.data(), rects.size(), color) != 0)
			fail("SDL_FillRects");
	}

	static inline void blit(Surface &src, const Rect &srcrect, Surface &dst, Rect &dstrect)
	{
		if (SDL_BlitSurface(src.get(), &srcrect, dst.get(), &dstrect) != 0)
			fail("SDL_BlitSurface");
	}

	void blit(Surface &dst, Rect &dstrect)
	{
		if (SDL_BlitSurface(get(), nullptr, dst.get(), &dstrect) != 0)
			fail("SDL_BlitSurface");
	}

	void blit(Surface &dst)
	{
		if (SDL_BlitSurface(get(), nullptr, dst.get(), nullptr) != 0)
			fail("SDL_BlitSurface");
	}

	void blitScaled(Surface &dst, Rect &dstrect)
	{
		if (SDL_BlitScaled(get(), nullptr, dst.get(), &dstrect) != 0)
			fail("SDL_BlitScaled");
	}

	void blitScaled(Surface &dst)
	{
		if (SDL_BlitScaled(get(), nullptr, dst.get(), nullptr) != 0)
			fail("SDL_BlitScaled");
	}

	int getWidth() const
	{
		return get()->w;
	}

	int getHeight() const
	{
		return get()->h;
	}

	std::span<Pixel> lock()
	{
		if (SDL_MUSTLOCK(get()) && SDL_LockSurface(get()) != 0)
			fail("SDL_LockSurface");

		size_t size = static_cast<size_t>(get()->pitch) / sizeof(Pixel) * static_cast<size_t>(get()->h);
		return { static_cast<Pixel *>(get()->pixels), size };
	}

	void unlock()
	{
		if (SDL_MUSTLOCK(get()))
			SDL_UnlockSurface(get());
	}
};

class Texture {
    private:
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture_;

	friend class Renderer;

	SDL_Texture *get() const
	{
		return texture_.get();
	}

    public:
	Texture(Texture &&) = default;
	Texture &operator=(const Texture &) = delete;
	Texture &operator=(Texture &&) = default;
	Texture(Renderer &renderer, Surface &surface)
		: texture_(SDL_CreateTextureFromSurface(renderer.get(), surface.get()), SDL_DestroyTexture)
	{
		if (texture_ == nullptr)
			fail("SDL_CreateTextureFromSurface");

		Uint32 format;
		SDL_QueryTexture(get(), &format, nullptr, nullptr, nullptr);
		SDL_assert(format == Pixel::format);
	}
	Texture(Renderer &renderer, int access, int w, int h)
		: texture_(SDL_CreateTexture(renderer.get(), Pixel::format, access, w, h), SDL_DestroyTexture)
	{
		if (texture_ == nullptr)
			fail("SDL_CreateTexture");
	}
	void update(const Rect &rect, const std::span<Pixel> pixels)
	{
		if (SDL_UpdateTexture(get(), &rect, pixels.data(), pixels.size() * sizeof(Pixel)) != 0)
			fail("SDL_UpdateTexture");
	}

	std::span<Pixel> lock(const Rect &rect)
	{
		void *pixels;
		int pitch;
		if (SDL_LockTexture(get(), &rect, &pixels, &pitch) != 0)
			fail("SDL_LockTexture");

		size_t size = static_cast<size_t>(pitch) / sizeof(Pixel) * static_cast<size_t>(rect.h);
		return { static_cast<Pixel *>(pixels), size };
	}

	std::span<Pixel> lock()
	{
		Rect rect = { 0, 0, 0, 0 };
		query(rect.w, rect.h);
		return lock(rect);
	}

	void unlock()
	{
		SDL_UnlockTexture(get());
	}

	void setBlendMode(SDL_BlendMode mode)
	{
		if (SDL_SetTextureBlendMode(get(), mode) != 0)
			fail("SDL_SetTextureBlendMode");
	}

	void setAlphaMod(Uint8 alpha)
	{
		if (SDL_SetTextureAlphaMod(get(), alpha) != 0)
			fail("SDL_SetTextureAlphaMod");
	}

	void setColorMod(Uint8 r, Uint8 g, Uint8 b)
	{
		if (SDL_SetTextureColorMod(get(), r, g, b) != 0)
			fail("SDL_SetTextureColorMod");
	}

	void query(Uint32 &format, int &access, int &w, int &h) const
	{
		if (SDL_QueryTexture(get(), &format, &access, &w, &h) != 0)
			fail("SDL_QueryTexture");
	}

	void query(int &w, int &h) const
	{
		if (SDL_QueryTexture(get(), nullptr, nullptr, &w, &h) != 0)
			fail("SDL_QueryTexture");
	}

	int getWidth() const
	{
		int w, h;
		query(w, h);
		return w;
	}

	int getHeight() const
	{
		int w, h;
		query(w, h);
		return h;
	}
};

inline Renderer::Renderer(Surface &surface)
	: renderer_(SDL_CreateSoftwareRenderer(surface.get()), SDL_DestroyRenderer)
{
	if (renderer_ == nullptr)
		fail("SDL_CreateSoftwareRenderer");
}

inline void Renderer::setTarget(Texture &texture)
{
	if (SDL_SetRenderTarget(get(), texture.get()) != 0)
		fail("SDL_SetRenderTarget");
}

inline void Renderer::copy(Texture &texture, const Rect &src, const Rect &dst)
{
	if (SDL_RenderCopy(get(), texture.get(), &src, &dst) != 0)
		fail("SDL_RenderCopy");
}

inline void Renderer::copy(Texture &texture, const Rect &src, const FRect &dst)
{
	if (SDL_RenderCopyF(get(), texture.get(), &src, &dst) != 0)
		fail("SDL_RenderCopyF");
}

inline void Renderer::copy(Texture &texture, const Rect &src, const Rect &dst, double angle, const Point &center,
			   RendererFlip flip)
{
	if (SDL_RenderCopyEx(get(), texture.get(), &src, &dst, angle, &center, flip) != 0)
		fail("SDL_RenderCopyEx");
}

inline void Renderer::copy(Texture &texture, const Rect &src, const FRect &dst, double angle, const FPoint &center,
			   RendererFlip flip)
{
	if (SDL_RenderCopyExF(get(), texture.get(), &src, &dst, angle, &center, flip) != 0)
		fail("SDL_RenderCopyExF");
}
}; // namespace SDL
