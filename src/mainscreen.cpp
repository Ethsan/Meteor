#include "mainscreen.h"

bool is_in_rect(int x, int y, SDL::Rect rect)
{
	return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

void MainScreen::step(int x, int y)
{
	// Clear the screen
	renderer_.setDrawColor(bg);
	renderer_.clear();
	// Draw the title
	title_.draw(renderer_);
	if (is_in_rect(x, y, play_.getRect())) {
		play_h.draw(renderer_);
	} else {
		play_.draw(renderer_);
	}
	if (is_in_rect(x, y, exit_.getRect())) {
		exit_h.draw(renderer_);
	} else {
		exit_.draw(renderer_);
	}
	// Present the screen
	renderer_.present();
}

std::shared_ptr<State> MainScreen::operator()()
{
	int x = 0, y = 0;
	step(x, y);
	for (;;) {
		bool needRedraw = false;

		auto event = SDL::waitEvent();
		if (event.type == SDL_QUIT) {
			throw std::runtime_error("SDL_QUIT");
		}
		if (event.type == SDL_MOUSEMOTION) {
			needRedraw = true;
			x = event.motion.x;
			y = event.motion.y;
		}
		if (needRedraw) {
			step(x, y);
		}
	}
}
