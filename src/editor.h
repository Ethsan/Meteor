#pragma once

#include "fsm.h"
#include "sdl.h"
#include "logic.h"
#include "widget.h"
#include "game.h"
#include <array>
#include <cstddef>
#include <memory>

class Editor : public State {
    public:
	Editor(const SDL::Window &window, const SDL::Renderer &renderer)
		: window_(window)
		, renderer_(renderer)
		, assets_{ renderer_ }
		, exit_("Exit", font, textColor, textHighlight, renderer_, 325, 15)
		, save_("Save", font, textColor, textHighlight, renderer_, 325, 258)
		, sources_({ Material{ renderer_, 5, true, 325, 90 }, Material{ renderer_, 4, false, 325, 120 },
			     Material{ renderer_, 3, false, 325, 150 }, Material{ renderer_, 2, false, 325, 180 },
			     Material{ renderer_, 1, false, 325, 210 } })
		, canva_(300, 300, false){};

	std::shared_ptr<State> operator()();

    private:
	SDL::Window window_;
	SDL::Renderer renderer_;

	Assets assets_;
	static inline SDL::Font font{ "assets/ticketing.regular.ttf", 30 };
	static constexpr SDL::Color textColor = { 0xff, 0xff, 0xff, 0xff };
	static constexpr SDL::Color textHighlight = { 0xff, 0xe0, 0x7e, 0xff };

	Label exit_, save_;

	std::array<Material, 5> sources_;
	unsigned int source_ = 0;

	Logic canva_;

	void draw(float x, float y);

	void onLeftClic(float x, float y);
	void onRightClic(float x, float y);
	void drag(float x, float y);

	inline bool is_in_canva(float x, float y) const
	{
		return x - Brick::rect_w / 2 <= canva_.get_width() - Brick::rect_w && x - Brick::rect_w / 2 >= 0 &&
		       y - Brick::rect_h / 2 <= canva_.get_height() - Brick::rect_h - 50 && y - Brick::rect_h / 2 >= 0;
	}

	std::optional<std::size_t> clicked_brick = std::nullopt;
	float click_offset_x = 0, click_offset_y = 0;
};
