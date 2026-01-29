//! Briefing config root
[BaseContainerProps(configRoot: true)]
class SCR_BriefingMenuConfig
{
	[Attribute("", UIWidgets.Object, "Game mode hints")]
	protected ref array<ref SCR_UIInfo> m_aGameModeHints;
	
	[Attribute("", UIWidgets.Object, "Victory conditions")]
	protected ref array<ref SCR_BriefingVictoryCondition> m_aWinConditions;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_UIInfo> GetHints()
	{
		return m_aGameModeHints;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_BriefingVictoryCondition> GetWinConditions()
	{
		return m_aWinConditions;
	}
}
