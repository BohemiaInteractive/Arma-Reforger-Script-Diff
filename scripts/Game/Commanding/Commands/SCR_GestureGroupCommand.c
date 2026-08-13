//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_GestureGroupCommand : SCR_WaypointGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity groupEnt, vector targetPosition, int playerID, bool isClient)
	{
		return true;
	}
}