#include "renderer.h"
#include <assimp/postprocess.h>

void Renderer::startUp(GLFWwindow* window, GLFWCallbackData* callbackData, EntityManager& entityManager) {
    callbackData->renderer = this;
    if (useDebugContext) {
        GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
        else {
            std::cerr << "Failed to create debug context\n";
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	printf("Vendor:          %s\n", glGetString(GL_VENDOR));
	printf("Renderer:        %s\n", glGetString(GL_RENDERER));
	printf("Version OpenGL:  %s\n", glGetString(GL_VERSION));
	printf("Version GLSL:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	projection = glm::perspective(cameraFovY, aspectRatio, zNear, zFar);
	basicShader.loadFile("Shaders/basic.vert", "Shaders/basic.frag");
	basicShader.useProgram();
	basicShader.setMat4("projection", glm::value_ptr(projection));

    posShader.loadFile("Shaders/basic.vert", "Shaders/pos.frag");
    posShader.useProgram();
    posShader.setMat4("projection", glm::value_ptr(projection));
	glGenBuffers(1, &cubeInstanceDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cubeInstanceDataBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxInstances * sizeof(InstanceData), NULL, GL_DYNAMIC_DRAW);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cubeInstanceDataBuffer);
    glGenBuffers(1, &sphereInstanceDataBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereInstanceDataBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxInstances*sizeof(InstanceData), NULL, GL_DYNAMIC_DRAW);

    unsigned int assimpFlags = aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_SortByPType |
        aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices;
    if (!loadModel(cubeMesh, "Assets/cube.obj", assimpFlags)) {
        std::cerr << "Error: Unable to load cube mesh\n";
        exit(EXIT_FAILURE);
    }
    if (!loadModel(sphereMesh, "Assets/low-poly-sphere.obj", assimpFlags)) {
        std::cerr << "Error: Unable to load sphere mesh\n";
        exit(EXIT_FAILURE);
    }

	glClearColor(101.0 / 255.0, 226.0 / 255.0, 235.0 / 255.0, 1.0f);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION_STRING);
    ImGuiIO& Imgui_io = ImGui::GetIO(); (void)Imgui_io;
}

void Renderer::shutDown(EntityManager& entityManager) {
	glFinish();
    for (uint32_t entity : entityList) {
        entityManager.destroyEntity(entity);
    }
	glDeleteBuffers(1, &cubeInstanceDataBuffer);
    glDeleteBuffers(1, &sphereInstanceDataBuffer);
    cubeMesh.destroy();
    sphereMesh.destroy();
}

void Renderer::renderFrame(GLFWwindow* window, EntityManager& entityManager, UIState& uiState) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    std::vector<InstanceData> sphereInstanceData;
    std::vector<InstanceData> cubeInstanceData;
    for (auto& pair : entityManager.renderables) {
        Renderable& renderable = pair.second;
        InstanceData instanceData;
        instanceData.model = renderable.model;
        instanceData.color = glm::vec4(renderable.color, 1.0f);
        if (renderable.mesh == cubeMesh) cubeInstanceData.push_back(instanceData);
        else sphereInstanceData.push_back(instanceData);
    }
    //if (useBasicShader) basicShader.useProgram();
    //else posShader.useProgram();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereInstanceDataBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sphereInstanceData.size() * sizeof(InstanceData), sphereInstanceData.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cubeInstanceDataBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, cubeInstanceData.size() * sizeof(InstanceData), cubeInstanceData.data());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphereInstanceDataBuffer);
    glBindVertexArray(sphereMesh.vao);
    glDrawElementsInstanced(GL_TRIANGLES, sphereMesh.numIndices, GL_UNSIGNED_INT, NULL, sphereInstanceData.size());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cubeInstanceDataBuffer);
    glBindVertexArray(cubeMesh.vao);
    glDrawElementsInstanced(GL_TRIANGLES, cubeMesh.numIndices, GL_UNSIGNED_INT, NULL, cubeInstanceData.size());

    static bool firstFrame = true;

    if (firstFrame) {
        for (auto& gameEntity : entityManager.gameEntities) {
            entityList.push_back(gameEntity.second.getIndex());
        }
        firstFrame = false;
    }
    if (currentEntity >= entityList.size()) currentEntity = entityList.size() - 1;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if (uiState.showWindow) {
        if (!ImGui::Begin("Settings", &uiState.showWindow, 0)) {
            ImGuiIO& io = ImGui::GetIO();
            uiState.windowHovered = io.WantCaptureMouse;
            ImGui::End();
        } else {
            ImGuiIO& io = ImGui::GetIO();
            uiState.windowHovered = io.WantCaptureMouse;
            if (!entityList.empty()) {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Entity index: %d", currentEntity);
                ImGui::SameLine();

                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
                    currentEntity = (currentEntity == 0) ? 0 : (currentEntity - 1);
                }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
                    currentEntity = std::min(currentEntity + 1, entityList.size() - 1);
                }
                ImGui::PopButtonRepeat();
                uint32_t currentEntityIndex = entityList.at(currentEntity);
                GameEntity& entity = entityManager.gameEntities.at(currentEntityIndex);
                glm::vec3 inputPos = entity.getPos();
                ImGui::DragFloat3(" Pos xyz", glm::value_ptr(inputPos), 0.1f, -1000.0f, 1000.0f);
                if (ImGui::IsItemEdited()) {
                    entityManager.setPos(currentEntityIndex, inputPos);
                }
                glm::vec3 inputScale = entity.getScale();
                switch (entity.getBoundType()) {
                case BoundType::Sphere: {
                    float inputRadius = inputScale.x;
                    ImGui::DragFloat(" Radius", &inputRadius, 0.1f, 0.1f, 1000.0f);
                    inputScale = glm::vec3(inputRadius);
                    break;
                }
                case BoundType::AABB:
                    ImGui::DragFloat3(" Scale xyz", glm::value_ptr(inputScale), 0.1f, 0.1f, 1000.0f);
                    break;
                }
                
                if (ImGui::IsItemEdited()) {
                    entityManager.setScale(currentEntityIndex, inputScale);
                }
                if (ImGui::Button("Remove current entity")) {
                    entityManager.destroyEntity(currentEntityIndex);
                    entityList.erase(entityList.begin() + currentEntity);
                    if (currentEntity >= entityList.size() && currentEntity > 0) {
                        currentEntity = entityList.size() - 1;
                    }
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();


            if (ImGui::Button("Add sphere")) {
                entityList.push_back( entityManager.createEntity(sphereMesh, BoundType::Sphere) );
            }
            ImGui::SameLine();
            if (ImGui::Button("Add box")) {
                entityList.push_back(entityManager.createEntity(cubeMesh, BoundType::AABB, glm::vec3(0.5f)));
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Basic shader", &useBasicShader);

            ImGui::End();
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    } else {
        uiState.windowHovered = false;
        ImGui::EndFrame();
    }

	glfwSwapBuffers(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    }

    std::cout << std::endl;
    std::cout << std::endl;
}