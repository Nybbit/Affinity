#pragma once
#include "component.h"
#include "entity.h"

#include <glm/glm.hpp>
#include <functional>

class Engine;

struct Transform final : Component
{
	Transform(glm::vec2 position = glm::vec2(0), glm::float32_t rotation = 0, glm::vec2 scale = glm::vec2(1))
		: position(position), rotation(rotation), scale(scale)
	{
	}

	glm::vec2 position;      // 8
	glm::float32_t rotation; // 4
	glm::vec2 scale;         // 8
};

struct Sprite final : Component
{
	Sprite(glm::vec4 uv = glm::vec4(0, 0, 1, 1), glm::float32_t depth = 0, glm::vec4 color = glm::vec4(1))
		: uv(uv), color(color), depth(depth)
	{
	}

	glm::vec4 uv;         // 16
	glm::vec4 color;      // 16
	glm::float32_t depth; // 4
};

struct Camera final : Component
{
	Camera()
	{
	}

	glm::vec2 ortho = glm::vec2(1280, 720);
};

struct Script final : Component
{
	Script(std::function<void(Engine&, Entity)> script = {})
		: script(script)
	{
	}

	std::function<void(Engine&, Entity)> script;
};

struct Boat final : Component
{
	enum class BoatTeamEnum
	{
		NEUTRAL,
		RED,
		GREEN
	};

	Boat(BoatTeamEnum team = BoatTeamEnum::NEUTRAL, glm::float32_t max_speed = 2.0f)
		: team(team), maxSpeed(max_speed)
	{
	}

	BoatTeamEnum team;
	glm::float32_t maxSpeed;

	glm::float32_t cooldownRemaining = 0;
	glm::float32_t speed             = 0.0f;
	int health                       = 3;
};

struct BoatAi final : Component
{
	BoatAi(Entity target = Entity())
		: target(target)
	{
	}

	enum class BehaviorEnum
	{
		WANDER,
		APPROACH,
		ALIGN
	} behavior = BehaviorEnum::WANDER; // 4 bytes

	Entity target; // 12 bytes
};

struct Cannonball final : Component
{
	Cannonball(glm::vec2 direction = glm::vec2(1, 0), glm::float32_t speed = 0.0f, Entity parent_ship = Entity())
		: direction(direction), speed(speed), parentShip(parent_ship)
	{
	}

	glm::vec2 direction;  // 8 bytes
	glm::float32_t speed; // 4 bytes;
	Entity parentShip;    // 12 bytes
};

struct Particle final : Component
{
	Particle(glm::float32_t lifetime = 0.0f, glm::float32_t fadetime = 0.0f)
		: lifetime(lifetime), fadetime(fadetime)
	{
	}

	glm::float32_t lifetime; // 4 bytes
	glm::float32_t fadetime; // 4 bytes
};
