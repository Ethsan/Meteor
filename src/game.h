#pragma once

#include "fsm.h"
#include "sdl.h"
#include "mainscreen.h"

#include <vector>

class Game : public State {
	SDL::Window window_;
	SDL::Renderer renderer_;

	class Object {
	    public:
		SDL::Texture texture_;
		SDL::Rect src, dest;
		Object(SDL::Texture &texture, const SDL::Rect dst)
			: texture_(texture)
			, src(texture_.getRect())
			, dest(dst)
		{
		}

		void draw(SDL::Renderer &renderer) const
		{
			renderer.copy(texture_, src, dest);
		}
	};

	std::vector<Object> bricks{};
	std::vector<Object> balls{};

    public:
	Game(SDL::Window &window, SDL::Renderer renderer)
		: window_(window)
		, renderer_(renderer)
	{
		SDL::Texture brick(renderer_, "assets/brick.png");
		SDL::Texture ball(renderer_, "assets/ball.png");

		std::pair<int, int> winSize = window_.getSize();

		for (int i = 0; i < 10; i++) {
			bricks.push_back(Object(brick, { i * 32, 0, 32, 16 }));
		}

		balls.push_back(Object(ball, { winSize.first / 2, winSize.second / 2, 16, 16 }));
	}

	std::shared_ptr<State> operator()() override;
};
