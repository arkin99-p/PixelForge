#pragma once
#include <iostream>

// 2D Vector
class Vector2 {
public:
    float x, y;

    Vector2();
    Vector2(float x, float y);

    void add(const Vector2& v);
    void sub(const Vector2& v);
    void scale(float factor);

    float length() const noexcept;

    std::string toString() const;

    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2 operator*(float factor) const;
    Vector2 operator/(float factor) const;
    float operator*(const Vector2& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
        return os << v.toString();
    }
};
inline Vector2 operator*(float factor, const Vector2& v) {
    return v * factor;
}

// 3D Vector
class Vector3 {
public:
    float x, y, z;

    Vector3();
    Vector3(float x, float y, float z);
    Vector3(Vector2 vector, float z);

    void add(const Vector3& v);
    void sub(const Vector3& v);
    void crossWith(const Vector3& v);
    void scale(float factor);

    float length() const noexcept;

    std::string toString() const;

    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    float operator*(const Vector3& other) const;
    Vector3 operator*(float factor) const;
    Vector3 operator/(float factor) const;

    friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        return os << v.toString();
    }
};
inline Vector3 operator*(float factor, const Vector3& v) {
    return v * factor;
}

// 4D Vector
class Vector4 {
public:
    float x, y, z, w;

    Vector4();
    Vector4(float x, float y, float z, float w);
    Vector4(Vector3 vector, float w);

    void add(const Vector4& v);
    void sub(const Vector4& v);
    void scale(float factor);

    float length() const noexcept;

    std::string toString() const;

    Vector4 operator+(const Vector4& other) const;
    Vector4 operator-(const Vector4& other) const;
    float operator*(const Vector4& other) const;
    Vector4 operator*(float factor) const;
    Vector4 operator/(float factor) const;

    friend std::ostream& operator<<(std::ostream& os, const Vector4& v) {
        return os << v.toString();
    }
};
inline Vector4 operator*(float factor, const Vector4& v) {
    return v * factor;
}

// 3x3 Matrix
class Matrix3 {
public:
    float m[3][3];

    Matrix3();
    Matrix3(float a11, float a12, float a13,
        float a21, float a22, float a23,
        float a31, float a32, float a33);

    Matrix3 operator*(const Matrix3& other) const;
    Vector2 operator*(const Vector2& v) const;
    Vector3 operator*(const Vector3& v) const;

    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& os, const Matrix3& v) {
        return os << v.toString();
    }

    static Matrix3 identity() {
        return Matrix3{};
    }
    static Matrix3 rotation(float angle);
    static Matrix3 scale(float sx, float sy);
    static Matrix3 translation(float tx, float ty);
};

// 4x4 Matrix
class Matrix4 {
public:
    float m[4][4];

    Matrix4();
    Matrix4(float a11, float a12, float a13, float a14,
        float a21, float a22, float a23, float a24,
        float a31, float a32, float a33, float a34,
        float a41, float a42, float a43, float a44);

    Vector3 transformPoint(const Vector3& v) const;
    Vector3 transformDirection(const Vector3& v) const;

    Matrix4 operator*(const Matrix4& other) const;
    Vector4 operator*(const Vector4& v) const;

    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& os, const Matrix4& v) {
        return os << v.toString();
    }

    static Matrix4 identity() {
        return Matrix4{};
    }
    static Matrix4 translation(float tx, float ty, float tz);
    static Matrix4 rotationX(float angle);
    static Matrix4 rotationY(float angle);
    static Matrix4 rotationZ(float angle);
    static Matrix4 scale(float sx, float sy, float sz);

    static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);
    static Matrix4 perspective(float fov, float aspect, float near, float far);
};

// Vector functions
float getAngle(const Vector2& v1, const Vector2& v2);
Vector3 getCross(const Vector3& v1, const Vector3& v2);
float getMixedProduct(const Vector3& v1, const Vector3& v2, const Vector3& v3);
Vector3 homogeneousNormalize(const Vector4& v);
Vector3 normalize(const Vector3& v);