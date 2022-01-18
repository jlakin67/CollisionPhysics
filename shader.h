#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

class Shader {
public:
    Shader(const char* vertexShaderSource, const char* fragmentShaderSource,
        const char* geometryShaderSource = NULL);
    Shader();
    ~Shader();
    void loadFile(const char* vertexPath, const char* fragmentPath,
        const char* geometryPath = NULL);
    void useProgram();
    void setMat4(std::string name, const GLfloat* value);
    void setInt(std::string name, GLint value);
    void setFloat(std::string name, GLfloat value);
    void setVec2(std::string name, const GLfloat* value);
    void setVec3(std::string name, const GLfloat* value);
    void setVec4(std::string name, const GLfloat* value);
    void setBool(std::string name, bool value);
    void compile(const char* vertexShaderSource, const char* fragmentShaderSource,
        const char* geometryShaderSource = NULL);
private:
    bool isCompiled;
    GLint program;
};