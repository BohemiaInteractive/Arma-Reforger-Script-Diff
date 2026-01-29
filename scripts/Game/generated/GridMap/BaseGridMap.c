/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GridMap
\{
*/

/*!
Represent a grid map.
Mask in all methods should be a bit mask. The value 0 means no check of the mask.
*/
class BaseGridMap: Managed
{
	void BaseGridMap(float cellSize = 100.0) {}

	//! Force update the position of an entity
	proto external void UpdatePosition(notnull IEntity entity, vector newPosition);
	//! Remove the entity from the GridMap.
	proto external void Remove(IEntity entity);
	//! Update the grid map entities positions. Update the position of dynamic entities and static entities if set up.
	proto external void Update();
	//! Returns registered entities in given categories within a given range with a specific mask.
	proto external int FindEntitiesInRange(out notnull array<IEntity> entities, vector origin, float range, int mask = 0);
	//! Set if the static entities should still be checked during the update.
	proto external void SetUpdateOfStaticEntities(bool state);
	//! Set the number of static entities updated per update.
	proto external void SetNmbOfStaticEntitiesUpdated(int nmb);
}

/*!
\}
*/
