#include "entity.h"

Entity::Entity()
	: m_idx(0), m_version(0)
{
}

uint32_t Entity::getIdx() const
{
	return m_idx;
}

uint32_t Entity::getVersion() const
{
	return m_version;
}

bool Entity::isValid() const
{
	return m_ecs && m_idx < m_ecs->m_numEntities && m_ecs->m_versions[m_idx] == m_version;
}

void Entity::destroy()
{
	m_ecs->destroyEntity(*this);
}

std::string Entity::getTag() const
{
	if (!isValid())
	{
		return "";
	}
	return m_ecs->m_tags[m_idx];
}

void Entity::setTag(const std::string& tag)
{
	if (isValid())
	{
		m_ecs->m_tags[m_idx] = tag;
	}
}

Entity::Entity(EntityComponentSystem* ecs, uint32_t idx, uint32_t version)
	: m_idx(idx), m_version(version), m_ecs(ecs)
{
}