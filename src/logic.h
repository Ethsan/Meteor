#include "collisiongrid.h"

#include <vector>
#include <variant>

struct Paddle {
	uint id;
	float x, y;

	static constexpr float w = 112, h = 60;
};

struct Ball {
	uint id;
	float x, y;
	float vx, vy;

	bool is_alive;

	static constexpr float r = 16;
};

struct Brick {
	uint id;
	float x, y;

	uint durability;
	int last_hit;

	static constexpr float w = 96, h = 32;
};

struct Empty {
	uint id;
};

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

	Logic(float width, float height, float cell_size)
		: objects()
		, collisionGrid_(width, height, cell_size)
		, width(width)
		, height(height)

	{
		init();
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

	template <typename T> void visit(T &&visitor)
	{
		for (const auto &obj : objects) {
			std::visit(std::forward<T>(visitor), obj);
		}
	}

	GameState getState() const
	{
		return state;
	}

    private:
	struct CollisionVisitor {
		Logic &l;

		void operator()(auto &, auto &);
		void operator()(Ball &ball, Brick &brick);
		void operator()(Brick &brick, Ball &ball);
		void operator()(Ball &ball1, Ball &ball2);
		void operator()(Paddle &paddle, Ball &ball);
		void operator()(Ball &ball, Paddle &paddle);
	};

	struct MoveVisitor {
		float dt;
		Logic &l;
		float &temp;

		void operator()(auto &);
		void operator()(Ball &ball);
		void operator()(Brick &brick);
		void operator()(Paddle &paddle);
	};

	friend CollisionVisitor;
	friend MoveVisitor;

	GameState state = RUNNING;
	std::vector<ObjectVariant> objects{};
	CollisionGrid collisionGrid_;

	float width, height;
	int brick_count = 0;
	int ball_count = 0;
	int tick = 0;

	int score = 0;
	int combo = 0;

	float speed = 200;
	int bounce_count = 0;

	void addBall(float x, float y);
	void addBrick(float x, float y, uint durability);
	void addPaddle(float x, float y);

	void init();
};
