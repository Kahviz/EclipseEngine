#include "Camera.h"
#include "GLOBALS.h"

Camera::Camera()
{
    this->pos = Vector3(0.0f, 0.0f, 0.0f);
    this->posVector = LoadFloat3(this->pos);
    this->rot = Vector3(0.0f, 0.0f, 0.0f);
    this->rotVector = LoadFloat3(this->rot);
    this->UpdateViewMatrix();
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
    float fovRadians = DegreesToRadians(fovDegrees);
    this->projectionMatrix = MatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const Matrix4x4& Camera::GetViewMatrix() const { 
    return this->viewMatrix;
}
const Matrix4x4& Camera::GetProjectionMatrix() const { return this->projectionMatrix; }
const Vector4& Camera::GetPositionVector() const { return this->posVector; }
const Vector3& Camera::GetPositionVector3() const { return this->pos; }
const Vector4& Camera::GetRotationVector() const { return this->rotVector; }
const Vector3& Camera::GetRotationVector3() const { return this->rot; }

void Camera::SetPosition(const Vector4& pos)
{
    StoreFloat3(&this->pos, pos);
    this->posVector = pos;
    this->UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
    this->pos = Vector3(x, y, z);
    this->posVector = LoadFloat3(this->pos);
    this->UpdateViewMatrix();
}

void Camera::AdjustPosition(const Vector4& pos)
{
    this->posVector.x += pos.x;
    this->posVector.y += pos.y;
    this->posVector.z += pos.z;
    StoreFloat3(&this->pos, this->posVector);
    this->UpdateViewMatrix();
}

void Camera::AdjustPosition(float x, float y, float z)
{
    this->pos.x += x;
    this->pos.y += y;
    this->pos.z += z;
    this->posVector = LoadFloat3(this->pos);
    this->UpdateViewMatrix();
}

void Camera::SetRotation(const Vector4& rot)
{
    this->rotVector = rot;
    StoreFloat3(&this->rot, rot);
    this->UpdateViewMatrix();
}

void Camera::SetRotation(float x, float y, float z)
{
    this->rot = Vector3(x, y, z);
    this->rotVector = LoadFloat3(this->rot);
    this->UpdateViewMatrix();
}

void Camera::AdjustRotation(const Vector4& rot)
{
    this->rotVector.x += rot.x;
    this->rotVector.y += rot.y;
    this->rotVector.z += rot.z;
    StoreFloat3(&this->rot, this->rotVector);
    this->UpdateViewMatrix();
}

void Camera::AdjustRotation(float x, float y, float z)
{
    this->rot.x += x;
    this->rot.y += y;
    this->rot.z += z;
    this->rotVector = LoadFloat3(this->rot);
    this->UpdateViewMatrix();
}

Vector3 Camera::GetForward() const
{
    Matrix4x4 rotMatrix = MatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);

    Vector3 forward3 = { DEFAULT_FORWARD_VECTOR.x, DEFAULT_FORWARD_VECTOR.y, DEFAULT_FORWARD_VECTOR.z };
    Vector3 transformed = Vector3TransformCoord(forward3, rotMatrix);

    Vector3 f;
    f.x = transformed.x;
    f.y = transformed.y;
    f.z = transformed.z;
    return f;
}

Vector3 Camera::GetRight() const
{
    Matrix4x4 rotMatrix = MatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);

    Vector3 right3 = { DEFAULT_RIGHT_VECTOR.x, DEFAULT_RIGHT_VECTOR.y, DEFAULT_RIGHT_VECTOR.z };
    Vector3 transformed = Vector3TransformCoord(right3, rotMatrix);

    Vector3 r;
    r.x = transformed.x;
    r.y = transformed.y;
    r.z = transformed.z;
    return r;
}

void Camera::UpdateViewMatrix()
{
    Matrix4x4 rotMatrix = MatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);

    Vector3 forward3 = { DEFAULT_FORWARD_VECTOR.x, DEFAULT_FORWARD_VECTOR.y, DEFAULT_FORWARD_VECTOR.z };
    Vector3 camTarget = Vector3TransformCoord(forward3, rotMatrix);
    camTarget.x += pos.x;
    camTarget.y += pos.y;
    camTarget.z += pos.z;

    Vector3 up3 = { DEFAULT_UP_VECTOR.x, DEFAULT_UP_VECTOR.y, DEFAULT_UP_VECTOR.z };
    Vector3 upDir = Vector3TransformCoord(up3, rotMatrix);

    this->viewMatrix = MatrixLookAtLH({ pos.x, pos.y, pos.z }, camTarget, upDir);
}
