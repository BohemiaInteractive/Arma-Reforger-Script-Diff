/*
Component for a line in scenario browser.
*/
class SCR_ContentBrowser_ScenarioLineComponent : SCR_BrowserListMenuEntryComponent
{
	protected ref SCR_ContentBrowser_ScenarioLineWidgets m_Widgets = new SCR_ContentBrowser_ScenarioLineWidgets;

	protected MissionWorkshopItem m_Mission;
	
	protected bool m_bMouseButtonsEnabled = true;
	
	protected const string MOD_ICON = "modIcon";
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Widgets.Init(w);

		if (!GetGame().InPlayMode())
			return;

		// Mouse interaction buttons
		m_aMouseButtons.Insert(m_Widgets.m_PlayComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_ContinueComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_RestartComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_HostComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_FindServersComponent0);

		m_Widgets.m_PlayComponent0.m_OnClicked.Insert(OnPlay);
		m_Widgets.m_ContinueComponent0.m_OnClicked.Insert(OnContinue);
		m_Widgets.m_RestartComponent0.m_OnClicked.Insert(OnRestart);
		m_Widgets.m_HostComponent0.m_OnClicked.Insert(OnHost);
		m_Widgets.m_FindServersComponent0.m_OnClicked.Insert(OnFindServers);

		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		if (!m_Mission)
			return;

		SCR_ScenarioUICommon.UpdateMouseButtonTooltips(m_CurrentTooltip, m_Mission);
		
		SCR_ERevisionAvailability availability = SCR_ScenarioUICommon.GetOwnerRevisionAvailability(m_Mission);
		bool inError = availability != SCR_ERevisionAvailability.ERA_AVAILABLE && availability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		
		if (m_Widgets.m_wSourceImageCommunity.IsVisible())
		{
			string icon = MOD_ICON;
			Color color = Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());
			
			if (inError)
			{
				icon = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(availability);
				if (m_bFocused)
					color = Color.FromInt(UIColors.WARNING.PackToInt());
				else
					color = Color.FromInt(UIColors.WARNING_DISABLED.PackToInt());
			}
			
			m_Widgets.m_wSourceImageCommunity.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
			m_Widgets.m_wSourceImageCommunity.SetIsColorInherited(!inError);
			m_Widgets.m_wSourceImageCommunity.SetColor(color);
		}

		SCR_ScenarioUICommon.UpdatePlayMouseButton(m_Widgets.m_PlayComponent0, m_Mission, m_bFocused, m_bMouseButtonsEnabled);
		SCR_ScenarioUICommon.UpdateContinueMouseButton(m_Widgets.m_ContinueComponent0, m_Mission, m_bFocused, m_bMouseButtonsEnabled);
		SCR_ScenarioUICommon.UpdateRestartMouseButton(m_Widgets.m_RestartComponent0, m_Mission, m_bFocused, m_bMouseButtonsEnabled);
		SCR_ScenarioUICommon.UpdateHostMouseButton(m_Widgets.m_HostComponent0, m_Mission, m_bFocused, m_bMouseButtonsEnabled);
		SCR_ScenarioUICommon.UpdateJoinMouseButton(m_Widgets.m_FindServersComponent0, m_Mission, m_bFocused,m_bMouseButtonsEnabled);
		
		super.UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	override bool SetFavorite(bool favorite)
	{
		if (!m_Mission || !super.SetFavorite(favorite))
			return false;

		m_Mission.SetFavorite(favorite);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		SCR_ScenarioUICommon.UpdateMouseButtonTooltips(tooltip, m_Mission);
		
		super.OnTooltipShow(tooltip);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		if (!m_Mission)
			return;

		// Fav button
		if (m_Mission)
			SetFavorite(m_Mission.IsFavorite());

		//! Name
		m_Widgets.m_wNameText.SetText(m_Mission.Name());

		//! Game mode
		/*
		// Not supported yet - todo
		*/

		//! Type/player count
		int playerCount = m_Mission.GetPlayerCount();
		bool mp = SCR_ScenarioUICommon.IsMultiplayer(m_Mission);
	
		m_Widgets.m_wSinglePlayerImage.SetVisible(!m_bMouseButtonsEnabled && !mp);
		m_Widgets.m_wMultiPlayerImage.SetVisible(!m_bMouseButtonsEnabled && mp);
		m_Widgets.m_wPlayerCountText.SetText(playerCount.ToString());

		//! Last played time text
		int timeSinceLastPlayedSeconds = m_Mission.GetTimeSinceLastPlay();
		string timeSinceLastPlayText;

		if (timeSinceLastPlayedSeconds > 0)
			timeSinceLastPlayText = SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlayedSeconds);
		else
			timeSinceLastPlayText = "-";

		m_Widgets.m_wLastPlayedText.SetText(timeSinceLastPlayText);

		//! Source addon
		WorkshopItem sourceAddon = m_Mission.GetOwner();
		m_Widgets.m_wSourceImageOfficial.SetVisible(sourceAddon == null);
		m_Widgets.m_wSourceImageCommunity.SetVisible(sourceAddon != null);
		m_Widgets.m_wSourceNameTextOfficial.SetVisible(sourceAddon == null);
		m_Widgets.m_wSourceNameTextCommunity.SetVisible(sourceAddon != null);
		string sourceAddonText = string.Empty;
		if (sourceAddon)
			m_Widgets.m_wSourceNameTextCommunity.SetText(sourceAddon.Name());
		else
			m_Widgets.m_wSourceNameTextOfficial.SetText("#AR-Editor_Attribute_OverlayLogo_Reforger");

		//! Modular buttons
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_PLAY);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_CONTINUE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestart()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_RESTART);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHost()
	{
		if (!SCR_ScenarioUICommon.CanHost(m_Mission))
			return;
		
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_HOST);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServers()
	{
		if (!SCR_ScenarioUICommon.CanJoin(m_Mission))
			return;
		
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_FIND_SERVERS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateModularButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Must be called when the scenario properties have been updated (set/clear favourite or similar)
	void NotifyScenarioUpdate(bool itemDownloaded)
	{
		m_bMouseButtonsEnabled = itemDownloaded;
		UpdateModularButtons();
		
		m_Widgets.m_wFavouriteButtonOverlay.SetVisible(itemDownloaded);
		m_Widgets.m_wLastPlayedTextSize.SetVisible(itemDownloaded);
		
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem mission)
	{
		m_Mission = mission;

		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetScenario()
	{
		return m_Mission;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowser_ScenarioLineComponent FindComponent(Widget w)
	{
		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}
}