class InteractionHandlerComponentClass: ExtBaseInteractionHandlerComponentClass
{
};

/*!
	Collects interactions by performing a sphere cast initially and then a sphere query at the end.
	Iterates through hit entities and tries to find closest entity with ActionsManagerComponent that
	has a valid context.
*/
class InteractionHandlerComponent : ExtBaseInteractionHandlerComponent
{
	/*!
		Called before a context is set to determine whether a change can occur.
	*/
	override protected event bool CanContextChange(UserActionContext currentContext, UserActionContext newContext)
	{
		return true;
	}

	/*!
		Should preferred context collection use intersect checks?
		This will make a ray check against the context's radius. This works well in tight places where relying on physical collisions is impossible - e.g. in vehicle interiors.
	*/
	override protected bool DoIntersectCheck(IEntity controlledEntity) { return false; }
	
	/*!
		Are any user actions available to interact with?
		Implement and return true if yes, false otherwise.
	*/
	override protected bool GetIsInteractionAvailableScript() { return false; }
	
	/*!
		Are we allowed to start collecting and processing interactions?
		Implement custom logic and return true if yes, false otherwise.
		\param controlledEntity The entity that wants to be able to interact (player controlled)
	*/
	override protected bool GetCanInteractScript(IEntity controlledEntity) { return false; }
	
	/*!
		Currently sected action or null if none.
		Implement and return currently selected action.
	*/
	override protected BaseUserAction GetSelectedActionScript() { return null; }	
};