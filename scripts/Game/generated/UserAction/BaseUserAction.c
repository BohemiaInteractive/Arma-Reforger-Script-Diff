/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UserAction
\{
*/

class BaseUserAction: ScriptAndConfig
{
	// Injects
	//! The reason why action cannot be performed.
	//! Should be set in CanBePerformed check
	protected string m_sCannotPerformReason;

	//! Sets the reason why action couldn't be performed.
	//! Should be done in CanBePerformed check
	void SetCannotPerformReason(string reason)
	{
		m_sCannotPerformReason = reason;
	}

	//! Returns the reason why action couldn't be performed.
	//! It is valid only within and after CanBePerformed is called
	string GetCannotPerformReason()
	{
		return m_sCannotPerformReason;
	}

	//! Can be filled in scripts to be used as params when name is being formatted when displayed in UI
	string ActionNameParams[9];

	//! Returns the index of the context this action is registered in as or -1 if none
	proto external int GetContextIndex(string sContextName);
	//! Setter for m_pActiveContext
	proto external void SetActiveContext(UserActionContext context);
	//! Getter for m_pActiveContext
	proto external UserActionContext GetActiveContext();
	//! Can this action be performed by the user?
	proto external bool CanBePerformed(IEntity user);
	//! Can this action be shown in the UI for the user?
	proto external bool CanBeShown(IEntity user);
	//! Does this action only have client side effect?
	proto external bool HasLocalEffectOnly();
	//! If HasLocalEffectOnly() is false this method tells if the server is supposed to broadcast this action to clients.
	proto external bool CanBroadcast();
	//! Should this action be performed every frame the input action is triggered?
	proto external bool ShouldPerformPerFrame();
	//! Return the name of this action.
	//! By default returns the value in UIInfo or empty string if UIInfo doesn't exist.
	proto external string GetActionName();
	//! Return the description of this action.
	//! By default returns the value in UIInfo or empty string if UIInfo doesn't exist.
	proto external string GetActionDescription();
	//! Returns the visibility range of this action in metres.
	proto external float GetVisibilityRange();
	//! Sets the duration of this action in seconds.
	proto external void SetActionDuration(float duration);
	//! Returns the duration of this action in seconds.
	proto external float GetActionDuration();
	//! Returns the progress of this action in seconds.
	proto external float GetActionProgress(float fProgress, float timeSlice);
	//! Returns true while continuous or timed action is being used.
	proto external bool IsInProgress();
	//! Returns the UIInfo set for this user action or null if none.
	proto external UIInfo GetUIInfo();
	//! Returns the parent entity of this action.
	proto external IEntity GetOwner();
	//! Whether action can be aggregated by name, this is a temp workaround for localization.
	proto external bool CanAggregate();
	//! Returns the ID with which this action is registered in its parent ActionsManagerComponent.
	proto external int GetActionID();
	//! Used to ask to send action data again during continuous action
	proto external void SetSendActionDataFlag();
	proto external ActionsManagerComponent GetActionsManager();
	//! Action can be enabled/disabled by the server. Disabled actions return false for CanBeShown(). Has no effect when called from the client.
	proto external void SetActionEnabled_S(bool enable);
	//! Was the action disabled by the server?
	proto external bool WasDisabledByServer();

	// callbacks

	event protected bool OnRplSave(ScriptBitWriter writer) { return true; };
	event protected bool OnRplLoad(ScriptBitReader reader) { return true; };
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	event protected bool OnSaveActionData(ScriptBitWriter writer) { return true; };
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	event protected bool OnLoadActionData(ScriptBitReader reader) { return true; };
	/*!
	Callback for when action is selected by the local player and the local player ONLY.
	*/
	event protected event void OnActionSelected();
	/*!
	Callback for when action is deselected by the local player and the local player ONLY.
	*/
	event protected event void OnActionDeselected();
}

/*!
\}
*/
