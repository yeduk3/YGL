//
//  camera.hpp
//  GPU_HW2
//
//  Created by 이용규 on 4/9/25.
//

#pragma once

#ifndef PI
#define PI 3.141592653589793f
#endif


#include <glew.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

struct Camera {
    
    glm::vec3 initPosition = glm::vec3(0, 0, 10);
    glm::vec3 look = glm::vec3(0, 0, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
    
    glm::vec3 curPosition = glm::vec3(0, 0, 10);
    
    float theta;
    float phi;
    float fovy = 45.f;
    
    glm::mat4 getRotate() {
        glm::mat4 rotY = glm::rotate(theta, glm::vec3(0, 1, 0));
        glm::mat4 rotX = glm::rotate(phi, glm::vec3(1, 0, 0));
        return rotY * rotX;
    }
    
    void rotatePosition() {
        curPosition = getRotate() * glm::vec4(initPosition, 1);
    }
    
    glm::mat4 lookAt() {
        return glm::lookAt(curPosition, look, up);
    }
    
    glm::mat4 perspective(float aspect, float zNear, float zFar) {
        return glm::perspective(fovy * PI / 180.f, aspect, zNear, zFar);
    }
    
    void glfwSetCallbacks(GLFWwindow* window) {
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);
    }
};

Camera camera;

namespace comp {
float min(const float &a, const float &b) {
    return a > b ? b : a;
}
float max(const float &a, const float &b) {
    return a > b ? a : b;
}
float clamp(const float &value, const float &left, const float &right) {
    return max(left, min(value, right));
}
}

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    static double lastX = 0;
    static double lastY = 0;
    // when left mouse button clicked
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
    {
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        camera.theta -= dx / w * PI; // related with y-axis rotation
        camera.phi -= dy / h * PI;   // related with x-axis rotation
        camera.phi = comp::clamp(camera.phi, -PI / 2 + 0.01f, PI / 2 - 0.01f);
        
        camera.rotatePosition();
    }
    // whenever, save current cursor position as previous one
    lastX = xpos;
    lastY = ypos;
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.fovy -= yoffset / 10;
    camera.fovy = comp::clamp(camera.fovy, 0.01f, 180.f-0.01f);
}
