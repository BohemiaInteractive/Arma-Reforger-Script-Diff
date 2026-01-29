[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetMissionEndScreen : SCR_ScenarioFrameworkActionBase
{
	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))]
	EGameOverTypes m_eGameOverType;

	[Attribute(desc: "Leave empty when a default value set in SCR_GameOverScreenManagerComponent should be used.")]
	LocalizedString m_sSubtitle;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		scenarioFrameworkSystem.SetMissionEndScreen(m_eGameOverType);

		SCR_GameOverScreenManagerComponent gameOverScreenMgr = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverScreenMgr)
			return;

		SCR_GameOverScreenConfig m_GameOverScreenConfig = gameOverScreenMgr.GetGameOverConfig();
		if (!m_GameOverScreenConfig)
			return;

		SCR_BaseGameOverScreenInfo targetScreenInfo;
		m_GameOverScreenConfig.GetGameOverScreenInfo(m_eGameOverType, targetScreenInfo);
		if (!targetScreenInfo)
			return;

		SCR_BaseGameOverScreenInfoOptional optionalParams = targetScreenInfo.GetOptionalParams();
		if (!optionalParams)
			return;

		if (m_sSubtitle.IsEmpty())
			return;

		optionalParams.m_sSubtitle = m_sSubtitle;
	}
}
