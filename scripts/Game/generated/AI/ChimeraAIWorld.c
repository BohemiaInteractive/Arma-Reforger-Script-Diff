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
