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
class PointGridMap: BaseGridMap
{
	void PointGridMap(float cellSize = 100.0) {}

	//! Insert the entity inside the GridMap with specific params.
	proto external void Insert(IEntity entity, bool isDynamic, int mask = 0);
}

/*!
\}
*/
