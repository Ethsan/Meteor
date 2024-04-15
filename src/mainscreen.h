#include "fsm.h"
#include "widget.h"

class MainScreen : public State {
    private:
	SDL::Window window_;
	SDL::Renderer renderer_;
	SDL::Font font_;
	Label title_, play_, exit_;
	Label play_h, exit_h;
	static constexpr SDL::Color bg = { 0, 0, 0, 255 };
	static constexpr SDL::Color fg = { 255, 255, 255, 255 };
	static constexpr SDL::Color hl = { 255, 0, 0, 255 };

    public:
	std::shared_ptr<State> operator()() override;
	MainScreen(SDL::Window window)
		: window_(window)
		, renderer_(window_)
		, font_("assets/Roboto.ttf", 24)
		, title_("Bricked", font_, fg, renderer_, 10, 10)
		, play_("Play", font_, fg, renderer_, 10, 50)
		, exit_("Exit", font_, fg, renderer_, 10, 90)
		, play_h("Play", font_, hl, renderer_, 10, 50)
		, exit_h("Exit", font_, hl, renderer_, 10, 90)
	{
	}

	void step(int x, int y);
};
