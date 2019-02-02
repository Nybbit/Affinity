#pragma once
#include "../system.h"

class ParticleSystem final : public System
{
public:
	void update(Engine& engine, EntityComponentSystem& ecs) override;
};
