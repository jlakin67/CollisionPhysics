#include "scene.h"
#include <assimp/Importer.hpp>
#include <assimp/config.h>
#include <assimp/scene.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <array>
#include <fstream>

uint32_t GameEntity::entitiesCreated = 0;

void EntityManager::setScale(uint32_t index, glm::vec3 scale) {
	auto gameEntityIt = gameEntities.find(index);
	if (gameEntityIt == gameEntities.end()) return;
	GameEntity& gameEntity = gameEntityIt->second;
	gameEntity.scale = scale;
	bool isSphere = false;
	switch (gameEntity.boundType) {
		case BoundType::Sphere: 
		{
			isSphere = true;
			auto sphereIt = boundingSpheres.find(index);
			if (sphereIt != boundingSpheres.end()) {
				sphereIt->second.radius = scale.x;
			}
		}
		break;
		case BoundType::AABB: 
		{
			auto aabbIt = aabbs.find(index);
			if (aabbIt != aabbs.end()) {
				aabbIt->second.halfExtent = scale / 2.0f;
			}
		}
		break;
	}
	if (isSphere) gameEntity.scale = glm::vec3(scale.x);
	auto renderableIt = renderables.find(index);
	if (renderableIt != renderables.end()) {
		glm::mat4 model{ 1.0f };
		glm::mat4 scaleMatrix = glm::scale(model, scale);
		model = scaleMatrix * model;
		model[3] = glm::vec4(gameEntity.pos, 1.0f);
		renderableIt->second.model = model;
	}
}

void EntityManager::setPos(uint32_t index, glm::vec3& pos) {
	auto gameEntityIt = gameEntities.find(index);
	if (gameEntityIt == gameEntities.end()) return;
	GameEntity& gameEntity = gameEntityIt->second;
	gameEntity.pos = pos;
	switch (gameEntity.boundType) {
		case BoundType::Sphere: 
		{
			auto sphereIt = boundingSpheres.find(index);
			if (sphereIt != boundingSpheres.end()) {
				sphereIt->second.center = pos;
			}
		}
		break;
		case BoundType::AABB:
		{
			auto aabbIt = aabbs.find(index);
			if (aabbIt != aabbs.end()) {
				aabbIt->second.center = pos;
			}
		}
		break;
	}
	auto renderableIt = renderables.find(index);
	if (renderableIt != renderables.end()) {
		glm::mat4 model{ 1.0f };
		glm::mat4 scaleMatrix = glm::scale(model, gameEntity.scale);
		model = scaleMatrix * model;
		model[3] = glm::vec4(pos, 1.0f);
		renderableIt->second.model = model;
	}
}

uint32_t EntityManager::createEntity(Mesh mesh, BoundType boundType, glm::vec3 pos, glm::vec3 scale) {
	GameEntity entity;
	entity.pos = pos;
	entity.scale = scale;
	entity.index = GameEntity::entitiesCreated;
	Renderable renderable;
	renderable.mesh = mesh;
	renderable.model = glm::scale(renderable.model, scale);
	renderable.model[3] = glm::vec4(pos, 1.0f);
	renderables.emplace(entity.index, renderable);
	entity.boundType = boundType;

	switch (boundType) {
		case BoundType::AABB: 
		{
			AABB aabb;
			aabb.center = pos;
			aabb.halfExtent = scale / 2.0f;
			aabbs.emplace(entity.index, aabb);
			spatialPartition.insert(entity.index, &aabbs.at(entity.index));
		}
		break;
		case BoundType::Sphere: 
		{
			BoundingSphere boundingSphere;
			boundingSphere.center = pos;
			boundingSphere.radius = scale.x;
			boundingSpheres.emplace(entity.index, boundingSphere);
			spatialPartition.insert(entity.index, &boundingSpheres.at(entity.index));
		}
		break;
	}
	gameEntities.emplace(entity.index, entity);
	GameEntity::entitiesCreated++;
	return entity.index;
}

bool EntityManager::destroyEntity(uint32_t index) {
	auto gameEntityIt = gameEntities.find(index);
	if (gameEntityIt == gameEntities.end()) return false;
	GameEntity& gameEntity = gameEntityIt->second;
	switch (gameEntity.boundType) {
		case BoundType::AABB: 
		{
			auto aabbIt = aabbs.find(index);
			if (aabbIt != aabbs.end()) {
				aabbs.erase(aabbIt);
			}
			spatialPartition.remove(index);
		}
		break;
		case BoundType::Sphere:
		{
			auto sphereIt = boundingSpheres.find(index);
			if (sphereIt != boundingSpheres.end()) {
				boundingSpheres.erase(sphereIt);
			}
			spatialPartition.remove(index);
		}
		break;
	}
	auto renderableIt = renderables.find(index);
	if (renderableIt != renderables.end()) {
		renderables.erase(renderableIt);
	}
	gameEntities.erase(gameEntityIt);
	return true;
}

bool loadModel(Mesh& mesh, const char* modelPath, unsigned int flags) {
	Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
	const aiScene* scene = importer.ReadFile(modelPath, flags);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
		return false;
	}
	std::vector<glm::vec3> vertexAttributes;
	aiMesh* ai_mesh = scene->mMeshes[0];
	assert(scene->mNumMeshes == 1);
	for (size_t i = 0; i < ai_mesh->mNumVertices; i++) {
		aiVector3D pos = ai_mesh->mVertices[i];
		vertexAttributes.push_back(glm::vec3(pos.x, pos.y, pos.z));
		aiVector3D norm = ai_mesh->mNormals[i];
		vertexAttributes.push_back(glm::vec3(norm.x, norm.y, norm.z));
	}
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexAttributes.size(), vertexAttributes.data(), GL_STATIC_DRAW);
	std::vector<GLuint> indices;
	for (size_t i = 0; i < ai_mesh->mNumFaces; i++) {
		aiFace face = ai_mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
	mesh.numIndices = indices.size();
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*) (sizeof(glm::vec3)));
	glBindVertexArray(0);
	mesh.initialized = true;
	mesh.vao = vao;
	mesh.vbo = vbo;
	mesh.ebo = ebo;
	return true;
}

void loadScene(EntityManager& entityManager, const char* path, Mesh& cubeMesh, Mesh& sphereMesh) {
	using json = nlohmann::json;
	using vec3 = std::array<float, 3>;

	json j;

	std::fstream file;
	file.open(path, std::ios_base::in);
	bool loadFailed = false;
	if (!file.is_open()) {
		std::cerr << "Failed to load scene json at path: " << path << std::endl;
		loadFailed = true;
		return;
	}

	try {
		file >> j;
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		loadFailed = true;
	}
	file.close();
	if (loadFailed) return;

	auto posIt = j.find("pos");
	if (posIt == j.end()) return;
	auto scaleIt = j.find("scale");
	if (scaleIt == j.end()) return;
	auto boundTypeIt = j.find("boundType");
	if (boundTypeIt == j.end()) return;

	std::vector<vec3> positions = *posIt;
	std::vector<vec3> scales = *scaleIt;
	std::vector<uint32_t> boundTypes = *boundTypeIt;

	for (size_t i = 0; i < positions.size(); i++) {
		Mesh mesh;
		BoundType boundType = static_cast<BoundType>(boundTypes.at(i));
		switch (boundType) {
		case BoundType::AABB:
			mesh = cubeMesh;
			break;
		case BoundType::Sphere:
			mesh = sphereMesh;
			break;
		}
		vec3& pos_ = positions.at(i);
		glm::vec3 pos{ pos_[0], pos_[1], pos_[2] };
		vec3& scale_ = scales.at(i);
		glm::vec3 scale{ scale_[0], scale_[1], scale_[2] };
		entityManager.createEntity(mesh, boundType, pos, scale);
	}
}

void storeScene(EntityManager& entityManager, const char* path) {
	using json = nlohmann::json;
	using vec3 = std::array<float, 3>;
	std::vector<vec3> positions;
	std::vector<vec3> scales;
	std::vector<uint32_t> boundType;

	json j;
	for (auto& pair : entityManager.gameEntities) {
		GameEntity& entity = pair.second;
		glm::vec3 pos = entity.getPos();
		positions.push_back({ pos.x, pos.y, pos.z });
		glm::vec3 scale = entity.getScale();
		scales.push_back({ scale.x, scale.y, scale.z });
		boundType.push_back(static_cast<uint32_t>(entity.getBoundType()));
	}
	j["pos"] = positions;
	j["scale"] = scales;
	j["boundType"] = boundType;
	std::fstream file;
	file.open(path, std::ios_base::out);
	file << j;
	file.close();
}

void NullPartition::insert(uint32_t entityIndex, BoundingVolume* boundingVolume) {
	list.emplace(entityIndex, boundingVolume);
}

std::vector<BoundingVolumePair> NullPartition::getNearestObjects(BoundingVolumePair& in) {
	std::vector<BoundingVolumePair> all;
	for (auto& pair : list) {
		if (in.first != pair.first) all.push_back(pair);
	}
	return all;
}

bool NullPartition::remove(uint32_t entityIndex) {
	auto it = list.find(entityIndex);
	if (it == list.end()) return false;
	list.erase(it);
	return true;
}
