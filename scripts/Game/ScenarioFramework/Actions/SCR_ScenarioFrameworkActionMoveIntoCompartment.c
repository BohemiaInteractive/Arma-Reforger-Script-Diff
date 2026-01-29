[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionMoveIntoCompartment : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity that will be moved into target entity.")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Target entity.")]
	ref SCR_ScenarioFrameworkGet m_GetterTargetEntity;

	[Attribute(defvalue: "0", desc: "Index of the target entity's compartment.", params: "0 inf")]
	int m_iCompartmentIndex;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		IEntity targetEntity;
		if (!ValidateInputEntity(object, m_GetterTargetEntity, targetEntity))
			return;

		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(targetEntity.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetCompartments(compartments);

		if (m_iCompartmentIndex >= compartments.Count())
		{
			Print(string.Format("ScenarioFramework Action: Number of available compartments for entity %2 Action %1 is lower than requested index. Max number is %3. Action won't do anything.", this, targetEntity.GetName(), compartments.Count()), LogLevel.ERROR);
			return;
		}

		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(entity.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
		
		compartmentAccess.GetInVehicle(targetEntity, compartments[m_iCompartmentIndex], true, -1, ECloseDoorAfterActions.INVALID, true);
	}
}
