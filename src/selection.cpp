#include "selection.h"

#include "exception.h"
#include "game.h"
#include "sdl.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

constexpr float y_shift = 20, y_off = 2;

std::shared_ptr<State> Selection::operator()()
{
	if (save_files.empty())
		return std::make_shared<Game>(window, renderer);

	float y = 0;
	bool is_redraw_needed = false;

	draw();

	for (;;) {
		auto event = SDL::waitEvent();

		switch (event.type) {
		case SDL_QUIT:
			throw Close();
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_UP) {
				up();
				is_redraw_needed = true;
			}
			if (event.key.keysym.sym == SDLK_DOWN) {
				down();
				is_redraw_needed = true;
			}
			if (event.key.keysym.sym == SDLK_RETURN) {
				try {
					return std::make_shared<Game>(window, renderer, save_files[target].get_text());
				} catch (Bad_format const &) {
					return std::make_shared<Game>(window, renderer);
				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			y = event.motion.y;
			on_click(y);
			is_redraw_needed = true;
			break;
		}

		if (is_redraw_needed) {
			draw();
			is_redraw_needed = false;
		}
	}
}

void Selection::draw()
{
	renderer.setDrawColor(bg);
	renderer.clear();

	for (size_t i = 0; i < save_files.size(); i++) {
		save_files[i].draw(renderer, i == target);
	}

	renderer.setDrawColor(cursor_color);
	renderer.fillRect(SDL::FRect{ 0, cursor * y_shift, 400, y_shift });

	renderer.present();
}

void Selection::up()
{
	if (target == 0)
		return;

	target--;

	if (cursor == 0) {
		vert_shift--;
		for (auto &file : save_files)
			file.y = file.y + y_shift;
	} else {
		cursor--;
	}
}

void Selection::down()
{
	constexpr int cursor_max = 14;

	if (target >= save_files.size() - 1)
		return;

	target++;

	if (cursor >= cursor_max) {
		vert_shift++;
		for (auto &file : save_files)
			file.y = file.y - y_shift;
	} else {
		cursor++;
	}
}

void Selection::on_click(float y)
{
	unsigned int new_cursor = static_cast<unsigned int>(y / y_shift);

	if (new_cursor + vert_shift < save_files.size()) {
		cursor = new_cursor;
		target = new_cursor + vert_shift;
	}
}

void Selection::init()
{
	constexpr float x = 15;
	float y = y_off;

	for (const auto &entry : std::filesystem::directory_iterator("save")) {
		std::ostringstream s;
		s << entry.path();

		std::string filename{ s.str() };
		filename.erase(std::remove(filename.begin(), filename.end(), '\"'), filename.end());

		save_files.emplace_back(filename, font, fg, hl, renderer, x, y);
		y = y + y_shift;
	}
}
