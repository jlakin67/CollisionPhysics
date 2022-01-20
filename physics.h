#pragma once
#include "scene.h"
#include <unordered_set>

template<typename T>
struct UnorderedPairHash {
	inline std::size_t operator()(const std::pair<T, T>& v) const {
		T smallest = std::min(v.first, v.second);
		T largest = std::max(v.first, v.second);
		std::size_t seed = 0;
		hash_combine(seed, smallest);
		hash_combine(seed, largest);
		return seed;
	}
};

template<typename T>
struct UnorderedPairPred {
	inline bool operator()(const std::pair<T, T>& lhs, const std::pair<T, T>& rhs) const {
		std::pair<T, T> first{ std::min(lhs.first, lhs.second), std::max(lhs.first, lhs.second) };
		std::pair<T, T> second{ std::min(rhs.first, rhs.second), std::max(rhs.first, rhs.second) };
		return (first == second);
	}
};

using UnorderedPairSet = std::unordered_set<std::pair<uint32_t, uint32_t>, UnorderedPairHash<uint32_t>, UnorderedPairPred<uint32_t>>;

class PhysicsManager {
public:
	void runPhysics(EntityManager& entityManager);
};