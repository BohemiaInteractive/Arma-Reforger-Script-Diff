//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_SetCombatModeGroupCommand : SCR_BaseGroupCommand
{
	[Attribute("-1", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAIGroupCombatMode))]
	protected EAIGroupCombatMode m_eCombatMode;
	
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
		
		SCR_AIGroupUtilityComponent groupUtility = SCR_AIGroupUtilityComponent.Cast(slaveGroup.FindComponent(SCR_AIGroupUtilityComponent));
		
		groupUtility.SetCombatMode(m_eCombatMode);
		
		// Cancel suppressive fire waypoint if Hold Fire was ordered
		if (m_eCombatMode == EAIGroupCombatMode.HOLD_FIRE)
		{
			array<AIWaypoint> currentWaypoints = {};
			slaveGroup.GetWaypoints(currentWaypoints);
			foreach(AIWaypoint currentwp : currentWaypoints)
			{
				if (SCR_SuppressWaypoint.Cast(currentwp))
					slaveGroup.RemoveWaypoint(currentwp);
			}
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