[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetFuelConsumption : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate fuel consumption (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "20", desc: "Fuel consumption at max power RPM\n[liters/hour]", params: "0 inf")]
	float m_fFuelConsumption;
	
	[Attribute(defvalue: "0.5", desc: "Fuel consumption idle\n[liters/hour]", params: "0 inf")]
	float m_fFuelConsumptionIdle;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_FuelConsumptionComponent fuelConsumptionComponent = SCR_FuelConsumptionComponent.Cast(entity.FindComponent(SCR_FuelConsumptionComponent));
		if (!fuelConsumptionComponent)
		{
			if (object)
				Print(string.Format("ScenarioFramework Action: No Fuel Consumption Component found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: No Fuel Consumption Components found for Action %1.", this), LogLevel.ERROR);

			return;
		}
		
		SCR_FuelConsumptionComponentClass componentData = SCR_FuelConsumptionComponentClass.Cast(fuelConsumptionComponent.GetComponentData(entity));
		if (!componentData)
			return;

		componentData.m_fFuelConsumption = m_fFuelConsumption;
		componentData.m_fFuelConsumptionIdle = m_fFuelConsumptionIdle;
	}
}