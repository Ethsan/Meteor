#include "logic.h"
#include "vec2.h"

#include <cstddef>
#include <istream>
#include <limits>
#include <cmath>
#include <stdexcept>

template <> void Logic::move(Ball &ball, float dt)
{
	if (!ball.is_alive)
		return;

	const float width = getWidth();
	const float height = getHeight();

	vec2f v = { ball.vx, ball.vy };
	v = v.normalized() * getSpeed();
	ball.vx = v.x;
	ball.vy = v.y;

	ball.x += ball.vx * dt;
	ball.y += ball.vy * dt;

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
		ball.is_alive = false;
	}
}

template <> void Logic::move(Paddle &paddle, float dt)
{
	const float width = getWidth();
	const float height = getHeight();

	if (dir == NONE) {
		;
	} else if (dir == LEFT) {
		paddle.x -= 200 * dt;
	} else if (dir == RIGHT) {
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

template <> bool Logic::collide(Ball &ball, Brick &brick)
{
	if (brick.durability == 0)
		return false;

	// simple distance check
	float diag = std::sqrt(brick.h * brick.w);
	vec2f vec = { ball.x - brick.x - brick.w / 2, ball.y - brick.y - brick.h / 2 };

	if (vec.norm() > diag + ball.r) {
		return false;
	}

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
			return false;
		}

		float norm = std::abs(circle_min - rect_max);
		if (norm == 0) { // weird edge case where the ball is exactly on the edge of the brick
			return false;
		}
		if (norm < min_overlap) {
			min_overlap = norm;
			min_translation = norm * normal;
		}
	}

	brick.last_hit = getTick();
	if (--brick.durability == 0) {
		brick_count--;
		score += 100;
	};

	vec2f v = { ball.vx, ball.vy };
	vec2f normal = min_translation.normalized();
	vec2f v_n = normal * (v.dot(normal));
	vec2f v_t = v - v_n;

	vec2f v_n_abs = normal * v_n.norm();

	ball.vx = v_t.x + v_n_abs.x;
	ball.vy = v_t.y + v_n_abs.y;

	bounce_count++;

	return true;
}

template <> bool Logic::collide(Ball &ball1, Ball &ball2)
{
	vec2f vec = { ball1.x - ball2.x, ball1.y - ball2.y };
	if (vec.norm() > ball1.r + ball2.r) {
		return false;
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

	return true;
}

template <> bool Logic::collide(Ball &ball, Paddle &paddle)
{
	vec2f vec = { ball.x - paddle.x, ball.y - paddle.y };
	if (vec.norm() > ball.r + std::max(paddle.h, paddle.w) / 2)
		return false;

	vec2f vec_unit = vec.normalized();
	vec2f ellipse_proj = { vec_unit.x * paddle.w / 2, vec_unit.y * paddle.h / 2 };

	if (vec.norm() > ball.r + ellipse_proj.norm()) {
		return false;
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
	return true;
}

void Logic::step(float dt)
{
	tick++;

	for (auto &ball : balls) {
		move(ball, dt);
	}

	move(paddle, dt);

	for (size_t i = 0; i < balls.size(); i++) {
		Ball &ball = balls[i];

		for (auto &brick : bricks) {
			collide(ball, brick);
		}

		for (size_t j = i + 1; j < balls.size(); j++) {
			auto &other = balls[j];
			collide(ball, other);
		}

		collide(ball, paddle);
	}

	if (bounce_count >= 4) {
		int incr = bounce_count / 4;
		bounce_count %= 4;
		speed += incr * .01 * height;
	}

	if (brick_count <= 0) {
		state = WIN;
	} else if (ball_count <= 0) {
		state = LOST;
	}
}

Brick Logic::brickLookup(float x, float y)
{
	for (auto &brick : bricks) {
		if (x >= brick.x && x <= brick.x + brick.w && y >= brick.y && y <= brick.y + brick.h) {
			return brick;
		}
	}
	return { -1, 0, 0, 0, 0 };
}

Brick Logic::placeNewBrick(float x, float y, uint durability)
{
	for (auto &brick : bricks) {
		if ((x - brick.x < Brick::w && x - brick.x > -Brick::w) &&
		    (y - brick.y < Brick::h && y - brick.y > -Brick::h)) {
			return { -1, 0, 0, 0, 0 };
		}
	}
	return { addBrick(x, y, durability), x, y, 0, 0 };
}

void Logic::placeBrick(float x, float y, int target_id)
{
	size_t target;
	for (size_t i = 0; i < bricks.size(); i++) {
		Brick &brick = bricks[i];
		if (target_id == brick.id) {
			target = i;
			continue;
		}

		if ((x - brick.x < Brick::w && x - brick.x > -Brick::w) &&
		    (y - brick.y < Brick::h && y - brick.y > -Brick::h))
			return;
	}
	bricks[target].x = x;
	bricks[target].y = y;
}

void Logic::removeBrick(int target_id)
{
	for (auto it = bricks.begin(); it != bricks.end(); it++) {
		if (target_id == it->id) {
			bricks.erase(it);
			brick_count--;
			break;
		}
	}
}

int Logic::addBall(float x, float y)
{
	int id = next_id++;

	Ball ball = { id, x, y, 0, 1, true };
	balls.emplace_back(ball);

	ball_count++;

	return id;
}

int Logic::addBrick(float x, float y, uint durability)
{
	int id = next_id++;

	Brick brick = { id, x, y, durability, -1 };
	bricks.emplace_back(brick);

	brick_count++;

	return id;
}

void Logic::init()
{
	state = RUNNING;
	tick = 0;
	score = 0;

	speed = height / 2;

	for (float x = 0; x < width; x += 50) {
		for (float y = 0; y < height - 150; y += 20) {
			addBrick(x, y, 1);
		}
	}

	addBall(width / 2, height / 2);
}

void Logic::init_canva()
{
	state = RUNNING;
	tick = 0;
	score = 0;

	speed = height / 2;

	addBall(width / 2, 6 * height / 7);
}

void Logic::save(std::ostream &output)
{
	output << width << "," << height << std::endl;
	output << next_id << std::endl;
	output << brick_count << "," << ball_count << "," << tick << std::endl;
	output << score << "," << combo << std::endl;
	output << speed << "," << bounce_count << std::endl;
	output << lives << std::endl;
	output << paddle.id << "," << paddle.x << "," << paddle.y << std::endl;
	output << balls.size() << std::endl;
	output << bricks.size() << std::endl;
	for (auto &ball : balls) {
		output << ball.id << "," << ball.x << "," << ball.y << "," << ball.vx << "," << ball.vy << ","
		       << ball.is_alive << std::endl;
	}
	for (auto &brick : bricks) {
		output << brick.id << "," << brick.x << "," << brick.y << "," << brick.durability << ","
		       << brick.last_hit << std::endl;
	}
}

void istreamHealthCkeck(std::istream &cin)
{
	if (cin.eof() || cin.bad() || cin.fail())
		throw std::runtime_error("Bad save format");
}

Logic::Logic(std::istream &save)
	: width(0)
	, height(0)
{
	constexpr auto max_size = std::numeric_limits<std::streamsize>::max();

	size_t numberOfBalls = 0;
	size_t numberOfBricks = 0;

	save.clear();

	istreamHealthCkeck(save);
	save >> width;
	istreamHealthCkeck(save);
	save.ignore(max_size, ',');
	save >> height;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> next_id;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> brick_count;
	istreamHealthCkeck(save);
	save.ignore(max_size, ',');
	save >> ball_count;
	istreamHealthCkeck(save);
	save.ignore(max_size, ',');
	save >> tick;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> score;
	istreamHealthCkeck(save);
	save.ignore(max_size, ',');
	save >> combo;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> speed;
	istreamHealthCkeck(save);
	save.ignore(max_size, ',');
	save >> bounce_count;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> lives;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> paddle.id;
	istreamHealthCkeck(save);
	save.ignore(max_size, ',');
	save >> paddle.x;
	istreamHealthCkeck(save);
	save.ignore(max_size, ',');
	save >> paddle.y;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> numberOfBalls;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');
	save >> numberOfBricks;
	istreamHealthCkeck(save);
	save.ignore(max_size, '\n');

	balls.reserve(numberOfBalls);
	bricks.reserve(numberOfBricks);

	for (size_t i = 0; i < numberOfBalls; i++) {
		int id;
		float x, y, vx, vy;
		bool is_alive;

		istreamHealthCkeck(save);
		save >> id;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> x;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> y;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> vx;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> vy;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> is_alive;
		istreamHealthCkeck(save);
		save.ignore(max_size, '\n');

		Ball ball = { id, x, y, vx, vy, is_alive };
		balls.emplace_back(ball);
	}

	for (size_t i = 0; i < numberOfBricks; i++) {
		int id;
		float x, y;
		uint durability;
		int last_hit;

		istreamHealthCkeck(save);
		save >> id;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> x;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> y;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> durability;
		istreamHealthCkeck(save);
		save.ignore(max_size, ',');
		save >> last_hit;
		istreamHealthCkeck(save);
		save.ignore(max_size, '\n');

		Brick brick = { id, x, y, durability, last_hit };
		bricks.emplace_back(brick);
	}
}