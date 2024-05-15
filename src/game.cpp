#include "game.h"

#include "exception.h"
#include "logic.h"
#include "mainscreen.h"
#include "sdl.h"
#include "widget.h"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>

struct RenderVisitor {
	SDL::Renderer &renderer;
	const Assets &assets;
	const Logic &logic;

	void operator()(const auto &)
	{
	}

	void operator()(const Powerup &powerup)
	{
		constexpr int dim = 16;

		if (!powerup.is_alive())
			return;

		int off = powerup.get_power();

		SDL::Rect src = { off * dim, 0, dim, dim };
		SDL::FRect dst = { powerup.get_x() - dim / 2.f, powerup.get_y() - dim / 2.f, dim, dim };
		renderer.copy(assets.powerups, src, dst);
	}

	void operator()(const Brick &brick)
	{
		constexpr int max_dura = 5;
		constexpr int dim = 96;

		int off = 0;
		if (brick.get_durability() == 0) {
			int anim = (logic.get_tick() - brick.get_last_hit()) / 4;
			if (anim >= 6)
				return;
			off = 4 + anim;
		} else {
			off = max_dura - static_cast<int>(brick.get_durability());
		}

		SDL::Rect src = { off * dim, 0, dim, dim };

		float x = brick.get_x() - dim / 2.;
		float y = brick.get_y() - dim / 2.;

		SDL::FRect dst = { x, y, dim, dim };
		switch (brick.get_form()) {
		case Brick::rect:
			return renderer.copy(assets.brick_rect, src, dst);
		case Brick::hex:
			return renderer.copy(assets.brick_hex, src, dst);
		}
	}

	void operator()(const Ball &ball)
	{
		constexpr int dim = 32;

		int off = logic.get_tick() / 4 % 8;

		SDL::Rect src = { off * dim, 0, dim, dim };

		float x = ball.get_x() - dim * 0.5;
		float y = ball.get_y() - dim * 0.5;
		SDL::FRect dst = { x, y, dim, dim };
		renderer.copy(assets.ball, src, dst);
	}

	void operator()(const Paddle &paddle)
	{
		constexpr int dim = 64;

		int off = 7;

		SDL::Rect src = { dim * off, 0, dim, dim };

		float x = paddle.get_x() - dim * 0.5;
		float y = paddle.get_y() - dim * 0.5;

		SDL::FRect dst = { x, y, dim, dim };
		renderer.copy(assets.paddle, src, dst);

		off = logic.get_tick() / 4 % 8;

		dst.y += 4;

		switch (paddle.get_dir()) {
		case Paddle::left:
			src = { dim * (off + 1), 0, dim, dim };
			renderer.copy(assets.ship_left, src, dst);
			break;
		case Paddle::right: {
			src = { dim * (off + 1), 0, dim, dim };
			renderer.copy(assets.ship_right, src, dst);
			break;
		}
		default: {
			src = { 0, 0, dim, dim };
			renderer.copy(assets.ship, src, dst);
			break;
		}
		}

		if (logic.get_ball_count() == 0) {
			(*this)(Ball(paddle.get_x(), paddle.get_y() - paddle.h / 2. - Ball::r, 0, 0));
		}
	}
};

std::shared_ptr<State> Game::operator()()
{
	std::optional<std::pair<int, int> > mouse_pos = std::make_pair(0, 0);
	mouse_pos = std::nullopt;

	for (;;) {
		while (auto event = SDL::pollEvent()) {
			switch (event->type) {
			case SDL_QUIT:
				throw Close();
			case SDL_KEYDOWN:
				switch (event->key.keysym.sym) {
				case SDLK_ESCAPE:
					if (auto state = pause()) {
						// quit to the menu
						return *state;
					}
					break;
				case SDLK_SPACE:
					logic.launch_ball();
					break;
				};
				break;
			case SDL_MOUSEMOTION:
				mouse_pos = std::make_pair(event->motion.x, event->motion.y);
				break;
			}
		}

		logic.step(16.f / 1000);

		if (logic.get_state() != Logic::GameState::RUNNING) {
			return end();
		}

		bool is_left_pressed = SDL::isPressed(SDL_SCANCODE_LEFT);
		bool is_right_pressed = SDL::isPressed(SDL_SCANCODE_RIGHT);

		if (is_left_pressed && !is_right_pressed) {
			logic.set_paddle_dir(Paddle::left);
			mouse_pos = std::nullopt;
		} else if (!is_left_pressed && is_right_pressed) {
			logic.set_paddle_dir(Paddle::right);
			mouse_pos = std::nullopt;
		} else if (mouse_pos) {
			float margin = 10;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // GCC false positive
			if (mouse_pos->first < logic.get_paddle().get_x() - margin) {
				logic.set_paddle_dir(Paddle::left);
			} else if (mouse_pos->first > logic.get_paddle().get_x() + margin) {
				logic.set_paddle_dir(Paddle::right);
#pragma GCC diagnostic pop
			} else {
				logic.set_paddle_dir(Paddle::none);
			}
		} else {
			logic.set_paddle_dir(Paddle::none);
		}

		draw();

		renderer.present();

		SDL::delay(16);
	}
}

void Game::draw()
{
	SDL::Rect src_bg = { 0, 0, assets.bg.getWidth(), assets.bg.getHeight() };
	SDL::FRect dst_bg = { 0, 0, static_cast<float>(src_bg.w), static_cast<float>(src_bg.h) };

	for (dst_bg.y = 0; dst_bg.y < 400; dst_bg.y += src_bg.h) {
		for (dst_bg.x = 0; dst_bg.x < 400; dst_bg.x += src_bg.w)
			renderer.copy(assets.bg, src_bg, dst_bg);
	}

	logic.visit(RenderVisitor{ renderer, assets, logic });

	constexpr int ball_dim = 32;
	constexpr int dim_x = 128;
	constexpr int dim_y = 512;

	SDL::Rect src = { 0, 0, dim_x, dim_y };
	SDL::Rect dst = { 400 - dim_x, 0, dim_x, dim_y };

	renderer.copy(assets.ui, src, dst);

	SDL::Texture score =
		ui_factory.create_label(renderer, std::to_string(logic.get_score()), { 255, 255, 255, 255 });

	SDL::Rect score_src = score.getRect();
	SDL::FRect score_dst = { 350.f - score_src.w * 0.5f, 30.f - score_src.h * 0.5f, static_cast<float>(score_src.w),
				 static_cast<float>(score_src.h) };

	SDL::Rect ball_src = { 0, 0, ball_dim, ball_dim };
	SDL::FRect ball_dst = { 330.f - ball_dim * 0.5f, 272 - ball_dim * 0.5f, static_cast<float>(ball_src.w),
				static_cast<float>(ball_src.h) };

	renderer.copy(score, score_src, score_dst);

	for (int i = 0; i < std::min(logic.get_lives(), 2); ++i) {
		ball_dst.x = 340.f - ball_dim * 0.5f + i * 48.f / 2.f;
		renderer.copy(assets.ball, ball_src, ball_dst);
	}
}

struct Button {
	SDL::Texture texture;
	SDL::Texture over;

	SDL::Rect src = texture.getRect();
	SDL::Rect dst = src;

	void draw(SDL::Renderer &renderer)
	{
		renderer.copy(texture, src, dst);
	}

	void draw_over(SDL::Renderer &renderer)
	{
		renderer.copy(over, src, dst);
	}

	bool contains(int x, int y)
	{
		return dst.x <= x && x <= dst.x + dst.w && dst.y <= y && y <= dst.y + dst.h;
	}
};

struct Element {
	SDL::Texture texture;

	SDL::Rect src = texture.getRect();
	SDL::Rect dst = src;

	void draw(SDL::Renderer &renderer)
	{
		renderer.copy(texture, src, dst);
	}
};

std::optional<std::shared_ptr<State> > Game::pause()
{
	int spacing = 30;

	int button_width = 75;
	int button_height = 25;

	std::array<Button, 3> buttons = {
		{ { ui_factory.create_button(renderer, "RESUME", button_width, button_height),
		    ui_factory.create_button_over(renderer, "RESUME", button_width, button_height) },
		  { ui_factory.create_button(renderer, "RESTART", button_width, button_height),
		    ui_factory.create_button_over(renderer, "RESTART", button_width, button_height) },
		  { ui_factory.create_button(renderer, "QUIT", button_width, button_height),
		    ui_factory.create_button_over(renderer, "QUIT", button_width, button_height) } }
	};

	int box_width = 100;
	int box_height = static_cast<int>(buttons.size() + 1) * spacing;

	for (size_t i = 0; i < buttons.size(); ++i) {
		SDL::Rect src = buttons[i].src;
		buttons[i].dst = { 200 - src.w / 2,
				   150 - box_height / 2 + static_cast<int>(i) * (spacing) + spacing / 2, src.w, src.h };
	};

	Element box(ui_factory.create_big_box(renderer, box_width, box_height));
	box.dst = { 200 - box_width / 2, 150 - box_height / 2, box_width, box_height };

	Element title(ui_factory.create_button(renderer, "PAUSED"));
	title.dst = { 200 - title.src.w / 2, box.dst.y - title.src.h / 2, title.src.w, title.src.h };

	for (;;) {
		// draw game as background
		draw();

		// blur the game
		renderer.setDrawColor(0, 0, 0, 128);
		renderer.fillRect((SDL::Rect){ 0, 0, 400, 400 });

		// draw pause menu
		box.draw(renderer);
		title.draw(renderer);

		SDL::Point win;
		SDL::getMouseState(win.x, win.y);
		SDL::FPoint pos = renderer.windowToLogical(win);

		for (auto &button : buttons) {
			if (button.contains(pos.x, pos.y)) {
				button.draw_over(renderer);
			} else {
				button.draw(renderer);
			}
		}

		renderer.present();

		SDL::delay(16);

		while (auto event = SDL::pollEvent()) {
			switch (event->type) {
			case SDL_QUIT:
				throw Close();
			case SDL_KEYDOWN:
				if (event->key.keysym.sym == SDLK_ESCAPE) {
					return resume();
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				int x = event->button.x;
				int y = event->button.y;
				if (buttons[0].contains(x, y)) {
					return resume();
				} else if (buttons[1].contains(x, y)) {
					if (!save_file.empty()) {
						try {
							return std::make_shared<Game>(window, renderer, save_file);
						} catch (Bad_format const &) {
							return std::make_shared<Game>(window, renderer);
						}
					}
					return std::make_shared<Game>(window, renderer);
				} else if (buttons[2].contains(x, y)) {
					return std::make_shared<MainScreen>(window, renderer);
				}
				break;
			}
		}
	}
}

std::optional<std::shared_ptr<State> > Game::resume()
{
	int tick = 179;

	for (;;) {
		draw();

		SDL::Texture counter =
			ui_factory.create_label(renderer, std::to_string(tick / 60 + 1), { 255, 255, 255, 255 });
		SDL::Rect src = { 0, 0, counter.getWidth(), counter.getHeight() };

		// simple scale animation
		float scale = 2.5 - static_cast<float>(tick % 60) / 60;
		float w = src.w * scale;
		float h = src.h * scale;
		SDL::FRect dst = { 150 - w / 2, 150 - h / 2, w, h };

		renderer.copy(counter, src, dst);

		renderer.present();

		SDL::delay(16);

		tick--;
		if (tick == 0) {
			// resume game
			return std::nullopt;
		}

		while (auto event = SDL::pollEvent()) {
			;
		}
	}
}

std::shared_ptr<State> Game::end()
{
	std::string title_text;
	if (logic.get_state() == Logic::WIN) {
		title_text = "YOU WIN";
	} else {
		title_text = "YOU LOSE";
	}

	Element title(ui_factory.create_button(renderer, title_text));

	int box_width = 120;
	int box_height = 140;

	Element box(ui_factory.create_big_box(renderer, box_width, box_height));

	box.dst = { 150 - box_width / 2, 150 - box_height / 2, box_width, box_height };
	title.dst = { 150 - title.src.w / 2, box.dst.y - title.src.h / 2, title.src.w, title.src.h };

	Element score_text(ui_factory.create_label(renderer, "Score :", { 255, 255, 255, 255 }));
	score_text.dst = { 150 - score_text.src.w / 2, box.dst.y + 16, score_text.src.w, score_text.src.h };

	Element score(ui_factory.create_label(renderer, std::to_string(logic.get_score()), { 255, 255, 255, 255 }));
	score.dst = { 150 - score.src.w / 2, score_text.dst.y + score_text.dst.h + 16, score.src.w, score.src.h };

	Button home(ui_factory.create_home_button(renderer), ui_factory.create_home_button_over(renderer));
	Button restart(ui_factory.create_restart_button(renderer), ui_factory.create_restart_button_over(renderer));

	int spacing = box.dst.w / 3;
	home.dst = { box.dst.x + spacing - home.src.w / 2, box.dst.y + box.dst.h - 32, home.src.w, home.src.h };
	restart.dst = { box.dst.x + 2 * spacing - restart.src.w / 2, box.dst.y + box.dst.h - 32, restart.src.w,
			restart.src.h };

	for (;;) { // draw game as background
		draw();

		// draw pause menu
		box.draw(renderer);
		title.draw(renderer);

		score_text.draw(renderer);
		score.draw(renderer);

		// draw separator
		renderer.setDrawColor(255, 255, 255, 255);
		int y = home.dst.y - 16;
		renderer.drawLine(box.dst.x + 10, y, box.dst.x + box.dst.w - 10, y);

		SDL::Point win;
		SDL::getMouseState(win.x, win.y);
		SDL::FPoint log = renderer.windowToLogical(win);

		if (home.contains(log.x, log.y)) {
			home.draw_over(renderer);
		} else {
			home.draw(renderer);
		}
		if (restart.contains(log.x, log.y)) {
			restart.draw_over(renderer);
		} else {
			restart.draw(renderer);
		}

		renderer.present();

		SDL::delay(16);

		while (auto event = SDL::pollEvent()) {
			switch (event->type) {
			case SDL_QUIT:
				throw Close();
			case SDL_MOUSEBUTTONDOWN:
				int x = event->button.x;
				int y = event->button.y;
				if (home.contains(x, y)) {
					return std::make_shared<MainScreen>(window, renderer);
				} else if (restart.contains(x, y)) {
					if (!save_file.empty()) {
						try {
							return std::make_shared<Game>(window, renderer, save_file);
						} catch (Bad_format const &) {
							return std::make_shared<Game>(window, renderer);
						}
					}
					return std::make_shared<Game>(window, renderer);
				}
				break;
			}
		}
	}
}
