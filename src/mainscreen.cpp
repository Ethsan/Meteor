#include "mainscreen.h"
#include "SDL_events.h"
#include "editor.h"
#include "selection.h"
#include "sdl.h"
#include "exception.h"

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
			throw Close();
		case SDL_MOUSEMOTION:
			keyTarget_ = 5;
			x = event.motion.x;
			y = event.motion.y;
			alpha = -(static_cast<double>(x) - (windowWidth / 2)) / (30 * windowWidth);
			beta = -(static_cast<double>(y) - (windowHight / 2)) / (30 * windowHight);
			needRedraw = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (play_.isOver(x, y)) {
				return std::make_shared<Selection>(window_, renderer_);
			}
			if (exit_.isOver(x, y)) {
				throw Close();
			}
			if (editor_.isOver(x, y)) {
				return std::make_shared<Editor>(window_, renderer_);
			}
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_UP) {
				keyTarget_ = (keyTarget_ + 2) % 3;
				needRedraw = true;
			}
			if (event.key.keysym.sym == SDLK_DOWN) {
				keyTarget_ = (keyTarget_ + 1) % 3;
				needRedraw = true;
			}
			if (event.key.keysym.sym == SDLK_RETURN) {
				switch (keyTarget_) {
				case 5:
				case 0:
					return std::make_shared<Selection>(window_, renderer_);
				case 1:
					throw Close();
				case 2:
					return std::make_shared<Editor>(window_, renderer_);
				}
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
			play_.draw(renderer_, (keyTarget_ == 5 && play_.isOver(x, y)) || keyTarget_ == 0);
			exit_.draw(renderer_, (keyTarget_ == 5 && exit_.isOver(x, y)) || keyTarget_ == 1);
			editor_.draw(renderer_, (keyTarget_ == 5 && editor_.isOver(x, y)) || keyTarget_ == 2);
			// Present the screen
			renderer_.present();
		}
	}
}
