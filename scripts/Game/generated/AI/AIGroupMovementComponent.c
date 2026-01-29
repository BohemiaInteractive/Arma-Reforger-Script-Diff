/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIGroupMovementComponentClass: AIBaseMovementComponentClass
{
}

class AIGroupMovementComponent: AIBaseMovementComponent
{
	const int DEFAULT_HANDLER_ID = 0;

	proto external void SetGroupCharactersWantedMovementType(EMovementType movementWanted);
	proto external EMovementType GetGroupCharactersMovementTypeWanted();
	/*!
	Group Movement Handler: Takes care of planning movement and organizing in formation a collection of agents in a group.
	Default handler 0 is initialized with the formation specified in 'DefaultFormation' variable. Contains all agents of the group unless moved. Can't be removed.
	With additional handlers, it's possible to subdivide the group into independent moving parts. Example: Part of the group organized in a vehicle convoy.
	Additional handlers need to be created and removed manually.
	*/
	proto external int GetAgentMoveHandlerId(notnull AIAgent agent);
	//Creates empty group move handler and returns it's identification
	proto external int CreateGroupMoveHandler(string formationType);
	//Returns how many agents there's in a handler and -1 if the handler doesn't exists.
	proto external int GetMoveHandlerAgentCount(int handlerId = 0);
	//Moves individual agent from one handler to another
	proto external void MoveAgentToHandler(notnull AIAgent agent, int originalHandlerId, int newHandlerId);
	//Moves individual agent from one handler to another and makes it the leader of the formation
	proto external void SetMoveHandlerLeader(notnull AIAgent agent, int originalHandlerId, int newHandlerId);
	//Removes the indicated group move handler. All it's agents go back to the default handler.
	proto external void RemoveGroupMoveHandler(int handlerId);
	//Removes all existing handlers. All agents go back to the default handler.
	proto external void ClearGroupMoveHandlers();
	//Formations getters and setters
	proto external bool SetFormationDefinition(int handlerId, string formation);
	proto external AIFormationDefinition GetFormationDefinition(int handlerId);
	/*!
	Displacement modifies formation indexes.
	Examples:
	Displacement = 0, Used formation indexes: 0, 1, ..., Max
	Displacement = 2, Used formation indexes: 2, 3, ..., Max
	*/
	proto external void SetFormationDisplacement(int iValue, int handlerId = 0);
	proto external int GetFormationDisplacement(int handlerId = 0);
	proto AIAgent GetHandlerLeaderAgent(int handlerId = 0);
	proto int GetAgentsInHandler(notnull out array<AIAgent> outAgents, int handlerId = 0);
	proto external bool IsFormationDisplaced(int handlerId = 0);

	// callbacks

	event void OnMoveHandlerAdded(int handlerId);
	event void OnMoveHandlerRemoved(int handlerId);
}

/*!
\}
*/
