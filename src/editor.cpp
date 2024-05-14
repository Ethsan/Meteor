#include "editor.h"
#include "SDL_events.h"
#include "SDL_mouse.h"
#include "logic.h"
#include "mainscreen.h"
#include "sdl.h"
#include "widget.h"
#include <SDL_events.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <optional>
#include "exception.h"

struct RenderVisitor {
	SDL::Renderer &renderer;
	const EditorAssets &assets;
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
		renderer.copy(assets.brick, src, dst);
	}
};

std::shared_ptr<State> Editor::operator()()
{
	float x = 0, y = 0;
	bool needRedraw = true;
	bool is_mouseLeft_pressed = false;

	for (;;) {
		auto event = SDL::waitEvent();

		is_mouseLeft_pressed = SDL::isPressedMouse(SDL_BUTTON_LMASK);
		if (!is_mouseLeft_pressed)
			clicked_brick = std::nullopt;

		switch (event.type) {
		case SDL_QUIT:
			throw Close();
		case SDL_MOUSEMOTION:
			x = event.motion.x;
			y = event.motion.y;
			if (is_mouseLeft_pressed) {
				drag(x, y);
			}
			needRedraw = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			x = event.motion.x;
			y = event.motion.y;
			if (exit_.isOver(x, y))
				return std::make_shared<MainScreen>(window_, renderer_);
			if (event.button.button == SDL_BUTTON_LEFT)
				onLeftClic(x, y);
			if (event.button.button == SDL_BUTTON_RIGHT)
				onRightClic(x, y);
			needRedraw = true;
			break;
		}

		if (needRedraw) {
			needRedraw = false;
			draw(x, y);
		}
	}
}

void Editor::draw(float x, float y)
{
	renderer_.clear();

	SDL::Rect src_bg = { 0, 0, assets_.bg.getWidth(), assets_.bg.getHeight() };
	SDL::FRect dst_bg = { 0, 0, static_cast<float>(src_bg.w), static_cast<float>(src_bg.h) };

	for (dst_bg.y = 0; dst_bg.y < 400; dst_bg.y += src_bg.h) {
		for (dst_bg.x = 0; dst_bg.x < 400; dst_bg.x += src_bg.w)
			renderer_.copy(assets_.bg, src_bg, dst_bg);
	}

	canva_.visit(RenderVisitor{ renderer_, assets_, canva_ });

	renderer_.setDrawColor(255, 0, 0, 25);
	SDL::FRect filter = { 0, canva_.get_height() - 50, canva_.get_width(), 50 };
	renderer_.fillRect(filter);

	constexpr int dim_x = 128;
	constexpr int dim_y = 512;

	SDL::Rect src = { 0, 0, dim_x, dim_y };
	SDL::Rect dst = { 400 - dim_x, 0, dim_x, dim_y };

	renderer_.copy(assets_.ui, src, dst);

	for (Material &a : sources_) {
		a.draw(renderer_);
	}

	exit_.draw(renderer_, exit_.isOver(x, y));
	save_.draw(renderer_, save_.isOver(x, y));

	renderer_.present();
}

void Editor::onLeftClic(float x, float y)
{
	auto idx_brick = canva_.get_brick(x, y);
	if (!idx_brick && is_in_canva(x, y)) {
		auto idx = canva_.add_brick_safe(x, y, sources_[source_].getDurability());
		if (idx)
			idx_brick = { { *idx, canva_.get_brick(*idx) } };
	}

	if (idx_brick) {
		clicked_brick = idx_brick->first;
		click_offset_x = idx_brick->second.get_x() - x;
		click_offset_y = idx_brick->second.get_y() - y;
	}

	for (unsigned int i = 0; i < sources_.size(); i++) {
		if (sources_[i].isOver(x, y)) {
			sources_[source_].setSelected(false);
			sources_[i].setSelected(true);
			source_ = i;
			break;
		}
	}

	if (save_.isOver(x, y)) {
		const auto t = std::time(nullptr);
		const auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S");
		auto time = oss.str();

		std::ofstream out("save/save_" + time + ".save", std::ios::out);
		canva_.save(out);
		out.close();
	}
}

void Editor::onRightClic(float x, float y)
{
	if (auto idx = canva_.get_brick(x, y)) {
		canva_.remove_brick(idx->first);
	}
}

void Editor::drag(float x, float y)
{
	if (!clicked_brick)
		return;

	float new_x = x + click_offset_x;
	float new_y = y + click_offset_y;

	canva_.replace_brick_safe(*clicked_brick, new_x, new_y);
}
