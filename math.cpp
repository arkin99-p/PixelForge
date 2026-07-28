#include <cmath>
#include <format>
#include "math.hpp"

Vector2::Vector2() : x(0), y(0) {}
Vector2::Vector2(double x, double y) : x(x), y(y) {}
void Vector2::add(const Vector2& v) {
    x += v.x;
    y += v.y;
}
void Vector2::sub(const Vector2& v) {
    x -= v.x;
    y -= v.y;
}
void Vector2::scale(double factor) {
    x *= factor;
    y *= factor;
}
double Vector2::length() const noexcept {
    return std::sqrt(x * x + y * y);
}
std::string Vector2::toString() const {
    return std::format("Vector2{{x: {}, y: {}}}", x, y);
}
Vector2 Vector2::operator+(const Vector2& other) const {
    return Vector2(x + other.x, y + other.y);
}
Vector2 Vector2::operator-(const Vector2& other) const {
    return Vector2(x - other.x, y - other.y);
}
Vector2 Vector2::operator*(double factor) const {
    return Vector2(x * factor, y * factor);
}
Vector2 Vector2::operator/(double factor) const {
    return Vector2(x / factor, y / factor);
}
double Vector2::operator*(const Vector2& other) const {
    return x * other.x + y * other.y;
}

Vector3::Vector3() : x(0), y(0), z(0) {}
Vector3::Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
Vector3::Vector3(Vector2 vector, double z) : x(vector.x), y(vector.y), z(z) {}
void Vector3::add(const Vector3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
}
void Vector3::sub(const Vector3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
}
void Vector3::crossWith(const Vector3& v) {
    double oldX = x, oldY = y, oldZ = z;
    x = oldY * v.z - oldZ * v.y;
    y = oldZ * v.x - oldX * v.z;
    z = oldX * v.y - oldY * v.x;
}
void Vector3::scale(double factor) {
    x *= factor;
    y *= factor;
    z *= factor;
}
double Vector3::length() const noexcept {
    return std::sqrt(x * x + y * y + z * z);
}
std::string Vector3::toString() const {
    return std::format("Vector3{{x: {}, y: {}, z: {}}}", x, y, z);
}
Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3(x + other.x, y + other.y, z + other.z);
}
Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
}
double Vector3::operator*(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}
Vector3 Vector3::operator*(double factor) const {
    return Vector3(x * factor, y * factor, z * factor);
}
Vector3 Vector3::operator/(double factor) const {
    return Vector3(x / factor, y / factor, z / factor);
}

Vector4::Vector4() : x(0), y(0), z(0), w(0) {}
Vector4::Vector4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
Vector4::Vector4(Vector3 vector, double w) : x(vector.x), y(vector.y), z(vector.z), w(w) {}
void Vector4::add(const Vector4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
}
void Vector4::sub(const Vector4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
}
void Vector4::scale(double factor) {
    x *= factor;
    y *= factor;
    z *= factor;
    w *= factor;
}
double Vector4::length() const noexcept {
    return std::sqrt(x * x + y * y + z * z + w * w);
}
std::string Vector4::toString() const {
    return std::format("Vector4{{x: {}, y: {}, z: {}, w: {}}}", x, y, z, w);
}
Vector4 Vector4::operator+(const Vector4& other) const {
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}
Vector4 Vector4::operator-(const Vector4& other) const {
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}
double Vector4::operator*(const Vector4& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}
Vector4 Vector4::operator*(double factor) const {
    return Vector4(x * factor, y * factor, z * factor, w * factor);
}
Vector4 Vector4::operator/(double factor) const {
    return Vector4(x / factor, y / factor, z / factor, w / factor);
}

Matrix3::Matrix3() {
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;

    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;

    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
}
Matrix3::Matrix3(double a11, double a12, double a13,
    double a21, double a22, double a23,
    double a31, double a32, double a33) {
    m[0][0] = a11;
    m[0][1] = a12;
    m[0][2] = a13;

    m[1][0] = a21;
    m[1][1] = a22;
    m[1][2] = a23;

    m[2][0] = a31;
    m[2][1] = a32;
    m[2][2] = a33;
}
Matrix3 Matrix3::operator*(const Matrix3& other) const {
    return Matrix3{
        m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0] + m[0][2] * other.m[2][0],
        m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1] + m[0][2] * other.m[2][1],
        m[0][0] * other.m[0][2] + m[0][1] * other.m[1][2] + m[0][2] * other.m[2][2],

        m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0] + m[1][2] * other.m[2][0],
        m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1] + m[1][2] * other.m[2][1],
        m[1][0] * other.m[0][2] + m[1][1] * other.m[1][2] + m[1][2] * other.m[2][2],

        m[2][0] * other.m[0][0] + m[2][1] * other.m[1][0] + m[2][2] * other.m[2][0],
        m[2][0] * other.m[0][1] + m[2][1] * other.m[1][1] + m[2][2] * other.m[2][1],
        m[2][0] * other.m[0][2] + m[2][1] * other.m[1][2] + m[2][2] * other.m[2][2]
    };
}
Vector2 Matrix3::operator*(const Vector2& v) const {
    return Vector2{
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * 1,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * 1
    };
}
Vector3 Matrix3::operator*(const Vector3& v) const {
    return Vector3{
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
    };
}
Matrix3 Matrix3::rotation(double angle) {
    return Matrix3{
        std::cos(angle), -std::sin(angle), 0,
        std::sin(angle), std::cos(angle), 0,
        0, 0, 1
    };
}
Matrix3 Matrix3::scale(double sx, double sy) {
    return Matrix3{
        sx, 0, 0,
        0, sy, 0,
        0, 0, 1
    };
}
Matrix3 Matrix3::translation(double tx, double ty) {
    return Matrix3{
        1, 0, tx,
        0, 1, ty,
        0, 0, 1
    };
}
std::string Matrix3::toString() const {
    return std::format("Matrix3{{ {{{}, {}, {}}}, {{{}, {}, {}}}, {{{}, {}, {}}} }}", 
        m[0][0], m[0][1], m[0][2],
        m[1][0], m[1][1], m[1][2],
        m[2][0], m[2][1], m[2][2]);
}

Matrix4::Matrix4() {
    m[0][0] = 1;
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;

    m[1][0] = 0;
    m[1][1] = 1;
    m[1][2] = 0;
    m[1][3] = 0;

    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = 1;
    m[2][3] = 0;

    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
}
Matrix4::Matrix4(double a11, double a12, double a13, double a14,
    double a21, double a22, double a23, double a24,
    double a31, double a32, double a33, double a34,
    double a41, double a42, double a43, double a44) {
    m[0][0] = a11;
    m[0][1] = a12;
    m[0][2] = a13;
    m[0][3] = a14;

    m[1][0] = a21;
    m[1][1] = a22;
    m[1][2] = a23;
    m[1][3] = a24;

    m[2][0] = a31;
    m[2][1] = a32;
    m[2][2] = a33;
    m[2][3] = a34;

    m[3][0] = a41;
    m[3][1] = a42;
    m[3][2] = a43;
    m[3][3] = a44;
}
Vector3 Matrix4::transformPoint(const Vector3& v) const {
    Vector4 result = *this * Vector4(v, 1.0);
    return homogeneousNormalize(result);
}
Vector3 Matrix4::transformDirection(const Vector3& v) const {
    Vector4 result = *this * Vector4(v, 0.0);
    return Vector3(result.x, result.y, result.z);
}
Matrix4 Matrix4::operator*(const Matrix4& other) const {
    return Matrix4{
        m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0] + m[0][2] * other.m[2][0] + m[0][3] * other.m[3][0],
        m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1] + m[0][2] * other.m[2][1] + m[0][3] * other.m[3][1],
        m[0][0] * other.m[0][2] + m[0][1] * other.m[1][2] + m[0][2] * other.m[2][2] + m[0][3] * other.m[3][2],
        m[0][0] * other.m[0][3] + m[0][1] * other.m[1][3] + m[0][2] * other.m[2][3] + m[0][3] * other.m[3][3],

        m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0] + m[1][2] * other.m[2][0] + m[1][3] * other.m[3][0],
        m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1] + m[1][2] * other.m[2][1] + m[1][3] * other.m[3][1],
        m[1][0] * other.m[0][2] + m[1][1] * other.m[1][2] + m[1][2] * other.m[2][2] + m[1][3] * other.m[3][2],
        m[1][0] * other.m[0][3] + m[1][1] * other.m[1][3] + m[1][2] * other.m[2][3] + m[1][3] * other.m[3][3],

        m[2][0] * other.m[0][0] + m[2][1] * other.m[1][0] + m[2][2] * other.m[2][0] + m[2][3] * other.m[3][0],
        m[2][0] * other.m[0][1] + m[2][1] * other.m[1][1] + m[2][2] * other.m[2][1] + m[2][3] * other.m[3][1],
        m[2][0] * other.m[0][2] + m[2][1] * other.m[1][2] + m[2][2] * other.m[2][2] + m[2][3] * other.m[3][2],
        m[2][0] * other.m[0][3] + m[2][1] * other.m[1][3] + m[2][2] * other.m[2][3] + m[2][3] * other.m[3][3],

        m[3][0] * other.m[0][0] + m[3][1] * other.m[1][0] + m[3][2] * other.m[2][0] + m[3][3] * other.m[3][0],
        m[3][0] * other.m[0][1] + m[3][1] * other.m[1][1] + m[3][2] * other.m[2][1] + m[3][3] * other.m[3][1],
        m[3][0] * other.m[0][2] + m[3][1] * other.m[1][2] + m[3][2] * other.m[2][2] + m[3][3] * other.m[3][2],
        m[3][0] * other.m[0][3] + m[3][1] * other.m[1][3] + m[3][2] * other.m[2][3] + m[3][3] * other.m[3][3]
    };
}
Vector4 Matrix4::operator*(const Vector4& v) const {
    return Vector4{
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
        m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
    };
}
std::string Matrix4::toString() const {
    return std::format("Matrix4{{ {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}} }}",
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]);
}
Matrix4 Matrix4::translation(double tx, double ty, double tz) {
    return Matrix4{
        1, 0, 0, tx,
        0, 1, 0, ty,
        0, 0, 1, tz,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::rotationX(double angle) {
    return Matrix4{
        1, 0, 0, 0,
        0, std::cos(angle), -std::sin(angle), 0,
        0, std::sin(angle), std::cos(angle), 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::rotationY(double angle) {
    return Matrix4{
        std::cos(angle), 0, std::sin(angle), 0,
        0, 1, 0, 0,
        -std::sin(angle), 0, std::cos(angle), 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::rotationZ(double angle) {
    return Matrix4{
        std::cos(angle), -std::sin(angle), 0, 0,
        std::sin(angle), std::cos(angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::scale(double sx, double sy, double sz) {
    return Matrix4{
        sx, 0, 0, 0,
        0, sy, 0, 0,
        0, 0, sz, 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::orthographic(double left, double right, double bottom, double top, double near, double far) {
    return Matrix4{
        2 / (right - left), 0, 0, -(right + left) / (right - left),
        0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
        0, 0, -2 / (far - near), -(far + near) / (far - near),
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::perspective(double fov, double aspect, double near, double far) {
    return Matrix4{
        1 / (aspect * std::tan(fov / 2)), 0, 0, 0,
        0, 1 / std::tan(fov / 2), 0, 0,
        0, 0, -(far + near) / (far - near), -(2 * far * near) / (far - near),
        0, 0, -1, 0
    };
}

double getAngle(const Vector2 &v1, const Vector2 &v2) {
    double len1 = v1.length();
    double len2 = v2.length();
    if (len1 == 0.0 || len2 == 0.0) return 0.0;

    double cosA = (v1 * v2) / (len1 * len2);

    if (cosA < -1.0) cosA = -1.0;
    if (cosA > 1.0)  cosA = 1.0;

    return std::acos(cosA);
}
Vector3 getCross(const Vector3& v1, const Vector3& v2) {
    return Vector3{
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}
double getMixedProduct(const Vector3& v1, const Vector3& v2, const Vector3& v3) {
    return v1 * getCross(v2, v3);
}
Vector3 homogeneousNormalize(const Vector4& v) {
    if (v.w == 0.0) return Vector3(0, 0, 0);

    return Vector3{ v.x / v.w, v.y / v.w, v.z / v.w };
}
Vector3 normalize(const Vector3& v) {
    double len = v.length();
    if (len == 0.0) return Vector3(0, 0, 0);
    return Vector3(v.x / len, v.y / len, v.z / len);
}