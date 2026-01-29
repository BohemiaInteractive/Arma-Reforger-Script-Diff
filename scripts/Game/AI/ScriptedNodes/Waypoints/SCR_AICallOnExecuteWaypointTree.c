class SCR_AICallOnExecuteWaypointTree : AITaskScripted
{
	protected SCR_AIGroupUtilityComponent m_Utility;
	
	override void OnInit(AIAgent owner)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return;
		}
		m_Utility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
	}
	
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Utility)
			NodeError(this, owner,"No utility component found!");
		m_Utility.OnExecuteWaypointTree();
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Calls event OnExecuteWaypointTree of SCR_AIWaypointState of current waypoint";
	}
};