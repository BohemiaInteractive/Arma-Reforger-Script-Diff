class SCR_AIDecoTestIsAgentInGroup : DecoratorTestScripted
{
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		AIGroup group = AIGroup.Cast(agent);
		if (!group)
			return false;
		AIControlComponent contr = AIControlComponent.Cast(controlled.FindComponent(AIControlComponent));
		if (!contr)
			return false;
		AIAgent agToTest = contr.GetAIAgent();
		if (!group || !agToTest)
			return false;
		
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		return agents.Contains(agToTest);
	}
};