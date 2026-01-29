class SCR_AIDecoTestIsLeader : DecoratorTestScripted
{	
	override protected bool TestFunction(AIAgent agent, IEntity controlled)
	{
		AIGroup group = agent.GetParentGroup();
		
		if (!group)
			return false;
		
		return group.GetLeaderAgent() == agent;
	}
};