#include "particle_system.h"
#include "../components.h"
#include "../../engine.h"

void ParticleSystem::update(Engine& engine, EntityComponentSystem& ecs)
{
	auto& pVec = ecs.getComponentVector<Particle>();
	auto& sVec = ecs.getComponentVector<Sprite>();

	ecs.entityLoop([&](uint32_t i)
	{
		if (const auto particle = static_cast<Particle*>(pVec[i].get()))
		{
			particle->lifetime -= static_cast<float>(engine.getFrameTimer().getDelta());
			if (particle->lifetime <= 0)
			{
				ecs.getEntityByIdx(i).destroy();
			}
			else if (const auto sprite = static_cast<Sprite*>(sVec[i].get()))
			{
				sprite->color.a = glm::clamp(particle->lifetime / particle->fadetime, 0.0f, 1.0f);
			}
		}
	});
}
