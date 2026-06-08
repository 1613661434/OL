#include "ol_camera.h"
#include <cmath>

namespace ol
{

    Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch, float fov)
        : m_position(position), m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_worldUp(worldUp), m_yaw(yaw), m_pitch(pitch), m_movementSpeed(3.0f), m_mouseSensitivity(0.1f), m_fov(fov)
    {
        updateCameraVectors();
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = m_movementSpeed * deltaTime;

        switch (direction)
        {
        case CameraMovement::FORWARD:
            m_position += m_front * velocity;
            break;
        case CameraMovement::BACKWARD:
            m_position -= m_front * velocity;
            break;
        case CameraMovement::LEFT:
            m_position -= m_right * velocity;
            break;
        case CameraMovement::RIGHT:
            m_position += m_right * velocity;
            break;
        }
    }

    void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
    {
        xOffset *= m_mouseSensitivity;
        yOffset *= m_mouseSensitivity;

        m_yaw += xOffset;
        m_pitch += yOffset;

        // 限制俯仰角，防止视角翻转
        if (constrainPitch)
        {
            if (m_pitch > 89.0f) m_pitch = 89.0f;
            if (m_pitch < -89.0f) m_pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void Camera::ProcessMouseScroll(float yOffset)
    {
        m_fov -= yOffset;
        if (m_fov < 1.0f) m_fov = 1.0f;
        if (m_fov > 60.0f) m_fov = 60.0f;
    }

    // ============ private ============
    void Camera::updateCameraVectors()
    {
        // 根据欧拉角计算前向量
        glm::vec3 front;
        front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        front.y = std::sin(glm::radians(m_pitch));
        front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        // 右向量 = front × worldUp
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        // 上向量 = right × front
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

} // namespace ol
