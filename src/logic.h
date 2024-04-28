#include <optional>
#include <span>
#include <stdexcept>
#include <sys/types.h>
#include <variant>
#include <vector>

template <typename T> class Grid {
    private:
	std::vector<T> data;
	std::size_t width, height;

    public:
	Grid(std::size_t width, std::size_t height)
		: data(width * height)
		, width(width)
		, height(height)
	{
	}

	Grid(std::size_t width, std::size_t height, const T &value)
		: data(width * height, value)
		, width(width)
		, height(height)
	{
	}

	T &operator()(std::size_t x, std::size_t y)
	{
		return data[y * width + x];
	}

	const T &operator()(std::size_t x, std::size_t y) const
	{
		return data[y * width + x];
	}

	T &at(std::size_t x, std::size_t y)
	{
		if (x < 0 || x >= width || y < 0 || y >= height) {
			throw std::out_of_range("Grid::at");
		}
		return data[y * width + x];
	}

	const T &at(std::size_t x, std::size_t y) const
	{
		if (x < 0 || x >= width || y < 0 || y >= height) {
			throw std::out_of_range("Grid::at");
		}
		return data[y * width + x];
	}

	std::span<T> flatten()
	{
		return { data.begin(), data.size() };
	}

	std::span<const T> flatten() const
	{
		return { data.begin(), data.size() };
	}

	auto begin() const
	{
		return data.begin();
	}

	auto end() const
	{
		return data.end();
	}

	auto cbegin() const
	{
		return data.cbegin();
	}

	auto cend() const
	{
		return data.cend();
	}

	std::pair<std::size_t, std::size_t> size() const
	{
		return { width, height };
	}
};

class CollisionGrid { // grid collision class
    private:
	struct Cell {
		uint cycle;
		std::vector<uint> ids;
	};

	uint cycle = 0;
	float cell_size;

	Grid<Cell> grid;

    public:
	CollisionGrid(float width, float height, float cell_size)
		: cell_size(cell_size)
		, grid(width / cell_size, height / cell_size, { cycle, {} })
	{
	}

	void clear()
	{
		cycle++;
	}

	void addObject(float minX, float minY, float maxX, float maxY, uint id);

	std::vector<uint> getCollisions(float minX, float minY, float maxX, float maxY) const;

	std::vector<std::pair<uint, uint> > getAllCollisions() const;
};

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
