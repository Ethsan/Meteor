#include "game.h"

std::shared_ptr<State> Game::operator()()
{
	for (;;) {
		while (auto event = SDL::pollEvent()) {
			if (event->type == SDL_QUIT || event->type == SDL_KEYDOWN) {
				return std::make_shared<MainScreen>(window_, renderer_);
			}
		}
		renderer_.clear();
		for (const auto &brick : bricks) {
			brick.draw(renderer_);
		}
		for (const auto &ball : balls) {
			ball.draw(renderer_);
		}
		renderer_.present();
		SDL::delay(16);
	};
}
