/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UserAction
\{
*/

class ExtBaseInteractionHandlerComponentClass: BaseInteractionHandlerComponentClass
{
}

/*!
Collects interactions by performing a sphere cast initially and then a sphere query at the end.
Iterates through hit entities and tries to find closest entity with ActionsManagerComponent that
has a valid context.
*/
class ExtBaseInteractionHandlerComponent: BaseInteractionHandlerComponent
{
	/*!
	Sets currently select action. Only relevant to the owner of this component,
	ie. for the local player. Accepts null for when action is to be cleared.
	*/
	proto external protected void SetSelectedAction(BaseUserAction action);
	//! Returns currently gathered (active-preferred) context or null if none.
	proto external UserActionContext GetCurrentContext();
	//! Returns the controlled entity or null if none.
	proto external IEntity GetControlledEntity();
	/*!
	After collection is done, this method can be used for retrieving already checked user actions from current context.
	\param outActions Array that will be filled with user actions that can be shown
	\param outCanBePerformed Array that complements the outActions with result of CanBePerformed check
	\return Amount of actions gathered, can be 0 if none
	*/
	proto external int GetFilteredActions(out notnull array<BaseUserAction> outActions, out notnull array<bool> outCanBePerformed);
	//! Returns true when there is a gathered context available
	proto external bool IsContextAvailable();
	//! If set to true, we expect a list of nearby entities to be provided from the user instead
	proto external void SetManualNearbyCollectionOverride(bool enabled);
	//! Returns whether manual nearby collection override is set (we expect a list of nearby entities to be provided from the user instead)
	proto external bool GetManualNearbyCollectionOverride();
	//! If set to true, we expect a list of entities to be provided from the user instead
	proto external void SetManualCollectionOverride(bool enabled);
	//! Returns whether manual collection override is set (we expect a list of entities to be provided from the user instead)
	proto external bool GetManualCollectionOverride();
	/*!
	Sets whether nearby available context collection should be enabled.
	\param enabled True to enable collection, false to disable.
	*/
	proto external void SetNearbyCollectionEnabled(bool enabled);
	//! Returns whether nearby available context collection is enabled.
	proto external bool GetNearbyCollectionEnabled();
	//! Returns the radius in meters that is used for nearby available context collection.
	proto external float GetNearbyCollectionRadius();
	/*!
	If nearby collection is enabled, fills the provided array with collected contexts nearby.
	Only contexts that have at least one action that can be shown and performed are output.
	\param outContexts Array of contexts that will be cleared and filled with nearby available contexts.
	\return Returns the count of output contexts or 0 if none.
	*/
	proto external int GetNearbyAvailableContextList(out notnull array<UserActionContext> outContexts);
	/*!
	If nearby collection is enabled, fills the provided array with collected contexts nearby.
	Only contexts that have at least one action that can be shown, but not performed are output.
	\param outContexts Array of contexts that will be cleared and filled with nearby available contexts.
	\return Returns the count of output contexts or 0 if none.
	*/
	proto external int GetNearbyShowableContextList(out notnull array<UserActionContext> outContexts);
	/*!
	If nearby collection is enabled, fills the provided array with collected contexts nearby.
	Only contexts that have at least one action, but none can be shown or performed are output.
	\param outContexts Array of contexts that will be cleared and filled with nearby available contexts.
	\return Returns the count of output contexts or 0 if none.
	*/
	proto external int GetNearbyUnavailableContextList(out notnull array<UserActionContext> outContexts);
	//! The last known interaction reference point.
	proto external vector GetLastReferencePoint(float timeSlice);

	// callbacks

	event protected void OnControlledEntityChanged(IEntity from, IEntity to);
	/*!
	Event called when InteractionHandlerComponent finds new target UserActionContext and a change occurs.
	Both previous and new context might be 'null' if no context is caught in collection.
	\param previousContext Context that was active up until this collection - can be null
	\param newContext Context that will be active after this collection - can be null if nothing was caught
	*/
	event protected void OnContextChanged(UserActionContext previousContext, UserActionContext newContext);
	/*!
	Called once per frame after simulation.
	Can be used for drawing and updating UI.
	*/
	event protected void OnPostFrame(IEntity owner, IEntity controlledEntity, float timeSlice);
	/*!
	Called before a context is set to determine whether a change can occur.
	*/
	event protected bool CanContextChange(UserActionContext currentContext, UserActionContext newContext);
	/*!
	Should preferred context collection use intersect checks?
	This will make a ray check against the context's radius. This works well in tight places where relying on physical collisions is impossible - e.g. in vehicle interiors.
	*/
	event protected bool DoIntersectCheck(IEntity controlledEntity);
	/*!
	Are any user actions available to interact with?
	Implement and return true if yes, false otherwise.
	*/
	event protected bool GetIsInteractionAvailableScript();
	/*!
	Are we allowed to start collecting and processing interactions?
	Implement custom logic and return true if yes, false otherwise.
	\param controlledEntity The entity that wants to be able to interact (player controlled)
	*/
	event protected bool GetCanInteractScript(IEntity controlledEntity);
	/*!
	Currently sected action or null if none.
	Implement and return currently selected action.
	*/
	event protected BaseUserAction GetSelectedActionScript();
	/*!
	Callback when manual collection override is enabled. See SetManualCollectionOverride.
	It may return nullptrs inside array if this is called between deletion of instance inside the array and updating of the array.
	\param owner Parent entity
	\param referencePoint Point in world space used to calculate distance and sort contexts by
	*/
	event protected array<IEntity> GetManualOverrideList(IEntity owner, out vector referencePoint);
	/*!
	Callback when manual nearby collection override is enabled. See SetManualNearbyCollectionOverride.
	It may return nullptrs inside array if this is called between deletion of instance inside the array and updating of the array.
	\param owner Parent entity
	\param referencePoint Point in world space used to calculate distance and sort contexts by
	*/
	event protected array<IEntity> GetManualNearbyOverrideList(IEntity owner, out vector referencePoint);
	/*!
	*/
	event protected bool IsPerformingAction();
}

/*!
\}
*/
