#include "script_system.h"
#include "../components.h"

void ScriptSystem::update(Engine& engine, EntityComponentSystem& ecs)
{
	auto& sVec = ecs.getComponentVector<Script>();

	ecs.entityLoop([&](uint32_t i)
	{
		const auto script = static_cast<Script*>(sVec[i].get());
		if (script)
		{
			script->script(engine, ecs.getEntityByIdx(i));
		}
	});
}
