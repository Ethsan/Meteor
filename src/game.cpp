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
		int off = 0;
		if (brick.durability == 0) {
			int anim = (logic.getTick() - brick.last_hit) / 4;
			if (anim >= 6)
				return;
			off = 4 + anim;
		} else {
			off = max_durability - static_cast<int>(brick.durability);
		}

		SDL::Rect src = { off * 96, 0, 96, 96 };

		float w = 96 * brick.w / 48;
		float h = 96 * brick.h / 16;
		float x = brick.x + brick.w / 2 - w / 2;
		float y = brick.y + brick.h / 2 - h / 2;

		SDL::FRect dst = { x, y, w, h };
		renderer.copy(assets.brick, src, dst);
	}
	void operator()(const Ball &ball)
	{
		int off = logic.getTick() / 4 % 8;

		SDL::Rect src = { off * 32, 0, 32, 32 };
		float size = 32 * ball.r * 2 / 16;
		float x = ball.x - size / 2;
		float y = ball.y - size / 2;
		SDL::FRect dst = { x, y, size, size };
		renderer.copy(assets.ball, src, dst);
	}

	void operator()(const Paddle &paddle)
	{
		SDL::Rect src = { 0, 0, assets.paddle.getWidth(), assets.paddle.getHeight() };
		SDL::FRect dst = { paddle.x - paddle.w / 2, paddle.y - paddle.h / 2, paddle.w, paddle.h / 2 };
		renderer.copy(assets.paddle, src, dst);
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
