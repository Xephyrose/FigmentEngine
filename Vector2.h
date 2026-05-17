#ifndef SDL_TEST_VECTOR2_H
#define SDL_TEST_VECTOR2_H

struct Vector2 {
    float x;
    float y;

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
    [[nodiscard]] float Length();
    [[nodiscard]] float LengthSquared();
    Vector2 Normalize();
    Vector2 Normalized();
    [[nodiscard]] float DistanceTo(const Vector2& vector2) const;
    [[nodiscard]] float DistanceSquaredTo(const Vector2& vector2) const;
    [[nodiscard]] float DotProductTo(const Vector2& vector2);

};


#endif //SDL_TEST_VECTOR2_H
