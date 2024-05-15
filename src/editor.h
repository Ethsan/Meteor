#pragma once

#include "fsm.h"
#include "game.h"
#include "logic.h"
#include "sdl.h"
#include "widget.h"

#include <array>
#include <memory>
#include <optional>
#include <string>

// The Editor class is similar as the Game class, but it is responsible for
// the editor logic.
class Editor : public State {
    public:
	Editor(const SDL::Window &w, const SDL::Renderer &r)
		: window(w)
		, renderer(r)
		, assets{ renderer }
		, exit("Exit", font, fg_color, hl_color, renderer, 325, 15)
		, save("Save", font, fg_color, hl_color, renderer, 325, 258)
		, sources({ Material{ renderer, 5, true, 325, 90 }, Material{ renderer, 4, false, 325, 120 },
			    Material{ renderer, 3, false, 325, 150 }, Material{ renderer, 2, false, 325, 180 },
			    Material{ renderer, 1, false, 325, 210 } })
		, canva(300, 300, false){};

	std::shared_ptr<State> operator()();

    private:
	SDL::Window window;
	SDL::Renderer renderer;

	Assets assets;
	static inline SDL::Font font{ "assets/ticketing.regular.ttf", 30 };
	static constexpr SDL::Color fg_color = { 0xff, 0xff, 0xff, 0xff };
	static constexpr SDL::Color hl_color = { 0xff, 0xe0, 0x7e, 0xff };

	Label exit, save;

	std::array<Material, 5> sources;
	unsigned int source = 0;

	Logic canva;

	void draw(float x, float y);

	void on_left_click(float x, float y);
	void on_right_click(float x, float y);
	void drag(float x, float y);

	inline bool is_in_canva(float x, float y) const
	{
		return x - Brick::rect_w / 2 <= canva.get_width() - Brick::rect_w && x - Brick::rect_w / 2 >= 0 &&
		       y - Brick::rect_h / 2 <= canva.get_height() - Brick::rect_h - 50 && y - Brick::rect_h / 2 >= 0;
	}

	std::optional<std::size_t> clicked_brick = std::nullopt;
	float click_offset_x = 0, click_offset_y = 0;
};
