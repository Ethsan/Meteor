#include "logic.h"
#include "vec2.h"

#include <cstddef>
#include <iostream>
#include <istream>
#include <limits>
#include "exception.h"
#include <optional>
#include <span>

constexpr float inf = std::numeric_limits<float>::infinity();

template <> void Logic::move(Powerup &p, float dt)
{
	p.y += get_speed() * dt * 0.1;
}

template <> void Logic::move(Ball &ball, float dt)
{
	if (!ball.alive)
		return;

	const float width = get_width();
	const float height = get_height();

	vec2f v = { ball.vx, ball.vy };
	if (v.norm() != 0) {
		v = v.normalized() * get_speed();
		ball.vx = v.x;
		ball.vy = v.y;

		ball.x += ball.vx * dt;
		ball.y += ball.vy * dt;
	}

	if (ball.x - ball.r < 0) {
		ball.x = ball.r;
		ball.vx = -ball.vx;
		bounce_count++;
	}
	if (ball.x + ball.r > width) {
		ball.x = width - ball.r;
		ball.vx = -ball.vx;
		bounce_count++;
	}
	if (ball.y - ball.r < 0) {
		ball.y = ball.r;
		ball.vy = -ball.vy;
		bounce_count++;
	}

	if (ball.y - ball.r > height) {
		ball_count--;
		ball.alive = false;
	}
}

template <> void Logic::move(Paddle &paddle, float dt)
{
	const float width = get_width();
	const float height = get_height();

	if (dir == NONE) {
		;
	} else if (dir == LEFT) {
		paddle.x -= paddle_speed * dt;
	} else if (dir == RIGHT) {
		paddle.x += paddle_speed * dt;
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
}

vec2f closest_point(vec2f point, std::span<std::pair<float, float> > vertices)
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

template <> void Logic::collide(Ball &ball, Brick &brick)
{
	if (brick.dura == 0 || !ball.alive)
		return;

	auto vertices = brick.get_points();

	std::vector<vec2f> normals;
	normals.reserve(vertices.size() + 1);

	for (size_t i = 0; i < vertices.size(); i++) {
		vec2f edge = vec2f(vertices[(i + 1) % vertices.size()]) - vertices[i];
		normals.emplace_back(vec2f{ -edge.y, edge.x }.normalized());
	}

	vec2f closest = closest_point({ ball.x, ball.y }, vertices);
	vec2f vec_ball = { ball.x - closest.x, ball.y - closest.y };
	normals.emplace_back(vec_ball.normalized());

	vec2f min_translation = { 0, 0 };
	float min_overlap = inf;

	for (const auto &normal : normals) {
		float rect_max = -inf;
		float rect_min = inf;

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

	vec2f v = { ball.vx, ball.vy };
	vec2f normal = min_translation.normalized();
	vec2f v_n = normal * (v.dot(normal));
	vec2f v_t = v - v_n;

	vec2f v_n_abs = normal * v_n.norm();

	ball.vx = v_t.x + v_n_abs.x;
	ball.vy = v_t.y + v_n_abs.y;

	bounce_count++;

	brick.last_hit = get_tick();
	if (brick.dura <= 0)
		return;

	brick.dura--;
	if (brick.dura == 0) {
		brick_count--;
		score += brick_points;
		if (brick.powerup) {
			add_powerup(brick.x, brick.y, brick.powerup.value());
		}
	}
}

template <> void Logic::collide(Ball &ball1, Ball &ball2)
{
	if (!ball1.alive || !ball2.alive) {
		return;
	}

	vec2f vec = { ball1.x - ball2.x, ball1.y - ball2.y };
	if (vec.norm() > ball1.r + ball2.r) {
		return;
	}

	vec2f vec_unit = vec.normalized();

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

	bounce_count++;
}

template <> void Logic::collide(Ball &ball, Paddle &paddle)
{
	if (!ball.alive)
		return;

	vec2f vec = { ball.x - paddle.x, ball.y - paddle.y };
	if (vec.norm() > ball.r + std::max(paddle.h, paddle.w) / 2)
		return;

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

	bounce_count++;
}

template <> void Logic::collide(Ball &ball, Powerup &powerup)
{
	if (!ball.alive || !powerup.alive)
		return;

	vec2f vec = { ball.x - powerup.x, ball.y - powerup.y };
	if (vec.norm() > ball.r + powerup.r)
		return;

	switch (powerup.power) {
	case Powerup::SLOW_BALL:
		bonus_speed -= 0.4 * h;
		break;
	case Powerup::FAST_BALL:
		bonus_speed += 0.4 * h;
		break;
	case Powerup::EXTRA_BALL:
		add_ball(powerup.x, powerup.y, -ball.vx * .9, -ball.vy * .9);
		break;
	case Powerup::EXTRA_LIFE:
		if (lives < 3)
			lives++;
		break;

	case Powerup::SMALL_BALL:
		break;
	case Powerup::BIG_BALL:
		break;
	case Powerup::STRONG_BALL:
		break;
	}
	powerup.alive = false;
}

void Logic::step(float dt)
{
	tick++;

	for (auto &powerup : powerups)
		move(powerup, dt);

	for (auto &ball : balls)
		move(ball, dt);

	move(paddle, dt);

	for (size_t i = 0; i < balls.size(); i++) {
		for (auto &brick : bricks)
			collide(balls[i], brick);

		for (auto &powerup : powerups)
			collide(balls[i], powerup);

		for (size_t j = i + 1; j < balls.size(); j++)
			collide(balls[i], balls[j]);

		collide(balls[i], paddle);
	}

	if (brick_count <= 0) {
		state = WIN;
	} else if (ball_count <= 0 && lives <= 0) {
		state = LOST;
	}
}

void Logic::launch_ball()
{
	if (lives <= 0)
		return;
	add_ball(paddle.x, paddle.y - paddle.h / 2, 0, -1);
	lives--;
}

bool point_in_polygon(vec2f point, std::span<std::pair<float, float> > vertices)
{
	bool inside = false;
	for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
		if (((vertices[i].second > point.y) != (vertices[j].second > point.y)) &&
		    (point.x < (vertices[j].first - vertices[i].first) * (point.y - vertices[i].second) /
					       (vertices[j].second - vertices[i].second) +
				       vertices[i].first))
			inside = !inside;
	}
	return inside;
}

std::optional<std::pair<std::size_t, Brick &> > Logic::get_brick(float x, float y)
{
	for (std::size_t i = 0; i < bricks.size(); i++) {
		auto &brick = bricks[i];

		auto vertices = brick.get_points();
		if (point_in_polygon({ x, y }, vertices)) {
			return { { i, brick } };
		}
	}

	return std::nullopt;
}

std::optional<std::size_t> Logic::add_brick_safe(float x, float y, uint durability)
{
	for (auto &brick : bricks) {
		if ((x - brick.x < brick.rect_w && x - brick.x > -brick.rect_w) &&
		    (y - brick.y < brick.rect_h && y - brick.y > -brick.rect_h)) {
			return std::nullopt;
		}
	}
	return add_brick(x, y, Brick::RECT, durability, std::nullopt);
}

void Logic::replace_brick_safe(std::size_t index, float x, float y)
{
	Brick &brick = bricks.at(index);
	auto points = brick.get_points();

	for (auto &point : points) {
		float new_x = point.first + x - brick.x;
		if (new_x < 0)
			x -= new_x;
		if (new_x > w)
			x -= new_x - w;
		float new_y = point.second + y - brick.y;
		if (new_y < 0)
			y -= new_y;
		if (new_y > h)
			y -= new_y - h;
	}

	for (auto &point : points) {
		point.first += x - brick.x;
		point.second += y - brick.y;
		if (point.first < 0 || point.first > w || point.second < 0 || point.second > h)
			return;
	}

	for (auto &other : bricks) {
		if (&other == &brick)
			continue;

		auto vertices = other.get_points();
		for (auto &point : points) {
			if (point_in_polygon(point, vertices)) {
				return;
			}
		}
	}

	bricks[index].x = x;
	bricks[index].y = y;
}

void Logic::remove_brick(std::size_t index)
{
	bricks.erase(bricks.begin() + static_cast<long>(index));
}

int Logic::add_ball(float x, float y, float vx, float vy)
{
	Ball ball = { x, y, vx, vy };

	balls.emplace_back(ball);
	ball_count++;

	return balls.size() - 1;
}

int Logic::add_brick(float x, float y, Brick::Shape shape, uint durability, std::optional<Powerup::type> type)
{
	Brick brick = { x, y, shape, durability, type };
	bricks.emplace_back(brick);
	brick_count++;
	return bricks.size() - 1;
}

int Logic::add_powerup(float x, float y, Powerup::type type)
{
	Powerup power = { x, y, type };
	powerups.emplace_back(power);

	return powerups.size() - 1;
};

void Logic::init()
{
	state = RUNNING;
	tick = 0;
	score = 0;

	for (float x = Brick::hex_r; x < w - Brick::hex_r; x += 50)
		for (float y = Brick::hex_r; y < h / 3; y += 50)
			add_brick(x, y, Brick::HEX, 5, std::nullopt);
	//test powerups
	add_brick(w / 3, h / 2, Brick::RECT, 1, Powerup::EXTRA_BALL);
}

void Logic::save(std::ostream &output)
{
	output << w << "," << h << std::endl;
	output << tick << std::endl;
	output << score << "," << combo << std::endl;
	output << bonus_speed << "," << bounce_count << std::endl;
	output << lives << "," << paddle.x << "," << paddle.y << std::endl;
	output << ball_count << std::endl;
	for (auto &ball : balls) {
		if (!ball.alive)
			continue;
		output << ball.x << "," << ball.y << "," << ball.vx << "," << ball.vy << std::endl;
	}
	output << brick_count << std::endl;
	for (auto &brick : bricks) {
		if (brick.dura == 0)
			continue;
		auto powerup = brick.powerup ? brick.powerup.value() : -1;

		output << brick.x << "," << brick.y << "," << brick.dura << "," << brick.shape << "," << powerup
		       << std::endl;
	}
}

void health_check(std::istream &cin)
{
	if (cin.eof() || cin.bad() || cin.fail())
		throw BadSaveFormat();
}

Logic Logic::load(std::istream &save)
{
	constexpr auto max_size = std::numeric_limits<std::streamsize>::max();

	float w, h;
	save.clear();

	save >> w;
	save.ignore(max_size, ',');
	save >> h;
	save.ignore(max_size, '\n');
	health_check(save);

	Logic logic{ w, h };

	health_check(save);
	save >> logic.tick;
	health_check(save);
	save.ignore(max_size, '\n');

	save >> logic.score;
	health_check(save);
	save.ignore(max_size, ',');
	save >> logic.combo;
	health_check(save);
	save.ignore(max_size, '\n');

	save >> logic.bonus_speed;
	health_check(save);
	save.ignore(max_size, ',');
	save >> logic.bounce_count;
	health_check(save);
	save.ignore(max_size, '\n');

	save >> logic.lives;
	health_check(save);
	save.ignore(max_size, ',');
	save >> logic.paddle.x;
	health_check(save);
	save.ignore(max_size, ',');
	save >> logic.paddle.y;
	health_check(save);
	save.ignore(max_size, '\n');

	size_t ball_count = 0;
	save >> ball_count;
	health_check(save);
	save.ignore(max_size, '\n');

	for (size_t i = 0; i < ball_count; i++) {
		float x, y, vx, vy;

		health_check(save);
		save >> x;
		health_check(save);
		save.ignore(max_size, ',');
		save >> y;
		health_check(save);
		save.ignore(max_size, ',');
		save >> vx;
		health_check(save);
		save.ignore(max_size, ',');
		save >> vy;
		health_check(save);
		save.ignore(max_size, '\n');

		logic.add_ball(x, y, vx, vy);
	}

	size_t brick_count = 0;
	save >> brick_count;
	health_check(save);
	save.ignore(max_size, '\n');

	for (size_t i = 0; i < brick_count; i++) {
		float x, y;
		uint durability;
		int shape;
		int powerup;

		health_check(save);
		save >> x;
		health_check(save);
		save.ignore(max_size, ',');
		save >> y;
		health_check(save);
		save.ignore(max_size, ',');
		save >> durability;
		health_check(save);
		save.ignore(max_size, ',');
		save >> shape;
		health_check(save);
		save.ignore(max_size, ',');
		save >> powerup;
		save.ignore(max_size, '\n');
		health_check(save);

		std::optional<Powerup::type> p = powerup == -1 ? std::nullopt :
								 std::optional<Powerup::type>(Powerup::type(powerup));

		switch (shape) {
		case Brick::RECT:
		case Brick::HEX:
			break;
		default:
			throw BadSaveFormat();
		}
		logic.add_brick(x, y, Brick::Shape(shape), durability, p);
	}

	return logic;
}
