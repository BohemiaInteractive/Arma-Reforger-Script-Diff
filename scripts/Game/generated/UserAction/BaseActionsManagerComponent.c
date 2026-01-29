/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UserAction
\{
*/

class BaseActionsManagerComponent: GameComponent
{
	//! Tries to find context with provided name in the actions manager.
	//! Returns UserActionContext with provided name or null if none exists
	proto external UserActionContext GetContext(string contextName);
	//! Fills the provided array with the list of all user actions that are registered in this ActionsManagerComponent.
	//! Returns the number of output elements.
	proto external int GetActionsList(out notnull array<BaseUserAction> outActions);
	//! Returns the amount of actions of this (and all owned) managers
	proto external int GetActionsCount();
	//! Fills the provided array with the list of all contexts that are defined in this ActionsManagerComponent.
	//! Returns the number of output elements.
	proto external int GetContextList(out notnull array<UserActionContext> outContexts);
	//! Returns the amount of contexts that exist in this ActionsManagerComponent.
	proto external int GetContextCount();
	//! Return the first action from the list of all actions.
	proto external BaseUserAction GetFirstAction();
	//! Return action by ID
	proto external BaseUserAction FindAction(int actionID);
	/*!
	Returns whether this manager is enabled and can provide actions.
	This is introduced because an entity can be inactive but still
	provide actions or the other way around.
	(e.g. a burning wreck can be active, but not provide actions.)
	\return Returns true when active, false otherwise.
	*/
	proto external bool IsEnabled();
	//! Add a script function of type 'UserActionEventListener' which will receive EUserActionEvent events.
	proto bool AddUserActionEventListener(ScriptedUserAction action, UserActionEventListener listener);
	//! Remove the listener added using AddUserActionEventListener(ScriptedUserAction, UserActionEventListener).
	proto void RemoveUserActionEventListener(ScriptedUserAction action, UserActionEventListener listener);

	// callbacks

	event protected string NicifyGeneratedContextName(string name) { return name; };
}

/*!
\}
*/
