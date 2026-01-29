/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class MineManagerComponentClass: GenericComponentClass
{
}

class MineManagerComponent: GenericComponent
{
	//! Registers the given mine entity. The mine spawner will add it's awareness to it's faction of this mine.
	proto external void Register(notnull IEntity mine, IEntity mineSpawner);
	//! Removes the given mine entity.
	proto external void Remove(notnull IEntity mine);
	//! Update a mine's information awareness
	proto external void AddFactionAwareness(notnull IEntity mine, notnull IEntity awarenessAdder);
}

/*!
\}
*/
