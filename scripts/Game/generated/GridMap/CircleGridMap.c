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
class CircleGridMap: BaseGridMap
{
	void CircleGridMap(float cellSize = 100.0) {}

	//! Insert the entity inside the GridMap with specific params.
	proto external void Insert(IEntity entity, float radius, bool isDynamic, int mask = 0);
	//! Update the radius of the specified entity
	proto external void UpdateRadius(IEntity entity, float radius);
}

/*!
\}
*/
