#ifndef FIGMENTENGINE_VECTOR2_H
#define FIGMENTENGINE_VECTOR2_H

struct Vector2 {
    float x, y;

    Vector2(float x, float y);
    Vector2(const Vector2& vector2);
    Vector2();

    static const Vector2 ZERO;
    static const Vector2 ONE;
    static const Vector2 RIGHT;
    static const Vector2 UP;

    void Set(float x, float y);
    [[nodiscard]] float getRadians() const;
    [[nodiscard]] float getDegrees() const;
    [[nodiscard]] float Length() const;
    [[nodiscard]] float LengthSquared() const;
    void Normalize();
    Vector2 Normalized() const;
    [[nodiscard]] float DistanceTo(const Vector2& vector2) const;
    [[nodiscard]] float DistanceSquaredTo(const Vector2& vector2) const;
    [[nodiscard]] float DotProductTo(const Vector2& vector2) const;

    Vector2 operator+(const Vector2& vector2) const;
    void operator+=(const Vector2& vector2);

    Vector2 operator-(const Vector2& vector2) const;
    void operator-=(const Vector2& vector2);

    Vector2 operator*(const Vector2& vector2) const;
    void operator*=(const Vector2& vector2);

    Vector2 operator*(const float& scale) const;
    void operator*=(const float& scale);

    Vector2 operator/(const Vector2& vector2) const;
    void operator/=(const Vector2& vector2);

    Vector2 operator/(const float& scale) const;
    void operator/=(const float& scale);

    Vector2 operator-() const;

    bool operator==(const Vector2& vector2) const;
    bool operator!=(const Vector2& vector2) const;

    bool operator<(const Vector2& vector2) const;
    bool operator<=(const Vector2& vector2) const;

    bool operator>(const Vector2& vector2) const;
    bool operator>=(const Vector2& vector2) const;
};

#endif //FIGMENTENGINE_VECTOR2_H
