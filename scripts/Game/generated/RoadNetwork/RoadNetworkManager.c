/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup RoadNetwork
\{
*/

class RoadNetworkManager
{
	proto int GetClosestRoad(vector pos, out BaseRoad foundRoad, out float distance, bool skipNavlinks = false);
	proto int GetRoadsInAABB(vector aabbMin, vector aabbMax, out array<BaseRoad> outRoads);
	proto bool GetReachableWaypointInRoad(vector agentPos, vector goalPos, float range, out vector outPos);
}

/*!
\}
*/
