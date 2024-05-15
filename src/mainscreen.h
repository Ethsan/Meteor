#pragma once

#include "fsm.h"
#include "sdl.h"
#include "widget.h"

// Background: Class to handle one panel of the parallax effect.
class Background {
    public:
	SDL::Texture texture;
	SDL::Rect src;
	SDL::FRect dst;
	float distance;

	Background(SDL::Renderer &renderer, const std::string &filename, unsigned int p)
		: texture(renderer, filename)
		, src(texture.getRect())
		, dst(0, 0, static_cast<float>(src.w), static_cast<float>(src.h))
		, distance(p)
	{
	}

	void draw(SDL::Renderer &renderer, float alpha, float beta)
	{
		dst.x = std::tan(alpha) * distance - static_cast<float>(texture.getWidth()) / 17;
		dst.y = std::tan(beta) * distance - static_cast<float>(texture.getHeight()) / 17;
		renderer.copy(texture, src, dst);
	}
};

// MainScreen: Class to handle the main menu screen.
class MainScreen : public State {
    private:
	SDL::Window window;
	SDL::Renderer renderer;
	Label title, play, exit, editor;
	Background stars, dust, nebulae, planets;

	static inline SDL::Font title_font{ "assets/ticketing.regular.ttf", 60 };
	static inline SDL::Font menu_font{ "assets/ticketing.regular.ttf", 45 };
	static constexpr SDL::Color bg = { 46, 36, 36, 255 };
	static constexpr SDL::Color fg = { 0x34, 0xac, 0xba, 0xff };
	static constexpr SDL::Color hl = { 0xff, 0xe0, 0x7e, 0xff };

	unsigned int key_target = 5; //mod 3

    public:
	MainScreen(const SDL::Window &w, const SDL::Renderer &r)
		: window(w)
		, renderer(r)
		, title("Meteor", title_font, fg, hl, renderer, 132, 35)
		, play("Play", menu_font, fg, hl, renderer, 160, 95)
		, exit("Exit", menu_font, fg, hl, renderer, 160, 145)
		, editor("Editor", menu_font, fg, hl, renderer, 160, 195)
		, stars(renderer, "assets/stars.png", 1100)
		, dust(renderer, "assets/dust.png", 1000)
		, nebulae(renderer, "assets/nebulae.png", 880)
		, planets(renderer, "assets/planets.png", 500)
	{
	}

	std::shared_ptr<State> operator()() override;
};
