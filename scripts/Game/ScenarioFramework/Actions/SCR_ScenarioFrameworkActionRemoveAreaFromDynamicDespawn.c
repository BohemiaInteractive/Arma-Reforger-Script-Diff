[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRemoveAreaFromDynamicDespawn : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target Area to remove")]
	ref SCR_ScenarioFrameworkGetLayerBase		m_Getter;

	[Attribute(desc: "If set to false, area will be despawned")]
	bool m_bStaySpawned;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entityFrom = entityWrapper.GetValue();
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Area for Action %1.", this), LogLevel.ERROR);
			return;
		}

		scenarioFrameworkSystem.RemoveAreaSpecificDynamicDespawn(area, m_bStaySpawned);
	}
}