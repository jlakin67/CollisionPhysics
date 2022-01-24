#include "config.h"
#include "util.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "renderer.h"
#include "scene.h"
#include <vector>
#include "physics.h"
#include <chrono>

//NullPartition nullPartition;
SortedAABBList sortedAABBList;

EntityManager entityManager{ sortedAABBList };

Renderer renderer;

PhysicsManager physicsManager;

int main(int argc, char* argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, useDebugContext);
    GLFWwindow* window = glfwCreateWindow(scrWidth, scrHeight, "OpenGL 4.3", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    GLFWCallbackData callbackData;
    glfwSetWindowUserPointer(window, &callbackData);
    Camera camera;
    double deltaTime = 0, previousTime = 0;
    callbackData.camera = &camera;

    renderer.startUp(window, &callbackData, entityManager);
    loadScene(entityManager, "scene.json", renderer.cubeMesh, renderer.sphereMesh);

    glfwSetCursorPosCallback(window, cursorPositionCallback);

    UIState uiState{};
    callbackData.uiState = &uiState;

    glfwSetTime(0);
    while (!glfwWindowShouldClose(window)) {
        
        glm::mat4 view = camera.getView();
        renderer.setView(view);
        //auto start = std::chrono::high_resolution_clock::now();
        physicsManager.runPhysics2(entityManager);
        //auto finish = std::chrono::high_resolution_clock::now();
        //std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";
        renderer.renderFrame(window, entityManager, uiState);

        double currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime; //measures time taken to process and submit work for a frame to be shown
        previousTime = currentTime;
        callbackData.deltaTime = deltaTime;
        glfwPollEvents();
        processKeyboard(window, camera, deltaTime);
    }
    storeScene(entityManager, "scene.json");
    renderer.shutDown(entityManager);
    glfwDestroyWindow(window);
    glfwTerminate();

	return EXIT_SUCCESS;
}