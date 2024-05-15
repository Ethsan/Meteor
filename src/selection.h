#pragma once

#include "fsm.h"
#include "sdl.h"
#include "widget.h"

#include <memory>
#include <string>
#include <vector>

class Selection : public State {
    public:
	Selection(const SDL::Window &window, const SDL::Renderer &renderer)
		: window(window)
		, renderer(renderer)
	{
		init();
	};

	std::shared_ptr<State> operator()() override;

    private:
	SDL::Window window;
	SDL::Renderer renderer;

	static inline SDL::Font font{ "assets/ticketing.regular.ttf", 17 };
	static constexpr SDL::Color bg = { 46, 36, 36, 255 };
	static constexpr SDL::Color fg = { 0x34, 0xac, 0xba, 0xff };
	static constexpr SDL::Color hl = { 0xff, 0xe0, 0x7e, 0xff };
	static constexpr SDL::Color cursor_color = { 0Xff, 0xe7, 0xd6, 128 };

	unsigned int cursor = 0;
	unsigned int vert_shift = 0;

	std::vector<Label> save_files{};
	unsigned int target = 0;

	void draw();

	void up();
	void down();
	void on_click(float y);

	void init();
};
