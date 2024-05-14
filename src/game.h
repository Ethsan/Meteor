#pragma once

#include "fsm.h"
#include "sdl.h"
#include "logic.h"
#include "widget.h"

struct Assets {
	SDL::Texture brick_rect;
	SDL::Texture brick_hex;
	SDL::Texture ball;
	SDL::Texture powerups;
	SDL::Texture paddle;
	SDL::Texture ship;
	SDL::Texture ship_right;
	SDL::Texture ship_left;
	SDL::Texture ui;
	SDL::Texture bg;
};

class Game : public State {
    public:
	Game(const SDL::Window &window, const SDL::Renderer &renderer)
		: window_(window)
		, renderer_(renderer)
		, logic_(300, 300)
		, assets_{ .brick_rect = { renderer_, "assets/asteroid.png" },
			   .brick_hex = { renderer_, "assets/hex.png" },
			   .ball = { renderer_, "assets/ball.png" },
			   .powerups = { renderer_, "assets/powerups.png" },
			   .paddle = { renderer_, "assets/shield.png" },
			   .ship = { renderer_, "assets/ship_forward.png" },
			   .ship_right = { renderer_, "assets/ship_right.png" },
			   .ship_left = { renderer_, "assets/ship_left.png" },
			   .ui = { renderer_, "assets/side.png" },
			   .bg = { renderer_, "assets/bg.png" } }
		, ui_factory_(renderer_){};

	Game(const SDL::Window &window, const SDL::Renderer &renderer, const std::string save_file)
		: window_(window)
		, renderer_(renderer)
		, logic_(Logic::loadFromFile(save_file))
		, assets_{ .brick = { renderer_, "assets/asteroid.png" },
			   .ball = { renderer_, "assets/ball.png" },
			   .paddle = { renderer_, "assets/shield.png" },
			   .ship = { renderer_, "assets/ship_forward.png" },
			   .ship_right = { renderer_, "assets/ship_right.png" },
			   .ship_left = { renderer_, "assets/ship_left.png" },
			   .ui = { renderer_, "assets/side.png" },
			   .bg = { renderer_, "assets/bg.png" } }
		, ui_factory_(renderer_){};

	std::shared_ptr<State> operator()() override;
	void draw();

    private:
	SDL::Window window_;
	SDL::Renderer renderer_;

	Logic logic_;
	Assets assets_;
	UI_Factory ui_factory_;

	std::optional<std::shared_ptr<State> > pause();
	std::optional<std::shared_ptr<State> > resume();
	std::shared_ptr<State> end();

	friend class Pause;
};
