[BaseContainerProps()]
class SCR_BaseReconMarkerEntry
{
	[Attribute("1")]
	protected bool m_bEnabled;

	protected SCR_MapMarkerMenuEntry m_MenuEntry;
	protected SCR_MapEntity m_MapEntity;
	protected SCR_MapMarkerManagerComponent m_MarkerMgr;

	//------------------------------------------------------------------------------------------------
	//! \param[in] entry
	void SetMenuEntry(notnull SCR_MapMarkerMenuEntry entry)
	{
		m_MenuEntry = entry;
	}

	//------------------------------------------------------------------------------------------------
	//! \return SCR_MapMarkerMenuEntry
	SCR_MapMarkerMenuEntry GetMenuEntry()
	{
		return m_MenuEntry;
	}

	//------------------------------------------------------------------------------------------------
	//! \return if it can be displayed in the radial menu
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryPerformed(SCR_SelectionMenuEntry entry);

	//------------------------------------------------------------------------------------------------
	void SCR_BaseReconMarkerEntry()
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		m_MarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
	}
}
