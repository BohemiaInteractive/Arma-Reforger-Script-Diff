class SCR_DLCMenuUI : SCR_SuperMenuBase
{
	protected const ResourceName TILES_LAYOUT = "{D9EECF471236C774}UI/layouts/Menus/ContentBrowser/CampaignMenu/DLCTile_Vertical.layout";
	protected const ResourceName CONFIG = "{F92CCBE06F1054D8}Configs/MainMenu/DLCMenuEntries.conf";
	
	protected ref array<SCR_CampaignTileComponent> m_aTiles = {};
	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		SCR_InputButtonComponent buttonBack = m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK);
		if (buttonBack)
			buttonBack.m_OnActivated.Insert(Close);

		// Refresh licenses
		DlcManager manager = DlcManager.Get();
		if (manager)
			manager.RefreshLicenses();
		
		bool disableContent = GetGame().GetGameInstallStatus() != 1.0;
		PrepareTiles(disableContent);
		manager.OnLicenseGrantedInvoker.Insert(UpdateMenuDLC);
		
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
	void UpdateMenuDLC(EDlc dlc)
	{
		if (dlc != EDlc.RALLY)
			return;
		
		bool disableContent = GetGame().GetGameInstallStatus() != 1.0;
		UpdateTilesLockState(disableContent);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTilesLockState(bool isContentDisabled)
	{	
		foreach (SCR_CampaignTileComponent tile : m_aTiles)
		{
			if (!tile)
				continue;
			
			if (isContentDisabled)
			{
				tile.DisableTile();
				return;
			}

			if (!DlcManager.Get().HasOwnership(EDlc.RALLY))
				tile.DisableTile();
			else
				tile.EnableTile();
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
		Widget focusWidget = null;
		
		foreach (int i, ResourceName mission : missions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(TILES_LAYOUT, scenarioWidgetParent);
			if (!w)
				continue;

			SCR_CampaignTileComponent tile = SCR_CampaignTileComponent.Cast(w.GetChildren().FindHandler(SCR_CampaignTileComponent));
			if (!tile)
				continue;
			
			m_aTiles.Insert(tile);
			
			if (i == 0)
				AlignableSlot.SetPadding(w, 0, 0, 0, 0);

			MissionWorkshopItem item = SCR_ScenarioUICommon.GetInGameScenario(mission);
			tile.ShowMission(item);

			if (!focusWidget)
				focusWidget = w.GetChildren(); // Not sure why children, but we pick that as our focus

			if (!DlcManager.Get().HasOwnership(EDlc.RALLY))
				tile.DisableTile();

			if (isContentDisabled)
				tile.DisableTile();

		}
		
		/*
		When opening the menu with DLC locked it would induce disabled -> enabled.
		The focused tile is not "focused" in full way - not showing play button and description.
		Did not figured out how to fix it, but it happens only if the Rally was bought while being in main menu.
		*/
		GetGame().GetWorkspace().SetFocusedWidget(focusWidget);
	}
	
	override void OnMenuClose()
	{
		DlcManager manager = DlcManager.Get();
		if (manager)
			manager.OnLicenseGrantedInvoker.Remove(UpdateMenuDLC);

		super.OnMenuClose();
	}
}
