#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum class BoundType {None, AABB, Sphere};
class AABB; class BoundingSphere;


bool boxIntersection(const AABB& first, const AABB& second);

bool sphereIntersection(const BoundingSphere& first, const BoundingSphere& second);

bool boxSphereIntersection(const AABB& aabb, const BoundingSphere& boundingSphere);

class BoundingVolume {
public:
	virtual ~BoundingVolume(){}
	virtual BoundType getType() = 0;
	virtual bool intersect(BoundingVolume* boundingVolume) = 0;
	virtual bool intersect(const AABB& aabb) = 0;
	virtual bool intersect(const BoundingSphere& boundingSphere) = 0;
	virtual glm::vec3 getHalfExtent() = 0;
	virtual glm::vec3 getCenter() = 0;
};

class AABB : public BoundingVolume { //meant for static geometry
public:
	AABB() : center{ 0.0f }, halfExtent{ 0.0f } {}
	BoundType getType() override { return BoundType::AABB; }
	bool intersect(BoundingVolume* boundingVolume) override {
		return boundingVolume->intersect(*this);
	}
	bool intersect(const AABB& aabb) override {
		return boxIntersection(*this, aabb);
	}
	bool intersect(const BoundingSphere& boundingSphere) override {
		return boxSphereIntersection(*this, boundingSphere);
	}
	glm::vec3 getHalfExtent() override {
		return halfExtent;
	}
	glm::vec3 getCenter() override {
		return center;
	}
	glm::vec3 center;
	glm::vec3 halfExtent;
};

class BoundingSphere : public BoundingVolume {
public:
	BoundingSphere() : center{ 0.0f }, radius{ 0.0f } {}
	BoundType getType() override { return BoundType::Sphere; }
	glm::vec3 center;
	float radius;
	bool intersect(BoundingVolume* boundingVolume) override {
		return boundingVolume->intersect(*this);
	}
	bool intersect(const AABB& aabb) override {
		return boxSphereIntersection(aabb, *this);
	}
	bool intersect(const BoundingSphere& boundingSphere) override {
		return sphereIntersection(*this, boundingSphere);
	}
	glm::vec3 getHalfExtent() override {
		return glm::vec3(radius);
	}
	glm::vec3 getCenter() override {
		return center;
	}
};