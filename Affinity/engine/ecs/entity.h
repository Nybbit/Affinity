#pragma once
#include "ecs.h"

#include <plog/Log.h>

#include <cstdint>
#include <string>

/**
 * \brief A helper class that makes it easy to work with individual entities
 */
class Entity final
{
public:
	// Default invalid entity
	Entity();

	uint32_t getIdx() const;
	uint32_t getVersion() const;

	bool isValid() const;

	void destroy();

	/**
	 * \brief Get a component from the entity
	 * \tparam T Component type
	 * \return Pointer to component if available and entity is valid, otherwise nullptr
	 */
	template <typename T>
	T* getComponent();

	/**
	 * \brief Set the entity's component. Overwrites old component if already set, otherwise adds the component to the entity
	 * \tparam T Component type
	 * \param component Value to set component to
	 */
	template <typename T>
	void setComponent(T component = T());

	/**
	 * \brief Remove the entity's component
	 * \tparam T Component type
	 */
	template <typename T>
	void removeComponent();

	std::string getTag() const;

	void setTag(const std::string& tag);

	friend std::ostream& operator<<(std::ostream& os, const Entity& obj)
	{
		return os
			<< "Entity<" << obj.m_idx
			<< ", " << obj.m_version
			<< ", " << (obj.isValid() ? "valid" : "invalid")
			<< ">";
	}
private:
	friend class EntityComponentSystem;

	Entity(EntityComponentSystem* ecs, uint32_t idx, uint32_t version);

	uint32_t m_idx;
	uint32_t m_version;
	EntityComponentSystem* m_ecs = nullptr;
};

template <typename T>
T* Entity::getComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "T must have based class of type Component");

	if (!m_ecs->isComponentRegistered<T>())
	{
		LOG_WARNING << typeid(T).name() << " is not a registered component";
		return nullptr;
	}

	if (isValid())
	{
		return static_cast<T*>(m_ecs->m_components.at(typeid(T)).at(m_idx).get());
	}

	return nullptr;
}

template <typename T>
void Entity::setComponent(T component)
{
	static_assert(std::is_base_of<Component, T>::value, "T must have based class of type Component");
	if (!isValid()) return;

	if (!m_ecs->isComponentRegistered<T>())
	{
		LOG_WARNING << typeid(T).name() << " is not a registered component";
		return;
	}

	if (const auto& c = m_ecs->m_components.at(typeid(T)).at(m_idx))
	{
		static_cast<T&>(*c) = component;
	}
	else
	{
		m_ecs->m_components.at(typeid(T)).at(m_idx) = std::make_unique<T>(component);
	}
}

template <typename T>
void Entity::removeComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "T must have based class of type Component");
	if (!isValid()) return;

	if (!m_ecs->isComponentRegistered<T>())
	{
		LOG_WARNING << typeid(T).name() << " is not a registered component";
		return;
	}

	m_ecs->m_components.at(typeid(T)).at(m_idx) = nullptr;
}
