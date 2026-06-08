#include "ol_shader.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>

namespace ol
{

    Shader::~Shader()
    {
        if (isValid()) glDeleteProgram(m_id);
    }

    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        loadShader(vertexPath, fragmentPath);
        if (!isValid()) throw std::runtime_error("ERROR: Failed to create shader program!");
    }

    void Shader::loadShader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        // 旧 program 已经无效，清掉所有缓存
        if (isValid())
        {
            glDeleteProgram(m_id);
            m_id = 0;
        }
        m_uniformCache.clear();

        std::string vertexCode = loadSrc(vertexPath);
        std::string fragmentCode = loadSrc(fragmentPath);

        if (vertexCode.empty())
        {
            std::cerr << "ERROR: Failed to load vertex shader: " << vertexPath << "\n";
            return;
        }
        if (fragmentCode.empty())
        {
            std::cerr << "ERROR: Failed to load fragment shader: " << fragmentPath << "\n";
            return;
        }

        // 编译顶点着色器
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexSrc = vertexCode.c_str();
        glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
        glCompileShader(vertexShader);

        if (!checkShaderError(vertexShader, false))
        {
            glDeleteShader(vertexShader);
            return;
        }

        // 编译片段着色器
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentSrc = fragmentCode.c_str();
        glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
        glCompileShader(fragmentShader);

        if (!checkShaderError(fragmentShader, false))
        {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return;
        }

        // 链接着色器程序
        m_id = glCreateProgram();
        glAttachShader(m_id, vertexShader);
        glAttachShader(m_id, fragmentShader);
        glLinkProgram(m_id);

        if (!checkShaderError(m_id, true))
        {
            glDeleteProgram(m_id);
            m_id = 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // ============ Uniform 设置 ============

    void Shader::setBool(const std::string& name, bool value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform1i(loc, static_cast<int>(value));
    }

    void Shader::setInt(const std::string& name, int value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform1i(loc, value);
    }

    void Shader::setFloat(const std::string& name, float value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform1f(loc, value);
    }

    void Shader::setVec2(const std::string& name, const glm::vec2& value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform2fv(loc, 1, glm::value_ptr(value));
    }

    void Shader::setVec2(const std::string& name, float x, float y) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform2f(loc, x, y);
    }

    void Shader::setVec3(const std::string& name, const glm::vec3& value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(value));
    }

    void Shader::setVec3(const std::string& name, float x, float y, float z) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform3f(loc, x, y, z);
    }

    void Shader::setVec4(const std::string& name, const glm::vec4& value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(value));
    }

    void Shader::setVec4(const std::string& name,
                         float x, float y, float z, float w) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniform4f(loc, x, y, z, w);
    }

    void Shader::setMat3(const std::string& name, const glm::mat3& value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::setMat4(const std::string& name, const glm::mat4& value) const noexcept
    {
        GLint loc = getLoc(name);
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    // ============ private ============

    std::string Shader::loadSrc(const std::string& filePath) const
    {
        std::ifstream ifile(filePath, std::ios::in | std::ios::binary);
        if (!ifile) return "";

        std::stringstream stream;
        stream << ifile.rdbuf();
        ifile.close();

        if (stream.fail()) return "";
        return stream.str();
    }

    bool Shader::checkShaderError(GLuint shader, bool isProgram) const
    {
        GLint success = GL_FALSE;
        char infoLog[512] = {0};

        if (isProgram)
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
                std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
                          << infoLog << "\n";
            }
        }
        else
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
                std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                          << infoLog << "\n";
            }
        }
        return success == GL_TRUE;
    }

    void Shader::use() const noexcept
    {
        if (isValid())
            glUseProgram(m_id);
        else
            std::cerr << "WARNING: Invalid shader program used!\n";
    }

    GLint Shader::getLoc(const std::string& name) const noexcept
    {
        // 先查缓存
        auto it = m_uniformCache.find(name);
        if (it != m_uniformCache.end())
            return it->second;

        // 未命中 → 查询 OpenGL → 存入缓存
        GLint loc = glGetUniformLocation(m_id, name.c_str());
        m_uniformCache[name] = loc;

        if (loc == -1) std::cerr << "WARNING: Uniform not found: " << name << "\n";

        return loc;
    }

} // namespace ol
