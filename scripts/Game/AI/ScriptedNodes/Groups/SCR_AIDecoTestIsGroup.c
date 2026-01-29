class SCR_AIDecoTestIsGroup : DecoratorTestScripted
{	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		AIGroup group = AIGroup.Cast(controlled);		

		return group != null;
	}
};