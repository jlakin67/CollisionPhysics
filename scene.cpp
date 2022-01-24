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
	spatialPartition.update(index);
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
	spatialPartition.update(index);
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
	assert(boundingVolume);
	list.emplace(entityIndex, boundingVolume);
}

std::vector<BoundingVolumePair>& NullPartition::getNearestObjects(BoundingVolumePair& in) {
	static std::vector<BoundingVolumePair> all;
	all.clear();
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

bool NullPartition::update(uint32_t entityIndex) {
	return true;
}

void SortedAABBList::insert(uint32_t entityIndex, BoundingVolume* boundingVolume) {
	assert(boundingVolume);
	Node minNode{};
	minNode.isMax = false;
	minNode.index = entityIndex;
	minNode.boundingVolume = boundingVolume;
	nodes.emplace(std::pair<uint32_t, bool>(entityIndex, false), minNode);
	Node* minNodeRef = &nodes.find(std::pair<uint32_t, bool>(entityIndex, false))->second;
	insertHelper(minNodeRef);
	Node maxNode{};
	maxNode.isMax = true;
	maxNode.index = entityIndex;
	maxNode.boundingVolume = boundingVolume;
	nodes.emplace(std::pair<uint32_t, bool>(entityIndex, true), maxNode);
	Node* maxNodeRef = &nodes.find(std::pair<uint32_t, bool>(entityIndex, true))->second;
	insertHelper(maxNodeRef);
}

void SortedAABBList::insertHelper(Node* node) {
	for (int i = 0; i < 3; i++) {
		if (!head[i]) {
			head[i] = node;
		}
		else {
			BoundingVolume* boundingVolume = node->boundingVolume;
			float value;
			if (node->isMax) {
				value = boundingVolume->getCenter()[i] + boundingVolume->getHalfExtent()[i];
			}
			else {
				value = boundingVolume->getCenter()[i] - boundingVolume->getHalfExtent()[i];
			}
			for (Node* p = head[i]; p != nullptr; p = p->next[i]) {
				BoundingVolume* compBoundingVolume = p->boundingVolume;
				float compValue;
				if (p->isMax) {
					compValue = compBoundingVolume->getCenter()[i] + compBoundingVolume->getHalfExtent()[i];
				}
				else {
					compValue = compBoundingVolume->getCenter()[i] - compBoundingVolume->getHalfExtent()[i];
				}
				if (value <= compValue) {
					if (p == head[i]) {
						node->next[i] = p;
						p->prev[i] = node;
						head[i] = node;
						node->prev[i] = nullptr;
						break;
					}
					else {
						Node* p_prev = p->prev[i];
						p_prev->next[i] = node;
						p->prev[i] = node;
						node->prev[i] = p_prev;
						node->next[i] = p;
						break;
					}
				}
				if (!p->next[i]) {
					p->next[i] = node;
					node->prev[i] = p;
					node->next[i] = nullptr;
					break;
				}
			}
		}
	}
}

bool SortedAABBList::remove(uint32_t entityIndex) {
	auto min_it = nodes.find(std::pair<uint32_t, bool>(entityIndex, false));
	if (min_it == nodes.end()) return false;
	auto max_it = nodes.find(std::pair<uint32_t, bool>(entityIndex, true));
	if (max_it == nodes.end()) return false;
	Node& min_node = min_it->second;
	for (int i = 0; i < 3; i++) {
		if (head[i] == &min_node) {
			if(min_node.next[i]) min_node.next[i]->prev[i] = nullptr;
			head[i] = min_node.next[i];
		}
		else {
			min_node.prev[i]->next[i] = min_node.next[i];
			if (min_node.next[i]) min_node.next[i]->prev[i] = min_node.prev[i];
		}
	}
	nodes.erase(min_it);
	Node& max_node = max_it->second;
	for (int i = 0; i < 3; i++) {
		if (head[i] == &max_node) {
			if (max_node.next[i]) max_node.next[i]->prev[i] = nullptr;
			head[i] = max_node.next[i];
		}
		else {
			max_node.prev[i]->next[i] = max_node.next[i];
			if (max_node.next[i]) max_node.next[i]->prev[i] = max_node.prev[i];
		}
	}
	nodes.erase(max_it);
	return true;
}

bool SortedAABBList::update(uint32_t entityIndex) {
	auto min_it = nodes.find(std::pair<uint32_t, bool>(entityIndex, false));
	if (min_it == nodes.end()) return false;
	auto max_it = nodes.find(std::pair<uint32_t, bool>(entityIndex, true));
	if (max_it == nodes.end()) return false;
	updateHelper(min_it);
	updateHelper(max_it);
	return true;
}

void SortedAABBList::updateHelper(decltype(nodes)::iterator it) {
	Node& node = it->second;
	for (int i = 0; i < 3; i++) {
		float value;
		if (node.isMax) {
			value = node.boundingVolume->getCenter()[i] + node.boundingVolume->getHalfExtent()[i];
		}
		else {
			value = node.boundingVolume->getCenter()[i] - node.boundingVolume->getHalfExtent()[i];
		}
		Node* prev_node = node.prev[i];
		if (prev_node) {
			float prev_value;
			if (prev_node->isMax) {
				prev_value = prev_node->boundingVolume->getCenter()[i] + prev_node->boundingVolume->getHalfExtent()[i];
			}
			else {
				prev_value = prev_node->boundingVolume->getCenter()[i] - prev_node->boundingVolume->getHalfExtent()[i];
			}
			while (prev_value > value && prev_node != nullptr) {
				swap(i, prev_node, &node);
				prev_node = prev_node->prev[i];
				if (prev_node->isMax) {
					prev_value = prev_node->boundingVolume->getCenter()[i] + prev_node->boundingVolume->getHalfExtent()[i];
				}
				else {
					prev_value = prev_node->boundingVolume->getCenter()[i] - prev_node->boundingVolume->getHalfExtent()[i];
				}
			}
		}
		Node* next_node = node.next[i];
		if (next_node) {
			float next_value;
			if (next_node->isMax) {
				next_value = next_node->boundingVolume->getCenter()[i] + next_node->boundingVolume->getHalfExtent()[i];
			}
			else {
				next_value = next_node->boundingVolume->getCenter()[i] - next_node->boundingVolume->getHalfExtent()[i];
			}
			while (next_value < value && next_node != nullptr) {
				swap(i, &node, next_node);
				next_node = next_node->next[i];
				if (next_node->isMax) {
					next_value = next_node->boundingVolume->getCenter()[i] + next_node->boundingVolume->getHalfExtent()[i];
				}
				else {
					next_value = next_node->boundingVolume->getCenter()[i] - next_node->boundingVolume->getHalfExtent()[i];
				}
			}
		}
	}
}


void SortedAABBList::swap(int i, Node* first, Node* second) {
	assert(first && second && (i < 3) && (i >= 0));
	if (first == second) return;
	if (head[i] == first) head[i] = second;
	else if (head[i] == second) head[i] = first;
	Node* first_prev = first->prev[i];
	Node* first_next = first->next[i];
	Node* second_prev = second->prev[i];
	Node* second_next = second->next[i];
	if (first_next == second) {
		if (first_prev) first_prev->next[i] = second;
		second->prev[i] = first_prev;
		second->next[i] = first;
		if (second_next) second_next->prev[i] = first;
		first->prev[i] = second;
		first->next[i] = second_next;
	} else if (first_prev == second) {
		if (second_prev) second_prev->next[i] = first;
		if (first_next) first_next->prev[i] = second;
		first->prev[i] = second_prev;
		first->next[i] = second;
		second->prev[i] = first;
		second->next[i] = first_next;
	} else {
		if (first_prev) first_prev->next[i] = second;
		second->prev[i] = first_prev;
		second->next[i] = first_next;
		if (first_next) first_next->prev[i] = second;
		if (second_prev) second_prev->next[i] = first;
		first->prev[i] = second_prev;
		first->next[i] = second_next;
		if (second_next) second_next->prev[i] = first;
	}
}

std::vector<BoundingVolumePair>& SortedAABBList::getNearestObjects(BoundingVolumePair& in) {
	//debugPrint();
	static std::vector<BoundingVolumePair> nearestObjects; //called every frame anyway so made static to avoid reallocating
	nearestObjects.clear();
	uint32_t index = in.first;
	BoundingVolume* boundingVolume = in.second;
	assert(boundingVolume);
	auto min_it = nodes.find(std::pair<uint32_t, bool>(index, false));
	if (min_it == nodes.end()) return nearestObjects;
	auto max_it = nodes.find(std::pair<uint32_t, bool>(index, true));
	if (max_it == nodes.end()) return nearestObjects;
	Node& min = min_it->second;
	Node& max = max_it->second;
	assert(min.boundingVolume == max.boundingVolume);
	glm::vec3 min_value = min.boundingVolume->getCenter() - min.boundingVolume->getHalfExtent();
	glm::vec3 max_value = max.boundingVolume->getCenter() + max.boundingVolume->getHalfExtent();
	std::unordered_map<uint32_t, uint32_t> potentialNearest;
	for (int i = 0; i < 3; i++) {
		assert(min_value[i] <= max_value[i]);
		for (Node* p = min.next[i]; p != &max; p = p->next[i]) {
			assert(p);
			if (p->boundingVolume != boundingVolume) {
				auto it = potentialNearest.find(p->index);
				if (it == potentialNearest.end()) {
					potentialNearest.emplace(p->index, 1);
				}
				else {
					it->second++;
				}
			}
		}
	}
	for (auto pair : potentialNearest) {
		if (pair.second >= 3) {
			Node& node = nodes.at(std::pair<uint32_t,bool>(pair.first, true));
			nearestObjects.push_back(BoundingVolumePair{ pair.first, node.boundingVolume });
		}
	}
	return nearestObjects;
}

UnorderedPairMap& SortedAABBList::getCollisionPairs() {
	static UnorderedPairMap collisionPairs;
	static std::unordered_map<uint32_t, BoundingVolume*> collisionList;
	collisionPairs.clear();
	for (int i = 0; i < 3; i++) {
		collisionList.clear();
		for (Node* p = head[i]; p != nullptr; p = p->next[i]) {
			if (!p->isMax) collisionList.emplace(p->index, p->boundingVolume);
			else {
				for (auto& pair : collisionList) {
					if (pair.first == p->index) continue;
					collisionPairs.emplace(std::pair<uint32_t, uint32_t>(p->index, pair.first), std::pair<BoundingVolume*, BoundingVolume*>(p->boundingVolume, pair.second));
					collisionList.erase(p->index);
				}
			}
		}
	}
	return collisionPairs;
}

void SortedAABBList::debugSizeCheck() {
	for (int i = 0; i < 3; i++) {
		uint32_t size = 0;
		for (Node* p = head[i]; p != nullptr; p = p->next[i]) {
			size++;
		}
		assert(size == nodes.size());
	}
}

void SortedAABBList::debugPrint() {
	for (int i = 0; i < 3; i++) {
		std::cout << i << ": ";
		for (Node* p = head[i]; p != nullptr; p = p->next[i]) {
			BoundingVolume* boundingVolume = p->boundingVolume;
			float value;
			if (p->isMax) {
				value = boundingVolume->getCenter()[i] + boundingVolume->getHalfExtent()[i];
			} else {
				value = boundingVolume->getCenter()[i] - boundingVolume->getHalfExtent()[i];
			}
			if (p->next[i]) {
				std::cout << "index: " << p->index << " value: " << value << ", ";
			} else {
				std::cout << "index: " << p->index << " value: " << value;
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}