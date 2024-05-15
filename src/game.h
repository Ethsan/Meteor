#pragma once

#include "fsm.h"
#include "sdl.h"
#include "logic.h"
#include "widget.h"
#include <string>

struct Assets {
	Assets(SDL::Renderer &renderer)
		: brick_rect{ renderer, "assets/asteroid.png" }
		, brick_hex{ renderer, "assets/hex.png" }
		, ball{ renderer, "assets/ball.png" }
		, powerups{ renderer, "assets/powerups.png" }
		, paddle{ renderer, "assets/shield.png" }
		, ship{ renderer, "assets/ship_forward.png" }
		, ship_right{ renderer, "assets/ship_right.png" }
		, ship_left{ renderer, "assets/ship_left.png" }
		, ui{ renderer, "assets/side.png" }
		, bg{ renderer, "assets/bg.png" } {};

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
		, save_file_()
		, logic_(300, 300, true)
		, assets_(renderer_)
		, ui_factory_(renderer_){};

	Game(const SDL::Window &window, const SDL::Renderer &renderer, const std::string save_file)
		: window_(window)
		, renderer_(renderer)
		, save_file_(save_file)
		, logic_(Logic::load(save_file))
		, assets_{ renderer_ }
		, ui_factory_(renderer_){};

	std::shared_ptr<State> operator()() override;
	void draw();

    private:
	SDL::Window window_;
	SDL::Renderer renderer_;

	const std::string save_file_;

	Logic logic_;
	Assets assets_;
	UI_Factory ui_factory_;

	std::optional<std::shared_ptr<State> > pause();
	std::optional<std::shared_ptr<State> > resume();
	std::shared_ptr<State> end();

	friend class Pause;
};
