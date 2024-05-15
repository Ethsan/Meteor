#include "mainscreen.h"

#include "editor.h"
#include "sdl.h"
#include "selection.h"

#include <memory>

std::shared_ptr<State> MainScreen::operator()()
{
	int x = 0, y = 0;
	bool is_redraw_needed = true;
	float width = window.getSize().first, height = window.getSize().second;
	float alpha = 0, beta = 0;

	for (;;) {
		auto event = SDL::waitEvent();

		switch (event.type) {
		case SDL_QUIT:
			throw Close();
		case SDL_MOUSEMOTION:
			key_target = 5;
			x = event.motion.x;
			y = event.motion.y;
			alpha = -(static_cast<double>(x) - (width / 2)) / (30 * width);
			beta = -(static_cast<double>(y) - (height / 2)) / (30 * height);
			is_redraw_needed = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (play.is_over(x, y)) {
				return std::make_shared<Selection>(window, renderer);
			}
			if (exit.is_over(x, y)) {
				throw Close();
			}
			if (editor.is_over(x, y)) {
				return std::make_shared<Editor>(window, renderer);
			}
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_UP) {
				key_target = (key_target + 2) % 3;
				is_redraw_needed = true;
			}
			if (event.key.keysym.sym == SDLK_DOWN) {
				key_target = (key_target + 1) % 3;
				is_redraw_needed = true;
			}
			if (event.key.keysym.sym == SDLK_RETURN) {
				switch (key_target) {
				case 5:
				case 0:
					return std::make_shared<Selection>(window, renderer);
				case 1:
					throw Close();
				case 2:
					return std::make_shared<Editor>(window, renderer);
				}
			}
			break;
		}

		if (is_redraw_needed) {
			is_redraw_needed = false;
			renderer.setDrawColor(bg);
			renderer.clear();
			// Draw the background
			stars.draw(renderer, alpha, beta);
			dust.draw(renderer, alpha, beta);
			nebulae.draw(renderer, alpha, beta);
			planets.draw(renderer, alpha, beta);
			// Draw the title
			title.draw(renderer);
			play.draw(renderer, (key_target == 5 && play.is_over(x, y)) || key_target == 0);
			exit.draw(renderer, (key_target == 5 && exit.is_over(x, y)) || key_target == 1);
			editor.draw(renderer, (key_target == 5 && editor.is_over(x, y)) || key_target == 2);
			// Present the screen
			renderer.present();
		}
	}
}
