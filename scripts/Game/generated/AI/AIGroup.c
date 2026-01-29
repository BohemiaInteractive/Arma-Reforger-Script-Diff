/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIGroupClass: AIAgentClass
{
}

class AIGroup: AIAgent
{
	proto external void AddAgent(AIAgent pAgent);
	proto external void RemoveAgent(AIAgent pAgent);
	proto external int GetAgents(notnull out array<AIAgent> outAgents);
	proto external int GetAgentsCount();
	proto external void ActivateAllMembers();
	proto external void DeactivateAllMembers();
	proto external AIFormationComponent GetFormationComponent();
	// waypoints handling
	proto external void AddWaypoint(AIWaypoint w);
	proto external void RemoveWaypoint(AIWaypoint w);
	proto external void AddWaypointAt(AIWaypoint w, int index);
	proto external void RemoveWaypointAt(int index);
	//! Completes specified waypoint and removes it from the list of waypoints, no need to call RemoveWaypoint
	proto external void CompleteWaypoint(AIWaypoint w);
	proto external AIWaypoint GetCurrentWaypoint();
	proto external int GetWaypoints(out array<AIWaypoint> outWaypoints);
	proto external AIAgent GetLeaderAgent();
	proto external IEntity GetLeaderEntity();
	// Returns center of mass of the group, which is average of all member positions
	proto external vector GetCenterOfMass();
	proto external void SetNewLeader(AIAgent newLeader);

	// callbacks

	// WP Events
	event protected void OnCurrentWaypointChanged(AIWaypoint currentWP, AIWaypoint prevWP);
	event protected void OnWaypointCompleted(AIWaypoint wp);
	event protected void OnWaypointAdded(AIWaypoint wp);
	event protected void OnWaypointRemoved(AIWaypoint wp, bool isCurrentWaypoint);
	event void OnEmpty();
	event void OnAgentAdded(AIAgent child);
	event void OnAgentRemoved(AIAgent child);
	event void OnLeaderChanged(AIAgent currentLeader, AIAgent prevLeader);
}

/*!
\}
*/
