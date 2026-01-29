class SCR_GMSaveDialog : SCR_ScenarioConfirmationDialogUi
{
	protected WorldSaveItem m_Save;

	//! OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		
	}

	//! PROTECTED

	//------------------------------------------------------------------------------------------------
	protected void FillSaveDetails()
	{
		if (!m_Save)
			return;

		//! Update visuals
		SetTitle(m_Save.Name());

		Widget backgroundImageBackend = GetRootWidget().FindAnyWidget("BackgroundImageBackend");
		if (backgroundImageBackend)
		{
			SCR_ScenarioBackendImageComponent backendImageComp = SCR_ScenarioBackendImageComponent.Cast(backgroundImageBackend.FindHandler(SCR_ScenarioBackendImageComponent));
			if (backendImageComp)
				backendImageComp.SetImage(m_Save.Thumbnail());
		}

		//! Content layout
		Widget contentLayoutRoot = GetContentLayoutRoot();
		if (!contentLayoutRoot)
			return;

		TextWidget name = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("m_SourceNameText"));
		TextWidget description = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("m_SourceDescriptionText"));
		Widget sourceImageOfficial = contentLayoutRoot.FindAnyWidget("SourceImageOfficialOverlay");
		Widget sourceImageCommunity = contentLayoutRoot.FindAnyWidget("SourceImageCommunityOverlay");

		//! Source addon
		WorkshopItem sourceAddon = m_Scenario.GetOwner();
		
		name.SetText(m_Scenario.Name());
		description.SetText(m_Scenario.Description()); 

		sourceImageOfficial.SetVisible(sourceAddon == null);
		sourceImageCommunity.SetVisible(sourceAddon != null);

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
		UpdateFavoriteWidgets(m_Save.IsFavorite());
	}
	
	//! PUBLIC
	//------------------------------------------------------------------------------------------------
	void SetSave(WorldSaveItem save)
	{
		m_Save = save;
		
		// Scenario 
		Revision activeRevision = save.GetActiveRevision();
		if (activeRevision)
		{
			array<MissionWorkshopItem> missions = {};
			activeRevision.GetScenarios(missions);
			
			if (!missions.IsEmpty())
				m_Scenario = missions[0];
		}
		
		FillSaveDetails();
	}
}