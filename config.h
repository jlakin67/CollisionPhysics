#pragma once
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cassert>

#ifdef NDEBUG
const bool useDebugContext = false;
#else
const bool useDebugContext = true;
#endif // NDEBUG

class Camera;
class Renderer;
struct UIState;

struct GLFWCallbackData {
	double deltaTime = 0;
	Camera* camera = nullptr;
	Renderer* renderer = nullptr;
	UIState* uiState = nullptr;
};

const uint32_t scrWidth = 1600;
const uint32_t scrHeight = 900;
const float zNear = 0.1f;
const float zFar = 100.0f;
const float aspectRatio = (float)scrWidth / (float)scrHeight;
const float cameraFovY = 45.0f;
const double cameraSpeed = 8.0f;
const double mouseSensitivity = 0.8f;
const double mouseSmoothingFactor = 0.5;
const uint32_t maxInstances = 1000; //80 MB size storage buffer
const uint32_t maxEntities = 1000;

constexpr const char* GLSL_VERSION_STRING = "#version 430 core";
