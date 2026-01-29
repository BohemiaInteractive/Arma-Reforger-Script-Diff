class SCR_CampaignTileComponent: SCR_MainMenuTileComponent
{
	protected const string WIDGET_NAME_FINISHED = "Finished";
	protected const string WIDGET_NAME_LOCKED = "LockedPanel";
	
	protected Widget m_wFinished;
	protected Widget m_wLocked;
	
	protected bool m_bIsFinished;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		m_wFinished = w.FindAnyWidget(WIDGET_NAME_FINISHED);
		m_wFinished.SetVisible(false);
		
		m_wLocked = m_wRoot.FindAnyWidget(WIDGET_NAME_LOCKED);
		m_wLocked.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_bDisabled && !m_bIsFinished)
			return super.OnFocus(w, x, y);

		m_bFocused = true;
		float position[4] = {20.0, 0.0, 20.0, 25.0};
		
		AnimateWidget.Padding(m_wTextContent, position, 5.0);
		m_wDescription.SetVisible(true);
		m_wBackground.SetVisible(true);		
		
		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	bool UpdateCampaignButtons()
	{		
		if (SCR_ScenarioSequenceProgress.IsScenarioLocked(m_Item))
			DisableTile();
		else if (SCR_ScenarioSequenceProgress.IsNextScenarioLocked(m_Item))
			SetFinished(true);
		else if (CanPlay() || CanContinue())
		{
			SetLowOpacity();
			return true;
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	override void ShowMission(notnull MissionWorkshopItem item, bool showRecommended = false)
	{
		super.ShowMission(item, showRecommended);
		
		m_wFavoriteIcon.SetVisible(false);
		m_wRecentIcon.SetVisible(false);
		m_wNewIcon.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisableTile()
	{
		super.DisableTile();
		
		m_wLocked.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLowOpacity()
	{
		m_DisabledTileShadow.SetVisible(true);
		m_DisabledTileShadow.SetOpacity(0.3);
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanPlay()
	{
		return m_Play.IsEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanContinue()
	{
		return m_Continue.IsEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsFinished()
	{
		return m_bIsFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFinished(bool state)
	{
		m_bIsFinished = state;
		
		if (m_wFinished)
			m_wFinished.SetVisible(true);
	}
}

