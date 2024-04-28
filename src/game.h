#pragma once

#include "fsm.h"
#include "sdl.h"
#include "logic.h"
#include "widget.h"

struct Assets {
	SDL::Texture brick;
	SDL::Texture ball;
	SDL::Texture paddle;
	SDL::Texture ship;
	SDL::Texture ship_right;
	SDL::Texture ship_left;
	SDL::Texture ui;
	SDL::Texture bg;
	SDL::Font font;
};

struct Text {
	Label score;
	Label speed;
	Label times;
};

class Game : public State {
    public:
	Game(const SDL::Window &window, const SDL::Renderer &renderer)
		: window_(window)
		, renderer_(renderer)
		, logic_(600, 600, 32)
		, assets_{ .brick = { renderer_, "assets/asteroid.png" },
			   .ball = { renderer_, "assets/ball.png" },
			   .paddle = { renderer_, "assets/shield.png" },
			   .ship = { renderer_, "assets/ship_forward.png" },
			   .ship_right = { renderer_, "assets/ship_right.png" },
			   .ship_left = { renderer_, "assets/ship_left.png" },
			   .ui = { renderer_, "assets/ui.png" },
			   .bg = { renderer_, "assets/bg.png" },
			   .font = SDL::Font("assets/Roboto.ttf", 24) } {};

	std::shared_ptr<State> operator()() override;

    private:
	SDL::Window window_;
	SDL::Renderer renderer_;

	Logic logic_;
	Assets assets_;

	void render();
	void render_menu();
};
