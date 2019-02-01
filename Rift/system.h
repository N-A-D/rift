#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include "entity.h"

namespace rift {

	// The BaseSystem class
	// Provides the interface in which systems implement their logic.
	// Note:
	// - This class should not be subclassed directly as systems need to be registered. 
	//   See the System class below. 
	class BaseSystem {
	public:
		virtual ~BaseSystem() = default;
		virtual void update(EntityManager& em, double dt) = 0;
	protected:
		using Family = std::size_t;
		// Used internally for generating system type ids.
		inline static Family family_counter = 0;
	};

	class SystemManager;

	// The System class
	// Classes that are meant to be systems must inherit from this class for registration as a system.
	// Example:
	// class MovementSystem : public System<MovementSystem> {}
	template <class Derived>
	class System : public BaseSystem {
	public:
		virtual ~System() = default;
	private:
		friend class SystemManager;
		// Used internally for system registration.
		static Family family() noexcept {
			static Family system_family = family_counter++;
			return system_family;
		}
	};
	
	// The SystemManager class
	// Manages a single instance of different system types.
	class SystemManager final {
	public:

		// Creates a new System manager.
		SystemManager(rift::EntityManager& em) noexcept : entity_manager(em) {}

		// Adds a new managed system.
		// Note: 
		// - Asserts the system type is not already managed.
		// Example:
		// EntityManager em;
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		template <class S, class... Args>
		void add(Args&& ...args) noexcept;

		// Removes a managed system if any.
		// Note:
		// - Asserts the system type is managed.
		// Example:
		// EntityManager em;
		// SystemManager sm(em);
		// sm.remove<MovementSystem>();
		template <class S>
		void remove() noexcept;

		// Checks if the manager has a managed system.
		// Example:
		// EntityManager em;
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// bool has = sm.has<MovementSystem>();
		template <class S>
		bool has() const noexcept;

		// Retrieves the system of type S if any.
		// Note:
		// - Asserts the system type is managed.
		// Example:
		// EntityManager em;
		// SystemManager sm(em);
		// sm.add<MovementSystem>();
		// if (sm.get<MovementSystem>()) {...}
		template <class S>
		std::shared_ptr<S> get() const noexcept;

		// Updates all systems.
		void update_all(double dt) const noexcept;
		
		// Updates a list of systems.
		// Note:
		// - Asserts that each system type is managed.
		// - The systems are updated in the same order they're specified.
		// Example:
		// EntityManager em;
		// SystemManager sm(em);
		// sm.update<Movement, Collision>(dt);
		template <class First, class... Rest>
		void update(double dt) const noexcept;
		
	private:

		// Fetches a generic pointer to a specific system.
		template <class S>
		std::shared_ptr<BaseSystem> fetch_system() const noexcept;

		// The entity manager systems query.
		rift::EntityManager& entity_manager;

		// Collection of systems managed by the manager.
		std::vector<std::shared_ptr<BaseSystem>> systems;
	};

} // namespace rift
#include "system.inl"
