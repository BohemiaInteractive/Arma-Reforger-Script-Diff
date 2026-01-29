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
class AABGridMap: BaseGridMap
{
	void AABGridMap(float cellSize = 100.0) {}

	//! Insert the entity inside the GridMap with specific params.
	proto external void Insert(IEntity entity, vector mins, vector maxs, bool isDynamic, int mask = 0);
	//! Update the AABB of the specified entity
	proto external void UpdateAABB(IEntity entity, vector mins, vector maxs);
}

/*!
\}
*/
