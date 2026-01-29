class SCR_CampaignMenuUI : SCR_SuperMenuBase
{
	protected const ResourceName TILES_LAYOUT = "{056BE72745839C20}UI/layouts/Menus/ContentBrowser/CampaignMenu/CampaignTile_Vertical.layout";
	protected const ResourceName CONFIG = "{0B1DDB81597DFF04}Configs/ContentBrowser/MainMenu/CampaignMenuEntries.conf";
	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		SCR_InputButtonComponent buttonBack = m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK);
		if (buttonBack)
			buttonBack.m_OnActivated.Insert(Close);
		
		bool disableContent = GetGame().GetGameInstallStatus() != 1.0;
		PrepareTiles(disableContent);
		
		if (System.GetPlatform() == EPlatform.PS5 || System.GetPlatform() == EPlatform.PS4 || System.GetPlatform() == EPlatform.PS5_PRO)
		{
			// Hide tabs for PS
			if (m_SuperMenuComponent && m_SuperMenuComponent.GetTabView())
				m_SuperMenuComponent.GetTabView().RemoveTab(EScenarioSubMenuMode.MODE_SAVES);
			
			// Hide news menu button (top right corner) on PS
			Widget newsButton = GetRootWidget().FindAnyWidget("NewsButton");
			if (newsButton)
			{
				newsButton.SetVisible(false);
				newsButton.SetEnabled(false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareTiles(bool isContentDisabled = false)
	{
		Resource resource = BaseContainerTools.LoadContainer(CONFIG);
		if (!resource)
			return;

		BaseContainer container = resource.GetResource().ToBaseContainer();
		SCR_CampaignMenuConfiguration menuConf = SCR_CampaignMenuConfiguration.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!menuConf)
			return;

		array<ResourceName> missions = menuConf.m_aCampaignMenuScenarios;
		
		Widget scenarioWidgetParent = GetRootWidget().FindAnyWidget("ScenariosHorizontalLayout");
		
		foreach (int i, ResourceName mission : missions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(TILES_LAYOUT, scenarioWidgetParent);
			if (!w)
				continue;

			SCR_CampaignTileComponent tile = SCR_CampaignTileComponent.Cast(w.GetChildren().FindHandler(SCR_CampaignTileComponent));
			if (!tile)
				continue;
			
			if (i == 0)
				AlignableSlot.SetPadding(w, 0, 0, 0, 0);

			MissionWorkshopItem item = SCR_ScenarioUICommon.GetInGameScenario(mission);
			
			tile.ShowMission(item);
			if (tile.UpdateCampaignButtons())
			{
				Widget focusWidget = w.GetChildren();
				if (focusWidget)
					GetGame().GetWorkspace().SetFocusedWidget(focusWidget);
			}
			
			if (isContentDisabled)
				tile.DisableTile();
		}	
	}
}
