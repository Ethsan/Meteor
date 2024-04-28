#include "logic.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>
#include <cmath>

template <typename T = float> class vec2 {
    public:
	T x, y;

	vec2 operator+(const vec2 &v) const
	{
		return { x + v.x, y + v.y };
	}

	vec2 operator-(const vec2 &v) const
	{
		return { x - v.x, y - v.y };
	}

	vec2 operator*(T s) const
	{
		return { x * s, y * s };
	}

	friend vec2 operator*(T s, const vec2 v)
	{
		return v * s;
	}

	vec2 operator/(T s) const
	{
		return { x / s, y / s };
	}

	vec2 &operator+=(const vec2 &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	vec2 &operator-=(const vec2 &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	vec2 &operator*=(T s)
	{
		x *= s;
		y *= s;
		return *this;
	}
	vec2 &operator/=(T s)
	{
		x /= s;
		y /= s;
		return *this;
	}

	vec2 operator-() const
	{
		return { -x, -y };
	}
	vec2 operator+() const
	{
		return { x, y };
	}

	vec2 normalized() const
	{
		return { x / norm(), y / norm() };
	}

	T norm() const
	{
		return std::sqrt(x * x + y * y);
	}

	T dot(const vec2 &v) const
	{
		return x * v.x + y * v.y;
	}
	vec2 rotate(T angle) const
	{
		float s = std::sin(angle);
		T c = std::cos(angle);
		return { x * c - y * s, x * s + y * c };
	}

	vec2 rotate90CCW() const
	{
		return { -y, x };
	}

	vec2 rotate90CW() const
	{
		return { y, x };
	}

	bool isnan() const
	{
		return std::isnan(x) || std::isnan(y);
	}

	friend std::ostream &operator<<(std::ostream &os, const vec2 &v)
	{
		return os << "(" << v.x << ", " << v.y << ")";
	}
};

using vec2f = vec2<float>;

template <typename T> void remove_duplicates(std::vector<T> &v)
{
	std::sort(v.begin(), v.end());
	auto it = std::unique(v.begin(), v.end());
	v.erase(it, v.end());
}

void CollisionGrid::addObject(float minX, float minY, float maxX, float maxY, uint id)
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

std::vector<uint> CollisionGrid::getCollisions(float minX, float minY, float maxX, float maxY) const
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

std::vector<std::pair<uint, uint> > CollisionGrid::getAllCollisions() const
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

void Logic::MoveVisitor::operator()(auto &)
{
	return;
}

void Logic::MoveVisitor::operator()(Brick &brick)
{
	if (brick.durability == 0)
		return;
	l.collisionGrid_.addObject(brick.x, brick.y, brick.x + brick.w, brick.y + brick.h, brick.id);
}

void Logic::MoveVisitor::operator()(Ball &ball)
{
	if (!ball.is_alive)
		return;
	const float width = l.getWidth();
	const float height = l.getHeight();

	vec2f v = { ball.vx, ball.vy };
	v = v.normalized() * 300;
	ball.vx = v.x;
	ball.vy = v.y;

	ball.x += ball.vx * dt;
	ball.y += ball.vy * dt;

	if (ball.x - ball.r < 0) {
		ball.x = ball.r;
		ball.vx = -ball.vx;
		l.bounce_count++;
	}
	if (ball.x + ball.r > width) {
		ball.x = width - ball.r;
		ball.vx = -ball.vx;
		l.bounce_count++;
	}
	if (ball.y - ball.r < 0) {
		ball.y = ball.r;
		ball.vy = -ball.vy;
		l.bounce_count++;
	}
	if (ball.y - ball.r > height) {
		l.ball_count--;
		ball.is_alive = false;
	}
	l.collisionGrid_.addObject(ball.x - ball.r, ball.y - ball.r, ball.x + ball.r, ball.y + ball.r, ball.id);
}

void Logic::MoveVisitor::operator()(Paddle &paddle)
{
	const float width = l.getWidth();
	const float height = l.getHeight();

	if (l.dir == NONE) {
		;
	} else if (l.dir == LEFT) {
		paddle.x -= 200 * dt;
	} else if (l.dir == RIGHT) {
		paddle.x += 200 * dt;
	}

	if (paddle.x - paddle.w / 2 < 0) {
		paddle.x = paddle.w / 2;
	}
	if (paddle.x + paddle.w / 2 > width) {
		paddle.x = width - paddle.w / 2;
	}
	if (paddle.y - paddle.h / 2 < 0) {
		paddle.y = paddle.h / 2;
	}
	if (paddle.y + paddle.h / 2 > height) {
		paddle.y = height - paddle.h / 2;
	}
	l.collisionGrid_.addObject(paddle.x - paddle.w / 2, paddle.y - paddle.h / 2, paddle.x + paddle.w / 2,
				   paddle.y + paddle.h / 2, paddle.id);
}

vec2f closest_point(vec2f point, std::span<vec2f> vertices)
{
	vec2f closest = vertices[0];
	float min_dist = (point - closest).norm();
	for (size_t i = 1; i < vertices.size(); i++) {
		vec2f v = vertices[i];
		float dist = (point - v).norm();
		if (dist < min_dist) {
			min_dist = dist;
			closest = v;
		}
	}
	return closest;
}

void Logic::CollisionVisitor::operator()(auto &, auto &)
{
	return;
}

void Logic::CollisionVisitor::operator()(Ball &ball, Brick &brick)
{
	this->Logic::CollisionVisitor::operator()(brick, ball);
}

void Logic::CollisionVisitor::operator()(Brick &brick, Ball &ball)
{
	std::array<vec2f, 4> vertices = { vec2f{ brick.x + brick.w, brick.y },
					  vec2f{ brick.x + brick.w, brick.y + brick.h },
					  vec2f{ brick.x, brick.y + brick.h }, vec2f{ brick.x, brick.y } };

	vec2f closest = closest_point({ ball.x, ball.y }, vertices);
	vec2f vec_ball = { ball.x - closest.x, ball.y - closest.y };
	std::array<vec2f, 5> normals = { (vertices[1] - vertices[0]).normalized(),
					 (vertices[2] - vertices[1]).normalized(),
					 (vertices[3] - vertices[2]).normalized(),
					 (vertices[0] - vertices[3]).normalized(), vec_ball.normalized() };
	vec2f min_translation = { 0, 0 };
	float min_overlap = std::numeric_limits<float>::infinity();

	for (const auto &normal : normals) {
		float rect_max = -std::numeric_limits<float>::infinity();
		float rect_min = std::numeric_limits<float>::infinity();

		for (const auto &vert : vertices) {
			float proj = normal.dot(vert);
			if (proj > rect_max)
				rect_max = proj;
			if (proj < rect_min)
				rect_min = proj;
		}

		float proj = normal.dot({ ball.x, ball.y });
		float circle_max = proj + ball.r;
		float circle_min = proj - ball.r;

		if (rect_min > circle_max || rect_max < circle_min) {
			return;
		}

		float norm = std::abs(circle_min - rect_max);
		if (norm == 0) { // weird edge case where the ball is exactly on the edge of the brick
			return;
		}
		if (norm < min_overlap) {
			min_overlap = norm;
			min_translation = norm * normal;
		}
	}

	brick.last_hit = l.getTick();
	if (--brick.durability == 0) {
		l.brick_count--;
		l.score += 100;
	};

	vec2f v = { ball.vx, ball.vy };
	vec2f normal = min_translation.normalized();
	vec2f v_n = normal * (v.dot(normal));
	vec2f v_t = v - v_n;

	vec2f v_n_abs = normal * v_n.norm();

	ball.vx = v_t.x + v_n_abs.x;
	ball.vy = v_t.y + v_n_abs.y;

	l.bounce_count++;
}

void Logic::CollisionVisitor::operator()(Ball &ball1, Ball &ball2)
{
	vec2f vec = { ball1.x - ball2.x, ball1.y - ball2.y };
	vec2f vec_unit = vec.normalized();
	if (vec.norm() > ball1.r + ball2.r) {
		return;
	}

	auto intersection = (ball1.r + ball2.r - vec.norm()) / 2;

	ball1.x += vec_unit.x * intersection;
	ball1.y += vec_unit.y * intersection;
	ball2.x -= vec_unit.x * intersection;
	ball2.y -= vec_unit.y * intersection;

	vec2f v1 = { ball1.vx, ball1.vy };
	vec2f v1n = vec_unit * (v1.dot(vec_unit));
	vec2f v1t = v1 - v1n;

	vec2f v2 = { ball2.vx, ball2.vy };
	vec2f v2n = vec_unit * (v2.dot(vec_unit));
	vec2f v2t = v2 - v2n;

	ball1.vx = v2n.x + v1t.x;
	ball1.vy = v2n.y + v1t.y;
	ball2.vx = v1n.x + v2t.x;
	ball2.vy = v1n.y + v2t.y;

	l.bounce_count++;
}

void Logic::CollisionVisitor::operator()(Paddle &paddle, Ball &ball)
{
	return this->Logic::CollisionVisitor::operator()(ball, paddle);
}

void Logic::CollisionVisitor::operator()(Ball &ball, Paddle &paddle)
{
	vec2f vec = { ball.x - paddle.x, ball.y - paddle.y };
	vec2f vec_unit = vec.normalized();

	vec2f ellipse_proj = { vec_unit.x * paddle.w / 2, vec_unit.y * paddle.h / 2 };

	if (vec.norm() > ball.r + ellipse_proj.norm()) {
		return;
	}

	auto intersection = (ball.r + ellipse_proj.norm() - vec.norm());

	ball.x += vec_unit.x * intersection;
	ball.y += vec_unit.y * intersection;

	vec2f v = { ball.vx, ball.vy };
	vec2f v_n = vec_unit * (v.dot(vec_unit));
	vec2f v_t = v - v_n;

	vec2f new_v_n = vec_unit * v_n.norm();

	ball.vx = v_t.x + new_v_n.x;
	ball.vy = v_t.y + new_v_n.y;

	l.bounce_count++;
}

void Logic::step(float dt)
{
	tick++;
	collisionGrid_.clear();

	float temp = 0;
	for (auto &obj : objects) {
		std::visit(MoveVisitor{ dt, *this, temp }, obj);
	}

	for (const auto &collision : collisionGrid_.getAllCollisions()) {
		auto &obj1 = objects[collision.first];
		auto &obj2 = objects[collision.second];
		std::visit(CollisionVisitor{ *this }, obj1, obj2);
	}

	if (bounce_count >= 4) {
		int incr = bounce_count / 4;
		bounce_count %= 4;
		speed += incr * 50;
	}

	if (brick_count <= 0) {
		state = WIN;
	} else if (ball_count <= 0) {
		state = LOST;
	}
}

void Logic::addBall(float x, float y)
{
	uint id = objects.size();
	Ball ball = { id, x, y, 0, 1, true };
	objects.push_back(ball);

	ball_count++;
}

void Logic::addBrick(float x, float y, uint durability)
{
	uint id = objects.size();
	Brick brick = { id, x, y, durability, -1 };
	objects.push_back(brick);

	brick_count++;
}

void Logic::addPaddle(float x, float y)
{
	uint id = objects.size();
	Paddle paddle = { id, x, y };
	objects.push_back(paddle);
}

void Logic::init()
{
	state = RUNNING;
	tick = 0;
	score = 0;

	speed = 200;

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			addBrick(96 * i, 32 * j, 5);
		}
	}

	addBall(width / 2, height / 2);
	addPaddle(width / 2, height - 56);
}
