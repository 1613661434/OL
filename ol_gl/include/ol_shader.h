#ifndef OL_SHADER_H
#define OL_SHADER_H 1

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ol
{
    class Shader
    {
    private:
        GLuint m_id = 0;
        // uniform 位置缓存 —— 避免每帧字符串查找
        mutable std::unordered_map<std::string, GLint> m_uniformCache;

    public:
        Shader() = default;
        explicit Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader();

        // 禁用拷贝
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        // 支持移动
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        void loadShader(const std::string& vertexPath, const std::string& fragmentPath);
        void use() const noexcept;
        GLuint getID() const noexcept { return m_id; }
        bool isValid() const noexcept { return m_id != 0; }

        // ---- Uniform 设置 ----
        void setBool(const std::string& name, bool value) const noexcept;
        void setInt(const std::string& name, int value) const noexcept;
        void setFloat(const std::string& name, float value) const noexcept;

        void setVec2(const std::string& name, const glm::vec2& value) const noexcept;
        void setVec2(const std::string& name, float x, float y) const noexcept;
        void setVec3(const std::string& name, const glm::vec3& value) const noexcept;
        void setVec3(const std::string& name, float x, float y, float z) const noexcept;
        void setVec4(const std::string& name, const glm::vec4& value) const noexcept;
        void setVec4(const std::string& name, float x, float y,
                     float z, float w) const noexcept;

        void setMat3(const std::string& name, const glm::mat3& value) const noexcept;
        void setMat4(const std::string& name, const glm::mat4& value) const noexcept;

    private:
        std::string loadSrc(const std::string& filePath) const;
        bool checkShaderError(GLuint shader, bool isProgram) const;

        // 带缓存的 uniform 位置查询
        GLint getLoc(const std::string& name) const noexcept;
    };
} // namespace ol

#endif
