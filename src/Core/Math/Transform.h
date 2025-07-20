#pragma once

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4.h"

namespace GameEngine {
    class Transform {
    public:
        Transform();
        Transform(const Vector3& position, const Quaternion& rotation = Quaternion::Identity(), const Vector3& scale = Vector3::One);
        
        // Position
        const Vector3& GetPosition() const { return m_position; }
        void SetPosition(const Vector3& position);
        void Translate(const Vector3& translation);
        
        // Rotation
        const Quaternion& GetRotation() const { return m_rotation; }
        void SetRotation(const Quaternion& rotation);
        void Rotate(const Quaternion& rotation);
        void Rotate(const Vector3& axis, float angle);
        
        // Scale
        const Vector3& GetScale() const { return m_scale; }
        void SetScale(const Vector3& scale);
        void SetScale(float uniformScale);
        
        // Direction vectors
        Vector3 GetForward() const;
        Vector3 GetRight() const;
        Vector3 GetUp() const;
        
        // Matrix operations
        Matrix4 GetLocalToWorldMatrix() const;
        Matrix4 GetWorldToLocalMatrix() const;
        
        // Hierarchy (basic support)
        void SetParent(Transform* parent);
        Transform* GetParent() const { return m_parent; }
        
        Vector3 GetWorldPosition() const;
        Quaternion GetWorldRotation() const;
        Vector3 GetWorldScale() const;
        
    private:
        void MarkDirty();
        void UpdateMatrices() const;
        
        Vector3 m_position;
        Quaternion m_rotation;
        Vector3 m_scale;
        
        Transform* m_parent = nullptr;
        
        mutable Matrix4 m_localToWorld;
        mutable Matrix4 m_worldToLocal;
        mutable bool m_isDirty = true;
    };
}
