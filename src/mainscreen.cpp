#include "mainscreen.h"
#include "game.h"

inline bool is_in_rect(int x, int y, SDL::Rect rect)
{
	return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

std::shared_ptr<State> MainScreen::operator()()
{
	int x = 0, y = 0;
	bool needRedraw = true;

	for (;;) {
		auto event = SDL::waitEvent();

		switch (event.type) {
		case SDL_QUIT:
			throw std::runtime_error("Quit");
		case SDL_MOUSEMOTION:
			x = event.motion.x;
			y = event.motion.y;
			needRedraw = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (is_in_rect(x, y, play_.getRect())) {
				return std::make_shared<Game>(window_, std::move(renderer_));
			}
			if (is_in_rect(x, y, exit_.getRect())) {
				throw std::runtime_error("Quit");
			}
			break;
		}

		if (needRedraw) {
			needRedraw = false;
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
	}
}
