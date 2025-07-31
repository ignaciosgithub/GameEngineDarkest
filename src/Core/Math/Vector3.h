#pragma once

#include <cmath>
#include <algorithm>

namespace GameEngine {
    struct Vector3 {
        float x, y, z;
        
        Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
        Vector3(float value) : x(value), y(value), z(value) {}
        
        // Basic operations
        Vector3 operator+(const Vector3& other) const {
            return Vector3(x + other.x, y + other.y, z + other.z);
        }
        
        Vector3 operator-(const Vector3& other) const {
            return Vector3(x - other.x, y - other.y, z - other.z);
        }
        
        Vector3 operator-() const {
            return Vector3(-x, -y, -z);
        }
        
        Vector3 operator*(float scalar) const {
            return Vector3(x * scalar, y * scalar, z * scalar);
        }
        
        Vector3 operator*(const Vector3& other) const {
            return Vector3(x * other.x, y * other.y, z * other.z);
        }
        
        Vector3 operator/(float scalar) const {
            return Vector3(x / scalar, y / scalar, z / scalar);
        }
        
        Vector3& operator+=(const Vector3& other) {
            x += other.x; y += other.y; z += other.z;
            return *this;
        }
        
        Vector3& operator-=(const Vector3& other) {
            x -= other.x; y -= other.y; z -= other.z;
            return *this;
        }
        
        Vector3& operator*=(float scalar) {
            x *= scalar; y *= scalar; z *= scalar;
            return *this;
        }
        
        Vector3& operator/=(float scalar) {
            x /= scalar; y /= scalar; z /= scalar;
            return *this;
        }
        
        // Vector operations
        float Dot(const Vector3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }
        
        Vector3 Cross(const Vector3& other) const {
            return Vector3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }
        
        float Length() const {
            return std::sqrt(x * x + y * y + z * z);
        }
        
        float LengthSquared() const {
            return x * x + y * y + z * z;
        }
        
        Vector3 Normalized() const {
            float len = Length();
            if (len > 0.0f) {
                return *this / len;
            }
            return Vector3(0.0f);
        }
        
        void Normalize() {
            float len = Length();
            if (len > 0.0f) {
                *this /= len;
            }
        }
        
        // Array access
        float operator[](int index) const {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default: return x;
            }
        }
        
        float& operator[](int index) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default: return x;
            }
        }
        
        // Static utility methods
        static Vector3 Min(const Vector3& a, const Vector3& b) {
            return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
        }
        
        static Vector3 Max(const Vector3& a, const Vector3& b) {
            return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
        }
        
        // Static constants
        static const Vector3 Zero;
        static const Vector3 One;
        static const Vector3 Up;
        static const Vector3 Right;
        static const Vector3 Forward;
        
        static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
    };
    
    // Global operators
    inline Vector3 operator*(float scalar, const Vector3& vector) {
        return vector * scalar;
    }
}
