#include "game.h"
#include "engine/ecs/components.h"
#include "engine/config.h"

int Game::run()
{
	if (!m_engine.init())
	{
		return 1;
	}

	init();

	return m_engine.run();
}

void Game::init()
{
	auto& ecs = m_engine.getEntityComponentSystem();
	auto& renderer = m_engine.getRenderer();
	m_engine.getWindow().setVsync(false);

	// Create player
	auto player = ecs.createEntity();
	player.setTag("player");
	player.setComponent<Transform>(Transform(glm::vec2(5000), 0, config::BOAT_SIZE));
	player.setComponent<Boat>();
	player.setComponent<Sprite>(Sprite(renderer.getSpritesheet().getUv("ship_0")));
	player.setComponent<Script>(Script(
		[](Engine& engine, Entity entity)
	{
		auto t = entity.getComponent<Transform>();

		const auto playerSpeed = entity.getComponent<Boat>()->maxSpeed;
		const auto playerT = entity.getComponent<Transform>();
		auto& input = engine.getInput();
		if (input.isKeyDown(config::TURN_RIGHT))
		{
			playerT->rotation -= static_cast<float>(engine.getFrameTimer().getDelta()) * config::ROTATION_SPEED;
		}
		if (input.isKeyDown(config::TURN_LEFT))
		{
			t->rotation += static_cast<float>(engine.getFrameTimer().getDelta()) * config::ROTATION_SPEED;
		}

		if (input.isKeyDown(config::MOVE_FORWARD))
		{
			t->position += glm::normalize(glm::vec2(cos(t->rotation), sin(t->rotation))) * playerSpeed
				* static_cast<float>(engine.getFrameTimer().getDelta());
		}
	}
	));

	// Create camera
	auto camera = ecs.createEntity();
	camera.setComponent<Transform>();
	camera.setComponent<Camera>();
	camera.setComponent<Script>(Script(
		[](Engine& engine, Entity entity)
	{
		auto& input = engine.getInput();
		auto t = entity.getComponent<Transform>();
		if (input.isKeyDown(config::ZOOM_IN))
		{
			t->scale += static_cast<float>(engine.getFrameTimer().getDelta()) * 0.01f;
		}
		if (input.isKeyDown(config::ZOOM_OUT))
		{
			t->scale -= static_cast<float>(engine.getFrameTimer().getDelta()) * 0.01f;
		}

		t->scale = glm::clamp(t->scale, 0.01f, 1.5f);

		// Elastic follow
		glm::vec2 targetPosition;
		const auto players = engine.getEntityComponentSystem().findEntitiesWithTag("player");
		if (!players.empty())
		{
			auto player = players[0];
			targetPosition = player.getComponent<Transform>()->position;
		}
		else
		{
			const auto boats = engine.getEntityComponentSystem().findEntitiesWithTag("boat");
			if (!boats.empty())
			{
				auto boat = boats[0];
				targetPosition = boat.getComponent<Transform>()->position;
			}
		}

		const auto stiffness = 1.0f / 10.0f;
		const auto force = -stiffness * (t->position - targetPosition);
		t->position += force * static_cast<float>(engine.getFrameTimer().getDelta());
	}
	));
	renderer.setCamera(camera);

	// Create boats
	constexpr auto numEntities = 10000;
	const auto sideLength = static_cast<int>(ceil(numEntities / 5.0f));

	for (auto i = 0; i < numEntities; ++i)
	{
		const auto team = static_cast<Boat::BoatTeamEnum>(i % 2 + 1);

		auto e = ecs.createEntity();
		e.setTag("boat");
		e.setComponent<Transform>(Transform(glm::vec2(rand() % sideLength, rand() % sideLength) * 10.0f, 0,
			config::BOAT_SIZE));
		e.setComponent<Sprite>(
			Sprite(renderer.getSpritesheet().getUv("ship_" + std::to_string(static_cast<int>(team)))));
		e.setComponent<Boat>(Boat(team));
		e.setComponent<BoatAi>();
	}
}
