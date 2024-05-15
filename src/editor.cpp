#include "editor.h"

#include "exception.h"
#include "game.h"
#include "logic.h"
#include "mainscreen.h"
#include "sdl.h"
#include "widget.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

struct Render_visitor {
	SDL::Renderer &renderer;
	const Assets &assets;
	const Logic &logic;

	void operator()(const auto &)
	{
	}

	void operator()(const Brick &brick)
	{
		constexpr int max_durability = 5;
		constexpr int dim = 96;

		int off = max_durability - static_cast<int>(brick.get_durability());

		SDL::Rect src = { off * dim, 0, dim, dim };

		float x = brick.get_x() - dim * 0.5;
		float y = brick.get_y() - dim * 0.5;

		SDL::FRect dst = { x, y, dim, dim };
		switch (brick.get_form()) {
		case Brick::rect:
			return renderer.copy(assets.brick_rect, src, dst);
		case Brick::hex:
			return renderer.copy(assets.brick_hex, src, dst);
		}
	}
};

std::shared_ptr<State> Editor::operator()()
{
	float x = 0, y = 0;
	bool is_redraw_needed = true;
	bool is_left_pressed = false;

	for (;;) {
		auto event = SDL::waitEvent();

		is_left_pressed = SDL::isPressedMouse(SDL_BUTTON_LMASK);
		if (!is_left_pressed)
			clicked_brick = std::nullopt;

		switch (event.type) {
		case SDL_QUIT:
			throw Close();
		case SDL_MOUSEMOTION:
			x = event.motion.x;
			y = event.motion.y;
			if (is_left_pressed) {
				drag(x, y);
			}
			is_redraw_needed = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			x = event.motion.x;
			y = event.motion.y;
			if (exit.is_over(x, y))
				return std::make_shared<MainScreen>(window, renderer);
			if (event.button.button == SDL_BUTTON_LEFT)
				on_left_click(x, y);
			if (event.button.button == SDL_BUTTON_RIGHT)
				on_right_click(x, y);
			is_redraw_needed = true;
			break;
		}

		if (is_redraw_needed) {
			is_redraw_needed = false;
			draw(x, y);
		}
	}
}

void Editor::draw(float x, float y)
{
	renderer.clear();

	SDL::Rect src_bg = { 0, 0, assets.bg.getWidth(), assets.bg.getHeight() };
	SDL::FRect dst_bg = { 0, 0, static_cast<float>(src_bg.w), static_cast<float>(src_bg.h) };

	for (dst_bg.y = 0; dst_bg.y < 400; dst_bg.y += src_bg.h) {
		for (dst_bg.x = 0; dst_bg.x < 400; dst_bg.x += src_bg.w)
			renderer.copy(assets.bg, src_bg, dst_bg);
	}

	canva.visit(Render_visitor{ renderer, assets, canva });

	renderer.setDrawColor(255, 0, 0, 25);
	SDL::FRect filter = { 0, canva.get_height() - 50, canva.get_width(), 50 };
	renderer.fillRect(filter);

	constexpr int dim_x = 128;
	constexpr int dim_y = 512;

	SDL::Rect src = { 0, 0, dim_x, dim_y };
	SDL::Rect dst = { 400 - dim_x, 0, dim_x, dim_y };

	renderer.copy(assets.ui, src, dst);

	for (Material &a : sources) {
		a.draw(renderer);
	}

	exit.draw(renderer, exit.is_over(x, y));
	save.draw(renderer, save.is_over(x, y));

	renderer.present();
}

void Editor::on_left_click(float x, float y)
{
	auto idx_brick = canva.get_brick(x, y);
	if (!idx_brick && is_in_canva(x, y)) {
		auto idx = canva.add_brick_safe(x, y, sources[source].get_dura());
		if (idx)
			idx_brick = { { *idx, canva.get_brick(*idx) } };
	}

	if (idx_brick) {
		clicked_brick = idx_brick->first;
		click_offset_x = idx_brick->second.get_x() - x;
		click_offset_y = idx_brick->second.get_y() - y;
	}

	for (unsigned int i = 0; i < sources.size(); i++) {
		if (sources[i].is_over(x, y)) {
			sources[source].set_selected(false);
			sources[i].set_selected(true);
			source = i;
			break;
		}
	}

	if (save.is_over(x, y)) {
		const auto t = std::time(nullptr);
		const auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S");
		auto time = oss.str();

		std::ofstream out("save/save_" + time + ".save", std::ios::out);
		canva.save(out);
		out.close();
	}
}

void Editor::on_right_click(float x, float y)
{
	if (auto idx = canva.get_brick(x, y)) {
		canva.remove_brick(idx->first);
	}
}

void Editor::drag(float x, float y)
{
	if (!clicked_brick)
		return;

	float new_x = x + click_offset_x;
	float new_y = y + click_offset_y;

	if (!is_in_canva(new_x, new_y))
		return;

	canva.replace_brick_safe(*clicked_brick, new_x, new_y);
}
