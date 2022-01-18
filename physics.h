#pragma once
#include "scene.h"
#include <unordered_set>

class PhysicsManager {
public:
	void runPhysics(EntityManager& entityManager);
private:
	void runCollisionChecks(EntityManager& entityManager, BoundingVolumePair ref, std::vector<BoundingVolumePair>& refs, 
		std::unordered_set<uint32_t>& found);
};