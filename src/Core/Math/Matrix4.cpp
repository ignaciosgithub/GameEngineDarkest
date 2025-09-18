#include "Matrix4.h"
#include <cmath>
#include <cstring>

namespace GameEngine {

Matrix4::Matrix4() {
    std::fill(m.begin(), m.end(), 0.0f);
}

Matrix4::Matrix4(float diagonal) {
    std::fill(m.begin(), m.end(), 0.0f);
    m[0] = m[5] = m[10] = m[15] = diagonal;
}

Matrix4::Matrix4(const std::array<float, 16>& values) : m(values) {}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result(row, col) = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result(row, col) += (*this)(row, k) * other(k, col);
            }
        }
    }
    return result;
}

Vector3 Matrix4::operator*(const Vector3& vector) const {
    return Vector3(
        m[0] * vector.x + m[4] * vector.y + m[8] * vector.z + m[12],
        m[1] * vector.x + m[5] * vector.y + m[9] * vector.z + m[13],
        m[2] * vector.x + m[6] * vector.y + m[10] * vector.z + m[14]
    );
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
    *this = *this * other;
    return *this;
}

Matrix4 Matrix4::Identity() {
    return Matrix4(1.0f);
}

Matrix4 Matrix4::Translation(const Vector3& translation) {
    Matrix4 result = Identity();
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;
    return result;
}

Matrix4 Matrix4::Rotation(const Vector3& axis, float angle) {
    Matrix4 result = Identity();
    
    float c = std::cos(angle);
    float s = std::sin(angle);
    float t = 1.0f - c;
    
    Vector3 safeAxis = axis;
    if (safeAxis.LengthSquared() < 0.0001f) {
        safeAxis = Vector3(0.0f, 1.0f, 0.0f); // Default Y-axis rotation
    }
    
    Vector3 normalizedAxis = safeAxis.Normalized();
    float x = normalizedAxis.x;
    float y = normalizedAxis.y;
    float z = normalizedAxis.z;
    
    result.m[0] = t * x * x + c;
    result.m[1] = t * x * y + s * z;
    result.m[2] = t * x * z - s * y;
    
    result.m[4] = t * x * y - s * z;
    result.m[5] = t * y * y + c;
    result.m[6] = t * y * z + s * x;
    
    result.m[8] = t * x * z + s * y;
    result.m[9] = t * y * z - s * x;
    result.m[10] = t * z * z + c;
    
    return result;
}

Matrix4 Matrix4::Scale(const Vector3& scale) {
    Matrix4 result = Identity();
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    return result;
}

Matrix4 Matrix4::Perspective(float fov, float aspect, float near, float far) {
    Matrix4 result;
    std::fill(result.m.begin(), result.m.end(), 0.0f);

    float tanHalfFov = std::tan(fov * 0.5f);

    result(0, 0) = 1.0f / (aspect * tanHalfFov);
    result(1, 1) = 1.0f / tanHalfFov;
    result(2, 2) = -(far + near) / (far - near);
    result(2, 3) = -1.0f;
    result(3, 2) = -(2.0f * far * near) / (far - near);
    result(3, 3) = 0.0f;

    return result;
}

Matrix4 Matrix4::Orthographic(float left, float right, float bottom, float top, float near, float far) {
    Matrix4 result = Identity();
    
    result.m[0] = 2.0f / (right - left);
    result.m[5] = 2.0f / (top - bottom);
    result.m[10] = -2.0f / (far - near);
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(far + near) / (far - near);
    
    return result;
}

Matrix4 Matrix4::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
    Vector3 direction = center - eye;

    if (direction.LengthSquared() < 0.0001f) {
        direction = Vector3(0.0f, 0.0f, -1.0f);
    }

    Vector3 f = direction.Normalized();
    Vector3 crossProduct = f.Cross(up);

    if (crossProduct.LengthSquared() < 0.0001f) {
        Vector3 alternateUp = (std::abs(f.y) < 0.9f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
        crossProduct = f.Cross(alternateUp);
    }

    Vector3 s = crossProduct.Normalized();
    Vector3 u = s.Cross(f);

    Matrix4 result = Identity();

    result(0, 0) = s.x;  result(1, 0) = s.y;  result(2, 0) = s.z;  result(3, 0) = -s.Dot(eye);
    result(0, 1) = u.x;  result(1, 1) = u.y;  result(2, 1) = u.z;  result(3, 1) = -u.Dot(eye);
    result(0, 2) = -f.x; result(1, 2) = -f.y; result(2, 2) = -f.z; result(3, 2) =  f.Dot(eye);
    result(0, 3) = 0.0f; result(1, 3) = 0.0f; result(2, 3) = 0.0f; result(3, 3) = 1.0f;

    return result;
}

Matrix4 Matrix4::Transposed() const {
    Matrix4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result(col, row) = (*this)(row, col);
        }
    }
    return result;
}

Matrix4 Matrix4::Inverted() const {
    Matrix4 result = *this;
    
    Matrix4 rotation = Identity();
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            rotation(i, j) = (*this)(i, j);
        }
    }
    
    Matrix4 rotationT = rotation.Transposed();
    
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result(i, j) = rotationT(i, j);
        }
    }
    
    Vector3 translation(m[12], m[13], m[14]);
    Vector3 invertedTranslation = rotationT * (translation * -1.0f);
    result.m[12] = invertedTranslation.x;
    result.m[13] = invertedTranslation.y;
    result.m[14] = invertedTranslation.z;
    
    return result;
}

float Matrix4::Determinant() const {
    return m[0] * m[5] * m[10] * m[15] +
           m[0] * m[6] * m[11] * m[13] +
           m[0] * m[7] * m[9] * m[14] -
           m[0] * m[7] * m[10] * m[13] -
           m[0] * m[6] * m[9] * m[15] -
           m[0] * m[5] * m[11] * m[14];
}

}
