[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionToggleEngine : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to turn on/off the engine (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "1", desc: "If true, engine will be turned on. Otherwise it will turn it off.")]
	bool m_bTurnedOn;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		VehicleWheeledSimulation vehicleSimulation = VehicleWheeledSimulation.Cast(entity.FindComponent(VehicleWheeledSimulation));
		if (!vehicleSimulation)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Vehicle Wheeled Simulation Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Vehicle Wheeled Simulation Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		if (m_bTurnedOn)
			vehicleSimulation.EngineStart();
		else
			vehicleSimulation.EngineStop();
	}
}