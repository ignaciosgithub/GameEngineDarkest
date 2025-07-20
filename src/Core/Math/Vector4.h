#pragma once

#include <cmath>
#include <algorithm>

namespace GameEngine {
    struct Vector4 {
        float x, y, z, w;
        
        Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        Vector4(float value) : x(value), y(value), z(value), w(value) {}
        
        // Basic operations
        Vector4 operator+(const Vector4& other) const {
            return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
        }
        
        Vector4 operator-(const Vector4& other) const {
            return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
        }
        
        Vector4 operator-() const {
            return Vector4(-x, -y, -z, -w);
        }
        
        Vector4 operator*(float scalar) const {
            return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
        }
        
        Vector4 operator*(const Vector4& other) const {
            return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
        }
        
        Vector4 operator/(float scalar) const {
            return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
        }
        
        Vector4& operator+=(const Vector4& other) {
            x += other.x; y += other.y; z += other.z; w += other.w;
            return *this;
        }
        
        Vector4& operator-=(const Vector4& other) {
            x -= other.x; y -= other.y; z -= other.z; w -= other.w;
            return *this;
        }
        
        Vector4& operator*=(float scalar) {
            x *= scalar; y *= scalar; z *= scalar; w *= scalar;
            return *this;
        }
        
        Vector4& operator/=(float scalar) {
            x /= scalar; y /= scalar; z /= scalar; w /= scalar;
            return *this;
        }
        
        // Vector operations
        float Dot(const Vector4& other) const {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }
        
        float Length() const {
            return std::sqrt(x * x + y * y + z * z + w * w);
        }
        
        float LengthSquared() const {
            return x * x + y * y + z * z + w * w;
        }
        
        Vector4 Normalized() const {
            float len = Length();
            if (len > 0.0f) {
                return *this / len;
            }
            return Vector4(0.0f);
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
                case 3: return w;
                default: return x;
            }
        }
        
        float& operator[](int index) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                default: return x;
            }
        }
        
        // Static utility methods
        static Vector4 Min(const Vector4& a, const Vector4& b) {
            return Vector4(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w));
        }
        
        static Vector4 Max(const Vector4& a, const Vector4& b) {
            return Vector4(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w));
        }
        
        // Static constants
        static const Vector4 Zero;
        static const Vector4 One;
    };
    
    // Global operators
    inline Vector4 operator*(float scalar, const Vector4& vector) {
        return vector * scalar;
    }
}
