#pragma once
#include "../system.h"

/**
 * \brief Runs scripts
 */
class ScriptSystem final : public System
{
public:
	void update(Engine& engine, EntityComponentSystem& ecs) override;
};
