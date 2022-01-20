#pragma once
#include <glad/glad.h>
#include "camera.h"

struct UIState {
	bool showWindow = true;
	bool windowHovered = false;
};

void processKeyboard(GLFWwindow* window, Camera& camera, double deltaTime);

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

extern const float cubeVertices[24];

extern const uint32_t cubeIndices[36];

//boost hash combine
template<typename T> 
void hash_combine(size_t& seed, T const& v) {
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}