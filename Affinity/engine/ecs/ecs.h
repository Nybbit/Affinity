#pragma once
#include "component.h"
#include "system.h"

#include <plog/Log.h>

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <unordered_set>

// todo: maybe parallelize?
// todo: add ability to "compress" if the majority of entities are unused

class Engine;
class Entity;
class System;

class EntityComponentSystem final
{
public:
	EntityComponentSystem() = default;

	void update(Engine& engine);

	template <typename T>
	void addSystem();

	// for later use, possible multithreading
	void entityLoop(const std::function<void(uint32_t)>& entity_func) const;

	/**
	 * \brief Register a component in the ECS
	 * \tparam T Component type
	 */
	template <typename T>
	void registerComponent();

	/**
	 * \brief Create a new blank entity
	 * \return The created entity
	 */
	Entity createEntity();

	// todo: destroy entity by idx too
	void destroyEntity(Entity entity);

	/**
	 * \brief Check if a component has been registered in the ECS
	 * \tparam T Component type
	 * \return Whether or not the component is registered
	 */
	template <typename T>
	bool isComponentRegistered();

	/**
	 * \brief Get a reference to the component vector for data oriented access
	 * \tparam T Component type
	 * \return Reference to vector for component
	 */
	template <typename T>
	std::vector<std::unique_ptr<Component>>& getComponentVector();

	/**
	 * \brief Get the amount of entities
	 * \return Amount of entities
	 */
	uint32_t getNumEntities() const;

	/**
	 * \brief Get the amount of non-destroyed entities, not really useful
	 * \return Amount of non-destroyed entities
	 */
	uint32_t getNumActiveEntities() const;

	/**
	 * \brief Get the entity at a certain index
	 * \param idx Index
	 * \return The entity at that index, invalid entity if idx is out of bounds
	 */
	Entity getEntityByIdx(uint32_t idx);

	std::vector<Entity> findEntitiesWithTag(const std::string& tag);
private:
	friend class Entity;

	uint32_t m_numEntities = 0;

	std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> m_components;

	std::vector<uint32_t> m_versions;
	std::vector<std::string> m_tags;

	std::unordered_set<uint32_t> m_availableIds;

	std::vector<std::unique_ptr<System>> m_systems;
};

template <typename T>
void EntityComponentSystem::addSystem()
{
	static_assert(std::is_base_of<System, T>::value, "T must have base class of type System");

	m_systems.emplace_back(std::make_unique<T>());
}

template <typename T>
void EntityComponentSystem::registerComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "T must have base class of type Component");

	// Only register component if it doesn't already exist
	if (!isComponentRegistered<T>())
	{
		m_components[typeid(T)] = std::vector<std::unique_ptr<Component>>(m_numEntities);
	}
}

template <typename T>
bool EntityComponentSystem::isComponentRegistered()
{
	const auto result = m_components.find(typeid(T)) != m_components.end();
	return result;
}

template <typename T>
std::vector<std::unique_ptr<Component>>& EntityComponentSystem::getComponentVector()
{
	if (!isComponentRegistered<T>())
	{
		LOG_FATAL << typeid(T).name() << " is not a registered Component";
	}

	return m_components.at(typeid(T));
}
