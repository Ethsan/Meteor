#pragma once

#include "collisiongrid.h"

#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <fstream>

struct Paddle {
	int id;
	float x, y;

	static constexpr float w = 56, h = 30;
};

struct Ball {
	int id;
	float x, y;
	float vx, vy;

	bool is_alive;

	static constexpr float r = 8;
};

struct Brick {
	int id;
	float x, y;

	uint durability;
	int last_hit;

	static constexpr float w = 48, h = 16;
};

struct Empty {
	uint id;
};

template <typename T> concept Object = std::is_same_v<T, Ball> || std::is_same_v<T, Brick> || std::is_same_v<T, Paddle>;

enum Paddle_dir {
	NONE,
	LEFT,
	RIGHT,
};

class Logic {
    public:
	enum GameState {
		RUNNING,
		WIN,
		LOST,
	};
	using ObjectVariant = std::variant<Empty, Ball, Brick, Paddle>;

	Paddle_dir dir = NONE;

	Logic(float width, float height, bool canva = false)
		: width(width)
		, height(height)
	{
		if (canva)
			init_canva();
		else
			init();
	}

	Logic(std::istream &save);

	static Logic loadFromFile(const std::string &save_file)
	{
		std::ifstream save_import(save_file, std::ios::in);
		return Logic(save_import);
	}

	void step(float dt);

	float getWidth() const
	{
		return width;
	}

	float getHeight() const
	{
		return height;
	}

	int getTick() const
	{
		return tick;
	}

	int getScore() const
	{
		return score;
	}

	float getSpeed() const
	{
		return speed;
	}

	int getLives() const
	{
		return lives;
	}

	template <typename T> void visit(T &&visitor)
	{
		for (auto &ball : balls) {
			visitor(ball);
		}
		for (auto &brick : bricks) {
			visitor(brick);
		}
		visitor(paddle);
	}

	Brick brickLookup(float x, float y);

	Brick placeNewBrick(float x, float y, uint durability);

	void placeBrick(float x, float y, int target_id);

	void removeBrick(int target_id);

	GameState getState() const
	{
		return state;
	}

	/*	
	************* SAVE FORMAT **************
	----------------------------------------
	| width,height                         |
	| next_id                              |
	| brick_count,ball_count,tick          |
	| score,combo                          |
	| speed,bounce_count                   |
	| lives                                |
	| paddle.id,paddle.x,paddle.y          |
	| numberOfBalls                        |
	| numberOfBricks                       |
	| #balls(id,x,y,vx,vy,isalive)         |
	| #bricks(id,x,y,durability,last_hit)  |
	----------------------------------------
	*/

	void save(std::ostream &output);

    private:
	float width, height;

	GameState state = RUNNING;

	int next_id = 0;

	std::vector<Ball> balls{};
	std::vector<Brick> bricks{};
	Paddle paddle{ next_id++, width / 2, height - Paddle::h };

	int brick_count = 0;
	int ball_count = 0;
	int tick = 0;

	int score = 0;
	int combo = 0;

	float speed = 150;
	int bounce_count = 0;

	int lives = 3;

	int addBall(float x, float y);
	int addBrick(float x, float y, uint durability);

	template <Object T> void move(T &obj, float dt);

	template <Object T1, Object T2> bool collide(T1 &a, T2 &b);

	void init();

	void init_canva();
};
