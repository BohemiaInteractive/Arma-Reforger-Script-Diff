/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIPathfindingComponentClass: AIComponentClass
{
}

class AIPathfindingComponent: AIComponent
{
	proto external bool RayTrace(vector position, vector destination, vector hitPos);
	proto external void SetAreaCosts(ResourceName config);
	/*!
	Gets NavmeshComponent currently asociated with this Pathfinding
	*/
	proto external NavmeshWorldComponent GetNavmeshComponent();
	/*!
	Gets closest position on navmesh from given center when point is expanded using halfExtents to create search axis aligned bounding box.
	\param	position The position.
	\param	searchHalfExtents Extents of the half to create aabb around center point.
	\param	correctedPosition The corrected position.
	\return	true if it succeeds, false if it fails in which case correctedPosition is not touched at all.
	 */
	proto external bool GetClosestPositionOnNavmesh(vector position, vector searchHalfExtents, out vector correctedPosition);
}

/*!
\}
*/
