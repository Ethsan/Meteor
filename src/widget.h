#include "sdl.h"

class Label {
    private:
	SDL::Texture texture_;

    public:
	int x, y;

	Label(const std::string &text, const SDL::Font &font, const SDL::Color &color, SDL::Renderer &renderer, int x,
	      int y)
		: texture_(renderer, font.renderText(text, color))
		, x(x)
		, y(y)
	{
	}

	void draw(SDL::Renderer &renderer)
	{
		const SDL::Rect textureRect = texture_.getRect();
		const SDL::Rect rect = { x, y, textureRect.w, textureRect.h };
		renderer.copy(texture_, textureRect, rect);
	}

	void setText(const std::string &text, const SDL::Font &font, const SDL::Color &color, SDL::Renderer &renderer)
	{
		texture_ = SDL::Texture(renderer, font.renderText(text, color));
	}

	SDL::Rect getRect()
	{
		return { x, y, texture_.getRect().w, texture_.getRect().h };
	}
};
