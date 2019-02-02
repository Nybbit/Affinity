#include "physics_system.h"
#include "../components.h"
#include "../../engine.h"

// Sorry there's also some ugly code here, todo: clean up
void PhysicsSystem::update(Engine& engine, EntityComponentSystem& ecs)
{
	auto& tVec = ecs.getComponentVector<Transform>();
	auto& cVec = ecs.getComponentVector<CannonBall>();
	auto& bVec = ecs.getComponentVector<Boat>();
	auto& sVec = ecs.getComponentVector<Sprite>();

	// Spatial partitioning into grid based system
	std::unordered_map<glm::ivec2, std::vector<uint32_t>> cannonBallGrid;
	std::unordered_map<glm::ivec2, std::vector<uint32_t>> boatGrid;

	ecs.entityLoop([&](uint32_t i)
	{
		if (const auto t = static_cast<Transform*>(tVec[i].get()))
		{
			const auto pos = glm::ivec2(floor(t->position / CHUNK_SIZE));

			if (static_cast<CannonBall*>(cVec[i].get()))
			{
				cannonBallGrid[pos].emplace_back(i);
			}
			else if (static_cast<Boat*>(bVec[i].get()))
			{
				boatGrid[pos].emplace_back(i);
			}
		}
	});

	// Extremely basic AABB collision detection, does not prevent tunneling
	// todo: maybe better collision detection, alternatively tunneling is a feature and it simulates missing a shot
	// todo: fix spatial partitioning over chunk edges
	for (auto& gridSquare : cannonBallGrid)
	{
		for (auto& cIdx : gridSquare.second)
		{
			auto transform = static_cast<Transform*>(tVec[cIdx].get());
			auto cannonBall = static_cast<CannonBall*>(cVec[cIdx].get());

			if (transform && cannonBall)
			{
				transform->position += cannonBall->direction * cannonBall->speed
					* static_cast<float>(engine.getFrameTimer().getDelta());
				cannonBall->speed -= static_cast<float>(engine.getFrameTimer().getDelta());

				if (cannonBall->speed <= 0)
				{
					ecs.getEntityByIdx(cIdx).destroy();
					continue;
				}

				// Check collisions with boats
				for (auto& bIdx : boatGrid[gridSquare.first])
				{
					auto bTransform = static_cast<Transform*>(tVec[bIdx].get());
					auto boat = static_cast<Boat*>(bVec[bIdx].get());
					if (bTransform && boat)
					{
						const auto boatBox = glm::vec4(-bTransform->scale, bTransform->scale);

						const auto relativeBallPos = transform->position - bTransform->position;

						const auto c = cos(-bTransform->rotation);
						const auto s = sin(-bTransform->rotation);
						const auto rotatedBallBox = glm::vec4(
							glm::vec2(relativeBallPos.x * c - relativeBallPos.y * s,
								relativeBallPos.x * s + relativeBallPos.y * c) - transform->scale,
							transform->scale);

						if (!(rotatedBallBox.x < boatBox.x || rotatedBallBox.y < boatBox.y || rotatedBallBox.x >
							boatBox.x + boatBox.z || rotatedBallBox.y > boatBox.y + boatBox.w))
						{
							if (cannonBall->parentShip.getIdx() == bIdx && cannonBall->parentShip.isValid())
							{
								continue;
							}

							// collide
							ecs.getEntityByIdx(cIdx).destroy();
							auto explosionParticle = ecs.createEntity();
							explosionParticle.setComponent<Transform>(
								Transform(relativeBallPos + bTransform->position, 0, glm::vec2(60, 59)));
							explosionParticle.setComponent<Sprite>(
								Sprite(engine.getRenderer().getSpritesheet().getUv("explosion")));
							explosionParticle.setComponent<Particle>(Particle(60, 60));

							if (--boat->health <= 0)
							{
								ecs.getEntityByIdx(bIdx).destroy();
							}
						}
					}
				}
			}
		}
	}
}
