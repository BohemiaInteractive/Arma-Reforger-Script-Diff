//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_StopMovementGroupCommand : SCR_BaseGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient)
		{
			//place to place a logic that would be executed for other players
			return true;
		}
		
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(target);
		if (!slaveGroup)
			return false;
		
		array<AIWaypoint> currentWaypoints = {};
		slaveGroup.GetWaypoints(currentWaypoints);
		foreach(AIWaypoint currentwp : currentWaypoints)
		{
			slaveGroup.RemoveWaypoint(currentwp);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		if (!CanBeShownInCurrentLifeState())
			return false;
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;
		
		if (!CanRoleShow())
			return false;
		
		return true;
	}
}