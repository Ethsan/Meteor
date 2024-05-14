#include <cmath>
#include <ostream>

template <typename T = float> struct vec2 {
	T x, y;

	vec2() = default;

	vec2(T x, T y)
		: x(x)
		, y(y)
	{
	}

	vec2(std::pair<T, T> p)
		: x(p.first)
		, y(p.second)
	{
	}

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
		return { y, -x };
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
