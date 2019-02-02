#pragma once
class Engine;
class EntityComponentSystem;

struct Transform;
struct Boat;
struct BoatAi;

class System
{
public:
	System() = default;
	virtual ~System() = default;
	System(const System& other) = default;
	System(System&& other) noexcept = default;
	System& operator=(const System& other) = default;
	System& operator=(System&& other) noexcept = default;

	virtual void update(Engine& engine, EntityComponentSystem& ecs) = 0;
};
