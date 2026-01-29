[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionVehicle : SCR_ScenarioFrameworkActionBase
{
    [Attribute(desc: "Target entity for Vehicle Action")];
	ref SCR_ScenarioFrameworkGet m_Getter;

    [Attribute(desc: "Vehicle actions that will be executed on target entity")];
	ref array<ref SCR_ScenarioFrameworkVehicleActionBase> m_aVehicleActions;

    //------------------------------------------------------------------------------------------------
    override void OnActivate(IEntity object)
    {
        if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		Vehicle vehicle = Vehicle.Cast(entity);
		if (!vehicle)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Vehicle for Action %1.", this), LogLevel.ERROR);
			return;
		}

		foreach (SCR_ScenarioFrameworkVehicleActionBase vehicleAction : m_aVehicleActions)
		{
			vehicleAction.Init(vehicle);
		}
    }
}