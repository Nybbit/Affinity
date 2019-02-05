#include "physics_system.h"
#include "../components.h"
#include "../../engine.h"

// This code could be improved a little more but it's fine
void PhysicsSystem::update(Engine& engine, EntityComponentSystem& ecs)
{
	auto& tVec = ecs.getComponentVector<Transform>();
	auto& cVec = ecs.getComponentVector<Cannonball>();
	auto& bVec = ecs.getComponentVector<Boat>();

	// Spatial partition into grid based system, todo: move into ecs so it's accessible by other systems
	std::unordered_map<glm::ivec2, std::vector<uint32_t>> cannonballGrid;
	std::unordered_map<glm::ivec2, std::vector<uint32_t>> boatGrid;

	ecs.entityLoop([&](uint32_t i)
	{
		// Requires a transform component to be spatially partitioned
		if (const auto t = static_cast<Transform*>(tVec[i].get()))
		{
			// Convert the position into grid coordinates
			const auto pos = glm::ivec2(floor(t->position / CHUNK_SIZE));

			// If it's a cannonball, put it into the cannonball grid
			if (static_cast<Cannonball*>(cVec[i].get()))
			{
				cannonballGrid[pos].emplace_back(i);
			}
				// Otherwise, if it's a boat, put it into the boat grid
			else if (static_cast<Boat*>(bVec[i].get()))
			{
				boatGrid[pos].emplace_back(i);
			}
		}
	});

	// Extremely basic AABB collision detection, does not prevent tunneling
	// todo: maybe better collision detection, alternatively tunneling is a feature and it simulates missing a shot
	// todo: fix spatial partitioning over chunk edges
	for (auto& cannonballGridSquare : cannonballGrid)
	{
		// Go over each cannonball
		for (auto& cannonballIdx : cannonballGridSquare.second)
		{
			auto transform  = static_cast<Transform*>(tVec[cannonballIdx].get());
			auto cannonball = static_cast<Cannonball*>(cVec[cannonballIdx].get());

			// The transform and cannonball should be guaranteed
			if (!(transform && cannonball))
			{
				continue;
			}

			// Move the cannonball and update velocity
			transform->position += cannonball->direction * cannonball->speed
				* static_cast<float>(engine.getFrameTimer().getDelta());
			cannonball->speed -= static_cast<float>(engine.getFrameTimer().getDelta());

			// Cannonball should "fall into the water" when speed is 0
			if (cannonball->speed <= 0)
			{
				ecs.getEntityByIdx(cannonballIdx).destroy();
				continue;
			}

			// Check collisions with boats in same grid
			for (auto& bIdx : boatGrid[cannonballGridSquare.first])
			{
				auto bTransform = static_cast<Transform*>(tVec[bIdx].get());
				auto boat       = static_cast<Boat*>(bVec[bIdx].get());

				// Again, should be guaranteed already
				if (!(bTransform && boat))
				{
					continue;
				}

				// Get box for boat
				const auto boatBox = glm::vec4(-bTransform->scale, bTransform->scale);

				// Get cannonball position relative to boat
				const auto relativeBallPos = transform->position - bTransform->position;

				// Convert into boat coordinate plane
				const auto c              = cos(-bTransform->rotation);
				const auto s              = sin(-bTransform->rotation);
				const auto rotatedBallBox = glm::vec4(
					glm::vec2(relativeBallPos.x * c - relativeBallPos.y * s,
					          relativeBallPos.x * s + relativeBallPos.y * c) - transform->scale,
					transform->scale);

				// If cannonball doesn't collide with boat, move on
				if ((rotatedBallBox.x < boatBox.x || rotatedBallBox.y < boatBox.y || rotatedBallBox.x >
				     boatBox.x + boatBox.z || rotatedBallBox.y > boatBox.y + boatBox.w))
				{
					continue;
				}

				// Cannonballs shouldn't hit their mothership
				if (cannonball->parentShip.getIdx() == bIdx && cannonball->parentShip.isValid())
				{
					continue;
				}

				// Destroy the cannonball
				ecs.getEntityByIdx(cannonballIdx).destroy();

				// Create the explosion
				auto explosionParticle = ecs.createEntity();
				explosionParticle.setComponent<Transform>(
					Transform(relativeBallPos + bTransform->position, 0, glm::vec2(60, 59)));
				explosionParticle.setComponent<Sprite>(
					Sprite(engine.getRenderer().getSpritesheet().getUv("explosion")));
				explosionParticle.setComponent<Particle>(Particle(60, 60));

				// Damage the ship and destroy it if need be
				if (--boat->health <= 0)
				{
					ecs.getEntityByIdx(bIdx).destroy();
				}
			}
		}
	}
}
