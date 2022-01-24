#pragma once
#include "scene.h"
#include <unordered_set>

class PhysicsManager {
public:
	void runPhysics(EntityManager& entityManager);
	void runPhysics2(EntityManager& entityManager);
};