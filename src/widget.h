#pragma once

#include "sdl.h"

#include <string>

class Label {
    private:
	SDL::Texture texture;
	SDL::Texture texture_h;
	std::string text;

    public:
	int x, y;

	Label(const std::string &text, const SDL::Font &font, const SDL::Color &color, const SDL::Color &color_h,
	      SDL::Renderer &renderer, int x, int y)
		: texture(renderer, font.renderText(text, color))
		, texture_h(renderer, font.renderText(text, color_h))
		, text(text)
		, x(x)
		, y(y)
	{
	}

	void draw(SDL::Renderer &renderer)
	{
		const SDL::Rect textureRect = texture.getRect();
		const SDL::Rect rect = { x, y, textureRect.w, textureRect.h };
		renderer.copy(texture, textureRect, rect);
	}

	void draw(SDL::Renderer &renderer, bool highlighted)
	{
		if (highlighted) {
			const SDL::Rect textureRect = texture_h.getRect();
			const SDL::Rect rect = { x, y, textureRect.w, textureRect.h };
			renderer.copy(texture_h, textureRect, rect);
		} else
			draw(renderer);
	}

	inline bool is_over(int a, int b)
	{
		return a >= x && a <= x + texture.getWidth() && b >= y && b <= y + texture.getHeight();
	};

	void set_text(const std::string &str, const SDL::Font &font, const SDL::Color &color, const SDL::Color &color_h,
		      SDL::Renderer &renderer)
	{
		texture = SDL::Texture(renderer, font.renderText(str, color));
		texture_h = SDL::Texture(renderer, font.renderText(str, color_h));
		text = str;
	}

	inline std::string get_text()
	{
		return text;
	}

	inline SDL::Rect get_rect()
	{
		return { x, y, texture.getRect().w, texture.getRect().h };
	}
};

class UI_Factory {
    private:
	SDL::Texture sprites;
	SDL::Font font = SDL::Font("assets/mag.ttf", 32);

	SDL::Texture create_box(SDL::Renderer &renderer, int w, int h, int dim, int off_x, int off_y)
	{
		SDL::Rect corner_tl = { off_x, off_y, dim, dim };
		SDL::Rect corner_tr = { off_x + dim * 2, off_y, dim, dim };
		SDL::Rect corner_bl = { off_x, off_y + dim * 2, dim, dim };
		SDL::Rect corner_br = { off_x + dim * 2, off_y + dim * 2, dim, dim };
		SDL::Rect edge_top = { off_x + dim, off_y, dim, dim };
		SDL::Rect edge_bottom = { off_x + dim, off_y + dim * 2, dim, dim };
		SDL::Rect edge_left = { off_x, off_y + dim, dim, dim };
		SDL::Rect edge_right = { off_x + dim * 2, off_y + dim, dim, dim };
		SDL::Rect center = { off_x + dim, off_y + dim, dim, dim };

		SDL::Texture texture(renderer, SDL_TEXTUREACCESS_TARGET, w, h);
		texture.setBlendMode(SDL_BLENDMODE_BLEND);
		SDL::Rect dst;

		renderer.setTarget(texture);

		for (int j = dim; j < h - dim; j += dim) {
			for (int i = dim; i < w - dim; i += dim) {
				dst = { i, j, dim, dim };
				renderer.copy(sprites, center, dst);
			}
		}

		for (int i = dim; i < w - dim; i += dim) {
			dst = { i, 0, dim, dim };
			renderer.copy(sprites, edge_top, dst);

			dst = { i, h - dim, dim, dim };
			renderer.copy(sprites, edge_bottom, dst);
		}

		for (int i = dim; i < h - dim; i += dim) {
			dst = { 0, i, dim, dim };
			renderer.copy(sprites, edge_left, dst);

			dst = { w - dim, i, dim, dim };
			renderer.copy(sprites, edge_right, dst);
		}
		dst = { 0, 0, dim, dim };
		renderer.copy(sprites, corner_tl, dst);

		dst = { w - dim, 0, dim, dim };
		renderer.copy(sprites, corner_tr, dst);

		dst = { 0, h - dim, dim, dim };
		renderer.copy(sprites, corner_bl, dst);

		dst = { w - dim, h - dim, dim, dim };
		renderer.copy(sprites, corner_br, dst);

		renderer.resetTarget();

		return texture;
	}

	void center(SDL::Renderer &renderer, SDL::Texture &texture, SDL::Texture &text_texture, int w, int h)
	{
		SDL::Rect src = text_texture.getRect();
		SDL::Rect dst = { w / 2 - src.w / 2, h / 2 - src.h / 2, src.w, src.h };

		renderer.setTarget(texture);
		renderer.copy(text_texture, text_texture.getRect(), dst);
		renderer.resetTarget();
	}

	SDL::Texture create_texture(SDL::Renderer &renderer, int w, int h, int x, int y)
	{
		SDL::Texture texture(renderer, SDL_TEXTUREACCESS_TARGET, w, h);
		renderer.setDrawColor(0, 0, 0, 0);
		renderer.clear();
		texture.setBlendMode(SDL_BLENDMODE_BLEND);
		renderer.setTarget(texture);
		SDL::Rect src = { x, y, w, h };
		SDL::Rect dst = { 0, 0, w, h };
		renderer.copy(sprites, src, dst);
		renderer.resetTarget();
		return texture;
	}

    public:
	UI_Factory(SDL::Renderer &renderer)
		: sprites(renderer, "assets/ui.png")
	{
	}

	SDL::Texture create_big_box(SDL::Renderer &renderer, int w, int h)
	{
		return create_box(renderer, w, h, 16, 0, 0);
	}

	SDL::Texture create_button_box(SDL::Renderer &renderer, int w, int h)
	{
		return create_box(renderer, w, h, 8, 48, 0);
	}

	SDL::Texture create_button_over_box(SDL::Renderer &renderer, int w, int h)
	{
		return create_box(renderer, w, h, 8, 48, 24);
	}

	SDL::Texture create_button(SDL::Renderer &renderer, std::string text, int w, int h)
	{
		SDL::Color white = { 255, 255, 255, 255 };

		SDL::Texture texture = create_button_box(renderer, w, h);
		SDL::Texture text_texture(renderer, font.renderText(text, white));

		center(renderer, texture, text_texture, w, h);

		return texture;
	}

	SDL::Texture create_button(SDL::Renderer &renderer, std::string text)
	{
		SDL::Color white = { 255, 255, 255, 255 };

		SDL::Texture text_texture(renderer, font.renderText(text, white));
		int w = text_texture.getWidth() + 10;
		int h = text_texture.getHeight() + 5;

		SDL::Texture texture = create_button_box(renderer, w, h);

		center(renderer, texture, text_texture, w, h);

		return texture;
	}

	SDL::Texture create_button_over(SDL::Renderer &renderer, std::string text, int w, int h)
	{
		SDL::Color white = { 255, 255, 255, 255 };

		SDL::Texture texture = create_button_over_box(renderer, w, h);
		SDL::Texture text_texture(renderer, font.renderText(text, white));

		center(renderer, texture, text_texture, w, h);

		return texture;
	}

	SDL::Texture create_button_over(SDL::Renderer &renderer, std::string text)
	{
		SDL::Color white = { 255, 255, 255, 255 };
		SDL::Texture text_texture(renderer, font.renderText(text, white));

		int w = text_texture.getWidth() + 10;
		int h = text_texture.getHeight() + 5;
		SDL::Texture texture = create_button_over_box(renderer, w, h);

		center(renderer, texture, text_texture, w, h);
		return texture;
	}

	SDL::Texture create_label(SDL::Renderer &renderer, std::string text, SDL::Color color)
	{
		return { renderer, font.renderText(text, color) };
	}

	SDL::Texture create_home_button(SDL::Renderer &renderer)
	{
		return create_texture(renderer, 16, 16, 0, 48);
	}

	SDL::Texture create_home_button_over(SDL::Renderer &renderer)
	{
		return create_texture(renderer, 16, 16, 0, 64);
	}

	SDL::Texture create_restart_button(SDL::Renderer &renderer)
	{
		return create_texture(renderer, 16, 16, 16, 48);
	}

	SDL::Texture create_restart_button_over(SDL::Renderer &renderer)
	{
		return create_texture(renderer, 16, 16, 16, 64);
	};
};

class Material {
    private:
	SDL::Texture rect;
	SDL::Texture rect_h;

	static constexpr float w = 48, h = 16;

	uint dura;
	bool selected;

    public:
	float x, y;
	Material(SDL::Renderer renderer, uint durability, bool is_selected, int x, int y)
		: rect(renderer, "assets/asteroid.png")
		, rect_h(renderer, "assets/asteroid_highlight.png")
		, dura(durability)
		, selected(is_selected)
		, x(x)
		, y(y){};

	inline uint get_dura() const
	{
		return dura;
	};

	inline bool is_selected() const
	{
		return selected;
	};

	inline void set_selected(bool s)
	{
		selected = s;
	};

	inline bool is_over(float a, float b)
	{
		return a >= x && a <= x + w && b >= y && b <= y + h;
	};

	void draw(SDL::Renderer renderer)
	{
		constexpr int max_durability = 5;
		constexpr int dim = 96;

		int off = max_durability - static_cast<int>(dura);

		SDL::Rect src = { off * dim, 0, dim, dim };

		float a = x + w / 2 - dim * 0.5;
		float b = y + h / 2 - dim * 0.5;

		SDL::FRect dst = { a, b, dim, dim };
		if (selected)
			renderer.copy(rect_h, src, dst);
		else
			renderer.copy(rect, src, dst);
	}
};
