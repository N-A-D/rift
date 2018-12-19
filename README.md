# What is an Entity Component System?
The Entity Component System design pattern separates state and behaviour motivated by efficient CPU cache usage. The approach focuses on storing data contiguously and applying transformations on that data.

Entity Component Systems are usually split up into three parts:
1. Entities:   Objects whose state is defined by its set of components.
1. Components: Blocks of data that individually describe some aspect of an entity.
1. Systems:    Operators that transform entity states en masse.

For more information on entity component systems:   
http://www.roguebasin.com/index.php?title=Entity_Component_System  
http://gameprogrammingpatterns.com/component.html   
https://medium.com/ingeniouslysimple/entities-components-and-systems-89c31464240d  
https://github.com/junkdog/artemis-odb/wiki/Introduction-to-Entity-Systems   

# Library overview
Rift is an Entity Component System written in C++ 14. It offers very fast iteration speeds by grouping entities based on system search criterias. Entities are keys (column indices) into a transposed table of component typess, where each row of the table is a different type. Systems query for entities they need using a list of component types and submit functions that transform those entities and components. The idea to group entities based on their components in order to speed up search comes from indexing in relational databases. The library makes use of sparse integer sets to compactly store entities for faster queries. For more information about sparse integer sets https://programmingpraxis.com/2012/03/09/sparse-sets/

**NOTE: Rift was not created with multithreading in mind.**

## Entities
As mentioned earlier, entities are essentially column indices into a component type table. As such, `rift::Entity` is a proxy class for a `std::uint64_t` index. The index is composed of two parts: a 32 bit version and a 32 bit index. The 32 bit version distinguishes between stale and valid entities that have the same index. It is necessary as the 32 bit index maps an *entity* to its components.

Entities in Rift cannot be created directly, they must be created using a `rift::EntityManager`. This is to avoid errors related to invalid entities.

Entities are created as follows:
```cpp
rift::EntityManager manager;
rift::Entity entity = manager.create_entity();
```

## Components 
In Rift, Components are *Plain Old Data* types.
### Implementation notes:
- Every *component* is required to subclass `rift::Component` to ensure its considered a *component* by Rift.
- Every *component* must include a default constructor as well as a constructor that initializes all of its POD member variables. 

For instance, the following is an example of a *Position* component:
```cpp
struct Position : public rift::Component<Position> {
  Position() : x(0.0f), y(0.0f) {}
  Position(float x, float y) : x(x), y(y) {}
  float x, y;
};
```

Continuing on with the example above, an entity can add the component as follows:
```cpp
entity.add<Position>(100.0f, 25.0f);
```

## Systems
In Rift, Systems are what define the behaviour of different entities.
### Implementation notes
- Every *system* must inherit from `rift::System` in order for the type to be considered a *System*. 
- Every *system* must implement the `rift::BaseSystem::update(rift::EntityManager&, double)` member. This function is where entity transformations should be carried out. 

Systems submit functions to an entity manager which is then carried out on every entity that matches the system's search criteria. 
For example, suppose there were two components *Position* and *Direction*, then a system's submitted query could look like the following:
```cpp
entity_manager.for_entities_with<Position, Direction>([](rift::Entity entity, Position& pos, Direction& dir){
    pos.x += dir.x;
    pos.y += dir.y;
});
```
With regards to intersystem communication, Rift does not include any form of messaging system. It is up to the user to implement such a system in the case that systems need to communicate with each other. 
