/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIActionBase: ScriptAndConfig
{
	proto external float Evaluate();
	proto external float GetPriority();
	proto external void SetPriority(float priority);
	proto external bool GetActionIsSuspended();
	proto external void SetActionIsSuspended(bool val);
	proto external bool GetIsUniqueInActionQueue();
	proto external void SetIsUniqueInActionQueue(bool unique);
	proto external EAIActionState GetActionState();
	proto external void SetActionState(EAIActionState state);
	proto external void Fail();
	proto external void Complete();
	proto external bool GetRemoveAction();
	proto external void SetRemoveAction(bool val);
	proto external AIActionBase GetRelatedGroupActivity();
	proto external void SetRelatedGroupActivity(AIActionBase related);

	// callbacks

	event float CustomEvaluate() { return GetPriority(); };
	event float EvaluatePriorityLevel() { return 0.0; };
	event void OnSetSuspended(bool suspended);
	event void OnSetActionState(EAIActionState state);
	event void OnFail();
	event void OnComplete();
	/* Called when any new message arrives, regardless of state of this action.
	When any of the messages returns true, the reaction is not invoked.*/
	event bool OnMessage(AIMessage msg) { return false; };
	// Called after behavior was selected after different behavior
	event void OnActionSelected();
	// Called after behavior different behavior was selected instead of this one
	event void OnActionDeselected();
	// Called when action is removed from utility component
	event void OnActionRemoved();
}

/*!
\}
*/
