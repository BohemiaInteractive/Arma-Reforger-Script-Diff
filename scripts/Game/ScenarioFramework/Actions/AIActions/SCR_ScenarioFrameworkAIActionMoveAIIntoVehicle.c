[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionMoveAIIntoVehicle : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "Vehicle Getter")]
	ref SCR_ScenarioFrameworkGet m_VehicleGetter;
	
	[Attribute(defvalue: "1", desc: "Allow partial move of AI units, if there is not enough compartments for whole group.")]
	bool m_bAllowPartialMove;
	
	[Attribute(defvalue: "0", desc: "Check Slots for compartments. (For example, turret on M1025)")]
	bool m_bCheckSlots;
	
	[Attribute("1", UIWidgets.CheckBox, "Occupy driver")]
	bool m_bAllowDriver;

	[Attribute("1", UIWidgets.CheckBox, "Occupy gunner")]
	bool m_bAllowGunner;

	[Attribute("1", UIWidgets.CheckBox, "Occupy cargo")]
	bool m_bAllowCargo;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_AIGroup targetAIGroup, IEntity entity)
	{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_VehicleGetter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);

			return;
		}

		array<AIAgent> agents = {};
		targetAIGroup.GetAgents(agents);
		if (agents.IsEmpty())
			return;
		
		IEntity vehicleEntity = entityWrapper.GetValue();
		if (!vehicleEntity)
			return;
		
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(vehicleEntity.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetCompartments(compartments);
		
		if (m_bCheckSlots)
			GetNestedCompartmentSlots(vehicleEntity, compartments);
		
		for (int i = compartments.Count()-1; i >= 0; i--)
		{
			if (!FilterCompartmentSlot(compartments[i]))
				compartments.Remove(i);
		}
		
		if (!m_bAllowPartialMove && (compartments.Count() > agents.Count()))
			return;
		
		IEntity agentEnt;
		SCR_CompartmentAccessComponent compartmentAccess;
		foreach (int i, AIAgent agent : agents)
		{
			agentEnt = agent.GetControlledEntity();
			if (!agentEnt)
				continue;
			
			compartmentAccess = SCR_CompartmentAccessComponent.Cast(agentEnt.FindComponent(SCR_CompartmentAccessComponent));
			if (!compartmentAccess)
				continue;
		
			if (compartments.IsIndexValid(i))
				compartmentAccess.GetInVehicle(vehicleEntity, compartments[i], true, -1, ECloseDoorAfterActions.CLOSE_DOOR, false);
		}
			
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FilterCompartmentSlot(notnull BaseCompartmentSlot slot)
	{
		if (slot.GetOccupant())
			return false;
		
		switch (slot.GetType())
		{
			case ECompartmentType.PILOT:
				return m_bAllowDriver;
			
			case ECompartmentType.TURRET:
				return m_bAllowGunner;
			
			case ECompartmentType.CARGO:
				return m_bAllowCargo;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetNestedCompartmentSlots(notnull IEntity owner, notnull out array<BaseCompartmentSlot> outCompartments)
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slotInfos = {};
		slotManager.GetSlotInfos(slotInfos);
		
		IEntity attachedEntity;
		BaseCompartmentManagerComponent compartmentManager;
		array<BaseCompartmentSlot> compartments = {};
		
		foreach (EntitySlotInfo slotInfo : slotInfos)
		{
			attachedEntity = slotInfo.GetAttachedEntity();
			if (!attachedEntity)
				continue;
			
			GetNestedCompartmentSlots(attachedEntity, outCompartments);
			
			compartmentManager = BaseCompartmentManagerComponent.Cast(attachedEntity.FindComponent(BaseCompartmentManagerComponent));
			if (!compartmentManager)
				continue;
			
			compartmentManager.GetCompartments(compartments);
			outCompartments.InsertAll(compartments);
		}
	}
}