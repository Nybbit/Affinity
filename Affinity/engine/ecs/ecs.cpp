#include "ecs.h"
#include "entity.h"
#include "components.h"
#include "system.h"

void EntityComponentSystem::update(Engine& engine)
{
	for (auto& s : m_systems)
	{
		s->update(engine, *this);
	}
}

void EntityComponentSystem::entityLoop(const std::function<void(uint32_t)>& entity_func) const
{
	for (uint32_t i = 0; i < m_numEntities; ++i)
	{
		entity_func(i);
	}
}

Entity EntityComponentSystem::createEntity()
{
	// If can reuse an id
	if (!m_availableIds.empty())
	{
		const auto id = *m_availableIds.begin();
		m_availableIds.erase(m_availableIds.begin());

		for (auto& c : m_components)
		{
			c.second.at(id) = nullptr;
		}

		return Entity(this, id, m_versions[id]);
	}

	// Otherwise, if no ids are available to be reused
	for (auto& c : m_components)
	{
		c.second.emplace_back(nullptr);
	}
	m_versions.emplace_back(0);
	m_tags.emplace_back("entity");

	return Entity(this, m_numEntities++, 0u);
}

void EntityComponentSystem::destroyEntity(Entity entity)
{
	if (!entity.isValid()) return;

	for (auto& c : m_components)
	{
		c.second.at(entity.getIdx()) = nullptr;
	}

	++m_versions[entity.getIdx()];
	m_availableIds.emplace(entity.getIdx());
	m_tags[entity.getIdx()] = "entity";
}

uint32_t EntityComponentSystem::getNumEntities() const
{
	return m_numEntities;
}

uint32_t EntityComponentSystem::getNumActiveEntities() const
{
	return m_numEntities - static_cast<uint32_t>(m_availableIds.size());
}

Entity EntityComponentSystem::getEntityByIdx(uint32_t idx)
{
	if (idx < m_numEntities)
	{
		return Entity(this, idx, m_versions[idx]);
	}

	return Entity();
}

std::vector<Entity> EntityComponentSystem::findEntitiesWithTag(const std::string& tag)
{
	// todo: replace with faster method, cache in set?

	auto vec = std::vector<Entity>();

	for (auto i = 0u; i < m_numEntities; ++i)
	{
		if (m_tags[i] == tag)
		{
			vec.emplace_back(getEntityByIdx(i));
		}
	}

	return vec;
}
