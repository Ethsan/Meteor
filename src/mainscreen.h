#pragma once

#include "fsm.h"
#include "sdl.h"
#include "widget.h"
#include <cmath>

class MainScreen : public State {
    private:
	SDL::Window window_;
	SDL::Renderer renderer_;
	Label title_, play_, exit_, editor_;
	class Background {
	    public:
		SDL::Texture texture_;
		SDL::Rect src;
		SDL::FRect dst;
		float distance;
		Background(SDL::Renderer &renderer, const std::string &filename, unsigned int p)
			: texture_(renderer, filename)
			, src(texture_.getRect())
			, dst(0, 0, static_cast<float>(src.w), static_cast<float>(src.h))
			, distance(p)
		{
		}

		void draw(SDL::Renderer &renderer, float alpha, float beta)
		{
			dst.x = std::tan(alpha) * distance - static_cast<float>(texture_.getWidth()) / 17;
			dst.y = std::tan(beta) * distance - static_cast<float>(texture_.getHeight()) / 17;
			renderer.copy(texture_, src, dst);
		}
	} stars_, dust_, nebulae_, planets_;

	static inline SDL::Font title{ "assets/ticketing.regular.ttf", 60 };
	static inline SDL::Font menu{ "assets/ticketing.regular.ttf", 45 };
	static constexpr SDL::Color bg = { 46, 36, 36, 255 };
	static constexpr SDL::Color fg = { 0x34, 0xac, 0xba, 0xff };
	static constexpr SDL::Color hl = { 0xff, 0xe0, 0x7e, 0xff };

	unsigned int keyTarget_ = 5; //mod 3

    public:
	MainScreen(const SDL::Window &window, const SDL::Renderer &renderer)
		: window_(window)
		, renderer_(renderer)
		, title_("Bricked", title, fg, hl, renderer_, 105, 60)
		, play_("Play", menu, fg, hl, renderer_, 160, 115)
		, exit_("Exit", menu, fg, hl, renderer_, 160, 165)
		, editor_("Editor", menu, fg, hl, renderer_, 160, 215)
		, stars_(renderer_, "assets/stars.png", 1100)
		, dust_(renderer_, "assets/dust.png", 1000)
		, nebulae_(renderer_, "assets/nebulae.png", 880)
		, planets_(renderer_, "assets/planets.png", 500)
	{
	}

	std::shared_ptr<State> operator()() override;
};
