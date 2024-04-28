#include "game.h"
#include "SDL_render.h"
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

		if (logic_.getState() != Logic::GameState::RUNNING) {
			return std::make_shared<MainScreen>(window_, renderer_);
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

		render();

		SDL::delay(16);
	};
}

void Game::render_menu()
{
	constexpr int dim = 32;
	constexpr int size = 64;

	SDL::Rect corner = { 0, 0, dim, dim };
	SDL::Rect border_left = { 2 * dim, 0, dim, dim };
	SDL::Rect bg = { 3 * dim, 0, dim, dim };

	SDL::FRect dst = { 600 - static_cast<float>(size) / 2, -static_cast<float>(size) / 2, size, size };
	renderer_.copy(assets_.ui, corner, dst, 0, { 0, 0 }, SDL_FLIP_NONE);

	for (dst.y = size * .5; dst.y < 600 - size; dst.y += size) {
		renderer_.copy(assets_.ui, border_left, dst, 0, { 0, 0 }, SDL_FLIP_NONE);
	}

	dst.y = 600 - static_cast<float>(size) * 3. / 2;
	renderer_.copy(assets_.ui, border_left, dst, 0, { 0, 0 }, SDL_FLIP_NONE);

	dst.y += size;
	renderer_.copy(assets_.ui, corner, dst, 0, { 0, 0 }, SDL_FLIP_VERTICAL);

	for (dst.x = 600 + size * .5; dst.x < 800; dst.x += size) {
		for (dst.y = 0; dst.y < 600; dst.y += size) {
			renderer_.copy(assets_.ui, bg, dst, 0, { 0, 0 }, SDL_FLIP_NONE);
		}
	}

	SDL::Texture score =
		SDL::Texture(renderer_, assets_.font.renderText("Score: " + std::to_string(logic_.getScore()),
								{ 255, 255, 255, 255 }));
	SDL::Rect scoreRect = score.getRect();
	SDL::FRect scoreDst = { 600 + size * .5, static_cast<float>(size) * 3. / 2, static_cast<float>(scoreRect.w),
				static_cast<float>(scoreRect.h) };

	SDL::Texture speed = SDL::Texture(
		renderer_, assets_.font.renderText("Speed: " + std::to_string(static_cast<int>(logic_.getSpeed())),
						   { 255, 255, 255, 255 }));
	SDL::Rect speedRect = speed.getRect();
	SDL::FRect speedDst = { 600 + size * .5, static_cast<float>(size * 3. / 2 + scoreRect.h),
				static_cast<float>(speedRect.w), static_cast<float>(speedRect.h) };

	SDL::Texture times =
		SDL::Texture(renderer_, assets_.font.renderText("Time: " + std::to_string(logic_.getTick() / 60),
								{ 255, 255, 255, 255 }));
	SDL::Rect timesRect = times.getRect();
	SDL::FRect timesDst = { 600 + size * .5, static_cast<float>(size * 3. / 2 + scoreRect.h + speedRect.h),
				static_cast<float>(timesRect.w), static_cast<float>(timesRect.h) };

	renderer_.copy(score, scoreRect, scoreDst);
	renderer_.copy(speed, speedRect, speedDst);
	renderer_.copy(times, timesRect, timesDst);
}

void Game::render()
{
	SDL::Rect src_bg = { 0, 0, assets_.bg.getWidth(), assets_.bg.getHeight() };
	SDL::FRect dst_bg = { 0, 0, static_cast<float>(src_bg.w), static_cast<float>(src_bg.h) };

	for (dst_bg.y = 0; dst_bg.y < 800; dst_bg.y += src_bg.h) {
		for (dst_bg.x = 0; dst_bg.x < 800; dst_bg.x += src_bg.w)
			renderer_.copy(assets_.bg, src_bg, dst_bg);
	}

	logic_.visit(RenderVisitor{ renderer_, assets_, logic_ });

	render_menu();

	renderer_.present();
}
