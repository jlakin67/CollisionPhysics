#include "collision.h"
#include "scene.h"

bool boxIntersection(const AABB& first, const AABB& second) {
	glm::vec3 first_min = first.center - first.halfExtent;
	glm::vec3 first_max = first.center + first.halfExtent;
	glm::vec3 second_min = second.center - second.halfExtent;
	glm::vec3 second_max = second.center + second.halfExtent;
	if (first_min.x > second_max.x || second_min.x > first_max.x) return false;
	if (first_min.y > second_max.y || second_min.y > first_max.y) return false;
	if (first_min.z > second_max.z || second_min.z > first_max.z) return false;
	return true;

}

bool sphereIntersection(const BoundingSphere& first, const BoundingSphere& second) {
	glm::vec3 d = first.center - second.center;
	float dist_sq = glm::dot(d, d);
	float radiusSum = first.radius + second.radius;
	return dist_sq <= radiusSum * radiusSum;
}

bool boxSphereIntersection(const AABB& aabb, const BoundingSphere& boundingSphere) {
	glm::vec3 aabb_min = aabb.center - aabb.halfExtent;
	glm::vec3 aabb_max = aabb.center + aabb.halfExtent;
	glm::vec3 e = glm::max(aabb_min - boundingSphere.center, glm::vec3(0.0f));
	e += glm::max(boundingSphere.center - aabb_max, glm::vec3(0.0f));
	float d = glm::dot(e, e);
	return d <= boundingSphere.radius * boundingSphere.radius;
}
