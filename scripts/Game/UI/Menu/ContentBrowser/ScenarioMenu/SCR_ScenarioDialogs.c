/*!
Classes for Scenario dialogs
*/
//------------------------------------------------------------------------------------------------
class SCR_ScenarioDialogs
{
	static protected const ResourceName DIALOGS_CONFIG = "{F020A20CC93DB3C7}Configs/ContentBrowser/ScenarioDialogs.conf";

	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ScenarioConfirmationDialogUi CreateScenarioConfirmationDialog(MissionWorkshopItem scenario, ScriptInvokerBool onFavoritesResponse = null)
	{
		SCR_ScenarioConfirmationDialogUi dialogUI = new SCR_ScenarioConfirmationDialogUi(scenario, onFavoritesResponse);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "SCENARIO_CONFIRMATION", dialogUI);

		return dialogUI;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioConfirmationDialogUi : SCR_ConfigurableDialogUi
{
	MissionWorkshopItem m_Scenario;

	protected SCR_InputButtonComponent m_Favorite;
	protected SCR_InputButtonComponent m_Host;
	protected SCR_InputButtonComponent m_FindServers;
	protected ref array<SCR_InputButtonComponent> m_aRightFooterButtons = {};
	
	protected SCR_ModularButtonComponent m_FavoriteStarButton;

	//! If true, the dialog itself will set the scenario favorite state, otherwise it will live it to the menu or handler class
	protected bool m_bHandleFavoriting = true;

	protected ref ScriptInvokerMissionWorkshopItem m_OnFavorite;

	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioConfirmationDialogUi(MissionWorkshopItem scenario, ScriptInvokerBool onFavoritesResponse = null)
	{
		m_Scenario = scenario;

		if (onFavoritesResponse)
		{
			m_bHandleFavoriting = false;
			onFavoritesResponse.Insert(UpdateFavoriteWidgets);
		}
	}

	//! OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		if (!m_Scenario)
			return;

		// Connection state
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		//! Update visuals
		SetTitle(m_Scenario.Name());

		Widget backgroundImageBackend = GetRootWidget().FindAnyWidget("BackgroundImageBackend");
		if (backgroundImageBackend)
		{
			SCR_ScenarioBackendImageComponent backendImageComp = SCR_ScenarioBackendImageComponent.Cast(backgroundImageBackend.FindHandler(SCR_ScenarioBackendImageComponent));
			if (backendImageComp)
				backendImageComp.SetImage(m_Scenario.Thumbnail());
		}

		//! Content layout
		Widget contentLayoutRoot = GetContentLayoutRoot();
		if (!contentLayoutRoot)
			return;

		Widget singlePlayerImage = contentLayoutRoot.FindAnyWidget("SinglePlayerImageOverlay");
		Widget multiPlayerImage = contentLayoutRoot.FindAnyWidget("MultiPlayerImageOverlay");
		TextWidget playerCountLabelText = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("PlayerCountLabelText"));

		Widget sourceImageOfficial = contentLayoutRoot.FindAnyWidget("SourceImageOfficialOverlay");
		Widget sourceImageCommunity = contentLayoutRoot.FindAnyWidget("SourceImageCommunityOverlay");
		TextWidget sourceNameTextOfficial = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("SourceNameTextOfficial"));
		TextWidget sourceNameTextCommunity = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("SourceNameTextCommunity"));

		//! Type and player count
		int playerCount = m_Scenario.GetPlayerCount();
		bool mp = SCR_ScenarioUICommon.IsMultiplayer(m_Scenario);
		singlePlayerImage.SetVisible(!mp);
		multiPlayerImage.SetVisible(mp);
		
		playerCountLabelText.SetText(WidgetManager.Translate("#AR-Scenario_Players", playerCount.ToString()));
		
		//! Source addon
		bool isSourceAddonValid;
		WorkshopItem sourceAddon = m_Scenario.GetOwner();

		if (sourceAddon)
		{
			isSourceAddonValid = true;
			sourceNameTextCommunity.SetText(sourceAddon.Name());
		}
		else
		{
			isSourceAddonValid = false;
		}

		sourceImageOfficial.SetVisible(!isSourceAddonValid);
		sourceImageCommunity.SetVisible(isSourceAddonValid);
		sourceNameTextOfficial.SetVisible(!isSourceAddonValid);
		sourceNameTextCommunity.SetVisible(isSourceAddonValid);

		//! Footer Buttons
		SCR_ScenarioUICommon.UpdateInputButtons(m_Scenario, m_aRightFooterButtons);
		
		m_FindServers = FindButton(SCR_ScenarioUICommon.BUTTON_FIND_SERVERS);
		m_Host = FindButton(SCR_ScenarioUICommon.BUTTON_HOST);

		m_Favorite = FindButton(SCR_ScenarioUICommon.BUTTON_FAVORITE);
		if (m_Favorite)
			m_Favorite.m_OnActivated.Insert(OnFavoritesButton);

		//! Star button
		Widget favButton = m_wRoot.FindAnyWidget("FavoriteButton");
		if (favButton)
		{
			m_FavoriteStarButton = SCR_ModularButtonComponent.FindComponent(favButton);
			if (m_FavoriteStarButton)
				m_FavoriteStarButton.m_OnClicked.Insert(OnFavoritesButton);
		}

		//! Favorites widgets update
		UpdateFavoriteWidgets(m_Scenario.IsFavorite());
	}

	//----------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnButtonPressed(SCR_InputButtonComponent button)
	{
		if (GetButtonTag(button) == SCR_ScenarioUICommon.BUTTON_FAVORITE)
			return; // We use a specific invoker for favorites button
		
		super.OnButtonPressed(button);

		bool close = true;
		switch (m_sLastPressedButtonTag)
		{
			case SCR_ScenarioUICommon.BUTTON_FIND_SERVERS:	close = SCR_ScenarioUICommon.CanJoin(m_Scenario); break;
			case SCR_ScenarioUICommon.BUTTON_HOST:			close = SCR_ScenarioUICommon.CanHost(m_Scenario); break;
		}
		
		if (close)
			Close();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_InputButtonComponent CreateButton(SCR_ConfigurableDialogUiButtonPreset buttonPreset)
	{
		SCR_InputButtonComponent comp = super.CreateButton(buttonPreset);
		
		Widget w;
		if (comp)
			w = comp.GetRootWidget();
		
		if (w)
			w.SetName(buttonPreset.m_sTag);
		
		if (buttonPreset.m_eAlign == EConfigurableDialogUiButtonAlign.RIGHT)
			m_aRightFooterButtons.Insert(comp);
		
		return comp;
	}
	
	//! PROTECTED
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		SCR_ScenarioUICommon.UpdateJoinInputButton(m_FindServers, m_Scenario);
		SCR_ScenarioUICommon.UpdateHostInputButton(m_Host, m_Scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFavoritesButton()
	{
		if (!m_Scenario)
			return;

		if (!m_bHandleFavoriting && m_OnFavorite)
		{
			m_OnFavorite.Invoke(m_Scenario);
			return;
		}

		bool isFavorite = !m_Scenario.IsFavorite();
		m_Scenario.SetFavorite(isFavorite);
		UpdateFavoriteWidgets(isFavorite);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFavoriteWidgets(bool isFavorite)
	{
		// Footer Button
		m_Favorite.SetLabel(UIConstants.GetFavoriteLabel(isFavorite));

		// Star Button
		if (m_FavoriteStarButton)
			m_FavoriteStarButton.SetToggled(isFavorite, false);
	}

	//! PUBLIC
	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetScenario()
	{
		return m_Scenario;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerMissionWorkshopItem GetOnFavorite()
	{
		if (!m_OnFavorite)
			m_OnFavorite = new ScriptInvokerMissionWorkshopItem();

		return m_OnFavorite;
	}
}
