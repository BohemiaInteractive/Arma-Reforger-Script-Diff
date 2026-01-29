
class DecoratorTestNotNull : DecoratorTestScripted
{
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		return controlled != null;
	}
}