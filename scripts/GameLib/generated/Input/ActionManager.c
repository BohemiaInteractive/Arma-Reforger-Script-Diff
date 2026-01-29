/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Input
\{
*/

//! ActionManager holds information about states of registered Contexts and Actions
sealed class ActionManager
{
	private void ActionManager();
	private void ~ActionManager();

	//! Show debug info via DbgUI
	proto external void SetDebug(int debugMode);
	proto external bool ActivateContext(string contextName, int duration = 0);
	proto external bool IsContextActive(string contextName);
	proto external void SetContextDebug(string contextName, bool bDebug);
	proto external bool ActivateAction(string actionName, int duration = 0);
	proto external bool IsActionActive(string actionName);
	//! Return normalized value of action
	proto external float GetActionValue(string actionName);
	//! Return type of last input contributing to this action
	proto external EActionValueType GetActionInputType(string actionName);
	//! Return TRUE if value of action is above threshold(0.99) and is active
	proto external bool GetActionTriggered(string actionName);
	proto external void SetActionValue(string actionName, float value);
	proto external int GetActionCount();
	proto external string GetActionName(int actionIndex);
	proto external void AddActionListener(string actionName, EActionTrigger trigger,  ActionListenerCallback callback);
	proto external bool RemoveActionListener(string actionName, EActionTrigger trigger,  ActionListenerCallback callback);
}

/*!
\}
*/
