#include "game.h"
#include "SDL_scancode.h"
#include "mainscreen.h"
#include "sdl.h"

struct RenderVisitor {
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
		constexpr int sprite_w = 48;
		constexpr int sprite_h = 16;

		float scale_x = brick.w / sprite_w;
		float scale_y = brick.h / sprite_h;

		int off = 0;
		if (brick.durability == 0) {
			int anim = (logic.getTick() - brick.last_hit) / 4;
			if (anim >= 6)
				return;
			off = 4 + anim;
		} else {
			off = max_durability - static_cast<int>(brick.durability);
		}

		SDL::Rect src = { off * dim, 0, dim, dim };

		float w = dim * scale_x;
		float h = dim * scale_y;
		float x = brick.x + brick.w / 2 - w / 2;
		float y = brick.y + brick.h / 2 - h / 2;

		SDL::FRect dst = { x, y, w, h };
		renderer.copy(assets.brick, src, dst);
	}
	void operator()(const Ball &ball)
	{
		constexpr int dim = 32;
		constexpr int sprite_size = 16;

		float scale = ball.r * 2 / sprite_size;

		int off = logic.getTick() / 4 % 8;

		SDL::Rect src = { off * dim, 0, dim, dim };

		float size = dim * scale;
		float x = ball.x - size / 2;
		float y = ball.y - size / 2;
		SDL::FRect dst = { x, y, size, size };
		renderer.copy(assets.ball, src, dst);
	}

	void operator()(const Paddle &paddle)
	{
		constexpr int dim = 64;
		constexpr int sprite_w = 56;
		constexpr int sprite_h = 33;

		float scale_x = paddle.w / sprite_w;
		float scale_y = paddle.h / sprite_h;

		int off = 7;

		SDL::Rect src = { dim * off, 0, dim, dim };

		float w = dim * scale_x;
		float h = dim * scale_y;
		float x = paddle.x - w / 2;
		float y = paddle.y - h / 2;

		SDL::FRect dst = { x, y, w, h };
		renderer.copy(assets.paddle, src, dst);

		constexpr int dim_ship = 64;

		off = logic.getTick() / 4 % 8;

		w = dim_ship * scale_x;
		h = dim_ship * scale_y;
		y += 4 * scale_y;

		dst = { x, y, w, h };
		if (logic.dir == LEFT) {
			src = { dim * (off + 1), 0, dim, dim };
			renderer.copy(assets.ship_left, src, dst);
		} else if (logic.dir == RIGHT) {
			src = { dim * (off + 1), 0, dim, dim };
			renderer.copy(assets.ship_right, src, dst);
		} else {
			src = { 0, 0, dim, dim };
			renderer.copy(assets.ship, src, dst);
		}
	}
};

std::shared_ptr<State> Game::operator()()
{
	for (;;) {
		while (auto event = SDL::pollEvent()) {
			if (event->type == SDL_QUIT) {
				return std::make_shared<MainScreen>(window_, renderer_);
			}
		}
		auto n = 1;
		for (auto i = 0; i < n; ++i) {
			logic_.step(16.0 / n / 1000);
		}

		bool is_left_pressed = SDL::isPressed(SDL_SCANCODE_LEFT);
		bool is_right_pressed = SDL::isPressed(SDL_SCANCODE_RIGHT);

		if (is_left_pressed && !is_right_pressed) {
			logic_.dir = LEFT;
		} else if (!is_left_pressed && is_right_pressed) {
			logic_.dir = RIGHT;
		} else {
			logic_.dir = NONE;
		}

		renderer_.clear();
		logic_.visit(RenderVisitor{ renderer_, assets_, logic_ });
		if (logic_.getState() != Logic::GameState::RUNNING) {
			return std::make_shared<MainScreen>(window_, renderer_);
		}
		renderer_.present();

		SDL::delay(16);
	};
}
