#include "physics.h"

void PhysicsManager::runPhysics(EntityManager& entityManager) {
	//collision
	std::unordered_set<uint32_t> found;
	for (auto& pair : entityManager.gameEntities) {
		GameEntity& gameEntity = pair.second;
		uint32_t index = gameEntity.getIndex();
		if (found.find(index) != found.end()) continue;
		switch (gameEntity.getBoundType()) {
			case BoundType::AABB: {
				AABB& aabb = entityManager.aabbs.at(index);
				std::vector<BoundingVolumePair> refs = entityManager.spatialPartition.getNearestObjects(aabb);
				runCollisionChecks(entityManager, { index, aabb }, refs, found);
				break;
			}
			case BoundType::Sphere: {
				BoundingSphere& boundingSphere = entityManager.boundingSpheres.at(index);
				std::vector<BoundingVolumePair> refs = entityManager.spatialPartition.getNearestObjects(boundingSphere);
				runCollisionChecks(entityManager, { index, boundingSphere }, refs, found);
				break;
			}
		}
		
	}
}

void PhysicsManager::runCollisionChecks(EntityManager& entityManager, BoundingVolumePair ref, std::vector<BoundingVolumePair>& refs,
	std::unordered_set<uint32_t>& found) {
	for (BoundingVolumePair& boundingVolumePair : refs) {
		if (isIntersecting(ref.second, boundingVolumePair.second)) {
			auto refPair = entityManager.renderables.find(boundingVolumePair.first);
			if (refPair != entityManager.renderables.end()) {
				Renderable& renderable = refPair->second;
				renderable.color = glm::vec3(1.0f, 0.0f, 0.0f);
			}
			refPair = entityManager.renderables.find(boundingVolumePair.first);
			if (refPair != entityManager.renderables.end()) {
				Renderable& renderable = refPair->second;
				renderable.color = glm::vec3(1.0f, 0.0f, 0.0f);
			}
			found.insert(ref.first);
			found.insert(boundingVolumePair.first);
		}
	}
}
