#include "sprite_render_system.h"
#include "../components.h"
#include "../../engine.h"

void SpriteRenderSystem::update(Engine& engine, EntityComponentSystem& ecs)
{
	auto& renderer = engine.getRenderer();

	// Component vectors
	auto& tVec = ecs.getComponentVector<Transform>();
	auto& sVec = ecs.getComponentVector<Sprite>();

	// Camera
	const auto camT = engine.getRenderer().getCamera().getComponent<Transform>();
	const auto ortho = engine.getRenderer().getCamera().getComponent<Camera>()->ortho;
	const auto camPos = camT->position;
	const auto camScale = camT->scale;
	const auto camBox = glm::vec4(camPos - ortho / 2.0f / camScale, camPos + ortho / 2.0f / camScale);

	ecs.entityLoop([&](uint32_t i)
	{
		const auto transform = static_cast<Transform*>(tVec[i].get());
		const auto sprite = static_cast<Sprite*>(sVec[i].get());

		// Entity needs transform and sprite component to be drawn
		if (transform && sprite)
		{
			// Check if in camera view
			const auto spriteBox = glm::vec4(transform->position - transform->scale,
				transform->position + transform->scale);

			if (!(spriteBox.z < camBox.x || spriteBox.w < camBox.y || spriteBox.x > camBox.z
				|| spriteBox.y > camBox.w))
			{
				renderer.getSpritebatch().addSprite(*transform, *sprite);
			}
		}
	});
}
