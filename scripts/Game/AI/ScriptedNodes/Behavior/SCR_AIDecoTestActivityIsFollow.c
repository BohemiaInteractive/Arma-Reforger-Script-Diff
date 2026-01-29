 class SCR_AIDecoTestActivityIsFollow : DecoratorTestScripted
{
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		SCR_AIBaseUtilityComponent utilityComp = SCR_AIBaseUtilityComponent.Cast(agent.FindComponent(SCR_AIBaseUtilityComponent));
		if (!utilityComp)
			return false;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		if (!group)
			return false;
		
		SCR_AIActivityBase activity = SCR_AIActivityBase.Cast(utilityComp.GetExecutedAction());
		if (!activity)
			return false;
		
		SCR_AIFollowActivity followActivity = SCR_AIFollowActivity.Cast(activity);
		if (!followActivity)
				return false;
		
		return true;
	}
};