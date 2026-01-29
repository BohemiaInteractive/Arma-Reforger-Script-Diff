/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class ChimeraCoverManagerComponentClass: CoverManagerComponentClass
{
}

class ChimeraCoverManagerComponent: CoverManagerComponent
{
	/*!
	Performs cover query. Gets nearby cover points, scores them, and select the best one.
	Returns true if a cover point has been found.
	navmeshWorld - name of navmesh world.
	pathfindingComp - pathfinding component of the Agent which will be executing movement.
	It is required because navmesh traversal rules depend on agent.
	queryParams - parameters of query.
	outCoverPos - output, cover position where agent must stand.
	outCoverTallestPos - output, tallest position of geometry which provides the cover point along cover direction.
	outTileX, outTileY - output, x,y index of the tile where cover point was found.
	outCoverId - output, cover point index within the tile.
	*/
	proto bool GetBestCover(string navmeshWorld, notnull AIPathfindingComponent pathfindingComp, notnull CoverQueryProperties queryParams, out vector outCoverPos, out vector outCoverTallestPos, out int outTileX, out int outTileY, out int outCoverId);
	/*!
	Marks the cover as occupied. Occupied cover points are filtered out during cover search.
	CoverManager does not store which agents use which cover points, this must be handled on user side.
	After calling GetBestCover, if the cover was found, mark the cover as occupied with SetOccupiedCover().
	This will prevent several agents from selecting same cover point.
	When cover point is no longer used, mark it as not occupied to release it.
	*/
	proto external void SetOccupiedCover(int tileX, int tileY, int coverId, bool occupied);
}

/*!
\}
*/
