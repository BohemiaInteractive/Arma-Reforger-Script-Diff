[BaseContainerProps()]
class SCR_VehiclePilotVacancyCheck : SCR_VehicleConditionCheck
{
	protected SCR_AIGroup m_IgnoredAiGroup;

	//------------------------------------------------------------------------------------------------
	override bool CheckCondition(Vehicle vehicle)
	{
		if (!super.CheckCondition(vehicle))
			return false;

		return CheckPilotVacancy(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetIgnoredAiGroup()
	{
		return m_IgnoredAiGroup;
	}

	//------------------------------------------------------------------------------------------------
	void SetIgnoredAiGroup(SCR_AIGroup aiGroup)
	{
		m_IgnoredAiGroup = aiGroup;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CheckPilotVacancy(notnull Vehicle vehicle)
	{
		PilotCompartmentSlot pilotCompartment = GetPilotCompartment(vehicle);
		if (!pilotCompartment)
			return true;

		if (!m_IgnoredAiGroup)
			return !pilotCompartment.IsOccupied();

		// If the slot is unoccupied, the condition is met.
		if (!pilotCompartment.IsOccupied())
			return true;

		IEntity occupant = pilotCompartment.GetOccupant();
		if (!occupant)
			return true;

		AIAgent occupantAgent = SCR_AIUtils.GetAIAgent(occupant);
		if (!occupantAgent)
			return true;

		// If the occupant does not belong to the ignored group, the pilot compartment is considered occupied.
		if (occupantAgent.GetParentGroup() != m_IgnoredAiGroup)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected PilotCompartmentSlot GetPilotCompartment(notnull Vehicle vehicle)
	{
		SCR_BaseCompartmentManagerComponent compartmentMan = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentMan)
			return null;

		array<BaseCompartmentSlot> compartmentSlots = {};
		compartmentMan.GetCompartments(compartmentSlots);
		PilotCompartmentSlot pilotComp;
		foreach (BaseCompartmentSlot slot : compartmentSlots)
		{
			pilotComp = PilotCompartmentSlot.Cast(slot);
			if (pilotComp)
				return pilotComp;
		}

		return null;
	}
}
