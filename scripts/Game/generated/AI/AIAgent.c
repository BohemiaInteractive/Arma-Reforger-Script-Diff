/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIAgentClass: GenericControllerClass
{
}

class AIAgent: GenericController
{
	proto external IEntity GetControlledEntity();
	proto external AIGroup GetParentGroup();
	proto external void SetLOD(int newLOD);
	proto external int GetLOD();
	/*
	* Sets LOD that is to be forced upon the Agent.
	* @param lod between -1 and MaxLOD
	*/
	proto external void SetPermanentLOD(int lod);
	proto external int GetPermanentLOD();
	/*
	* AIAgent won't be able to reach MaxLOD unless set by PermanentLOD;
	* Warning: If the AIAgent is in LOD10 this won't change it;
	*/
	proto external void PreventMaxLOD(float preventTime = -1);
	proto external void AllowMaxLOD();
	/*
	* AIAgents get disabled in MaxLOD by default.
	*/
	static proto int GetMaxLOD();
	//Does AIAgent affects AIWorld limit of AIs
	proto external bool CountTowardsAIWorldLimit();
	proto external int GetDangerEventsCount();
	//! Internally similar danger events can be aggregated. In that case the new danger event is not put into the queue, but instead counter of previous similar event is increased.
	proto AIDangerEvent GetDangerEvent(int index, out int outCount);
	proto external void ClearDangerEvents(int howMany);
	// It moves events form agent into an array and clear the queue in agent
	proto external void ExtractAIEvents(out notnull array<AIWaypoint> aiEvents);
	proto external ActionManager GetActionManager();
	proto external AICommunicationComponent GetCommunicationComponent();
	proto external AIControlComponent GetControlComponent();
	proto external AIBaseMovementComponent GetMovementComponent();
	//GetAimingComponent() returns nullptr as base AIAgent doesn't provide aiming
	proto external AIBaseAimingComponent GetAimingComponent();
	// enabling, disabling, switching to player control on controlComponent
	proto external void ActivateAI();
	proto external void DeactivateAI();
	proto external bool IsAIActivated();

	// callbacks

	event void OnLODChanged(int oldLOD, int newLOD);
}

/*!
\}
*/
