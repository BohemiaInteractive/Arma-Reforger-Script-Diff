/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIWorldClass: GenericEntityClass
{
}

class AIWorld: GenericEntity
{
	//AIBudget
	proto external int GetCurrentAmountOfLimitedAIs();
	proto external int GetAILimit();
	proto external void SetAILimit(int max);
	proto external bool CanLimitedAIBeAdded();
	proto external int GetCurrentNumOfActiveAIs();
	proto external int GetLimitOfActiveAIs();
	proto external void SetLimitOfActiveAIs(int max);
	proto external bool CanAIBeActivated();
	proto external void GetAIAgents(out notnull array<AIAgent> agents);
	proto external void RequestBroadcastDangerEvent(AIDangerEvent pEvent);
	// get formation definitions
	proto external AIFormationDefinition GetFormation(string pName);
	proto external NavmeshWorldComponent GetNavmeshWorldComponent(string name);
	proto external void RequestNavmeshRebuild(vector min, vector max);
	proto external void RequestNavmeshLoad(vector pos);

	// callbacks

	event void OnDebugCameraTeleport(vector pos);
	event void OnDebugAgentTeleport(AIAgent agent);
	event void AddedAIAgent(AIAgent agent);
	event void RemovingAIAgent(AIAgent agent);
}

/*!
\}
*/
