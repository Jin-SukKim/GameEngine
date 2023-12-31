#include "Camera.h"

#include <iostream>
Camera::Camera(Vector3 pos, Vector3 lookAt) : m_camPos(pos), m_camLookAt(lookAt)
{
    // 카메라 시점 방향 - 카메라가 보는 방향, 걸어가는 방향
    m_camDir = m_camLookAt - m_camPos;
    m_camDir.Normalize();
    
    // 임시 up vector
    m_camUp = { 0.f, 1.f, 0.f };
    // 카메라 오른쪽 vector - 오른쪽 방향, eyeDir과 upDir로부터 계산
    // 곱하는 순서 조심하기
    Vector3 m_camRight = m_camUp.Cross(m_camDir);
    m_camRight.Normalize();

    // Up Vector
    m_camUp = m_camDir.Cross(m_camRight);
    m_camUp.Normalize();

    m_pitch = 0.f;	// x-축
    m_yaw = 0.f;	// y-축
    m_roll = 0.f;	// z-축

    m_camSpeed = 1.f; // 카메라 속도

    m_fovAngleY = 70.f;		 // 시야각
    m_nearZ = 0.01f;		 // viewing-frustum의 near plane
    m_farZ = 100.f;		 // viewing-frustum의 far plane
    m_aspect = 16.f / 9.f; // 대중적인 모니터 비율

    // 마우스 감도
    m_sensitivityX = 3.f;
    m_sensitivityY = 1.f;

    // 원근 투영을 사용할 것인지
    m_usePerspectiveProjection = true;
}

Matrix Camera::GetFPPViewRowMatrix()
{
    // View 좌표계에서는 정면 방향이 +z, 오른쪽이 +x, 위쪽이 +y이다.
    // 시점 변환은 가상 세계가 통째로 반대로 움직이는 것과 같다.
    return Matrix::CreateRotationX(-m_pitch)
        * Matrix::CreateRotationY(-m_yaw)
        * Matrix::CreateTranslation(-m_camPos);
}

Matrix Camera::GetProjRowMatrix()
{
    // 투영 행렬
    if (m_usePerspectiveProjection)
    {
        // 원근 투영
        // (시야각, 화면비율, Near-Plane, Far-Plane) - 시야각은 radian을 사용하기에 각도를 변환해준다. 
        return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fovAngleY), m_aspect, m_nearZ, m_farZ);
    }
    else
    {
        // 정투영
        // (x 방향 범위 변수 2개, y 방향 범위 변수 2개, Near-Plane, Far-Plane)
        return DirectX::XMMatrixOrthographicOffCenterLH(-m_aspect, m_aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
    }
}

Matrix Camera::GetFocusViewRowMatrix()
{
    // (카메라 위치, 바라보는 위치(초점), 카메라의 upVector)    
    return DirectX::XMMatrixLookAtLH(
        m_camPos, m_camLookAt, m_camUp);
}

// 회전하는 것이 아니라면 방향 벡터들은 업데이트할 필요없다.
void Camera::MoveForward(float dt)
{
    // 컴퓨터마다 속도가 다르기에 시간값(delta time)을 곱해준다.
    m_camPos += m_camDir * m_camSpeed * dt;
    m_camLookAt += m_camDir * m_camSpeed * dt;
}

void Camera::MoveRight(float dt)
{
    m_camPos += m_camRight * m_camSpeed * dt;
    m_camLookAt += m_camRight * m_camSpeed * dt;
}

void Camera::MoveUp(float dt)
{
    m_camPos += m_camUp * m_camSpeed * dt;
    m_camLookAt += m_camUp * m_camSpeed * dt;
}

// TODO: Rotation doesn't work properly
void Camera::MouseRotate(float mouseNdcX, float mouseNdcY)
{
    
    // NDC좌표계 범위 [-1, 1]로 회전 반경 제한
    m_yaw = mouseNdcX * m_sensitivityX; // 위/아래 회전
    // mouseNdcY가 -값일 떄 아래로 회전하고 +면 위로 회전하려면 -를 붙여서 저장해준다.
    // 스크린 좌표계는 y값이 아래로 갈수록 커지기에 -로 반전시켜주는 것이다.
    m_pitch = -mouseNdcY * m_sensitivityY; // 좌우 회전

    Matrix rot = Matrix::CreateRotationX(m_yaw) * Matrix::CreateRotationY(m_pitch);
    m_camLookAt = Vector3::Transform(m_camLookAt, rot);
    m_camDir = m_camLookAt - m_camPos;
    m_camDir.Normalize();
    m_camUp = Vector3::Transform(m_camUp, rot);
    m_camUp.Normalize();
    m_camRight = m_camUp.Cross(m_camDir);
    m_camRight.Normalize();
}

void Camera::SetAspectRatio(float aspect)
{
    m_aspect = aspect;
}

void Camera::SetCameraSpeed(float speed)
{
    m_camSpeed = speed;
}

void Camera::SetFovAngle(float angle)
{
    m_fovAngleY = angle;
}

Vector3 Camera::GetCameraPos()
{
    return m_camPos;
}

void Camera::SetPerspectiveProjection(bool usePerspective)
{
    m_usePerspectiveProjection = usePerspective;
}
