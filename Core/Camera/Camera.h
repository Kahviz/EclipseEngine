#pragma once
#include "UntilitedMathLibrary.h"

class Camera
{
public:
    Camera();
    void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

    const Matrix4x4& GetViewMatrix() const;
    const Matrix4x4& GetProjectionMatrix() const;

    const Vector4& GetPositionVector() const;
    const Vector3& GetPositionVector3() const;
    const Vector4& GetRotationVector() const;
    const Vector3& GetRotationVector3() const;

    void SetPosition(const Vector4& pos);
    void SetPosition(float x, float y, float z);
    void AdjustPosition(const Vector4& pos);
    void AdjustPosition(float x, float y, float z);
    void SetRotation(const Vector4& rot);
    void SetRotation(float x, float y, float z);
    void AdjustRotation(const Vector4& rot);
    void AdjustRotation(float x, float y, float z);

    Vector3 GetForward() const;
    Vector3 GetRight() const;
    Vector3 GetUp() const;
    void UpdateViewMatrix();

private:
    Vector4 posVector;
    Vector4 rotVector;
    Vector3 pos;
    Vector3 rot;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;

    const Vector3 DEFAULT_RIGHT_VECTOR = { 1.0f, 0.0f, 0.0f };
    const Vector3 DEFAULT_FORWARD_VECTOR = { 0.0f, 0.0f, 1.0f };
    const Vector3 DEFAULT_UP_VECTOR = { 0.0f, 1.0f, 0.0f };
};