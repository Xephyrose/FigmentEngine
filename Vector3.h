#ifndef FIGMENTENGINE_VECTOR3_H
#define FIGMENTENGINE_VECTOR3_H

class Vector3 {
public:
    float x, y, z;

    Vector3(float x, float y, float z);
    Vector3(const Vector3& Vector3);
    Vector3();

    static const Vector3 ZERO;
    static const Vector3 ONE;
    static const Vector3 RIGHT;
    static const Vector3 UP;
    static const Vector3 FORWARD;

    void Set(float x, float y, float z);
    [[nodiscard]] float Length() const;
    [[nodiscard]] float LengthSquared() const;
    void Normalize();
    [[nodiscard]] Vector3 Normalized() const;
    [[nodiscard]] float DistanceTo(const Vector3& Vector3) const;
    [[nodiscard]] float DistanceSquaredTo(const Vector3& Vector3) const;
    [[nodiscard]] float DotProductTo(const Vector3& Vector3) const;

    Vector3 operator+(const Vector3& Vector3) const;
    void operator+=(const Vector3& Vector3);

    Vector3 operator-(const Vector3& Vector3) const;
    void operator-=(const Vector3& Vector3);

    Vector3 operator*(const Vector3& Vector3) const;
    void operator*=(const Vector3& Vector3);

    Vector3 operator*(const float& scale) const;
    void operator*=(const float& scale);

    Vector3 operator/(const Vector3& Vector3) const;
    void operator/=(const Vector3& Vector3);

    Vector3 operator/(const float& scale) const;
    void operator/=(const float& scale);

    Vector3 operator-() const;

    bool operator==(const Vector3& Vector3) const;
    bool operator!=(const Vector3& Vector3) const;

    bool operator<(const Vector3& Vector3) const;
    bool operator<=(const Vector3& Vector3) const;

    bool operator>(const Vector3& Vector3) const;
    bool operator>=(const Vector3& Vector3) const;
};

#endif //FIGMENTENGINE_VECTOR3_H
