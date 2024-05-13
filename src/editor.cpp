#include "editor.h"
#include "SDL_events.h"
#include "SDL_mouse.h"
#include "logic.h"
#include "mainscreen.h"
#include "sdl.h"
#include "widget.h"
#include <SDL_events.h>

inline bool is_in_rect(int x, int y, SDL::Rect rect)
{
	return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

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

		int off = max_durability - static_cast<int>(brick.durability);

		SDL::Rect src = { off * dim, 0, dim, dim };

		float x = brick.x + brick.w / 2 - dim * 0.5;
		float y = brick.y + brick.h / 2 - dim * 0.5;

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
		while (auto event = SDL::pollEvent()) {
			is_mouseLeft_pressed = SDL::isPressedMouse(SDL_BUTTON_LMASK);
			if (!is_mouseLeft_pressed)
				clicked_brick = -1;

			if (event->type == SDL_QUIT)
				return std::make_shared<MainScreen>(window_, renderer_);
			if (event->type == SDL_MOUSEMOTION) {
				x = event->motion.x;
				y = event->motion.y;
				if (is_mouseLeft_pressed) {
					drag(x, y);
				}
				needRedraw = true;
			}
			if (event->type == SDL_MOUSEBUTTONDOWN) {
				x = event->motion.x;
				y = event->motion.y;
				if (is_in_rect(x, y, exit_.getRect()))
					return std::make_shared<MainScreen>(window_, renderer_);
				if (event->button.button == SDL_BUTTON_LEFT)
					onLeftClic(x, y);
				if (event->button.button == SDL_BUTTON_RIGHT)
					onRightClic(x, y);
				needRedraw = true;
			}
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

	renderer_.setDrawColor(0, 0, 20, 200);
	SDL::FRect filter = { 0, canva_.getHeight() - 50, canva_.getWidth(), 50 };
	renderer_.fillRect(filter);

	constexpr int dim_x = 128;
	constexpr int dim_y = 512;

	SDL::Rect src = { 0, 0, dim_x, dim_y };
	SDL::Rect dst = { 400 - dim_x, 0, dim_x, dim_y };

	renderer_.copy(assets_.ui, src, dst);

	for (Material &a : sources_) {
		a.draw(renderer_);
	}

	exit_.draw(renderer_, is_in_rect(x, y, exit_.getRect()));

	renderer_.present();
}

void Editor::onLeftClic(float x, float y)
{
	Brick tmp;
	tmp = canva_.brickLookup(x, y);
	if (tmp.id == -1 && is_in_canva(x, y))
		tmp = canva_.placeNewBrick(x, y, sources_[source_].getDurability());
	if (tmp.id != -1) {
		clicked_brick = tmp.id;
		clicked_origin_x = x;
		clicked_origin_y = y;
		clicked_brickOrigin_x = tmp.x;
		clicked_brickOrigin_y = tmp.y;
	}

	for (unsigned int i = 0; i < sources_.size(); i++) {
		if (sources_[i].isOver(x, y)) {
			sources_[source_].setSelected(false);
			sources_[i].setSelected(true);
			source_ = i;
			break;
		}
	}
}

void Editor::onRightClic(float x, float y)
{
	canva_.removeBrick(canva_.brickLookup(x, y).id);
}

void Editor::drag(float x, float y)
{
	if (clicked_brick == -1)
		return;

	float shift_x = x - clicked_origin_x, shift_y = y - clicked_origin_y;
	float dest_x = clicked_brickOrigin_x + shift_x, dest_y = clicked_brickOrigin_y + shift_y;

	if (is_in_canva(dest_x, dest_y))
		canva_.placeBrick(dest_x, dest_y, clicked_brick);
}