#include "mainscreen.h"
#include "editor.h"
#include "selection.h"
#include "sdl.h"

inline bool is_in_rect(int x, int y, SDL::Rect rect)
{
	return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

std::shared_ptr<State> MainScreen::operator()()
{
	int x = 0, y = 0;
	bool needRedraw = true;
	float windowWidth = window_.getSize().first, windowHight = window_.getSize().second;
	float alpha = 0, beta = 0;

	for (;;) {
		auto event = SDL::waitEvent();

		switch (event.type) {
		case SDL_QUIT:
			throw std::runtime_error("Quit");
		case SDL_MOUSEMOTION:
			x = event.motion.x;
			y = event.motion.y;
			alpha = -(static_cast<double>(x) - (windowWidth / 2)) / (30 * windowWidth);
			beta = -(static_cast<double>(y) - (windowHight / 2)) / (30 * windowHight);
			needRedraw = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (is_in_rect(x, y, play_.getRect())) {
				return std::make_shared<Selection>(window_, renderer_);
			}
			if (is_in_rect(x, y, exit_.getRect())) {
				throw std::runtime_error("Quit");
			}
			if (is_in_rect(x, y, editor_.getRect())) {
				return std::make_shared<Editor>(window_, renderer_);
			}
			break;
		}

		if (needRedraw) {
			needRedraw = false;
			renderer_.setDrawColor(bg);
			renderer_.clear();
			// Draw the background
			stars_.draw(renderer_, alpha, beta);
			dust_.draw(renderer_, alpha, beta);
			nebulae_.draw(renderer_, alpha, beta);
			planets_.draw(renderer_, alpha, beta);
			// Draw the title
			title_.draw(renderer_);
			play_.draw(renderer_, is_in_rect(x, y, play_.getRect()));
			exit_.draw(renderer_, is_in_rect(x, y, exit_.getRect()));
			editor_.draw(renderer_, is_in_rect(x, y, editor_.getRect()));
			// Present the screen
			renderer_.present();
		}
	}
}
