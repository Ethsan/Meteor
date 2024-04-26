#pragma once

#include "fsm.h"
#include "sdl.h"
#include "logic.h"

struct Assets {
	SDL::Texture brick;
	SDL::Texture ball;
	SDL::Texture paddle;
};

class Game : public State {
	SDL::Window window_;
	SDL::Renderer renderer_;

	Logic logic_;
	Assets assets_;

    public:
	Game(const SDL::Window &window, const SDL::Renderer &renderer)
		: window_(window)
		, renderer_(renderer)
		, logic_(window.getSize().first, window.getSize().second, 32)
		, assets_{ { renderer_, "assets/asteroid.png" },
			   { renderer_, "assets/ball.png" },
			   { renderer_, "assets/shield.png" } }
	{
	}

	std::shared_ptr<State> operator()() override;
};
