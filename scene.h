#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "util.h"
#include "config.h"
#include "collision.h"
#include <unordered_map>
#include <functional>

class Mesh {
public:
	void destroy() {
		if (initialized) {
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ebo);
			vao = 0;
			vbo = 0;
			ebo = 0;
			initialized = false;
		}
	}
	Mesh() {}
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ebo = 0;
	GLuint numIndices = 0;
	bool initialized = false;
	friend bool operator ==(const Mesh& first, const Mesh& second) {
		bool cond = true;
		cond = cond && (first.vao == second.vao);
		cond = cond && (first.vbo == second.vbo);
		cond = cond && (first.ebo == second.ebo);
		cond = cond && (first.numIndices == second.numIndices);
		cond = cond && (first.initialized == second.initialized);
		return cond;
	}
};

class Renderable {
public:
	glm::mat4 model{ 1.0f };
	glm::vec3 color{ 0.5f };
	Mesh mesh;
	bool collisionOccurred = false;
	friend bool operator ==(const Renderable& first, const Renderable& second) {
		bool cond = true;
		cond = cond && (first.model == second.model);
		cond = cond && (first.mesh == second.mesh);
		cond = cond && (first.color == second.color);
		return cond;
	}
};

class GameEntity {
	friend class EntityManager;
	static uint32_t entitiesCreated;

	glm::vec3 pos{ 0.0f };
	glm::vec3 scale{ 1.0f };
	uint32_t index = UINT32_MAX;
	BoundType boundType = BoundType::None;
public:
	uint32_t getIndex() const { return index; }
	BoundType getBoundType() const { return boundType; }
	glm::vec3 getPos() const { return pos; }
	glm::vec3 getScale() const { return scale; }
	friend bool operator ==(const GameEntity& first, const GameEntity& second) {
		return first.index == second.index;
	}
};

bool loadModel(Mesh& mesh, const char* modelPath, unsigned int flags);

using BoundingVolumePair = std::pair<uint32_t, BoundingVolume*>;

class SpatialPartition {
public:
	virtual std::vector<BoundingVolumePair> getNearestObjects(BoundingVolumePair& in) = 0;
	virtual void insert(uint32_t entityIndex, BoundingVolume* boundingVolume) = 0;
	virtual bool remove(uint32_t entityIndex) = 0;
};

class NullPartition : public SpatialPartition {
public:
	std::vector<BoundingVolumePair> getNearestObjects(BoundingVolumePair& in) override;
	void insert(uint32_t entityIndex, BoundingVolume* boundingVolume) override;
	bool remove(uint32_t entityIndex) override;
	std::unordered_map<uint32_t, BoundingVolume*> list;
};

class EntityManager {
public:
	EntityManager(SpatialPartition& spatialPartition) : spatialPartition{ spatialPartition } {}
	~EntityManager() {}
	void setPos(uint32_t index, glm::vec3& pos);
	void setScale(uint32_t index, glm::vec3 scale);
	uint32_t createEntity(Mesh mesh, BoundType boundType, glm::vec3 pos = glm::vec3{ 0.0f }, glm::vec3 scale = glm::vec3{ 1.0f });
	bool destroyEntity(uint32_t index);
	std::unordered_map<uint32_t, GameEntity> gameEntities;
	std::unordered_map<uint32_t, Renderable> renderables;
	std::unordered_map<uint32_t, AABB> aabbs;
	std::unordered_map<uint32_t, BoundingSphere> boundingSpheres;
	SpatialPartition& spatialPartition;
};

void loadScene(EntityManager& entityManager, const char* path, Mesh& cubeMesh, Mesh& sphereMesh);

void storeScene(EntityManager& entityManager, const char* path);