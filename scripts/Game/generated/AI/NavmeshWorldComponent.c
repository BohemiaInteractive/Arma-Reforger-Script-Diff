/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class NavmeshWorldComponentClass: GenericComponentClass
{
}

class NavmeshWorldComponent: GenericComponent
{
	/*!
	Check if navmesh tile is loaded in or not
	\param wPos Tile pos in world coord space.
	\return true if not using streaming navmesh or if tile is loaded, false otherwise
	*/
	proto external bool IsTileLoaded(vector wPos);
	/*!
	Check if navmesh tile is valid for using
	\param wPos Tile pos in world coord space.
	\return true if tile is valid, false otherwise
	*/
	proto external bool IsTileValid(vector wPos);
	/*!
	Check if navmesh tile is requested
	\param wPos Tile pos in world coord space.
	\return true if tile is requested, false otherwise
	*/
	proto external bool IsTileRequested(vector wPos);
	/*!
	WARNING: This can take many (even 100+) frames
	\param wPos Tile pos in world coord space.
	\return false when tile cannot be loaded or not using streamable navmesh, true if it can
	*/
	proto external bool LoadTileIn(vector wPos);
	/*!
	Get point on navmesh that is reachable from the origin
	\param originPoint point from which to count
	\param distance distance to check
	\param outPosition reachable position out
	\return true if success, false if origin point isn't close enough to navmesh.
	*/
	proto external bool GetReachablePoint(vector originPoint, float distance, vector outPosition);
}

/*!
\}
*/
