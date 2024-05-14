#pragma once

#include "fsm.h"
#include "sdl.h"
#include "logic.h"
#include "widget.h"
#include <array>
#include <cmath>
#include <memory>

struct EditorAssets {
	SDL::Texture brick;
	SDL::Texture ui;
	SDL::Texture bg;
};

class Editor : public State {
    public:
	Editor(const SDL::Window &window, const SDL::Renderer &renderer)
		: window_(window)
		, renderer_(renderer)
		, assets_{ .brick = { renderer_, "assets/asteroid.png" },
			   .ui = { renderer_, "assets/side.png" },
			   .bg = { renderer_, "assets/bg.png" } }
		, exit_("Exit", font, textColor, textHighlight, renderer_, 325, 15)
		, save_("Save", font, textColor, textHighlight, renderer_, 325, 258)
		, sources_({ Material{ renderer_, 5, true, 325, 90 }, Material{ renderer_, 4, false, 325, 120 },
			     Material{ renderer_, 3, false, 325, 150 }, Material{ renderer_, 2, false, 325, 180 },
			     Material{ renderer_, 1, false, 325, 210 } })
		, canva_(300, 300, true){};

	std::shared_ptr<State> operator()();

    private:
	SDL::Window window_;
	SDL::Renderer renderer_;

	EditorAssets assets_;
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
		return x <= canva_.getWidth() - Brick::w && x >= 0 && y <= canva_.getHeight() - Brick::h - 50 && y >= 0;
	}

	int clicked_brick = -1;
	int clicked_origin_x = -1, clicked_origin_y = -1;
	int clicked_brickOrigin_x = -1, clicked_brickOrigin_y = -1;
};