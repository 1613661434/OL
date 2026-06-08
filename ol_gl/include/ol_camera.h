#ifndef OL_CAMERA_H
#define OL_CAMERA_H 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ol
{

    class Camera
    {
    public:
        // 摄像机移动方向枚举
        enum class CameraMovement : char
        {
            FORWARD,
            BACKWARD,
            LEFT,
            RIGHT
        };

    private:
        // 摄像机向量
        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;
        glm::vec3 m_worldUp;

        // 欧拉角（度）
        float m_yaw;   // 偏航角
        float m_pitch; // 俯仰角

        // 摄像机参数
        float m_movementSpeed;
        float m_mouseSensitivity;
        float m_fov; // 视场角（度）

    public:
        // 构造函数：可自定义初始位置、世界向上方向、欧拉角、FOV
        explicit Camera(
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f,
            float fov = 45.0f);

        // 获取观察矩阵 (view = lookAt)
        glm::mat4 GetViewMatrix() const;

        // 处理键盘移动（WASD）
        void ProcessKeyboard(CameraMovement direction, float deltaTime);

        // 处理鼠标移动（改变观察方向）
        void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

        // 处理鼠标滚轮（缩放/改变FOV）
        void ProcessMouseScroll(float yOffset);

        // ---- Getters ----
        float GetFOV() const noexcept { return m_fov; }
        glm::vec3 GetPosition() const noexcept { return m_position; }
        glm::vec3 GetFront() const noexcept { return m_front; }

        // ---- Setters ----
        void SetMovementSpeed(float speed) noexcept { m_movementSpeed = speed; }
        void SetMouseSensitivity(float sens) noexcept { m_mouseSensitivity = sens; }

    private:
        // 根据欧拉角重新计算 front / right / up 向量
        void updateCameraVectors();
    };

} // namespace ol

#endif
