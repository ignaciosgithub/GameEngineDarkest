#include "Quaternion.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GameEngine {

Quaternion::Quaternion(const Vector3& axis, float angle) {
    float halfAngle = angle * 0.5f;
    float sinHalf = std::sin(halfAngle);
    Vector3 normalizedAxis = axis.Normalized();
    
    x = normalizedAxis.x * sinHalf;
    y = normalizedAxis.y * sinHalf;
    z = normalizedAxis.z * sinHalf;
    w = std::cos(halfAngle);
}

Quaternion Quaternion::operator+(const Quaternion& other) const {
    return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
}

Quaternion Quaternion::operator-(const Quaternion& other) const {
    return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    return Quaternion(
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w,
        w * other.w - x * other.x - y * other.y - z * other.z
    );
}

Quaternion Quaternion::operator*(float scalar) const {
    return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
}

float Quaternion::Length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

float Quaternion::LengthSquared() const {
    return x * x + y * y + z * z + w * w;
}

Quaternion Quaternion::Normalized() const {
    float len = Length();
    if (len > 0.0f) {
        return *this * (1.0f / len);
    }
    return Identity();
}

void Quaternion::Normalize() {
    float len = Length();
    if (len > 0.0f) {
        float invLen = 1.0f / len;
        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;
    }
}

Quaternion Quaternion::Conjugate() const {
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Inverse() const {
    float lenSq = LengthSquared();
    if (lenSq > 0.0f) {
        Quaternion conj = Conjugate();
        return conj * (1.0f / lenSq);
    }
    return Identity();
}

Vector3 Quaternion::RotateVector(const Vector3& vector) const {
    Quaternion vecQuat(vector.x, vector.y, vector.z, 0.0f);
    Quaternion result = (*this) * vecQuat * Conjugate();
    return Vector3(result.x, result.y, result.z);
}

Vector3 Quaternion::ToEulerAngles() const {
    Vector3 angles;
    
    float sinr_cosp = 2 * (w * x + y * z);
    float cosr_cosp = 1 - 2 * (x * x + y * y);
    angles.x = std::atan2(sinr_cosp, cosr_cosp);
    
    float sinp = 2 * (w * y - z * x);
    if (std::abs(sinp) >= 1) {
        angles.y = std::copysign(M_PI / 2, sinp);
    } else {
        angles.y = std::asin(sinp);
    }
    
    float siny_cosp = 2 * (w * z + x * y);
    float cosy_cosp = 1 - 2 * (y * y + z * z);
    angles.z = std::atan2(siny_cosp, cosy_cosp);
    
    return angles;
}

Quaternion Quaternion::Identity() {
    return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Quaternion::FromEulerAngles(float pitch, float yaw, float roll) {
    float cy = std::cos(yaw * 0.5f);
    float sy = std::sin(yaw * 0.5f);
    float cp = std::cos(pitch * 0.5f);
    float sp = std::sin(pitch * 0.5f);
    float cr = std::cos(roll * 0.5f);
    float sr = std::sin(roll * 0.5f);
    
    return Quaternion(
        sr * cp * cy - cr * sp * sy,
        cr * sp * cy + sr * cp * sy,
        cr * cp * sy - sr * sp * cy,
        cr * cp * cy + sr * sp * sy
    );
}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle) {
    return Quaternion(axis, angle);
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t) {
    float dot = Dot(a, b);
    
    Quaternion b_copy = b;
    if (dot < 0.0f) {
        b_copy = b * -1.0f;
        dot = -dot;
    }
    
    if (dot > 0.9995f) {
        Quaternion result = a + (b_copy - a) * t;
        result.Normalize();
        return result;
    }
    
    float theta_0 = std::acos(dot);
    float theta = theta_0 * t;
    float sin_theta = std::sin(theta);
    float sin_theta_0 = std::sin(theta_0);
    
    float s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
    float s1 = sin_theta / sin_theta_0;
    
    return a * s0 + b_copy * s1;
}

float Quaternion::Dot(const Quaternion& a, const Quaternion& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

}
