#pragma once

#include "Vector3.h"
#include <array>

namespace GameEngine {
    struct Matrix4 {
        std::array<float, 16> m;
        
        Matrix4();
        Matrix4(float diagonal);
        Matrix4(const std::array<float, 16>& values);
        
        // Access operators
        float& operator[](int index) { return m[index]; }
        const float& operator[](int index) const { return m[index]; }
        
        float& operator()(int row, int col) { return m[row * 4 + col]; }
        const float& operator()(int row, int col) const { return m[row * 4 + col]; }
        
        // Matrix operations
        Matrix4 operator*(const Matrix4& other) const;
        Vector3 operator*(const Vector3& vector) const;
        Matrix4& operator*=(const Matrix4& other);
        
        // Transformation matrices
        static Matrix4 Identity();
        static Matrix4 Translation(const Vector3& translation);
        static Matrix4 Rotation(const Vector3& axis, float angle);
        static Matrix4 Scale(const Vector3& scale);
        static Matrix4 Perspective(float fov, float aspect, float near, float far);
        static Matrix4 Orthographic(float left, float right, float bottom, float top, float near, float far);
        static Matrix4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
        
        // Utility functions
        Matrix4 Transposed() const;
        Matrix4 Inverted() const;
        float Determinant() const;
        
        const float* Data() const { return m.data(); }
        float* Data() { return m.data(); }
    };
}
