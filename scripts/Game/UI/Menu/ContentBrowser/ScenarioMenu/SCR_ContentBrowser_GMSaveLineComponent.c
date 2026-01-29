/*
Component for a line in scenario browser.
*/
class SCR_ContentBrowser_GMSaveLineComponent : SCR_ContentBrowser_ScenarioLineComponent
{
	protected const string SAVE_ICON = "save";
	
	protected WorldSaveItem m_SaveItem;
	protected SCR_WorkshopItem m_SaveScrWorkshopItem;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPlay()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_PLAY);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnRestart()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_RESTART);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHost()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_HOST);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSaveItem(WorldSaveItem saveItem)
	{
		m_SaveItem = saveItem;
		if (!m_SaveItem)
			return;
		
		m_SaveScrWorkshopItem = SCR_AddonManager.GetInstance().GetItem(m_SaveItem.Id());
		
		// Get scenario - there is always one same scenario in one save for each version
		Revision rev = m_SaveItem.GetLatestRevision();
		if (!rev)
			rev = m_SaveItem.GetLocalRevision();
		
		array<MissionWorkshopItem> missions = {};
		rev.GetScenarios(missions);
		m_Mission = missions[0];
		
		// UI 
		UpdateAllWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	WorldSaveItem GetSave()
	{
		return m_SaveItem; 
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetSaveScrWorkshopItem()
	{
		return m_SaveScrWorkshopItem;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateAllWidgets()
	{
		super.UpdateAllWidgets();
		
		// Show name and update buttons
		m_Widgets.m_wNameText.SetText(m_SaveItem.Name());
		
		m_Widgets.m_HostComponent0.SetVisible(false);
		m_Widgets.m_RestartComponent0.SetVisible(false);
		
		m_Widgets.m_wSourceNameTextCommunity.SetText(m_Mission.Name());
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetScenario(MissionWorkshopItem mission)
	{
		m_Mission = mission;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		super.UpdateModularButtons();
		
		// Show play and hide other buttons
		m_Widgets.m_PlayComponent0.SetVisible(true);
		
		m_Widgets.m_ContinueComponent0.SetVisible(false);
		m_Widgets.m_HostComponent0.SetVisible(false);
		m_Widgets.m_RestartComponent0.SetVisible(false);
		m_Widgets.m_FindServersComponent0.SetVisible(false);
		
		m_Widgets.m_wSourceImageCommunity.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, SAVE_ICON);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetFavorite(bool favorite)
	{
		if (!m_SaveItem || !super.SetFavorite(favorite))
			return false;

		m_SaveItem.SetFavorite(null, favorite);
		return true;
	}
}