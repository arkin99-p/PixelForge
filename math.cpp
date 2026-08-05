#include <format>
#include <algorithm>
#include <xmmintrin.h>
#include <smmintrin.h>
#include "math.hpp"

// === Vector2 ===

Vector2::Vector2() : x(0), y(0) {}
Vector2::Vector2(float x, float y) : x(x), y(y) {}

void Vector2::add(const Vector2& other) {
    x += other.x;
    y += other.y;
}
void Vector2::sub(const Vector2& other) {
    x -= other.x;
    y -= other.y;
}
void Vector2::scale(float factor) {
    x *= factor;
    y *= factor;
}
float Vector2::length() const noexcept {
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
float Vector2::operator*(const Vector2& other) const {
    return x * other.x + y * other.y;
}
Vector2 Vector2::operator*(float factor) const {
    return Vector2(x * factor, y * factor);
}
Vector2 Vector2::operator/(float factor) const {
    return Vector2(x / factor, y / factor);
}
inline void Vector2::operator/=(float factor) {
    x /= factor;
    y /= factor;
}

// === Vector3 ===

Vector3::Vector3() : x(0), y(0), z(0) {}
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
Vector3::Vector3(Vector2 vector, float z) : x(vector.x), y(vector.y), z(z) {}

void Vector3::add(const Vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
}
void Vector3::sub(const Vector3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
}
void Vector3::crossWith(const Vector3& other) {
    float oldX = x, oldY = y, oldZ = z;
    x = oldY * other.z - oldZ * other.y;
    y = oldZ * other.x - oldX * other.z;
    z = oldX * other.y - oldY * other.x;
}
void Vector3::scale(float factor) {
    x *= factor;
    y *= factor;
    z *= factor;
}
void Vector3::norm() {
    float len = length();
    if (len > 0) {
        x /= len;
        y /= len;
        z /= len;
    }
}
float Vector3::length() const noexcept {
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
float Vector3::operator*(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}
Vector3 Vector3::operator*(float factor) const {
    return Vector3(x * factor, y * factor, z * factor);
}
Vector3 Vector3::operator/(float factor) const {
    return Vector3(x / factor, y / factor, z / factor);
}
inline void Vector3::operator/=(float factor) {
    x /= factor;
    y /= factor;
    z /= factor;
}

// === Vector4 ===

Vector4::Vector4() : x(0), y(0), z(0), w(0) {}
Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
Vector4::Vector4(Vector3 vector, float w) : x(vector.x), y(vector.y), z(vector.z), w(w) {}

void Vector4::add(const Vector4& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
}
void Vector4::sub(const Vector4& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
}
void Vector4::scale(float factor) {
    x *= factor;
    y *= factor;
    z *= factor;
    w *= factor;
}
float Vector4::length() const noexcept {
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
float Vector4::operator*(const Vector4& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}
Vector4 Vector4::operator*(float factor) const {
    return Vector4(x * factor, y * factor, z * factor, w * factor);
}
Vector4 Vector4::operator/(float factor) const {
    return Vector4(x / factor, y / factor, z / factor, w / factor);
}
inline void Vector4::operator/=(float factor) {
    x /= factor;
    y /= factor;
    z /= factor;
    w /= factor;
}

// === Matrix3 ===

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
Matrix3::Matrix3(float a11, float a12, float a13,
    float a21, float a22, float a23,
    float a31, float a32, float a33) {
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

Matrix3 Matrix3::rotation(float angle) {
    return Matrix3{
        std::cos(angle), -std::sin(angle), 0,
        std::sin(angle), std::cos(angle), 0,
        0, 0, 1
    };
}
Matrix3 Matrix3::scale(float sx, float sy) {
    return Matrix3{
        sx, 0, 0,
        0, sy, 0,
        0, 0, 1
    };
}
Matrix3 Matrix3::translation(float tx, float ty) {
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

// === Matrix4 ===

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
Matrix4::Matrix4(float a11, float a12, float a13, float a14,
    float a21, float a22, float a23, float a24,
    float a31, float a32, float a33, float a34,
    float a41, float a42, float a43, float a44) {
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
    // Point: w=1, after multiplication we perform perspective division
    Vector4 result = *this * Vector4(v, 1.0);
    return homogeneousNormalize(result);
}
Vector3 Matrix4::transformDirection(const Vector3& v) const {
    // Direction: w=0, translation is ignored, division is not necessary
    Vector4 result = *this * Vector4(v, 0.0);
    return Vector3(result.x, result.y, result.z);
}
void Matrix4::invert() {
    // Inversion of an affine matrix of the form [R | t; 0 0 0 1]
    // Result: [R^T | -R^T * t; 0 0 0 1]
    *this = Matrix4{
        m[0][0], m[1][0], m[2][0], 0.0f,
        m[0][1], m[1][1], m[2][1], 0.0f,
        m[0][2], m[1][2], m[2][2], 0.0f,

        -(m[3][0] * m[0][0] + m[3][1] * m[0][1] + m[3][2] * m[0][2]),
        -(m[3][0] * m[1][0] + m[3][1] * m[1][1] + m[3][2] * m[1][2]),
        -(m[3][0] * m[2][0] + m[3][1] * m[2][1] + m[3][2] * m[2][2]),
        1.0f
    };
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    // SIMD matrix multiplication (row-major)
    Matrix4 result;

    const float* bData = (const float*)&other.m;
    // Load strings of other into SSE registers
    __m128 rowB0 = _mm_loadu_ps(bData + 0);
    __m128 rowB1 = _mm_loadu_ps(bData + 4);
    __m128 rowB2 = _mm_loadu_ps(bData + 8);
    __m128 rowB3 = _mm_loadu_ps(bData + 12);

    const float* aData = (const float*)&this->m;

    for (int i = 0; i < 4; ++i) {
        int idx = i * 4;

        __m128 vX = _mm_set1_ps(aData[idx + 0]);
        __m128 vY = _mm_set1_ps(aData[idx + 1]);
        __m128 vZ = _mm_set1_ps(aData[idx + 2]);
        __m128 vW = _mm_set1_ps(aData[idx + 3]);

        __m128 resRow = _mm_mul_ps(vX, rowB0);
        resRow = _mm_add_ps(resRow, _mm_mul_ps(vY, rowB1));
        resRow = _mm_add_ps(resRow, _mm_mul_ps(vZ, rowB2));
        resRow = _mm_add_ps(resRow, _mm_mul_ps(vW, rowB3));

        float* rData = (float*)&result.m;
        _mm_storeu_ps(&rData[idx], resRow);
    }

    return result;
}
Vector4 Matrix4::operator*(const Vector4& v) const {
    // Vector v = (x, y, z, w) is stored in the order [w, z, y, x] for dot product
    __m128 vec = _mm_set_ps(v.w, v.z, v.y, v.x);

    __m128 row0 = _mm_loadu_ps(m[0]);
    __m128 row1 = _mm_loadu_ps(m[1]);
    __m128 row2 = _mm_loadu_ps(m[2]);
    __m128 row3 = _mm_loadu_ps(m[3]);

    // _mm_dp_ps with masks:
    // 0xF1 -> save only the x-component (bits 0-3)
    // 0xF2 -> save the y-component (bits 4-7)
    // 0xF4 -> save the z-component (bits 8-11)
    // 0xF8 -> save the w-component (bits 12-15)
    __m128 x_res = _mm_dp_ps(row0, vec, 0xF1);
    __m128 y_res = _mm_dp_ps(row1, vec, 0xF2);
    __m128 z_res = _mm_dp_ps(row2, vec, 0xF4);
    __m128 w_res = _mm_dp_ps(row3, vec, 0xF8);

    __m128 result = _mm_or_ps(_mm_or_ps(x_res, y_res), _mm_or_ps(z_res, w_res));

    Vector4 out;
    _mm_storeu_ps(&out.x, result);
    return out;
}
std::string Matrix4::toString() const {
    return std::format("Matrix4{{ {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}} }}",
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]);
}

Matrix4 Matrix4::translation(float tx, float ty, float tz) {
    return Matrix4{
        1, 0, 0, tx,
        0, 1, 0, ty,
        0, 0, 1, tz,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::rotationX(float angle) {
    return Matrix4{
        1, 0, 0, 0,
        0, std::cos(angle), -std::sin(angle), 0,
        0, std::sin(angle), std::cos(angle), 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::rotationY(float angle) {
    return Matrix4{
        std::cos(angle), 0, std::sin(angle), 0,
        0, 1, 0, 0,
        -std::sin(angle), 0, std::cos(angle), 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::rotationZ(float angle) {
    return Matrix4{
        std::cos(angle), -std::sin(angle), 0, 0,
        std::sin(angle), std::cos(angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::scale(float sx, float sy, float sz) {
    return Matrix4{
        sx, 0, 0, 0,
        0, sy, 0, 0,
        0, 0, sz, 0,
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float near, float far) {
    return Matrix4{
        2 / (right - left), 0, 0, -(right + left) / (right - left),
        0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
        0, 0, -2 / (far - near), -(far + near) / (far - near),
        0, 0, 0, 1
    };
}
Matrix4 Matrix4::perspective(float fov, float aspect, float near, float far) {
    return Matrix4{
        1 / (aspect * std::tan(fov / 2)), 0, 0, 0,
        0, 1 / std::tan(fov / 2), 0, 0,
        0, 0, -(far + near) / (far - near), -(2 * far * near) / (far - near),
        0, 0, -1, 0
    };
}

// === Quaternion ===

Quaternion::Quaternion(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}

void Quaternion::norm() {
    float len = length();
    if (len > 0) {
        x /= len;
        y /= len;
        z /= len;
        w /= len;
    }
}
float Quaternion::length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}
Quaternion Quaternion::conjugate() const {
    return Quaternion{-x, -y, -z, w};
}
Quaternion Quaternion::inverse() const {
    return conjugate() / pow(length(), 2);
}
Matrix4 Quaternion::toMatrix4() const {
    return Matrix4{
        1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w, 2 * x * z + 2 * y * w, 0,
        2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w, 0,
        2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0,
        0, 0, 0, 1
    };
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    return Quaternion(
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y + y * other.w + z * other.x - x * other.z,
        w * other.z + z * other.w + x * other.y - y * other.x,
        w * other.w - x * other.x - y * other.y - z * other.z
    );
}
Quaternion Quaternion::operator*(float scalar) const {
    return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
}
Quaternion Quaternion::operator/(float scalar) const {
    return Quaternion(x / scalar, y / scalar, z / scalar, w / scalar);
}
void Quaternion::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
}
void Quaternion::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
}

Quaternion Quaternion::fromAxisAngle(const Vector3& axis, float angle) {
    Vector3 normAxis = normalize(axis);
    float halfAngle = angle * 0.5f;
    float s = std::sin(halfAngle);
    float c = std::cos(halfAngle);
    return Quaternion(normAxis.x * s, normAxis.y * s, normAxis.z * s, c);
}
Quaternion Quaternion::slerp(const Quaternion& a, const Quaternion& b, float t) {
    Quaternion qa = normalize(a);
    Quaternion qb = normalize(b);

    float cosAngle = qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w;

    if (cosAngle < 0.0f) {
        qb = Quaternion(-qb.x, -qb.y, -qb.z, -qb.w);
        cosAngle = -cosAngle;
    }

    float angle = std::acos(std::clamp(cosAngle, -1.0f, 1.0f));
    float sinAngle = std::sin(angle);

    if (sinAngle < 1e-6f) {
        Quaternion result(
            (1.0f - t) * qa.x + t * qb.x,
            (1.0f - t) * qa.y + t * qb.y,
            (1.0f - t) * qa.z + t * qb.z,
            (1.0f - t) * qa.w + t * qb.w
        );
        return normalize(result);
    }

    float coef1 = std::sin((1.0f - t) * angle) / sinAngle;
    float coef2 = std::sin(t * angle) / sinAngle;

    Quaternion result(
        coef1 * qa.x + coef2 * qb.x,
        coef1 * qa.y + coef2 * qb.y,
        coef1 * qa.z + coef2 * qb.z,
        coef1 * qa.w + coef2 * qb.w
    );

    return normalize(result);
}

// === Methods ===

float radians(float angle) {
    return (angle * 3.1415926535) / 180;
}

Vector3 getCross(const Vector3& v1, const Vector3& v2) {
    return Vector3{
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}
float getMixedProduct(const Vector3& v1, const Vector3& v2, const Vector3& v3) {
    return v1 * getCross(v2, v3);
}
Vector3 homogeneousNormalize(const Vector4& v) {
    if (v.w == 0.0) return Vector3(0, 0, 0);

    return Vector3{ v.x / v.w, v.y / v.w, v.z / v.w };
}
Vector3 normalize(Vector3 vec) {
    float len = vec.length();
    if (len > 0)
        vec /= len;
    return vec;
}
// Builds a look-at matrix for the camera.
// Row-major matrix; transforms world coordinates into camera space.
// Axes: right, upCorrect, -forward (view direction).
// Translation: projection of the camera position onto each axis with a minus sign.
Matrix4 lookAt(Vector3& pos, Vector3& target, Vector3& up) {
    // forward = direction from camera to target
    Vector3 forward = normalize(target - pos);
    // right = perpendicular to forward and up (horizontal)
    Vector3 right = normalize(getCross(forward, up));
    // upCorrect = perpendicular to right and forward (recalculated up)
    Vector3 upCorrect = getCross(right, forward);

    Matrix4 matrix;
    matrix.m[0][0] = right.x;     matrix.m[0][1] = right.y;     matrix.m[0][2] = right.z;     matrix.m[0][3] = -(right * pos);
    matrix.m[1][0] = upCorrect.x; matrix.m[1][1] = upCorrect.y; matrix.m[1][2] = upCorrect.z; matrix.m[1][3] = -(upCorrect * pos);
    matrix.m[2][0] = -forward.x;  matrix.m[2][1] = -forward.y;  matrix.m[2][2] = -forward.z;  matrix.m[2][3] = forward * pos;
    matrix.m[3][0] = 0.0f;        matrix.m[3][1] = 0.0f;        matrix.m[3][2] = 0.0f;        matrix.m[3][3] = 1.0f;

    return matrix;
}
Quaternion normalize(Quaternion quat) {
    float len = quat.length();
    if (len > 0)
        quat /= len;
    return quat;
}