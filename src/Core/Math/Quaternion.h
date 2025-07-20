#pragma once

#include "Vector3.h"

namespace GameEngine {
    struct Quaternion {
        float x, y, z, w;
        
        Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
        Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        Quaternion(const Vector3& axis, float angle);
        
        // Basic operations
        Quaternion operator+(const Quaternion& other) const;
        Quaternion operator-(const Quaternion& other) const;
        Quaternion operator*(const Quaternion& other) const;
        Quaternion operator*(float scalar) const;
        
        // Quaternion operations
        float Length() const;
        float LengthSquared() const;
        Quaternion Normalized() const;
        void Normalize();
        Quaternion Conjugate() const;
        Quaternion Inverse() const;
        
        // Rotation operations
        Vector3 RotateVector(const Vector3& vector) const;
        Vector3 ToEulerAngles() const;
        
        // Static functions
        static Quaternion Identity();
        static Quaternion FromEulerAngles(float pitch, float yaw, float roll);
        static Quaternion FromAxisAngle(const Vector3& axis, float angle);
        static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);
        static float Dot(const Quaternion& a, const Quaternion& b);
    };
}
