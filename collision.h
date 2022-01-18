#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum class BoundType {None, AABB, Sphere};

class BoundingVolume {
public:
	virtual ~BoundingVolume(){}
	virtual BoundType getType() = 0;
};

class AABB : public BoundingVolume { //meant for static geometry
public:
	AABB() : center{ 0.0f }, halfExtent{ 0.0f } {}
	BoundType getType() override { return BoundType::AABB; }
	glm::vec3 center;
	glm::vec3 halfExtent;
};

class BoundingSphere : public BoundingVolume {
public:
	BoundingSphere() : center{ 0.0f }, radius{ 0.0f } {}
	BoundType getType() override { return BoundType::Sphere; }
	glm::vec3 center;
	float radius;
};

bool isIntersecting(BoundingVolume& first, BoundingVolume& second);

bool boxIntersection(AABB& first, AABB& second);

bool sphereIntersection(BoundingSphere& first, BoundingSphere& second);

bool boxSphereIntersection(AABB& aabb, BoundingSphere& boundingSphere);