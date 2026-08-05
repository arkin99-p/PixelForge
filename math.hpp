#pragma once
#include <iostream>

/**
 * @brief 2D Vector.
 *
 * Introduces basic operations: addition, subtraction, scaling, dot producting
 * All methods except those explicitly marked do not modify the original object.
 */
class Vector2 {
public:
    float x, y; //!< Vector components.

    /**
     * @brief Default constructor. Creates a null vector (0, 0).
     */
    Vector2();

    /**
     * @brief Constructor with assignment of all two components.
     * @param x Component x.
     * @param y Component y.
     */
    Vector2(float x, float y);

    // === Mutator methods ===

    /**
     * @brief Adds another vector to the current one (change object).
     * @param v Vector to be added.
     */
    void add(const Vector2& other);

    /**
     * @brief Subtracts another vector from the current one (change object).
     * @param v Vector to be subtracted.
     */
    void sub(const Vector2& other);

    /**
     * @brief multiplies all vector components by scalar (change object).
     * @param factor Factor.
     */
    void scale(float factor);

    // === Non-mutator methods ===

    /**
     * @brief Calculates the length of a vector.
     * @return The length of a vector.
     */
    float length() const noexcept;

    /**
     * @brief Returns a string representation of the vector.
     * @return A string like "Vector2{x: ..., y: ...}".
     */
    std::string toString() const;

    // === Operators ===

    /**
     * @brief Addition of two vectors.
     * @param other Second vector.
     * @return New vector - sum.
     */
    Vector2 operator+(const Vector2& other) const;

    /**
     * @brief Adds another vector to the current one (change object).
     * @param v Vector to be added.
     */
    inline void operator+=(const Vector2& other) {
        add(other);
    }

    /**
     * @brief Subtraction of two vectors.
     * @param other Second vector.
     * @return New vector - subtraction.
     */
    Vector2 operator-(const Vector2& other) const;

    /**
     * @brief Subtracts another vector to the current one (change object).
     * @param v Vector to be subtracted.
     */
    inline void operator-=(const Vector2& other) {
        sub(other);
    }

    /**
      * @brief Dot product of vectors.
      * @param other Second vector.
      * @return Scalar (float).
      */
    float operator*(const Vector2& other) const;

    /**
     * @brief Multiplies the vector by a scalar.
     * @param factor Factor.
     * @return New vector - components multiplies by the factor.
     */
    Vector2 operator*(float factor) const;

    /**
     * @brief Multiplies the vector by scalar (change object).
     * @param factor Factor.
     */
    inline void operator*=(float factor) {
        scale(factor);
    }

    /**
     * @brief Divides the vector by a scalar.
     * @param factor Factor (should not be equal to 0).
     * @return New vector – components divided by the factor.
     */
    Vector2 operator/(float factor) const;

    /**
     * @brief Divides the vector by scalar (change object).
     * @param factor Factor (should not be equal to 0).
     */
    inline void operator/=(float factor);

    /**
     * @brief Stream output operator.
     * @param os Output stream.
     * @param v Vector for output.
     * @return Stream link for output chains.
     */
    friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
        return os << v.toString();
    }
};
/**
 * @brief Multiplies the vector by a scalar.
 * @param factor Factor.
 * @param v Vector.
 * @return New vector - components multiplies by the factor.
 */
inline Vector2 operator*(float factor, const Vector2& v) {
    return v * factor;
}

/**
 * @brief 3D Vector.
 *
 * Introduces basic operations: addition, subtraction, scaling,
 * normalization, dot and cross multiplication
 * All methods except those explicitly marked do not modify the original object.
 */
class Vector3 {
public:
    float x, y, z; //!< Vector components.

    /**
     * @brief Default constructor. Creates a null vector (0, 0, 0).
     */
    Vector3();

    /**
     * @brief Constructor with assignment of all three components.
     * @param x Component x.
     * @param y Component y.
     * @param z Component z.
     */
    Vector3(float x, float y, float z);

    /**
     * @brief 2D vector and z component constructor.
     * @param vector 2D vector (x, y).
     * @param z Component z.
     */
    Vector3(Vector2 vector, float z);

    // === Mutator methods ===

    /**
     * @brief Adds another vector to the current one (change object).
     * @param v Vector to be added.
     */
    void add(const Vector3& other);

    /**
     * @brief Subtracts another vector from the current one (change object).
     * @param v Vector to be subtracted.
     */
    void sub(const Vector3& other);

    /**
     * @brief Calculates the cross product with another vector (change object).
     * @param v Vector for cross product.
     * @note Result = this × v.
     */
    void crossWith(const Vector3& other);

    /**
     * @brief multiplies all vector components by scalar (change object).
     * @param factor Factor.
     */
    void scale(float factor);

    /**
     * @brief Normalizes a vector (reduces it to unit length). If the length is 0, the vector is unchanged.
     */
    void norm();

    // === Non-mutator methods ===

    /**
     * @brief Calculates the length of a vector.
     * @return The length of a vector.
     */
    float length() const noexcept;

    /**
     * @brief Returns the x and y components as a 2D vector.
     * @return A Vector2 containing (x, y).
     * @note This is a non‑mutating getter; the original vector remains unchanged.
     */
    Vector2 xy() {
        return Vector2{ x, y };
    }

    /**
     * @brief Returns a string representation of the vector.
     * @return A string like "Vector3{x: ..., y: ..., z: ...}".
     */
    std::string toString() const;

    // === Operators ===

    /**
     * @brief Addition of two vectors.
     * @param other Second vector.
     * @return New vector - sum.
     */
    Vector3 operator+(const Vector3& other) const;

    /**
     * @brief Adds another vector to the current one (change object).
     * @param v Vector to be added.
     */
    inline void operator+=(const Vector3& other) {
        add(other);
    }

    /**
     * @brief Subtraction of two vectors.
     * @param other Second vector.
     * @return New vector - subtraction.
     */
    Vector3 operator-(const Vector3& other) const;

    /**
     * @brief Subtracts another vector to the current one (change object).
     * @param v Vector to be subtracted.
     */
    inline void operator-=(const Vector3& other) {
        sub(other);
    }

    /**
      * @brief Dot product of vectors.
      * @param other Second vector.
      * @return Scalar (float).
      */
    float operator*(const Vector3& other) const;

    /**
     * @brief Multiplies the vector by a scalar.
     * @param factor Factor.
     * @return New vector - components multiplies by the factor.
     */
    Vector3 operator*(float factor) const;

    /**
     * @brief Multiplies the vector by scalar (change object).
     * @param factor Factor.
     */
    inline void operator*=(float factor) {
        scale(factor);
    }

    /**
     * @brief Divides the vector by a scalar.
     * @param factor Factor (should not be equal to 0).
     * @return New vector – components divided by the factor.
     */
    Vector3 operator/(float factor) const;

    /**
     * @brief Divides the vector by scalar (change object).
     * @param factor Factor (should not be equal to 0).
     */
    inline void operator/=(float factor);

    /**
     * @brief Stream output operator.
     * @param os Output stream.
     * @param v Vector for output.
     * @return Stream link for output chains.
     */
    friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        return os << v.toString();
    }
};
/**
 * @brief Multiplies the vector by a scalar.
 * @param factor Factor.
 * @param v Vector.
 * @return New vector - components multiplies by the factor.
 */
inline Vector3 operator*(float factor, const Vector3& v) {
    return v * factor;
}

/**
 * @brief 4D Vector.
 *
 * Introduces basic operations: addition, subtraction, scaling
 * All methods except those explicitly marked do not modify the original object.
 */
class Vector4 {
public:
    float x, y, z, w; //!< Vector components.

    /**
     * @brief Default constructor. Creates a null vector (0, 0, 0, 0).
     */
    Vector4();

    /**
     * @brief Constructor with assignment of all four components.
     * @param x Component x.
     * @param y Component y.
     * @param z Component z.
     * @param w Component w.
     */
    Vector4(float x, float y, float z, float w);

    /**
     * @brief 3D vector and w component constructor.
     * @param vector 3D vector (x, y, z).
     * @param w Component w.
     */
    Vector4(Vector3 vector, float w);

    // === Mutator methods ===

    /**
     * @brief Adds another vector to the current one (change object).
     * @param v Vector to be added.
     */
    void add(const Vector4& other);

    /**
     * @brief Subtracts another vector from the current one (change object).
     * @param v Vector to be subtracted.
     */
    void sub(const Vector4& other);

    /**
     * @brief multiplies all vector components by scalar (change object).
     * @param factor Factor.
     */
    void scale(float factor);

    // === Non-mutator methods ===

    /**
     * @brief Calculates the length of a vector.
     * @return The length of a vector.
     */
    float length() const noexcept;

    /**
     * @brief Returns the x and y components as a 2D vector.
     * @return A new Vector2 containing (x, y).
     * @note The original vector is not modified.
     */
    Vector2 xy() {
        return Vector2(x, y);
    }

    /**
     * @brief Returns the x, y and z components as a 3D vector.
     * @return A new Vector3 containing (x, y, z).
     * @note The original vector is not modified; the w component is discarded.
     */
    Vector3 xyz() {
        return Vector3(x, y, z);
    }

    /**
     * @brief Returns a string representation of the vector.
     * @return A string like "Vector4{x: ..., y: ..., z: ..., w: ...}".
     */
    std::string toString() const;

    // === Operators ===

    /**
     * @brief Addition of two vectors.
     * @param other Second vector.
     * @return New vector - sum.
     */
    Vector4 operator+(const Vector4& other) const;

    /**
     * @brief Adds another vector to the current one (change object).
     * @param v Vector to be added.
     */
    inline void operator+=(const Vector4& other) {
        add(other);
    }

    /**
     * @brief Subtraction of two vectors.
     * @param other Second vector.
     * @return New vector - subtraction.
     */
    Vector4 operator-(const Vector4& other) const;

    /**
     * @brief Subtracts another vector to the current one (change object).
     * @param v Vector to be subtracted.
     */
    inline void operator-=(const Vector4& other) {
        sub(other);
    }

    /**
      * @brief Dot product of vectors.
      * @param other Second vector.
      * @return Scalar (float).
      */
    float operator*(const Vector4& other) const;

    /**
     * @brief Multiplies the vector by a scalar.
     * @param factor Factor.
     * @return New vector - components multiplies by the factor.
     */
    Vector4 operator*(float factor) const;

    /**
     * @brief Multiplies the vector by scalar (change object).
     * @param factor Factor.
     */
    inline void operator*=(float factor) {
        scale(factor);
    }

    /**
     * @brief Divides the vector by a scalar.
     * @param factor Factor (should not be equal to 0).
     * @return New vector – components divided by the factor.
     */
    Vector4 operator/(float factor) const;

    /**
     * @brief Divides the vector by scalar (change object).
     * @param factor Factor (should not be equal to 0).
     */
    inline void operator/=(float factor);

    /**
     * @brief Stream output operator.
     * @param os Output stream.
     * @param v Vector for output.
     * @return Stream link for output chains.
     */
    friend std::ostream& operator<<(std::ostream& os, const Vector4& v) {
        return os << v.toString();
    }
};
/**
 * @brief Multiplies the vector by a scalar.
 * @param factor Factor.
 * @param v Vector.
 * @return New vector - components multiplies by the factor.
 */
inline Vector4 operator*(float factor, const Vector4& v) {
    return v * factor;
}

/**
 * @brief 3x3 matrix for linear transformations and 2D affine transformations.
 *
 * This matrix is stored in row-major order and supports common operations:
 * matrix multiplication, vector transformation, and creation of standard
 * transformation matrices (rotation, scaling, translation) for 2D graphics.
 * For 2D points, homogeneous coordinates (x, y, 1) are assumed when multiplying
 * with Vector2.
 */
class Matrix3 {
public:
    float m[3][3]; //!< Matrix elements in row-major order (3 rows, 3 columns).

    /**
    * @brief Default constructor. Creates an identity matrix.
    */
    Matrix3();

    /**
     * @brief Constructs a matrix from 9 explicit elements (row-major order).
     * @param a11 Row 1, Column 1
     * @param a12 Row 1, Column 2
     * @param a13 Row 1, Column 3
     * @param a21 Row 2, Column 1
     * @param a22 Row 2, Column 2
     * @param a23 Row 2, Column 3
     * @param a31 Row 3, Column 1
     * @param a32 Row 3, Column 2
     * @param a33 Row 3, Column 3
     */
    Matrix3(float a11, float a12, float a13,
        float a21, float a22, float a23,
        float a31, float a32, float a33);

    // === Methods ===

    /**
     * @brief Returns a string representation of the matrix.
     * @return String like "Matrix3{{a11, a12, a13}, {a21, a22, a23}, {a31, a32, a33}}".
     */
    std::string toString() const;

    // === Operators ===

    /**
     * @brief Multiplies this matrix by another matrix.
     * @param other Matrix to multiply on the right.
     * @return Resulting matrix product.
     */
    Matrix3 operator*(const Matrix3& other) const;

    /**
     * @brief Transforms a 2D vector (point) using homogeneous coordinates.
     * @param v 2D vector (x, y) treated as (x, y, 1) in homogeneous space.
     * @return Resulting 2D vector after transformation.
     * @note The translation component is applied if the matrix contains translation.
     */
    Vector2 operator*(const Vector2& v) const;

    /**
     * @brief Transforms a 3D vector (linear transformation, no translation).
     * @param v 3D vector to be transformed.
     * @return Resulting 3D vector.
     * @note This is a pure linear transformation; translation components are ignored.
     */
    Vector3 operator*(const Vector3& v) const;

    /**
     * @brief Stream output operator.
     * @param os Output stream.
     * @param v Matrix to output.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Matrix3& v) {
        return os << v.toString();
    }

    // === Static factory methods ===

    /**
     * @brief Creates an identity matrix.
     * @return 3x3 identity matrix.
     */
    static Matrix3 identity() {
        return Matrix3{};
    }
    /**
     * @brief Creates a 2D rotation matrix.
     * @param angle Rotation angle in radians (counter-clockwise).
     * @return Rotation matrix.
     */
    static Matrix3 rotation(float angle);

    /**
     * @brief Creates a 2D scaling matrix.
     * @param sx Scale factor along the X-axis.
     * @param sy Scale factor along the Y-axis.
     * @return Scaling matrix.
     */
    static Matrix3 scale(float sx, float sy);

    /**
     * @brief Creates a 2D translation matrix (in homogeneous coordinates).
     * @param tx Translation along X-axis.
     * @param ty Translation along Y-axis.
     * @return Translation matrix.
     */
    static Matrix3 translation(float tx, float ty);
};

/**
 * @brief 4x4 matrix for 3D transformations and projections.
 *
 * This matrix is stored in row-major order and supports 3D affine transformations,
 * perspective and orthographic projections, and vector transformations.
 * Points are represented as (x, y, z, 1) and directions as (x, y, z, 0).
 */
class Matrix4 {
public:
    float m[4][4]; //!< Matrix elements in row-major order (4 rows, 4 columns).

    /**
     * @brief Default constructor. Creates an identity matrix.
     */
    Matrix4();

    /**
     * @brief Constructs a matrix from 16 explicit elements (row-major order).
     * @param a11 Row 1, Column 1
     * @param a12 Row 1, Column 2
     * @param a13 Row 1, Column 3
     * @param a14 Row 1, Column 4
     * @param a21 Row 2, Column 1
     * @param a22 Row 2, Column 2
     * @param a23 Row 2, Column 3
     * @param a24 Row 2, Column 4
     * @param a31 Row 3, Column 1
     * @param a32 Row 3, Column 2
     * @param a33 Row 3, Column 3
     * @param a34 Row 3, Column 4
     * @param a41 Row 4, Column 1
     * @param a42 Row 4, Column 2
     * @param a43 Row 4, Column 3
     * @param a44 Row 4, Column 4
     */
    Matrix4(float a11, float a12, float a13, float a14,
        float a21, float a22, float a23, float a24,
        float a31, float a32, float a33, float a34,
        float a41, float a42, float a43, float a44);

    // === Mutator methods ===

    /**
     * @brief Inverts this matrix in-place.
     * @note Only works for affine transformation matrices (rotation/translation/scaling).
     *       For projection matrices, inversion may be incorrect.
     *       The matrix is modified.
     */
    void invert();

    // === Non-mutator methods ===

    /**
     * @brief Transforms a 3D point (assumes w=1).
     * @param v 3D point to transform.
     * @return Resulting 3D point after affine transformation.
     * @note This method performs perspective division if needed (after multiplication).
     *       Equivalent to applying the matrix to (v.x, v.y, v.z, 1) and normalizing.
     */
    Vector3 transformPoint(const Vector3& v) const;

    /**
     * @brief Transforms a 3D direction vector (assumes w=0).
     * @param v 3D direction vector.
     * @return Resulting direction vector (translation is ignored).
     * @note This method does not perform perspective division.
     */
    Vector3 transformDirection(const Vector3& v) const;

    /**
    * @brief Returns a string representation of the matrix.
    * @return String like "Matrix4{{a11, a12, a13, a14}, ...}".
    */
    std::string toString() const;

    // === Operators ===

    /**
     * @brief Multiplies this matrix by another matrix.
     * @param other Matrix to multiply on the right.
     * @return Resulting matrix product.
     */
    Matrix4 operator*(const Matrix4& other) const;

    /**
     * @brief Multiplies the matrix by a 4D vector (homogeneous coordinates).
     * @param v 4D vector (x, y, z, w).
     * @return Resulting 4D vector.
     * @note This is a general multiplication; use transformPoint/Direction for convenience.
     */
    Vector4 operator*(const Vector4& v) const;

    /**
     * @brief Stream output operator.
     * @param os Output stream.
     * @param v Vector for output.
     * @return Stream link for output chains.
     */
    friend std::ostream& operator<<(std::ostream& os, const Matrix4& v) {
        return os << v.toString();
    }

    // === Static factory methods ===

    /**
     * @brief Creates an identity matrix.
     * @return 4x4 identity matrix.
     */
    static Matrix4 identity() {
        return Matrix4{};
    }

    /**
     * @brief Creates a 3D translation matrix.
     * @param tx Translation along X-axis.
     * @param ty Translation along Y-axis.
     * @param tz Translation along Z-axis.
     * @return Translation matrix.
     */
    static Matrix4 translation(float tx, float ty, float tz);

    /**
     * @brief Creates a rotation matrix around the X-axis.
     * @param angle Rotation angle in radians.
     * @return Rotation matrix.
     */
    static Matrix4 rotationX(float angle);

    /**
     * @brief Creates a rotation matrix around the Y-axis.
     * @param angle Rotation angle in radians.
     * @return Rotation matrix.
     */
    static Matrix4 rotationY(float angle);

    /**
     * @brief Creates a rotation matrix around the Z-axis.
     * @param angle Rotation angle in radians.
     * @return Rotation matrix.
     */
    static Matrix4 rotationZ(float angle);

    /**
     * @brief Creates a uniform/non-uniform scaling matrix.
     * @param sx Scale factor along X-axis.
     * @param sy Scale factor along Y-axis.
     * @param sz Scale factor along Z-axis.
     * @return Scaling matrix.
     */
    static Matrix4 scale(float sx, float sy, float sz);

    /**
     * @brief Creates an orthographic projection matrix.
     * @param left Left clipping plane.
     * @param right Right clipping plane.
     * @param bottom Bottom clipping plane.
     * @param top Top clipping plane.
     * @param near Near clipping plane (distance from camera, positive).
     * @param far Far clipping plane (distance from camera, positive).
     * @return Orthographic projection matrix (maps to NDC with z in [-1,1]).
     */
    static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);

    /**
     * @brief Creates a perspective projection matrix (OpenGL-style).
     * @param fov Field of view angle in radians (vertical).
     * @param aspect Aspect ratio (width/height).
     * @param near Near clipping plane (distance from camera, positive).
     * @param far Far clipping plane (distance from camera, positive).
     * @return Perspective projection matrix.
     */
    static Matrix4 perspective(float fov, float aspect, float near, float far);
};

/**
 * @brief Represents a quaternion for 3D rotations and orientations.
 *
 * Quaternions are used to represent rotations in 3D space without the
 * problems of gimbal lock. They are stored as (x, y, z, w) where w is the
 * real part and (x, y, z) are the imaginary parts.
 *
 * This class provides basic operations: normalization, length,
 * conversion to a 4x4 rotation matrix, application to vectors,
 * multiplication, and creation from axis‑angle representation.
 *
 * @note The class is intended for use in 3D graphics and physics.
 *       All angles are in radians.
 */
class Quaternion {
public:
    /**
     * @brief Imaginary part (x component).
     */
    float x;
    /**
     * @brief Imaginary part (y component).
     */
    float y;
    /**
     * @brief Imaginary part (z component).
     */
    float z;
    /**
     * @brief Real part (w component).
     */
    float w;

    /**
     * @brief Default constructor. Creates an identity quaternion (0,0,0,1).
     * @param x Imaginary x component (default 0).
     * @param y Imaginary y component (default 0).
     * @param z Imaginary z component (default 0).
     * @param w Real component (default 1).
     */
    Quaternion(float x, float y, float z, float w);

    // === Mutator methods ===

    /**
     * @brief Normalises this quaternion to unit length.
     * @note If the length is zero, the quaternion remains unchanged.
     */
    void norm();

    // === Non-mutator methods ===

    /**
     * @brief Computes the length (magnitude) of the quaternion.
     * @return The length.
     */
    float length() const;

    /**
     * @brief Returns the conjugate of this quaternion.
     * @return A new quaternion with negated imaginary parts (x,y,z).
     * @note The conjugate represents the opposite rotation for unit quaternions.
     */
    Quaternion conjugate() const;

    /**
     * @brief Returns the inverse of this quaternion.
     * @return A new quaternion that, when multiplied by the original, yields the identity.
     * @note For unit quaternions, this is the same as conjugate().
     *       For non‑unit quaternions, the result is divided by the squared length.
     */
    Quaternion inverse() const;

    /**
     * @brief Converts this quaternion to a 4x4 rotation matrix.
     * @return A 4x4 matrix representing the same rotation.
     * @note The matrix is in row‑major order and can be used with Matrix4.
     */
    Matrix4 toMatrix4() const;

    // === Operators ===

    /**
     * @brief Multiplies two quaternions (composition of rotations).
     * @param other The quaternion to multiply on the right.
     * @return The resulting quaternion (this * other).
     * @note Quaternion multiplication is not commutative.
     */
    Quaternion operator*(const Quaternion& other) const;

    /**
    * @brief Multiplies this quaternion by a scalar (scale all components).
     * @param scalar The scaling factor.
     * @return A new quaternion with all components multiplied by scalar.
     */
    Quaternion operator*(float scalar) const;

    /**
     * @brief Divides this quaternion by a scalar.
     * @param scalar The divisor (must not be zero).
     * @return A new quaternion with all components divided by scalar.
     */
    Quaternion operator/(float scalar) const;

    /**
     * @brief In‑place multiplication by a scalar.
     * @param scalar The scaling factor.
     */
    void operator*=(float scalar);

    /**
     * @brief In‑place division by a scalar.
     * @param scalar The divisor (must not be zero).
     */
    void operator/=(float scalar);

    // === Static factory methods ===

    /**
     * @brief Creates a quaternion from an axis and an angle.
     * @param axis The axis of rotation (does not need to be normalised).
     * @param angle The rotation angle in radians.
     * @return The corresponding unit quaternion.
     * @note The axis is normalised internally.
     */
    static Quaternion fromAxisAngle(const Vector3& axis, float angle);

    /**
     * @brief Spherical linear interpolation (slerp) between two quaternions.
     * @param a Start quaternion.
     * @param b End quaternion.
     * @param t Interpolation factor in [0,1].
     * @return The interpolated quaternion.
     * @note Assumes both quaternions are unit and takes the shortest path.
     */
    static Quaternion slerp(const Quaternion& a, const Quaternion& b, float t);
};
/**
* @brief Multiplies this quaternion by a scalar (scale all components).
* @param scalar The scaling factor.
* @return A new quaternion with all components multiplied by scalar.
*/
inline Quaternion operator*(float factor, const Quaternion& quat) {
    return quat * factor;
}

/**
 * @brief Converts an angle from degrees to radians.
 * @param angle Angle in degrees.
 * @return Angle in radians.
 */
float radians(float angle);

// === Vector functions ===

/**
 * @brief Computes the cross product of two 3D vectors.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @return New vector that is perpendicular to both v1 and v2.
 * @note The result is not normalized.
 */
Vector3 getCross(const Vector3& v1, const Vector3& v2);

/**
 * @brief Computes the scalar triple product (mixed product) of three vectors.
 * @param v1 First vector.
 * @param v2 Second vector.
 * @param v3 Third vector.
 * @return Scalar value equal to v1 · (v2 × v3).
 * @note This represents the signed volume of the parallelepiped formed by the vectors.
 */
float getMixedProduct(const Vector3& v1, const Vector3& v2, const Vector3& v3);

/**
 * @brief Normalizes a homogeneous vector by dividing by its w component.
 * @param v 4D homogeneous vector.
 * @return 3D vector (x/w, y/w, z/w). If w == 0, returns a zero vector.
 * @note Used for perspective projection and converting clip space to world space.
 */
Vector3 homogeneousNormalize(const Vector4& v);

/**
 * @brief Normalizes a 3D vector to unit length.
 * @param vec Input vector.
 * @return Normalized vector (length == 1). If length is zero, returns a zero vector.
 * @note This function does not modify the input; it returns a new vector.
 */
Vector3 normalize(Vector3 v);

// === Matrix functions ===

/**
 * @brief Builds a view matrix (look-at) for a camera.
 * @param pos Eye position of the camera.
 * @param target Point the camera is looking at.
 * @param up Up direction (usually (0,1,0)).
 * @return 4x4 view matrix that transforms world space to view space.
 */
Matrix4 lookAt(Vector3 &pos, Vector3 &target, Vector3 &up);

// === Quaternion functions ===

/**
 * @brief Normalizes a quaternion to unit length.
 * @param quat The quaternion to normalize.
 * @return A new quaternion with length = 1. If the length is zero,
 *         returns the identity quaternion (0, 0, 0, 1).
 * @note This function does not modify the input; it returns a copy.
 *       It is the non‑mutating counterpart of Quaternion::norm().
 */
Quaternion normalize(Quaternion quat);