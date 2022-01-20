#include "physics.h"


void PhysicsManager::runPhysics(EntityManager& entityManager) {
	//collision
	UnorderedPairSet collisionsFound;
	for (auto& entity : entityManager.gameEntities) {
		GameEntity& gameEntity = entity.second;
		std::vector<BoundingVolumePair> nearestObjects;
		BoundingVolume* boundingVolume = nullptr;
		switch (gameEntity.getBoundType()) {
		case BoundType::AABB: 
			{
				AABB& aabb = entityManager.aabbs.at(gameEntity.getIndex());
				boundingVolume = &aabb;
				BoundingVolumePair pair = { gameEntity.getIndex(), boundingVolume };
				nearestObjects = entityManager.spatialPartition.getNearestObjects(pair);
			}
			break;
		case BoundType::Sphere: 
			{
				BoundingSphere& boundingSphere = entityManager.boundingSpheres.at(gameEntity.getIndex());
				boundingVolume = &boundingSphere;
				BoundingVolumePair pair = { gameEntity.getIndex(), boundingVolume };
				nearestObjects = entityManager.spatialPartition.getNearestObjects(pair);
			}
			break;
		}
		for (BoundingVolumePair& boundingVolumePair : nearestObjects) {
			std::pair<uint32_t, uint32_t> collisionPair{ gameEntity.getIndex(), boundingVolumePair.first };
			if (collisionsFound.find(collisionPair) != collisionsFound.end()) {
				continue;
			}
			if (boundingVolume->intersect(boundingVolumePair.second)) {
				auto it = entityManager.renderables.find(gameEntity.getIndex());
				if (it != entityManager.renderables.end()) {
					it->second.collisionOccurred = true;
				}
				it = entityManager.renderables.find(boundingVolumePair.first);
				if (it != entityManager.renderables.end()) {
					it->second.collisionOccurred = true;
				}
				collisionsFound.insert(collisionPair);
			}
			//run physics based on whether collision found or not
		}
	}
}
