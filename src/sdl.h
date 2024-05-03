#pragma once

#include "SDL_stdinc.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <memory>
#include <optional>
#include <span>
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
using Scancode = SDL_Scancode;
using RendererFlip = SDL_RendererFlip;
using PixelFormatEnum = SDL_PixelFormatEnum;
using BlendMode = SDL_BlendMode;

inline std::string getError()
{
	return SDL_GetError();
}

inline Uint32 getMouseState(int &x, int &y)
{
	return SDL_GetMouseState(&x, &y);
}

inline Uint32 getGlobalMouseState(int &x, int &y)
{
	return SDL_GetGlobalMouseState(&x, &y);
}

inline Uint32 getRelativeMouseState(int &x, int &y)
{
	return SDL_GetRelativeMouseState(&x, &y);
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
			if (TTF_Init() != 0)
				fail("TTF_Init");
		}
		count++;
	}

	SDL(const SDL &)
	{
		count++;
	}
	SDL(SDL &&)
	{
		count++;
	}
	SDL &operator=(const SDL &)
	{
		return *this;
	}
	SDL &operator=(SDL &&)
	{
		return *this;
	}

	~SDL()
	{
		count--;
		if (count == 0) {
			IMG_Quit();
			SDL_Quit();
			TTF_Quit();
		}
	}
};

inline static bool isPressed(Scancode code)
{
	int len;
	const Uint8 *state = SDL_GetKeyboardState(&len);

	return state[code];
}

class Window {
    private:
	SDL sdl_{};
	std::shared_ptr<SDL_Window> window_;

	friend class Renderer;

	SDL_Window *get() const
	{
		return window_.get();
	}

    public:
	Window(SDL_Window *window)
		: window_(window, SDL_DestroyWindow)
	{
	}

    public:
	Window(Window &&) = default;
	Window &operator=(const Window &) = default;
	Window &operator=(Window &&) = default;
	Window(const Window &) = default;

	Window duplicate() const
	{
		SDL_Window *window = SDL_CreateWindowFrom(get());
		if (window == nullptr)
			fail("SDL_DuplicateWindow");
		return Window(window);
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

	std::pair<int, int> getSize() const
	{
		int w, h;
		SDL_GetWindowSize(get(), &w, &h);
		return { w, h };
	}

	void setSize(int w, int h)
	{
		SDL_SetWindowSize(get(), w, h);
	}

	std::pair<int, int> getPosition() const
	{
		int x, y;
		SDL_GetWindowPosition(get(), &x, &y);
		return { x, y };
	}

	void setPosition(int x, int y)
	{
		SDL_SetWindowPosition(get(), x, y);
	}

	Rect getRect() const
	{
		Rect rect;
		SDL_GetWindowPosition(get(), &rect.x, &rect.y);
		SDL_GetWindowSize(get(), &rect.w, &rect.h);
		return rect;
	}

	void setRect(const Rect &rect)
	{
		SDL_SetWindowPosition(get(), rect.x, rect.y);
		SDL_SetWindowSize(get(), rect.w, rect.h);
	}

	std::string getTitle() const
	{
		return SDL_GetWindowTitle(get());
	}

	void setTitle(const std::string &title)
	{
		SDL_SetWindowTitle(get(), title.c_str());
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
	SDL sdl_{};
	std::shared_ptr<SDL_Renderer> renderer_;

	friend class Texture;

	SDL_Renderer *get() const
	{
		return renderer_.get();
	}

    public:
	Renderer(const Renderer &) = default;
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

	void setDrawBlendMode(BlendMode mode)
	{
		if (SDL_SetRenderDrawBlendMode(get(), mode) != 0)
			fail("SDL_SetRenderDrawBlendMode");
	}

	BlendMode getDrawBlendMode() const
	{
		SDL_BlendMode mode;
		if (SDL_GetRenderDrawBlendMode(get(), &mode) != 0)
			fail("SDL_GetRenderDrawBlendMode");
		return mode;
	}

	void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		if (SDL_SetRenderDrawColor(get(), r, g, b, a) != 0)
			fail("SDL_SetRenderDrawColor");
	}

	void setDrawColor(const Color &color)
	{
		setDrawColor(color.r, color.g, color.b, color.a);
	}

	void setTarget(Texture &texture);
	void resetTarget()
	{
		if (SDL_SetRenderTarget(get(), nullptr) != 0)
			fail("SDL_SetRenderTarget");
	}
	void copy(const Texture &texture, const Rect &src, const Rect &dst);
	void copy(const Texture &texture, const Rect &src, const FRect &dst);

	void copy(const Texture &texture, const Rect &src, const Rect &dst, double angle, const Point &center,
		  RendererFlip flip);
	void copy(const Texture &texture, const Rect &src, const FRect &dst, double angle, const FPoint &center,
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

	void getLogicalSize(int &w, int &h) const
	{
		SDL_RenderGetLogicalSize(get(), &w, &h);
	}

	void setLogicalSize(int w, int h)
	{
		if (SDL_RenderSetLogicalSize(get(), w, h) != 0)
			fail("SDL_RenderSetLogicalSize");
	}

	bool getIntegerScale() const
	{
		return SDL_RenderGetIntegerScale(get()) == SDL_TRUE;
	}

	void setIntegerScale(bool enable)
	{
		if (SDL_RenderSetIntegerScale(get(), enable ? SDL_TRUE : SDL_FALSE) != 0)
			fail("SDL_RenderSetIntegerScale");
	}

	Rect getViewport() const
	{
		Rect rect;
		SDL_RenderGetViewport(get(), &rect);
		return rect;
	}

	void setViewport(const Rect &rect)
	{
		if (SDL_RenderSetViewport(get(), &rect) != 0)
			fail("SDL_RenderSetViewport");
	}

	Rect getClipRect() const
	{
		Rect rect;
		SDL_RenderGetClipRect(get(), &rect);
		return rect;
	}

	void setClipRect(const Rect &rect)
	{
		if (SDL_RenderSetClipRect(get(), &rect) != 0)
			fail("SDL_RenderSetClipRect");
	}

	bool isClipEnabled() const
	{
		return SDL_RenderIsClipEnabled(get()) == SDL_TRUE;
	}

	void getScale(float &scaleX, float &scaleY) const
	{
		SDL_RenderGetScale(get(), &scaleX, &scaleY);
	}

	void setScale(float scaleX, float scaleY)
	{
		if (SDL_RenderSetScale(get(), scaleX, scaleY) != 0)
			fail("SDL_RenderSetScale");
	}

	FPoint windowToLogical(const Point &point) const
	{
		FPoint fpoint;
		SDL_RenderWindowToLogical(get(), point.x, point.y, &fpoint.x, &fpoint.y);
		return fpoint;
	}

	Point logicalToWindow(const FPoint &fpoint) const
	{
		Point point;
		SDL_RenderLogicalToWindow(get(), fpoint.x, fpoint.y, &point.x, &point.y);
		return point;
	}
};

class Surface {
    private:
	SDL sdl_{};
	std::shared_ptr<SDL_Surface> surface_;
	friend class Texture;
	friend class Renderer;

	SDL_Surface *get() const
	{
		return surface_.get();
	}

    public:
	Surface(const Surface &) = default;
	Surface(Surface &&) = default;
	Surface &operator=(const Surface &) = default;
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

	Surface(SDL_Surface *surface)
		: surface_(surface, SDL_FreeSurface)
	{
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
	SDL sdl_{};
	std::shared_ptr<SDL_Texture> texture_;

	friend class Renderer;

	SDL_Texture *get() const
	{
		return texture_.get();
	}

    public:
	Texture(const Texture &) = default;
	Texture(Texture &&) = default;
	Texture &operator=(const Texture &) = default;
	Texture &operator=(Texture &&) = default;
	Texture(Renderer &renderer, Surface surface)
		: texture_(SDL_CreateTextureFromSurface(renderer.get(), surface.get()), SDL_DestroyTexture)
	{
		if (texture_ == nullptr)
			fail("SDL_CreateTextureFromSurface");

		Uint32 format;
		SDL_QueryTexture(get(), &format, nullptr, nullptr, nullptr);
		SDL_assert(format == Pixel::format);
		if (format != Pixel::format)
			SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Unsupported pixel format");
	}
	Texture(Renderer &renderer, int access, int w, int h)
		: texture_(SDL_CreateTexture(renderer.get(), Pixel::format, access, w, h), SDL_DestroyTexture)
	{
		if (texture_ == nullptr)
			fail("SDL_CreateTexture");
	}

	Texture(Renderer &renderer, const std::string &filename)
		: texture_(IMG_LoadTexture(renderer.get(), filename.c_str()), SDL_DestroyTexture)
	{
		if (texture_ == nullptr)
			fail("IMG_LoadTexture");
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

	Rect getRect() const
	{
		Rect rect = { 0, 0, 0, 0 };
		query(rect.w, rect.h);
		return rect;
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

inline void Renderer::copy(const Texture &texture, const Rect &src, const Rect &dst)
{
	if (SDL_RenderCopy(get(), texture.get(), &src, &dst) != 0)
		fail("SDL_RenderCopy");
}

inline void Renderer::copy(const Texture &texture, const Rect &src, const FRect &dst)
{
	if (SDL_RenderCopyF(get(), texture.get(), &src, &dst) != 0)
		fail("SDL_RenderCopyF");
}

inline void Renderer::copy(const Texture &texture, const Rect &src, const Rect &dst, double angle, const Point &center,
			   RendererFlip flip)
{
	if (SDL_RenderCopyEx(get(), texture.get(), &src, &dst, angle, &center, flip) != 0)
		fail("SDL_RenderCopyEx");
}

inline void Renderer::copy(const Texture &texture, const Rect &src, const FRect &dst, double angle,
			   const FPoint &center, RendererFlip flip)
{
	if (SDL_RenderCopyExF(get(), texture.get(), &src, &dst, angle, &center, flip) != 0)
		fail("SDL_RenderCopyExF");
}

class Font {
    private:
	SDL sdl_{};
	std::shared_ptr<TTF_Font> font_;

    public:
	Font(Font &&) = default;
	Font &operator=(const Font &) = delete;
	Font &operator=(Font &&) = default;
	Font(const std::string &file, int ptsize)
		: font_(TTF_OpenFont(file.c_str(), ptsize), TTF_CloseFont)
	{
		if (font_ == nullptr)
			fail("TTF_OpenFont");
	}
	Surface renderText(const std::string &text, const Color &color) const
	{
		SDL_Surface *surface = TTF_RenderText_Blended(font_.get(), text.c_str(), color);
		if (surface == nullptr)
			fail("TTF_RenderText_Blended");
		return Surface(surface);
	}
};
}; // namespace SDL
