#pragma once

#include <vector>
#include <bitset>
#include <set>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include "../logger/Logger.h"

const unsigned int MAX_COMPONENTS = 32;

//////////////////////////////////////
// Signature
/////////////////////////////////////
// Bitset (1s and 0s) used to keep track of which components an entity has
/////////////////////////////////////
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
	protected:
		static int nextId;
};

// Used to assign a unique id to a component type
template <typename T>
class Component : public IComponent {
	// Returns unique id of component<T>
	public:
		static int GetId() {
			static auto id = nextId++;
			return id;
		}
};

class Entity {
	private:
		int id;

	public:
		Entity(int id) : id(id) {};
		Entity(const Entity& entity) = default;
		int GetId() const;
		
		Entity& operator =(const Entity& other) = default;
		bool operator ==(const Entity& other) const { return id == other.id; }
		bool operator !=(const Entity& other) const { return id != other.id; }
		bool operator >(const Entity& other) const { return id > other.id; }
		bool operator <(const Entity& other) const { return id < other.id; }

		template <typename T, typename ...TArgs> void AddComponent(TArgs&& ...args);
		template <typename T> void RemoveComponent();
		template <typename T> bool HasComponent() const;
		template <typename T> T& GetComponent() const;

		// Hold a pointer to the entity's owner registry
		class Registry* registry = nullptr;
};

//////////////////////////////
// SYSTEM ARCHITECTURE		//
//////////////////////////////

class System {
	private:
		// Which components an entity must have for the system to consider the entity
		Signature componentSignature;

		// List of all entities the system is interested in
		std::vector<Entity> entities;

	public:
		System() = default;
		~System() = default;

		void AddEntity(Entity entity);
		void RemoveEntity(Entity entity);
		std::vector<Entity> GetEntities() const;
		const Signature& GetComponentSignature() const;

		// Define the component type T that entites must have to be considered by the system
		template <typename T> void RequireComponent();
};

/////////////////////////////////////////////////////
// Pool
////// Pool is a vector (contiguous data) of objects of type T
/////////////////////////////////////////////////////
class IPool {
	public:
		virtual ~IPool() {}
};

template <typename T>
class Pool : public IPool {
	private:
		std::vector<T> data;

	public:
		Pool(int size = 100) {
			data.resize(size);
		}

		virtual ~Pool() = default;

		bool isEmpty() const {
			return data.empty();
		}

		int GetSize() const {
			return data.size();
		}

		void Resize(int n) {
			data.resize(n);
		}

		void Clear() {
			data.clear();
		}

		void Add(T object) {
			data.push_back(object);
		}

		void Set(int index, T object) {
			data[index] = object;
		}

		T& Get(int index) {
			return static_cast<T&>(data[index]);
		}

		T& operator[](unsigned int index) {
			return data[index];
		}
};

/////////////////////////////////////////////////////
// Registry
////////////////////////////////////////////////////
// Registry manages the creation and destruction of entities, add systems and components
////////////////////////////////////////////////////

class Registry {
	private:
		// Keep track of how many entities were added to the scene
		int numEntities = 0;

		std::set<Entity> entitiesToBeAdded;
		std::set<Entity> entitiesToBeKilled;

		// Vector of component pools, each pool contains all the data for certain component type
		// Vector index = component type id
		// Pool index = entity id
		std::vector<std::shared_ptr<IPool>> componentPools;

		// Vector of component signatures
		// This lets us know which components are turned on for an entity
		std::vector<Signature> entityComponentSignatures;

		// Map of active systems [index = system type id]
		std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

	public:
		Registry() = default;
		// The registry Update() processes entities waiting to be added/removed
		void Update();

		// Entity management
		Entity CreateEntity();

		// Component management
		template <typename T, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
		template <typename T> void RemoveComponent(Entity entity);
		template <typename T> bool HasComponent(Entity entity) const;
		template <typename T> T& GetComponent(Entity entity) const;
		
		// System management
		template <typename T, typename ...TArgs> void AddSystem(TArgs&& ...args);
		template <typename T> void RemoveSystem();
		template <typename T> bool HasSystem() const;
		template <typename T> T& GetSystem() const;

		// Check component signature of entity, add entity to systems that are interested in it
		void AddEntityToSystems(Entity entity);

};

template <typename T, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
	std::shared_ptr<T> newSystem = std::make_shared<T>(std::forward<TArgs>(args)...);
	systems.insert(std::make_pair(std::type_index(typeid(T)), newSystem));
}

template <typename T>
void Registry::RemoveSystem() {
	auto system = systems.find(std::type_index(typeid(T)));
	systems.erase(system);
}

template <typename T>
bool Registry::HasSystem() const {
	return systems.find(std::type_index(typeid(T))) != systems.end();
}

template <typename T>
T& Registry::GetSystem() const {
	auto system = systems.find(std::type_index(typeid(T)));
	return *(std::static_pointer_cast<T>(system->second));
}

template <typename T>
void System::RequireComponent() {
	const auto componentId = Component<T>::GetId();
	componentSignature.set(componentId);
}

template <typename T, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
	const auto componentId = Component<T>::GetId();
	const auto entityId = entity.GetId();

	// If component id is greater than the current size of componentPools, resize the vector
	if (componentId >= componentPools.size()) {
		componentPools.resize(componentId + 1, nullptr);
	}

	// If we don't have a pool for that component type
	if (!componentPools[componentId]) {
		std::shared_ptr<Pool<T>> newComponentPool = std::make_shared<Pool<T>>();
		componentPools[componentId] = newComponentPool;
	}

	// Get the pool of component values for that component type
	std::shared_ptr<Pool<T>> componentPool = std::static_pointer_cast<Pool<T>>(componentPools[componentId]);

	// If the entity id is greater than the current size of the component pool, resize pool
	if (entityId >= componentPool->GetSize()) {
		componentPool->Resize(numEntities);
	}

	// Create a new component object and forward various parameters to constructor
	T newComponent(std::forward<TArgs>(args)...);

	// Add new component to component pools list, using entity id as index
	componentPool->Set(entityId, newComponent);

	// Change component signature of the entity and set the component id on the bitset to 1
	entityComponentSignatures[entityId].set(componentId);

	Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
}

template <typename T>
void Registry::RemoveComponent(Entity entity) {
	const auto componentId = Component<T>::GetId();
	const auto entityId = entity.GetId();

	entityComponentSignatures[entityId].set(componentId, false);

	Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
}

template <typename T>
bool Registry::HasComponent(Entity entity) const{
	const auto componentId = Component<T>::GetId();
	const auto entityId = entity.GetId();

	return entityComponentSignatures[entityId].test(componentId);
}

template <typename T>
T& Registry::GetComponent(Entity entity) const {
	const auto componentId = Component<T>::GetId();
	const auto entityId = entity.GetId();
	auto componentPool = std::static_pointer_cast<Pool<T>>(componentPools[componentId]);

	return componentPool->Get(entityId);
}

template <typename T, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args) {
	registry->AddComponent<T>(*this, std::forward<TArgs>(args)...);
}

template <typename T>
void Entity::RemoveComponent() {
	registry->RemoveComponent<T>(*this);
}

template <typename T>
bool Entity::HasComponent() const{
	return registry->HasComponent<T>(*this);
}

template <typename T>
T& Entity::GetComponent() const{
	return registry->GetComponent<T>(*this);
}