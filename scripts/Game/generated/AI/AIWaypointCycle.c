/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIWaypointCycleClass: AIWaypointClass
{
}

/*!
\brief	Cycled waypoints are used when we want a series of waypoints repeating for an infinite or finite time.
		Cycled waypoint inserts the cycling waypoints (and itself) into the agents waypoints array to keep
		the cycle running.
*/
class AIWaypointCycle: AIWaypoint
{
	/*!
	\brief      Checks whether the group can still cycle, if so, adds waypoints that the group's waypoints list

	\tparam     AIGroup*	Pointer to the group that will be operated on
	*/
	proto external void PerformOn(AIGroup runningAgent);
	proto external void SetRerunCounter(int val);
	proto external int GetRerunCounter();
	proto external void SetWaypoints(array<AIWaypoint> waypoints);
	proto external void GetWaypoints(out array<AIWaypoint> outWaypoints);
}

/*!
\}
*/
