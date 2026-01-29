[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeLayerTerminationStatus : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the layer to change the termination status")]
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;

	[Attribute(desc: "If layer will be terminated or not")]
	bool m_bTerminated;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layer)
				layer.SetIsTerminated(m_bTerminated);

			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Getter has issues for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity could not be found for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not LayerBase for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		layer.SetIsTerminated(m_bTerminated);
	}
}