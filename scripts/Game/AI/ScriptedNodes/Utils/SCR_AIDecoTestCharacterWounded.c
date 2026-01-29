/*
Returns true character is wounded.
*/
class SCR_AIDecoTestCharacterWounded : DecoratorTestScripted
{
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		
		return SCR_AIDamageHandling.IsCharacterWounded(controlled);
	}
};