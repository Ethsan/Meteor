#pragma once

#include "fsm.h"
#include "logic.h"
#include "sdl.h"
#include "widget.h"

#include <memory>
#include <optional>
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

// The Game class handle the interaction between the user and the game logic.
// It is responsible for rendering the game and handling user input.
class Game : public State {
    public:
	Game(const SDL::Window &w, const SDL::Renderer &r)
		: window(w)
		, renderer(r)
		, save_file()
		, logic(300, 300, true)
		, assets(renderer)
		, ui_factory(renderer){};

	Game(const SDL::Window &w, const SDL::Renderer &r, const std::string save_file)
		: window(w)
		, renderer(r)
		, save_file(save_file)
		, logic(Logic::load(save_file))
		, assets{ renderer }
		, ui_factory(renderer){};

	std::shared_ptr<State> operator()() override;
	void draw();

    private:
	SDL::Window window;
	SDL::Renderer renderer;

	const std::string save_file;

	Logic logic;
	Assets assets;
	UI_Factory ui_factory;

	std::optional<std::shared_ptr<State> > pause();
	std::optional<std::shared_ptr<State> > resume();
	std::shared_ptr<State> end();

	friend class Pause;
};
