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