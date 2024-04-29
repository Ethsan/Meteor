#pragma once

#include "fsm.h"
#include "widget.h"

class MainScreen : public State {
    private:
	SDL::Window window_;
	SDL::Renderer renderer_;
	Label title_, play_, exit_;
	Label play_h, exit_h;

	static inline SDL::Font roboto{ "assets/Roboto.ttf", 24 };
	static constexpr SDL::Color bg = { 0, 0, 0, 255 };
	static constexpr SDL::Color fg = { 255, 255, 255, 255 };
	static constexpr SDL::Color hl = { 255, 0, 0, 255 };

    public:
	MainScreen(SDL::Window &window, SDL::Renderer renderer)
		: window_(window)
		, renderer_(renderer)
		, title_("Bricked", roboto, fg, renderer_, 10, 10)
		, play_("Play", roboto, fg, renderer_, 10, 50)
		, exit_("Exit", roboto, fg, renderer_, 10, 90)
		, play_h("Play", roboto, hl, renderer_, 10, 50)
		, exit_h("Exit", roboto, hl, renderer_, 10, 90)
	{
	}

	std::shared_ptr<State> operator()() override;
};
