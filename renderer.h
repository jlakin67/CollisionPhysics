#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "config.h"
#include "util.h"
#include "shader.h"
#include "scene.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

struct InstanceData {
	glm::mat4 model;
	glm::vec4 color;
}; 

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

class Renderer {
public:
	Renderer() {}
	~Renderer() {}
	void startUp(GLFWwindow* window, GLFWCallbackData* callbackData, EntityManager& entityManager);
	void shutDown(EntityManager& entityManager);
	void setView(const glm::mat4& view) { 
		if (useBasicShader) {
			basicShader.useProgram();
			basicShader.setMat4("view", glm::value_ptr(view));
		}
		else {
			posShader.useProgram();
			posShader.setMat4("view", glm::value_ptr(view));
		}	
	}
	void renderFrame(GLFWwindow* window, EntityManager& entityManager, UIState& uiState);
	Mesh cubeMesh;
	Mesh sphereMesh;
	std::vector<uint32_t> entityList;
	size_t currentEntity = 0;
	bool useBasicShader = true;
private:
	friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	Shader basicShader;
	Shader posShader;
	GLuint cubeInstanceDataBuffer = 0;
	GLuint sphereInstanceDataBuffer = 0;
	glm::mat4 projection{ 1.0f };
};