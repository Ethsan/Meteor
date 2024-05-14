#include "selection.h"

#include "SDL_events.h"
#include "SDL_keycode.h"
#include "sdl.h"
#include "game.h"
#include <filesystem>
#include <memory>

constexpr float y_shift = 20, y_off = 2;

std::shared_ptr<State> Selection::operator()()
{
	if (save_files.empty())
		return std::make_shared<Game>(window_, renderer_);

	float y = 0;
	bool needRedraw = false;

	draw();

	for (;;) {
		auto event = SDL::waitEvent();

		switch (event.type) {
		case SDL_QUIT:
			throw std::runtime_error("Quit");
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_UP) {
				up();
				needRedraw = true;
			}
			if (event.key.keysym.sym == SDLK_DOWN) {
				down();
				needRedraw = true;
			}
			if (event.key.keysym.sym == SDLK_RETURN)
				return std::make_shared<Game>(window_, renderer_, save_files[target].getText());
			break;
		case SDL_MOUSEBUTTONDOWN:
			y = event.motion.y;
			onClic(y);
			needRedraw = true;
			break;
		}

		if (needRedraw) {
			draw();
			needRedraw = false;
		}
	}
}

void Selection::draw()
{
	renderer_.setDrawColor(bg);
	renderer_.clear();

	for (size_t i = 0; i < save_files.size(); i++) {
		save_files[i].draw(renderer_, i == target);
	}

	renderer_.setDrawColor(cursorColor);
	SDL::FRect cursorRect = { 0, cursor * y_shift, 400, y_shift };
	renderer_.fillRect(cursorRect);

	renderer_.present();
}

void Selection::up()
{
	if (target == 0)
		return;

	target--;

	if (cursor == 0) {
		verticalShift--;
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
		verticalShift++;
		for (auto &file : save_files)
			file.y = file.y - y_shift;
	} else {
		cursor++;
	}
}

void Selection::onClic(float y)
{
	unsigned int tmpCursor = static_cast<unsigned int>(y / y_shift);

	if (tmpCursor + verticalShift < save_files.size()) {
		cursor = tmpCursor;
		target = tmpCursor + verticalShift;
	}
}

void Selection::init()
{
	constexpr float x = 15;
	float y = y_off;

	for (const auto &entry : std::filesystem::directory_iterator("save")) {
		std::ostringstream s;
		s << entry.path();
		std::string fileName{ s.str() };
		fileName.erase(remove(fileName.begin(), fileName.end(), '\"'), fileName.end());

		save_files.emplace_back(fileName, font, textColor, textColor_h, renderer_, x, y);
		y = y + y_shift;
	}
}
