#pragma once

#include <cmath>
#include <algorithm>

namespace GameEngine {
    struct Vector2 {
        float x, y;
        
        Vector2() : x(0.0f), y(0.0f) {}
        Vector2(float x, float y) : x(x), y(y) {}
        Vector2(float value) : x(value), y(value) {}
        
        // Basic operations
        Vector2 operator+(const Vector2& other) const {
            return Vector2(x + other.x, y + other.y);
        }
        
        Vector2 operator-(const Vector2& other) const {
            return Vector2(x - other.x, y - other.y);
        }
        
        Vector2 operator-() const {
            return Vector2(-x, -y);
        }
        
        Vector2 operator*(float scalar) const {
            return Vector2(x * scalar, y * scalar);
        }
        
        Vector2 operator*(const Vector2& other) const {
            return Vector2(x * other.x, y * other.y);
        }
        
        Vector2 operator/(float scalar) const {
            return Vector2(x / scalar, y / scalar);
        }
        
        Vector2& operator+=(const Vector2& other) {
            x += other.x; y += other.y;
            return *this;
        }
        
        Vector2& operator-=(const Vector2& other) {
            x -= other.x; y -= other.y;
            return *this;
        }
        
        Vector2& operator*=(float scalar) {
            x *= scalar; y *= scalar;
            return *this;
        }
        
        Vector2& operator/=(float scalar) {
            x /= scalar; y /= scalar;
            return *this;
        }
        
        // Vector operations
        float Dot(const Vector2& other) const {
            return x * other.x + y * other.y;
        }
        
        float Length() const {
            return std::sqrt(x * x + y * y);
        }
        
        float LengthSquared() const {
            return x * x + y * y;
        }
        
        Vector2 Normalized() const {
            float len = Length();
            if (len > 0.0f) {
                return *this / len;
            }
            return Vector2(0.0f);
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
                default: return x;
            }
        }
        
        float& operator[](int index) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                default: return x;
            }
        }
        
        // Static utility methods
        static Vector2 Min(const Vector2& a, const Vector2& b) {
            return Vector2(std::min(a.x, b.x), std::min(a.y, b.y));
        }
        
        static Vector2 Max(const Vector2& a, const Vector2& b) {
            return Vector2(std::max(a.x, b.x), std::max(a.y, b.y));
        }
        
        // Static constants
        static const Vector2 Zero;
        static const Vector2 One;
        static const Vector2 Up;
        static const Vector2 Right;
    };
    
    // Global operators
    inline Vector2 operator*(float scalar, const Vector2& vector) {
        return vector * scalar;
    }
}
