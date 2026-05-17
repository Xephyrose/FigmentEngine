#include "Vector2.h"

#include <cmath>

const Vector2 Vector2::ZERO = Vector2(0.0f, 0.0f);
const Vector2 Vector2::ONE = Vector2(1.0f, 1.0f);
const Vector2 Vector2::RIGHT = Vector2(1.0f, 0.0f);
const Vector2 Vector2::UP = Vector2(0.0f, 1.0f);

Vector2::Vector2(float x, float y) :
        x(x),
        y(y)
{}

Vector2::Vector2(const Vector2& vector2) :
    x(vector2.x),
    y(vector2.y)
{}

Vector2::Vector2() :
    x(0.0f),
    y(0.0f)
{}

void Vector2::Set(float x, float y)
{
    this->x = x;
    this->y = y;
}

float Vector2::getRadians() const
{
    return fmodf((atan2f(y, x) + static_cast<float>(M_PI) * 4.0f), static_cast<float>(M_PI) * 2.0f);
}

float Vector2::getDegrees() const
{
    return fmodf((atan2f(y, x) * 180.0f / static_cast<float>(M_PI)) + 720.0f, 360.0f);
}

float Vector2::Length() const
{
    return sqrtf(LengthSquared());
}

float Vector2::LengthSquared() const
{
    return x * x + y * y;
}

void Vector2::Normalize()
{
    float length = LengthSquared();
    if (length != 0)
    {
        length = sqrtf(length);
        x /= length;
        y /= length;
    }
}

Vector2 Vector2::Normalized() const
{
    Vector2 vector2(x, y);
    vector2.Normalize();
    return vector2;
}

float Vector2::DistanceTo(const Vector2& aVector) const
{
    return sqrtf(DistanceSquaredTo(aVector));
}

float Vector2::DistanceSquaredTo(const Vector2& aVector) const
{
    return (x - aVector.x) * (x - aVector.x) + (y - aVector.y) * (y - aVector.y);
}

float Vector2::DotProductTo(const Vector2& vector2) const
{
    return x * vector2.x + y * vector2.y;
}

Vector2 Vector2::operator+(const Vector2& vector2) const
    {
        return {x + vector2.x, y + vector2.y};
    }

    void Vector2::operator+=(const Vector2& vector2)
    {
        x += vector2.x;
        y += vector2.y;
    }

    Vector2 Vector2::operator-(const Vector2& vector2) const
    {
        return {x - vector2.x, y - vector2.y};
    }

    void Vector2::operator-=(const Vector2& vector2)
    {
        x -= vector2.x;
        y -= vector2.y;
    }

    Vector2 Vector2::operator*(const Vector2& vector2) const
    {
        return {x * vector2.x, y * vector2.y};
    }

    Vector2 Vector2::operator*(const float& scale) const
    {
        return {x * scale, y * scale};
    }

    void Vector2::operator*=(const Vector2& vector2)
    {
        x *= vector2.x;
        y *= vector2.y;
    }

    void Vector2::operator*=(const float& scale)
    {
        x *= scale;
        y *= scale;
    }

    Vector2 Vector2::operator/(const Vector2& vector2) const
    {
        return {x / vector2.x, y / vector2.y};
    }

    Vector2 Vector2::operator/(const float& scale) const
    {
        return {x / scale, y / scale};
    }

    void Vector2::operator/=(const Vector2& vector2)
    {
        x /= vector2.x;
        y /= vector2.y;
    }

    void Vector2::operator/=(const float& scale)
    {
        x /= scale;
        y /= scale;
    }

    Vector2 Vector2::operator-() const
    {
        return {-x, -y};
    }

    bool Vector2::operator==(const Vector2& vector2) const
    {
        return x == vector2.x && y == vector2.y;
    }

    bool Vector2::operator != (const Vector2& vector2) const
    {
        return x != vector2.x || y != vector2.y;
    }

    bool Vector2::operator<(const Vector2& aVector2) const
    {
        return (x == aVector2.x) ? (y < aVector2.y) : (x < aVector2.x);
    }

    bool Vector2::operator<=(const Vector2& aVector2) const
    {
        return (x == aVector2.x) ? (y <= aVector2.y) : (x <= aVector2.x);
    }

    bool Vector2::operator>(const Vector2& aVector2) const
    {
        return (x == aVector2.x) ? (y > aVector2.y) : (x > aVector2.x);
    }

    bool Vector2::operator>=(const Vector2& aVector2) const
    {
        return (x == aVector2.x) ? (y >= aVector2.y) : (x >= aVector2.x);
    }

	Vector2 operator* (float scale, const Vector2& vector2)
	{
		return {vector2.x * scale, vector2.y * scale};
	}