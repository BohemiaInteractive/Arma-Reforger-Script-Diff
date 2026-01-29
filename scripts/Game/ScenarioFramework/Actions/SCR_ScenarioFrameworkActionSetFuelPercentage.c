[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetFuelPercentage : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate fuel (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "75", desc: "Percentage of a fuel to be set.", UIWidgets.Graph, "0 100 1")]
	int m_iFuelPercentage;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		array<SCR_FuelManagerComponent> fuelManagers = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(entity, fuelManagers);
		if (fuelManagers.IsEmpty())
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: No Fuel Managers found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: No Fuel Managers found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		SCR_FuelManagerComponent.SetTotalFuelPercentageOfFuelManagers(fuelManagers, m_iFuelPercentage * 0.01, 0, SCR_EFuelNodeTypeFlag.IS_FUEL_STORAGE);
	}
}