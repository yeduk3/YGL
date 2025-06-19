#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>

struct Program
{
    GLuint programID = 0;
    GLuint vertexShaderID = 0;
    std::string vertexShaderName = "";
    GLuint geomShaderID = 0;
    std::string fragShaderName = "";
    GLuint fragShaderID = 0;
    std::string geomShaderName = "";

    std::string loadText(const char *filename)
    {
        std::fstream file(filename);
        if (!file.is_open())
        {
            std::cerr << filename << " File Not Found" << std::endl;
            return "";
        }

        std::istreambuf_iterator<char> begin(file), end;
        return std::string(begin, end);
    }

    bool shaderCompileCheck(GLuint shaderID)
    {
        GLint isCompiled = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

            for (auto e : errorLog)
            {
                std::cout << e;
            }
            std::cout << std::endl;

            // Provide the infolog in whatever manor you deem best.
            // Exit with failure.
            glDeleteShader(shaderID); // Don't leak the shader.
            return false;
        }
        return true;
    }

    void loadShader(const char *vShaderFile, const char *fShaderFile)
    {
        cleanUp();
        
        // Create Program
        programID = glCreateProgram();
        std::cout << "Program " << programID << " created" << std::endl;
        
        loadShaderOf(vShaderFile, GL_VERTEX_SHADER);
        loadShaderOf(fShaderFile, GL_FRAGMENT_SHADER);
        
        linkShader();
    }
    
    void loadShader(const char *vShaderFile, const char *gShaderFile, const char *fShaderFile)
    {
        cleanUp();
        
        // Create Program
        programID = glCreateProgram();
        std::cout << "Program " << programID << " created" << std::endl;
        
        loadShaderOf(vShaderFile, GL_VERTEX_SHADER);
        loadShaderOf(gShaderFile, GL_GEOMETRY_SHADER);
        loadShaderOf(fShaderFile, GL_FRAGMENT_SHADER);
        
        linkShader();
    }
    
    void loadShader(const char *vShaderFile,
                    const char *tcShaderFile,
                    const char *teShaderFile,
                    const char *gShaderFile,
                    const char *fShaderFile)
    {
        cleanUp();
        
        // Create Program
        programID = glCreateProgram();
        std::cout << "Program " << programID << " created" << std::endl;
        
        loadShaderOf(vShaderFile, GL_VERTEX_SHADER);
        loadShaderOf(tcShaderFile, GL_TESS_CONTROL_SHADER);
        loadShaderOf(teShaderFile, GL_TESS_EVALUATION_SHADER);
        loadShaderOf(gShaderFile, GL_GEOMETRY_SHADER);
        loadShaderOf(fShaderFile, GL_FRAGMENT_SHADER);
        
        linkShader();
    }
    
    void loadShaderOf(const char *shaderFile, const GLenum shaderType) {
        std::string shaderName = std::string(shaderFile);
        
        std::string shaderText = loadText(shaderFile);
        
        GLuint shaderID = glCreateShader(shaderType);
        
        // Read Shader File
        // c_str()은 const char * 값을 반환.
        // Text로 받지 않으면 dangling pointer 발생.
        const GLchar* shaderCode = shaderText.c_str();
        glShaderSource(shaderID, 1, &shaderCode, 0);
        glCompileShader(shaderID);
        if(shaderCompileCheck(shaderID))
            glAttachShader(programID, shaderID);
        else {
            std::cout<< "Shader: " << shaderName << "(" << shaderText.length() << ") with ID " << shaderID << " compile failed." << std::endl;
            cleanUp();
        }
    }
    
    void printLog()
    {
        GLint maxLength = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(programID, maxLength, &maxLength, &errorLog[0]);

        for (auto e : errorLog)
        {
            std::cout << e;
        }
        std::cout << std::endl;
    }

    void linkShader()
    {
        // 다 붙이면 링크 후 사용 등록
        glLinkProgram(programID);
        GLint linkStatus;
        glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE)
        {
            std::cerr << "Shader Link Error on Program ID " << programID << "!!!!!!\n";

            printLog();

            return;
        }
        glUseProgram(programID);
    }
    
    void setUniform(const char *uniformName, const glm::vec4 &value) {
        glUniform4fv(glGetUniformLocation(programID, uniformName),
                     1,
                     glm::value_ptr(value));
    }
    
    void setUniform(const char *uniformName, const glm::vec3 &value) {
        glUniform3fv(glGetUniformLocation(programID, uniformName),
                     1,
                     glm::value_ptr(value));
    }
    
    void setUniform(const char *uniformName, const bool &value) {
        glUniform1i(glGetUniformLocation(programID, uniformName),
                    value);
    }
    void setUniform(const char *uniformName, const int &value) {
        glUniform1i(glGetUniformLocation(programID, uniformName),
                    value);
    }
    
    void setUniform(const char *uniformName, const float &value) {
        glUniform1f(glGetUniformLocation(programID, uniformName),
                    value);
    }
    
    void setUniform(const char *uniformName, const glm::mat4 &value, bool transpose = GL_FALSE) {
        glUniformMatrix4fv(glGetUniformLocation(programID, uniformName),
                           1,
                           transpose,
                           glm::value_ptr(value));
    }
    
    void setUniform(const char *uniformName, const glm::mat3 &value, bool transpose = GL_FALSE) {
        glUniformMatrix3fv(glGetUniformLocation(programID, uniformName),
                           1,
                           transpose,
                           glm::value_ptr(value));
    }
    
    void setSubroutine(const char *subroutineName) {
        GLuint subroutine = glGetSubroutineIndex(programID,
                                             GL_FRAGMENT_SHADER,
                                             subroutineName);
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutine);

    }
    
    void use() {
        glUseProgram(programID);
    }
    
    void cleanUp()
    {
        // Delete all programs
        if (programID)
            glDeleteProgram(programID);
        if (vertexShaderID)
            glDeleteShader(vertexShaderID);
        if (geomShaderID)
            glDeleteShader(geomShaderID);
        if (fragShaderID)
            glDeleteShader(fragShaderID);

        // value reset
        programID = vertexShaderID = geomShaderID = fragShaderID = 0;
    }
    ~Program()
    {
        cleanUp();
    }
};
