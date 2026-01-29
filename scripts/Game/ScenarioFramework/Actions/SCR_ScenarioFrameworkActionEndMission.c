[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionEndMission : SCR_ScenarioFrameworkActionBase
{
	[Attribute(UIWidgets.CheckBox, desc: "If true, it will override any previously set game over type with selected one down bellow")]
	bool		m_bOverrideGameOverType;

	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))]
	EGameOverTypes			m_eOverriddenGameOverType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		if (m_bOverrideGameOverType)
			scenarioFrameworkSystem.SetMissionEndScreen(m_eOverriddenGameOverType);

		scenarioFrameworkSystem.Finish();
	}
}