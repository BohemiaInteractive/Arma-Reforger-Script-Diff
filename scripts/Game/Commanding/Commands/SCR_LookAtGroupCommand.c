//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_LookAtGroupCommand : SCR_BaseGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity groupEnt, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient && playerID == SCR_PlayerController.GetLocalPlayerId())
			return true;

		if (!groupEnt || !targetPosition)
			return false;
		
		return LookAtPosition(groupEnt, targetPosition, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool LookAtPosition(IEntity groupEnt, vector targetPosition, int playerID)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(groupEnt);
		if (!group)
			return false;
		
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		
		SCR_AIUtilityComponent utilityComponent;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;
			
			utilityComponent = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
			if (!utilityComponent)
				continue;
			
			utilityComponent.LookAt(targetPosition);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		if (!CanBeShownInCurrentLifeState())
			return false;
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		return groupController && CanRoleShow();
	}
}