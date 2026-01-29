/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UserAction
\{
*/

class ScriptedUserAction: BaseUserAction
{
	//! Called when object is initialized and registered to actions manager
	event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent);
	//! Called when someone tries to perform the action, user entity is typically character
	event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity);
	//! Called when someone tries to perform the continuous action, user entity is typically character
	event void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice);
	//! If action passed as callback OnConfirmed will be called when reached execution, user entity is typically character
	event void OnConfirmed(IEntity pUserEntity);
	//! If action passed as callback OnRejected will be called when execution was rejected, user entity is typically character
	event void OnRejected(IEntity pUserEntity);
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	event void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity) { };
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	event void OnActionStart(IEntity pUserEntity) { };
	//! If overridden and true is returned, outName is returned when BaseUserAction.GetActionName is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	event bool GetActionNameScript(out string outName) { return false; };
	//! If overridden and true is returned, outName is returned when BaseUserAction.GetActionDescription is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	event bool GetActionDescriptionScript(out string outName) { return false; };
	//! Can this action be performed by the provided user entity?
	event bool CanBePerformedScript(IEntity user) { return true; };
	//! Can this entity be shown in the UI by the provided user entity?
	event bool CanBeShownScript(IEntity user) { return true; };
	//! Does this action only have client side effect?
	event bool HasLocalEffectOnlyScript() { return false; };
	//! If HasLocalEffectOnly() is false this method tells if the server is supposed to broadcast this action to clients.
	event bool CanBroadcastScript() { return true; };
	//! Returns the progress of this action in seconds.
	event float GetActionProgressScript(float fProgress, float timeSlice) { return fProgress + timeSlice; };
}

/*!
\}
*/
