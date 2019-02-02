#pragma once
#include "../system.h"

class SpriteRenderSystem final : public System
{
public:
	void update(Engine& engine, EntityComponentSystem& ecs) override;
};
