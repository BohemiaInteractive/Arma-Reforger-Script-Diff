/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIBaseUtilityComponentClass: AIComponentClass
{
}

class AIBaseUtilityComponent: AIComponent
{
	//! Evaluate all actions and return the highest evaluated action which is not suspended. Returns nullptr if none found
	proto external AIActionBase EvaluateActions();
	//! Adds an action
	proto external void AddAction(AIActionBase action);
	//! Removes actions which are failed or completed
	proto external bool RemoveObsoleteActions();
	//! Iterates through the actions changing the state of those of the type or that inherit from it.
	proto external void SetStateAllActionsOfType(typename actionType, EAIActionState actionState, bool includeInherited = false);
	//! Returns true when there is an action of exactly this type
	proto external bool HasActionOfType(typename actionType);
	//! Finds action of exactly this type
	proto external AIActionBase FindActionOfType(typename actionType);
	//! Finds action of provided type or a type inherited from it
	proto external AIActionBase FindActionOfInheritedType(typename actionType);
	proto external void FindActionsOfType(typename actionType, notnull array<ref AIActionBase> outActions);
	proto external void FindActionsOfInheritedType(typename actionType, notnull array<ref AIActionBase> outActions);
	proto external void SetStateOfRelatedAction(AIActionBase relatedAction, EAIActionState state);
	proto external bool CallActionsOnMessage(AIMessage msg);
	//Getters and Setters
	proto external void GetActions(notnull array<ref AIActionBase> outActions);
	proto external AIActionBase GetCurrentAction();
	proto external void SetCurrentAction(AIActionBase executed);
	proto external AIActionBase GetExecutedAction();
	proto external void SetExecutedAction(AIActionBase executed);
	proto external AIAgent GetOwner();

	// callbacks

	event void EOnInit(IEntity owner);
	event void OnPostInit(IEntity owner);
	event void EOnDeactivate(IEntity owner);
	event void EOnDiag(IEntity owner, float timeSlice);
	event void EOnActivate(IEntity owner);
}

/*!
\}
*/
