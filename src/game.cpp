#include "game.h"
#include "logic.h"
#include "sdl.h"
#include "mainscreen.h"
#include "widget.h"
#include "exception.h"
#include <optional>

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
		case Brick::RECT:
			return renderer.copy(assets.brick_rect, src, dst);
		case Brick::HEX:
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
	std::optional<std::pair<int, int> > mouse_pos = std::make_pair(0, 0);
	mouse_pos = std::nullopt;

	for (;;) {
		while (auto event = SDL::pollEvent()) {
			if (event->type == SDL_QUIT) {
				throw Close();
			} else if (event->type == SDL_KEYDOWN) {
				if (event->key.keysym.sym == SDLK_ESCAPE) {
					if (auto state = pause()) {
						return *state;
					}
				}
			} else if (event->type == SDL_MOUSEMOTION) {
				mouse_pos = std::make_pair(event->motion.x, event->motion.y);
			}
		}

		logic_.step(16.f / 1000);

		if (logic_.get_state() != Logic::GameState::RUNNING) {
			return end();
		}

		bool is_left_pressed = SDL::isPressed(SDL_SCANCODE_LEFT);
		bool is_right_pressed = SDL::isPressed(SDL_SCANCODE_RIGHT);

		if (is_left_pressed && !is_right_pressed) {
			logic_.dir = LEFT;
			mouse_pos = std::nullopt;
		} else if (!is_left_pressed && is_right_pressed) {
			logic_.dir = RIGHT;
			mouse_pos = std::nullopt;
		} else if (mouse_pos) {
			float margin = 10;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // GCC false positive
			if (mouse_pos->first < logic_.get_paddle().get_x() - margin) {
				logic_.dir = LEFT;
			} else if (mouse_pos->first > logic_.get_paddle().get_x() + margin) {
				logic_.dir = RIGHT;
#pragma GCC diagnostic pop
			} else {
				logic_.dir = NONE;
			}
		} else {
			logic_.dir = NONE;
		}

		draw();

		renderer_.present();

		SDL::delay(16);
	};
}

void Game::draw()
{
	SDL::Rect src_bg = { 0, 0, assets_.bg.getWidth(), assets_.bg.getHeight() };
	SDL::FRect dst_bg = { 0, 0, static_cast<float>(src_bg.w), static_cast<float>(src_bg.h) };

	for (dst_bg.y = 0; dst_bg.y < 400; dst_bg.y += src_bg.h) {
		for (dst_bg.x = 0; dst_bg.x < 400; dst_bg.x += src_bg.w)
			renderer_.copy(assets_.bg, src_bg, dst_bg);
	}

	logic_.visit(RenderVisitor{ renderer_, assets_, logic_ });

	constexpr int ball_dim = 32;
	constexpr int dim_x = 128;
	constexpr int dim_y = 512;

	SDL::Rect src = { 0, 0, dim_x, dim_y };
	SDL::Rect dst = { 400 - dim_x, 0, dim_x, dim_y };

	renderer_.copy(assets_.ui, src, dst);

	SDL::Texture score =
		ui_factory_.create_label(renderer_, std::to_string(logic_.get_score()), { 255, 255, 255, 255 });

	SDL::Rect scoreRect = score.getRect();
	SDL::FRect scoreDst = { 350.f - scoreRect.w * 0.5f, 30.f - scoreRect.h * 0.5f, static_cast<float>(scoreRect.w),
				static_cast<float>(scoreRect.h) };

	SDL::Rect ballRect = { 0, 0, ball_dim, ball_dim };
	SDL::FRect ballDst = { 330.f - ball_dim * 0.5f, 272 - ball_dim * 0.5f, static_cast<float>(ballRect.w),
			       static_cast<float>(ballRect.h) };

	renderer_.copy(score, scoreRect, scoreDst);

	for (int i = 0; i < logic_.get_lives() - 1; ++i) {
		ballDst.x = 340.f - ball_dim * 0.5f + i * 48.f / 2.f;
		renderer_.copy(assets_.ball, ballRect, ballDst);
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
		{ { ui_factory_.create_button(renderer_, "RESUME", button_width, button_height),
		    ui_factory_.create_button_over(renderer_, "RESUME", button_width, button_height) },
		  { ui_factory_.create_button(renderer_, "RESTART", button_width, button_height),
		    ui_factory_.create_button_over(renderer_, "RESTART", button_width, button_height) },
		  { ui_factory_.create_button(renderer_, "QUIT", button_width, button_height),
		    ui_factory_.create_button_over(renderer_, "QUIT", button_width, button_height) } }
	};

	int box_width = 100;
	int box_height = static_cast<int>(buttons.size() + 1) * spacing;

	for (size_t i = 0; i < buttons.size(); ++i) {
		SDL::Rect src = buttons[i].src;
		buttons[i].dst = { 200 - src.w / 2,
				   150 - box_height / 2 + static_cast<int>(i) * (spacing) + spacing / 2, src.w, src.h };
	};

	Element box(ui_factory_.create_big_box(renderer_, box_width, box_height));
	box.dst = { 200 - box_width / 2, 150 - box_height / 2, box_width, box_height };

	Element title(ui_factory_.create_button(renderer_, "PAUSED"));
	title.dst = { 200 - title.src.w / 2, box.dst.y - title.src.h / 2, title.src.w, title.src.h };

	for (;;) {
		// draw game as background
		draw();

		// blur the game
		renderer_.setDrawColor(0, 0, 0, 128);
		renderer_.fillRect((SDL::Rect){ 0, 0, 400, 400 });

		// draw pause menu
		box.draw(renderer_);
		title.draw(renderer_);

		SDL::Point win;
		SDL::getMouseState(win.x, win.y);
		SDL::FPoint pos = renderer_.windowToLogical(win);

		for (auto &button : buttons) {
			if (button.contains(pos.x, pos.y)) {
				button.draw_over(renderer_);
			} else {
				button.draw(renderer_);
			}
		}

		renderer_.present();

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
					if (!save_file_.empty()) {
						try {
							return std::make_shared<Game>(window_, renderer_, save_file_);
						} catch (BadSaveFormat const &) {
							return std::make_shared<Game>(window_, renderer_);
						}
					}
					return std::make_shared<Game>(window_, renderer_);
				} else if (buttons[2].contains(x, y)) {
					return std::make_shared<MainScreen>(window_, renderer_);
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
			ui_factory_.create_label(renderer_, std::to_string(tick / 60 + 1), { 255, 255, 255, 255 });
		SDL::Rect src = { 0, 0, counter.getWidth(), counter.getHeight() };

		// simple scale animation
		float scale = 2.5 - static_cast<float>(tick % 60) / 60;
		float w = src.w * scale;
		float h = src.h * scale;
		SDL::FRect dst = { 150 - w / 2, 150 - h / 2, w, h };

		renderer_.copy(counter, src, dst);

		renderer_.present();

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
	if (logic_.get_state() == Logic::GameState::WIN) {
		title_text = "YOU WIN";
	} else {
		title_text = "YOU LOSE";
	}

	Element title(ui_factory_.create_button(renderer_, title_text));

	int box_width = 120;
	int box_height = 140;

	Element box(ui_factory_.create_big_box(renderer_, box_width, box_height));

	box.dst = { 150 - box_width / 2, 150 - box_height / 2, box_width, box_height };
	title.dst = { 150 - title.src.w / 2, box.dst.y - title.src.h / 2, title.src.w, title.src.h };

	Element score_text(ui_factory_.create_label(renderer_, "Score :", { 255, 255, 255, 255 }));
	score_text.dst = { 150 - score_text.src.w / 2, box.dst.y + 16, score_text.src.w, score_text.src.h };

	Element score(ui_factory_.create_label(renderer_, std::to_string(logic_.get_score()), { 255, 255, 255, 255 }));
	score.dst = { 150 - score.src.w / 2, score_text.dst.y + score_text.dst.h + 16, score.src.w, score.src.h };

	Button home(ui_factory_.create_home_button(renderer_), ui_factory_.create_home_button_over(renderer_));
	Button restart(ui_factory_.create_restart_button(renderer_), ui_factory_.create_restart_button_over(renderer_));

	int spacing = box.dst.w / 3;
	home.dst = { box.dst.x + spacing - home.src.w / 2, box.dst.y + box.dst.h - 32, home.src.w, home.src.h };
	restart.dst = { box.dst.x + 2 * spacing - restart.src.w / 2, box.dst.y + box.dst.h - 32, restart.src.w,
			restart.src.h };

	for (;;) { // draw game as background
		draw();

		// draw pause menu
		box.draw(renderer_);
		title.draw(renderer_);

		score_text.draw(renderer_);
		score.draw(renderer_);

		// draw separator
		renderer_.setDrawColor(255, 255, 255, 255);
		int y = home.dst.y - 16;
		renderer_.drawLine(box.dst.x + 10, y, box.dst.x + box.dst.w - 10, y);

		SDL::Point win;
		SDL::getMouseState(win.x, win.y);
		SDL::FPoint log = renderer_.windowToLogical(win);

		if (home.contains(log.x, log.y)) {
			home.draw_over(renderer_);
		} else {
			home.draw(renderer_);
		}
		if (restart.contains(log.x, log.y)) {
			restart.draw_over(renderer_);
		} else {
			restart.draw(renderer_);
		}

		renderer_.present();

		SDL::delay(16);

		while (auto event = SDL::pollEvent()) {
			switch (event->type) {
			case SDL_QUIT:
				throw Close();
			case SDL_MOUSEBUTTONDOWN:
				int x = event->button.x;
				int y = event->button.y;
				if (home.contains(x, y)) {
					return std::make_shared<MainScreen>(window_, renderer_);
				} else if (restart.contains(x, y)) {
					if (!save_file_.empty()) {
						try {
							return std::make_shared<Game>(window_, renderer_, save_file_);
						} catch (BadSaveFormat const &) {
							return std::make_shared<Game>(window_, renderer_);
						}
					}
					return std::make_shared<Game>(window_, renderer_);
				}
				break;
			}
		}
	}
}
