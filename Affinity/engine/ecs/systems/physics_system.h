#pragma once
#include "../system.h"

/**
 * \brief Manages physics
 */
class PhysicsSystem final : public System
{
public:
	void update(Engine& engine, EntityComponentSystem& ecs) override;
private:
	static constexpr float CHUNK_SIZE = 1000.0f;
};
