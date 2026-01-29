class SCR_AIDecoTestAreAllocatedCompartmentsOccupied : DecoratorTestScripted
{
	// checks if all compartments allocated on the group for getIn / other activities are occupied
	// returns true if occupied, false if at least one is not occupied
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		SCR_AIGroup gr = SCR_AIGroup.Cast(agent);
		if (!gr)
			return false;
		
		array<BaseCompartmentSlot> compartments = {};
		gr.GetAllocatedCompartments(compartments);
		
		foreach (BaseCompartmentSlot slot : compartments)
		{
			if (!slot)
				continue;
			if (!slot.IsOccupied())
				return false;
		}
		
		return true;
	}
};