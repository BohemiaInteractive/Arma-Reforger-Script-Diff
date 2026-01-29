[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnEngineStartedOrStop : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If true, we execute actions On Engine Started. Otherwise On Engine Stop")]
	bool m_bStartedOrStop;

	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Actions that will be executed on one of these circumstances", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	void OnEngineStartedOrStop()
	{
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(entity.FindComponent(VehicleControllerComponent));
		if (!vehicleController)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Vehicle Controller Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Vehicle Controller Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		if (m_bStartedOrStop)
			vehicleController.GetOnEngineStart().Insert(OnEngineStartedOrStop);
		else
			vehicleController.GetOnEngineStop().Insert(OnEngineStartedOrStop);
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}