#pragma once

#include "fsm.h"
#include "sdl.h"
#include "widget.h"
#include <vector>

class Selection : public State {
    public:
	Selection(const SDL::Window &window, const SDL::Renderer &renderer)
		: window_(window)
		, renderer_(renderer)
		, save_files()
	{
		init();
	};

	std::shared_ptr<State> operator()() override;

    private:
	SDL::Window window_;
	SDL::Renderer renderer_;

	static inline SDL::Font font{ "assets/ticketing.regular.ttf", 17 };
	static constexpr SDL::Color bg = { 46, 36, 36, 255 };
	static constexpr SDL::Color textColor = { 0x34, 0xac, 0xba, 0xff };
	static constexpr SDL::Color textColor_h = { 0xff, 0xe0, 0x7e, 0xff };
	static constexpr SDL::Color cursorColor = { 0Xff, 0xe7, 0xd6, 128 };

	unsigned int cursor = 0;
	unsigned int verticalShift = 0;

	std::vector<Label> save_files;
	unsigned int target = 0;

	void draw();

	void up();
	void down();
	void onClic(float y);

	void init();
};