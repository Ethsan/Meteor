#include <algorithm>
#include <cmath>
#include <span>
#include <stdexcept>
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

class Collision_grid {
    private:
	struct Cell {
		uint cycle;
		std::vector<uint> ids;
	};

	uint cycle = 0;
	float cell_size;

	Grid<Cell> grid;

    public:
	Collision_grid(float width, float height, float cell_size)
		: cell_size(cell_size)
		, grid(width / cell_size, height / cell_size, { cycle, {} })
	{
	}

	void clear()
	{
		cycle++;
	}

	void add_object(float minX, float minY, float maxX, float maxY, uint id);

	std::vector<uint> get_collisions(float minX, float minY, float maxX, float maxY) const;

	std::vector<std::pair<uint, uint> > get_all_collisions() const;
};
template <typename T> void remove_duplicates(std::vector<T> &v)
{
	std::sort(v.begin(), v.end());
	auto it = std::unique(v.begin(), v.end());
	v.erase(it, v.end());
}

inline void Collision_grid::add_object(float minX, float minY, float maxX, float maxY, uint id)
{
	size_t width = grid.size().first;
	size_t height = grid.size().second;

	size_t x0 = std::clamp<size_t>(std::floor(minX / cell_size), 0, width - 1);
	size_t y0 = std::clamp<size_t>(std::floor(minY / cell_size), 0, height - 1);
	size_t x1 = std::clamp<size_t>(std::ceil(maxX / cell_size), 0, width - 1);
	size_t y1 = std::clamp<size_t>(std::ceil(maxY / cell_size), 0, height - 1);

	for (size_t y = y0; y <= y1; y++) {
		for (size_t x = x0; x <= x1; x++) {
			auto &cell = grid(x, y);

			if (cycle != cell.cycle) {
				cell.cycle = cycle;
				cell.ids.clear();
			}
			cell.ids.push_back(id);
		}
	}
}

inline std::vector<uint> Collision_grid::get_collisions(float minX, float minY, float maxX, float maxY) const
{
	std::vector<uint> result;

	size_t width = grid.size().first;
	size_t height = grid.size().second;

	size_t x0 = std::clamp<size_t>(std::floor(minX / cell_size), 0, width - 1);
	size_t y0 = std::clamp<size_t>(std::floor(minY / cell_size), 0, height - 1);
	size_t x1 = std::clamp<size_t>(std::ceil(maxX / cell_size), 0, width - 1);
	size_t y1 = std::clamp<size_t>(std::ceil(maxY / cell_size), 0, height - 1);

	for (size_t y = y0; y <= y1; y++) {
		for (size_t x = x0; x <= x1; x++) {
			const auto &cell = grid(x, y);
			if (cycle != cell.cycle) {
				continue;
			}

			result.insert(result.end(), cell.ids.begin(), cell.ids.end());
		}
	}

	remove_duplicates(result);

	return result;
}

inline std::vector<std::pair<uint, uint> > Collision_grid::get_all_collisions() const
{
	std::vector<std::pair<uint, uint> > collisions;

	for (const auto &cell : grid) {
		if (cycle != cell.cycle || cell.ids.size() < 2) {
			continue;
		}
		for (auto it = cell.ids.begin(); it != cell.ids.end(); it++) {
			for (auto jt = it + 1; jt != cell.ids.end(); jt++) {
				collisions.push_back({ *it, *jt });
			}
		}
	}

	remove_duplicates(collisions);

	return collisions;
}
