/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UserAction
\{
*/

class BaseInteractionHandlerComponentClass: GameComponentClass
{
}

//! Base class for interaction handler.
//! This component must be attached to a PlayerController.
class BaseInteractionHandlerComponent: GameComponent
{
	//! Returns true if any interaction can be done at given moment, i.e. UI should be shown
	proto external bool IsInteractionAvailable();
	/*!
	Return currently selected user action or null if none
	*/
	proto external BaseUserAction GetSelectedAction();
	//! Returns the global actions visibility range value defined by attribute in this component.
	proto external float GetVisibilityRange();

	// callbacks

	/*!
	Called on initialization.
	Might not be called unless we already own this item.
	*/
	event void OnInit(IEntity owner);
	//! Called every frame, only for local player controller.
	event void OnFrame(IEntity owner, float timeSlice);
	//! Called after post simulate, only for local player controller.
	event void OnAfterPostSimulate(IEntity owner, float timeSlice);
}

/*!
\}
*/
