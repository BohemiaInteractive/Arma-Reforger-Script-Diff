/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class ChimeraAIWorldClass: AIWorldClass
{
}

class ChimeraAIWorld: AIWorld
{
	// dispatcher re-checks HasObserverInRange before spawning.
	proto external void EnqueueSpawnRequest(AIGroup group, int slotsWanted, float observerRange = 0);
	// despawned so stale partial-fill requests do not re-materialise it on the next tick.
	proto external void PurgeSpawnRequestsForGroup(AIGroup group);
	// Diagnostic read-only: number of evictions performed in the last ProcessSpawnQueue pass.
	proto external int GetLastTickEvictions();
	proto external int GetCurrentAmountOfLimitedAIsForFaction(FactionKey factionKey);
	// Returns the Limit for the specified faction. Returns -1 if such limit is not defined
	proto external int GetAILimitForFaction(FactionKey factionKey);
	// Checks if the AI can be added for Faction. Returns always true if such limit is not defined
	proto external bool CanLimitedAIBeAddedForFaction(FactionKey factionKey);
	proto external void RequestRoadNetworkRebuild(vector min, vector max);
	// get road network
	proto external RoadNetworkManager GetRoadNetworkManager();
}

/*!
\}
*/
