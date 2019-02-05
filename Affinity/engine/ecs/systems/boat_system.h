#pragma once
#include "../system.h"

#include <glm/glm.hpp>
#include <utility>

/**
 * \brief Manages boats and boat AIs
 */
class BoatSystem final : public System
{
public:
	void update(Engine& engine, EntityComponentSystem& ecs) override;
private:
	static constexpr float EFFECTIVE_RANGE = 250.0f;

	std::pair<glm::vec2, glm::vec2> findContactsFromTangent(glm::vec2 circle_center, float radius,
		float tangent_slope) const;
	glm::vec2 getTangentVec(glm::vec2 circle_center, glm::vec2 point) const;
	void rotateBoatTowardVec(Engine& engine, float& rotation, glm::vec2 target_vec) const;
	bool isBoatInOptimalBox(Transform& t, Transform& target_t);

	void moveBoatForward(Engine& engine, Transform& t, Boat& b);

	void handleAi(Engine& engine, EntityComponentSystem& ecs, Boat& b, Transform& t, BoatAi& ai, uint32_t entity_index);

	void performWanderAi(Engine& engine, Transform& t, Boat& b);
	void performApproachAi(Engine& engine, Transform& t, Boat& b, BoatAi& ai);
	void performAlignAi(Engine& engine, EntityComponentSystem& ecs, Transform& t, Boat& b, BoatAi& ai, uint32_t entity_index);
	void findTarget(Engine& engine, EntityComponentSystem& ecs, Boat& b, Transform& t, BoatAi& ai);
};
