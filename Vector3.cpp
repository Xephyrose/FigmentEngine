#include "Vector3.h"
#include <cmath>

const Vector3 Vector3::ZERO = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONE = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::RIGHT = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UP = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::FORWARD = Vector3(0.0f, 0.0f, 1.0f);

Vector3::Vector3(float x, float y, float z) :
    x(x),
    y(y),
    z(z)
{}

Vector3::Vector3(const Vector3& Vector3) :
    x(Vector3.x),
    y(Vector3.y),
    z(Vector3.z)
{}

Vector3::Vector3() :
    x(0.0f),
    y(0.0f),
    z(0.0f)
{}

void Vector3::Set(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

float Vector3::Length() const
{
    return sqrtf(LengthSquared());
}

float Vector3::LengthSquared() const
{
    return x * x + y * y + z * z;
}

void Vector3::Normalize()
{
    float length = LengthSquared();
    if (length != 0)
    {
        length = sqrtf(length);
        x /= length;
        y /= length;
    }
}

Vector3 Vector3::Normalized() const
{
    Vector3 Vector3(x, y, z);
    Vector3.Normalize();
    return Vector3;
}

float Vector3::DistanceTo(const Vector3& aVector) const
{
    return sqrtf(DistanceSquaredTo(aVector));
}

float Vector3::DistanceSquaredTo(const Vector3& aVector) const
{
    return (x - aVector.x) * (x - aVector.x) + (y - aVector.y) * (y - aVector.y);
}

float Vector3::DotProductTo(const Vector3& Vector3) const
{
    return x * Vector3.x + y * Vector3.y + z * Vector3.z;
}

Vector3 Vector3::operator+(const Vector3& Vector3) const
    {
        return {x + Vector3.x, y + Vector3.y, z + Vector3.z};
    }

    void Vector3::operator+=(const Vector3& Vector3)
    {
        x += Vector3.x;
        y += Vector3.y;
        z += Vector3.z;
    }

    Vector3 Vector3::operator-(const Vector3& Vector3) const
    {
        return {x - Vector3.x, y - Vector3.y, z - Vector3.z};
    }

    void Vector3::operator-=(const Vector3& Vector3)
    {
        x -= Vector3.x;
        y -= Vector3.y;
        z -= Vector3.z;
    }

    Vector3 Vector3::operator*(const Vector3& Vector3) const
    {
        return {x * Vector3.x, y * Vector3.y, z * Vector3.z};
    }

    Vector3 Vector3::operator*(const float& scale) const
    {
        return {x * scale, y * scale, z * scale};
    }

void Vector3::operator*=(const Vector3& Vector3)
{
    x *= Vector3.x;
    y *= Vector3.y;
}

void Vector3::operator*=(const float& scale)
{
    x *= scale;
    y *= scale;
}

Vector3 Vector3::operator/(const Vector3& Vector3) const
{
    return {x / Vector3.x, y / Vector3.y, z / Vector3.z};
}

Vector3 Vector3::operator/(const float& scale) const
{
    return {x / scale, y / scale, z / scale};
}

void Vector3::operator/=(const Vector3& Vector3)
{
    x /= Vector3.x;
    y /= Vector3.y;
    z /= Vector3.z;
}

void Vector3::operator/=(const float& scale)
{
    x /= scale;
    y /= scale;
    z /= scale;
}

Vector3 Vector3::operator-() const
{
    return {-x, -y, -z};
}

bool Vector3::operator==(const Vector3& Vector3) const
{
    return x == Vector3.x && y == Vector3.y;
}

bool Vector3::operator != (const Vector3& Vector3) const
{
    return x != Vector3.x || y != Vector3.y;
}

bool Vector3::operator<(const Vector3& aVector3) const
{
    return (x == aVector3.x) ? (y < aVector3.y) : (x < aVector3.x);
}

bool Vector3::operator<=(const Vector3& aVector3) const
{
    return (x == aVector3.x) ? (y <= aVector3.y) : (x <= aVector3.x);
}

bool Vector3::operator>(const Vector3& aVector3) const
{
    return (x == aVector3.x) ? (y > aVector3.y) : (x > aVector3.x);
}

bool Vector3::operator>=(const Vector3& aVector3) const
{
    return (x == aVector3.x) ? (y >= aVector3.y) : (x >= aVector3.x);
}

Vector3 operator* (const float scale, const Vector3& Vector3)
{
    return {Vector3.x * scale, Vector3.y * scale, Vector3.z * scale};
}