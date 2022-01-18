#include "util.h"
#include <iostream>

void processKeyboard(GLFWwindow* window, Camera& camera, double deltaTime) {
    static bool spaceKeyPressed = false;
    GLFWCallbackData* callbackData = reinterpret_cast<GLFWCallbackData*>(glfwGetWindowUserPointer(window));
    assert(callbackData);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.processKeyboard(GLFW_KEY_W, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeyboard(GLFW_KEY_S, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeyboard(GLFW_KEY_D, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeyboard(GLFW_KEY_A, deltaTime);
    }
    if (!spaceKeyPressed) spaceKeyPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (spaceKeyPressed && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        callbackData->uiState->showWindow = !callbackData->uiState->showWindow;
        spaceKeyPressed = false;
    }
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMove = true;
    static double lastX = 0, lastY = 0, finalDeltaX = 0, finalDeltaY = 0;

    GLFWCallbackData* callbackData = reinterpret_cast<GLFWCallbackData*>(glfwGetWindowUserPointer(window));
    assert(callbackData);
    if (callbackData->uiState->windowHovered) return;

    double deltaX = 0, deltaY = 0;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (firstMove) {
            lastX = xpos;
            lastY = ypos;
            firstMove = false;
        }
        else {
            deltaX = xpos - lastX;
            deltaY = ypos - lastY;
            finalDeltaX = mouseSmoothingFactor * deltaX + (1.0 - mouseSmoothingFactor) * finalDeltaX;
            finalDeltaY = mouseSmoothingFactor * deltaY + (1.0 - mouseSmoothingFactor) * finalDeltaY;
        }
    }
    else {
        finalDeltaX = 0;
        finalDeltaY = 0;
    }

    lastX = xpos; lastY = ypos;
    callbackData->camera->processMouse(-finalDeltaX, -finalDeltaY, callbackData->deltaTime);
}

const float cubeVertices[24] =
{
    -0.5f, -0.5f, -0.5f, //lower left
    0.5f, -0.5f, -0.5f, //lower right
    0.5f, 0.5f, -0.5f, //upper right
    -0.5f, 0.5f, -0.5f, //upper left
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f
};

//clockwise
const uint32_t cubeIndices[36] = { 0, 1, 3, 3, 1, 2,
                                  1, 5, 2, 2, 5, 6,
                                  5, 4, 6, 6, 4, 7,
                                  4, 0, 7, 7, 0, 3,
                                  3, 2, 7, 7, 2, 6,
                                  4, 5, 0, 0, 5, 1 };