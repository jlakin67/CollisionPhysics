#include "shader.h"

Shader::Shader(const char* vertexShaderSource, const char* fragmentShaderSource, const char* geometryShaderSource) {
    compile(vertexShaderSource, fragmentShaderSource, geometryShaderSource);
}

void Shader::compile(const char* vertexShaderSource, const char* fragmentShaderSource, const char* geometryShaderSource) {
    GLuint vertexShader, fragmentShader;
    GLint success; GLchar vertexInfoLog[512]; GLchar fragmentInfoLog[512];
    GLchar programInfoLog[512];
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, vertexInfoLog);
        std::cout << vertexInfoLog << std::endl;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, fragmentInfoLog);
        std::cout << fragmentInfoLog << std::endl;
    }

    GLuint geometryShader;
    GLchar geometryInfoLog[512];
    if (geometryShaderSource) {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
        glCompileShader(geometryShader);
        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometryShader, 512, NULL, geometryInfoLog);
            std::cout << geometryInfoLog << std::endl;
        }
    }

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    if (geometryShaderSource) glAttachShader(program, geometryShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, programInfoLog);
        std::cout << programInfoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShaderSource) glDeleteShader(geometryShader);
    isCompiled = true;
}

Shader::Shader() {
    program = 0;
    isCompiled = false;
}

Shader::~Shader() {
    if (isCompiled) glDeleteProgram(program);
}

void Shader::useProgram() {
    if (isCompiled) glUseProgram(program);
    else std::cout << "No shader compiled\n";
}

void Shader::loadFile(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
    std::ifstream vertexFile;
    std::ifstream fragmentFile;
    std::ifstream geometryFile;
    std::string vertexShaderSource, fragmentShaderSource;
    std::string geometryShaderSource;
    vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    if (geometryPath) geometryFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vertexFile.open(vertexPath);
        fragmentFile.open(fragmentPath);
        std::stringstream vertexFileStream; std::stringstream fragmentFileStream;
        vertexFileStream << vertexFile.rdbuf();
        fragmentFileStream << fragmentFile.rdbuf();
        vertexFile.close();
        fragmentFile.close();
        vertexShaderSource = vertexFileStream.str();
        fragmentShaderSource = fragmentFileStream.str();
        if (geometryPath) {
            std::stringstream geometryFileStream;
            geometryFile.open(geometryPath);
            geometryFileStream << geometryFile.rdbuf();
            geometryFile.close();
            geometryShaderSource = geometryFileStream.str();
        }
    }
    catch (std::ifstream::failure) {
        std::cout << "Shader source file not successfully read\n";
    }
    compile(vertexShaderSource.c_str(), fragmentShaderSource.c_str(), (geometryPath) ? geometryShaderSource.c_str() : NULL);
}

void Shader::setFloat(std::string name, GLfloat value) {
    //must use Shader::useProgram() before calling this function
    glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}

void Shader::setVec2(std::string name, const GLfloat* value)
{
    //must use Shader::useProgram() before calling this function
    glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, value);
}

void Shader::setInt(std::string name, GLint value) {
    //must use Shader::useProgram() before calling this function
    GLint location = glGetUniformLocation(program, name.c_str());
    glUniform1i(location, value);
}

void Shader::setVec3(std::string name, const GLfloat* value) {
    //must use Shader::useProgram() before calling this function
    glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, value);
}

void Shader::setVec4(std::string name, const GLfloat* value) {
    //must use Shader::useProgram() before calling this function
    glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, value);
}

void Shader::setBool(std::string name, bool value)
{
    glUniform1i(glGetUniformLocation(program, name.c_str()), static_cast<int>(value));
}

void Shader::setMat4(std::string name, const GLfloat* value) {
    //must use Shader::useProgram() before calling this function
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, value);
}