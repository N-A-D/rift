#include "entity.h"

using namespace rift;

rift::Entity::Entity()
	: mgr(nullptr), m_id(0, 0)
{
}

rift::Entity::Entity(EntityManager * em, Entity::ID id)
	: mgr(em), m_id(id)
{
}

Entity::ID rift::Entity::id() const noexcept
{
	return m_id;
}

bool rift::Entity::valid() const noexcept
{
	return mgr && mgr->valid_id(m_id);
}

rift::Entity::operator bool() const noexcept
{
	return valid();
}

bool rift::Entity::pending_delete() const noexcept
{
	assert(valid() && "Cannot check if an invalid entity is waiting for deletion!");
	return mgr->pending_delete(m_id);
}

void rift::Entity::destroy() const noexcept
{
	assert(valid() && "Cannot destroy an invalid entity!");
	mgr->destroy(m_id);
}

rift::ComponentMask rift::Entity::component_mask() const noexcept
{
	assert(valid() && "Cannot get the component mask for an invalid entity!");
	return mgr->component_mask_for(m_id);
}

Entity rift::EntityManager::create_entity() noexcept
{
	if (free_indexes.empty()) {
		auto index = masks.size();
		masks.push_back(0);
		index_versions.push_back(1);
		return Entity(this, Entity::ID(static_cast<std::uint32_t>(index), index_versions.back()));
	}
	else {
		auto index = free_indexes.front();
		free_indexes.pop();
		return Entity(this, Entity::ID(index, index_versions[index]));
	}
}

std::size_t rift::EntityManager::size() const noexcept
{
	return masks.size() - free_indexes.size();
}

std::size_t rift::EntityManager::capacity() const noexcept
{
	return masks.size();
}

std::size_t rift::EntityManager::reusable_entities() const noexcept
{
	return free_indexes.size();
}

std::size_t rift::EntityManager::entities_to_destroy() const noexcept
{
	return ids.size();
}

void rift::EntityManager::update() noexcept
{
	for (auto id : ids) {
		delete_components_for(id);
		delete_any_caches_for(id);
		masks[id.index()].reset();
		index_versions[id.index()]++;
		free_indexes.push(id.index());
	}
	ids.clear();
}

bool rift::EntityManager::valid_id(const Entity::ID & id) const noexcept
{
	return index_versions[id.index()] == id.version();
}

void rift::EntityManager::destroy(const Entity::ID & id) noexcept
{
	if (!ids.exists(id.index())) {
		auto idx(id);
		ids.insert(id.index(), &idx);
	}
}

void rift::EntityManager::delete_components_for(const Entity::ID & id) noexcept
{
	auto mask = component_mask_for(id);
	for (std::size_t i = 0; i < mask.size(); i++) {
		if (mask.test(i)) {
			component_caches[i]->erase(id.index());
		}
	}
}

void rift::EntityManager::delete_any_caches_for(const Entity::ID & id) noexcept
{
	auto mask = component_mask_for(id);
	for (auto& search_cache : entity_caches) {
		if ((mask & search_cache.first) == search_cache.first) {
			search_cache.second.erase(id.index());
		}
	}
}

ComponentMask rift::EntityManager::component_mask_for(const Entity::ID & id) const noexcept
{
	return masks[id.index()];
}

bool rift::EntityManager::pending_delete(const Entity::ID & id) const noexcept
{
	return ids.exists(id.index());
}
