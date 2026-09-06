#include "ol_shader.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <utility>

namespace
{
    GLuint nextShader = 1;
    GLuint nextProgram = 100;
    std::unordered_map<GLuint, int> programDeletes;

    GLuint APIENTRY fakeCreateShader(GLenum) { return nextShader++; }
    void APIENTRY fakeShaderSource(GLuint, GLsizei, const GLchar* const*, const GLint*) {}
    void APIENTRY fakeCompileShader(GLuint) {}
    void APIENTRY fakeGetShaderiv(GLuint, GLenum, GLint* value) { *value = GL_TRUE; }
    GLuint APIENTRY fakeCreateProgram() { return nextProgram++; }
    void APIENTRY fakeAttachShader(GLuint, GLuint) {}
    void APIENTRY fakeLinkProgram(GLuint) {}
    void APIENTRY fakeGetProgramiv(GLuint, GLenum, GLint* value) { *value = GL_TRUE; }
    void APIENTRY fakeDeleteShader(GLuint) {}
    void APIENTRY fakeDeleteProgram(GLuint program) { ++programDeletes[program]; }

    bool require(bool condition, const char* message)
    {
        if (condition) return true;
        std::cerr << message << '\n';
        return false;
    }
}

int main()
{
    glad_glCreateShader = fakeCreateShader;
    glad_glShaderSource = fakeShaderSource;
    glad_glCompileShader = fakeCompileShader;
    glad_glGetShaderiv = fakeGetShaderiv;
    glad_glCreateProgram = fakeCreateProgram;
    glad_glAttachShader = fakeAttachShader;
    glad_glLinkProgram = fakeLinkProgram;
    glad_glGetProgramiv = fakeGetProgramiv;
    glad_glDeleteShader = fakeDeleteShader;
    glad_glDeleteProgram = fakeDeleteProgram;

    const char* vertexPath = "ol_shader_move_test.vert";
    const char* fragmentPath = "ol_shader_move_test.frag";
    {
        std::ofstream(vertexPath) << "void main() {}";
        std::ofstream(fragmentPath) << "void main() {}";
    }

    int result = 0;
    {
        ol::Shader source(vertexPath, fragmentPath);
        const GLuint program = source.getID();
        ol::Shader moved(std::move(source));
        if (!require(!source.isValid(), "move construction did not clear the source program")) result = 1;
        if (!require(moved.getID() == program, "move construction did not transfer the program")) result = 1;
    }
    if (!require(programDeletes[100] == 1, "move construction deleted the program more than once")) result = 1;

    {
        ol::Shader source(vertexPath, fragmentPath);
        ol::Shader target(vertexPath, fragmentPath);
        const GLuint sourceProgram = source.getID();
        const GLuint oldTargetProgram = target.getID();

        target = std::move(source);
        if (!require(!source.isValid(), "move assignment did not clear the source program")) result = 1;
        if (!require(target.getID() == sourceProgram, "move assignment did not transfer the program")) result = 1;
        if (!require(programDeletes[oldTargetProgram] == 1, "move assignment did not release the target's old program")) result = 1;
    }
    if (!require(programDeletes[101] == 1, "move-assigned program was not deleted exactly once")) result = 1;

    std::remove(vertexPath);
    std::remove(fragmentPath);

    if (result != 0) return result;
    std::cout << "Shader move ownership regression test passed\n";
    return 0;
}
