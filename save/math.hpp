#pragma once
#include <iostream>

// 2D Vector
class Vector2 {
public:
    double x, y;

    Vector2();
    Vector2(double x, double y);

    void add(const Vector2& v);
    void sub(const Vector2& v);
    void scale(double factor);

    double length() const noexcept;

    std::string toString() const;

    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2 operator*(double factor) const;
    Vector2 operator/(double factor) const;
    double operator*(const Vector2& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
        return os << v.toString();
    }
};
inline Vector2 operator*(double factor, const Vector2& v) {
    return v * factor;
}

// 3D Vector
class Vector3 {
public:
    double x, y, z;

    Vector3();
    Vector3(double x, double y, double z);
    Vector3(Vector2 vector, double z);

    void add(const Vector3& v);
    void sub(const Vector3& v);
    void crossWith(const Vector3& v);
    void scale(double factor);

    double length() const noexcept;

    std::string toString() const;

    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    double operator*(const Vector3& other) const;
    Vector3 operator*(double factor) const;
    Vector3 operator/(double factor) const;

    friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        return os << v.toString();
    }
};
inline Vector3 operator*(double factor, const Vector3& v) {
    return v * factor;
}

// 4D Vector
class Vector4 {
public:
    double x, y, z, w;

    Vector4();
    Vector4(double x, double y, double z, double w);
    Vector4(Vector3 vector, double w);

    void add(const Vector4& v);
    void sub(const Vector4& v);
    void scale(double factor);

    double length() const noexcept;

    std::string toString() const;

    Vector4 operator+(const Vector4& other) const;
    Vector4 operator-(const Vector4& other) const;
    double operator*(const Vector4& other) const;
    Vector4 operator*(double factor) const;
    Vector4 operator/(double factor) const;

    friend std::ostream& operator<<(std::ostream& os, const Vector4& v) {
        return os << v.toString();
    }
};
inline Vector4 operator*(double factor, const Vector4& v) {
    return v * factor;
}

// 3x3 Matrix
class Matrix3 {
public:
    double m[3][3];

    Matrix3();
    Matrix3(double a11, double a12, double a13,
    double a21, double a22, double a23,
    double a31, double a32, double a33);

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
    static Matrix3 rotation(double angle);
    static Matrix3 scale(double sx, double sy);
    static Matrix3 translation(double tx, double ty);
};

// 4x4 Matrix
class Matrix4 {
public:
    double m[4][4];

    Matrix4();
    Matrix4(double a11, double a12, double a13, double a14,
        double a21, double a22, double a23, double a24,
        double a31, double a32, double a33, double a34,
        double a41, double a42, double a43, double a44);

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
    static Matrix4 translation(double tx, double ty, double tz);
    static Matrix4 rotationX(double angle);
    static Matrix4 rotationY(double angle);
    static Matrix4 rotationZ(double angle);
    static Matrix4 scale(double sx, double sy, double sz);

    static Matrix4 orthographic(double left, double right, double bottom, double top, double near, double far);
    static Matrix4 perspective(double fov, double aspect, double near, double far);
};

// Vector functions
double getAngle(const Vector2& v1, const Vector2& v2);
Vector3 getCross(const Vector3& v1, const Vector3& v2);
double getMixedProduct(const Vector3& v1, const Vector3& v2, const Vector3& v3);
Vector3 homogeneousNormalize(const Vector4& v);
Vector3 normalize(const Vector3& v);