#include "Transform.h"

namespace GameEngine {

Matrix4 QuaternionToMatrix(const Quaternion& q) {
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, xy = x * y2, xz = x * z2;
    float yy = y * y2, yz = y * z2, zz = z * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    Matrix4 result;
    result[0] = 1.0f - (yy + zz);
    result[1] = xy + wz;
    result[2] = xz - wy;
    result[3] = 0.0f;
    
    result[4] = xy - wz;
    result[5] = 1.0f - (xx + zz);
    result[6] = yz + wx;
    result[7] = 0.0f;
    
    result[8] = xz + wy;
    result[9] = yz - wx;
    result[10] = 1.0f - (xx + yy);
    result[11] = 0.0f;
    
    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = 0.0f;
    result[15] = 1.0f;
    
    return result;
}

Transform::Transform() 
    : m_position(Vector3::Zero)
    , m_rotation(Quaternion::Identity())
    , m_scale(Vector3::One)
{
}

Transform::Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
    : m_position(position)
    , m_rotation(rotation)
    , m_scale(scale)
{
}

void Transform::SetPosition(const Vector3& position) {
    m_position = position;
    MarkDirty();
}

void Transform::Translate(const Vector3& translation) {
    m_position += translation;
    MarkDirty();
}

void Transform::SetRotation(const Quaternion& rotation) {
    m_rotation = rotation;
    MarkDirty();
}

void Transform::Rotate(const Quaternion& rotation) {
    m_rotation = m_rotation * rotation;
    MarkDirty();
}

void Transform::Rotate(const Vector3& axis, float angle) {
    Quaternion rotation = Quaternion::FromAxisAngle(axis, angle);
    Rotate(rotation);
}

void Transform::SetScale(const Vector3& scale) {
    m_scale = scale;
    MarkDirty();
}

void Transform::SetScale(float uniformScale) {
    m_scale = Vector3(uniformScale);
    MarkDirty();
}

Vector3 Transform::GetForward() const {
    return m_rotation.RotateVector(Vector3::Forward);
}

Vector3 Transform::GetRight() const {
    return m_rotation.RotateVector(Vector3::Right);
}

Vector3 Transform::GetUp() const {
    return m_rotation.RotateVector(Vector3::Up);
}

Matrix4 Transform::GetLocalToWorldMatrix() const {
    if (m_isDirty) {
        UpdateMatrices();
    }
    return m_localToWorld;
}

Matrix4 Transform::GetWorldToLocalMatrix() const {
    if (m_isDirty) {
        UpdateMatrices();
    }
    return m_worldToLocal;
}

void Transform::SetParent(Transform* parent) {
    m_parent = parent;
    MarkDirty();
}

Vector3 Transform::GetWorldPosition() const {
    if (m_parent) {
        return m_parent->GetLocalToWorldMatrix() * m_position;
    }
    return m_position;
}

Quaternion Transform::GetWorldRotation() const {
    if (m_parent) {
        return m_parent->GetWorldRotation() * m_rotation;
    }
    return m_rotation;
}

Vector3 Transform::GetWorldScale() const {
    if (m_parent) {
        Vector3 parentScale = m_parent->GetWorldScale();
        return Vector3(m_scale.x * parentScale.x, m_scale.y * parentScale.y, m_scale.z * parentScale.z);
    }
    return m_scale;
}

void Transform::MarkDirty() {
    m_isDirty = true;
}

void Transform::UpdateMatrices() const {
    Matrix4 translation = Matrix4::Translation(m_position);
    Matrix4 rotation = QuaternionToMatrix(m_rotation);
    Matrix4 scale = Matrix4::Scale(m_scale);
    
    m_localToWorld = translation * rotation * scale;
    
    if (m_parent) {
        m_localToWorld = m_parent->GetLocalToWorldMatrix() * m_localToWorld;
    }
    
    m_worldToLocal = m_localToWorld.Inverted();
    m_isDirty = false;
}

}
