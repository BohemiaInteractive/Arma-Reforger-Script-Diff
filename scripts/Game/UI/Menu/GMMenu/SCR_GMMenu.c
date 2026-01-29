
class SCR_GMMenu : ChimeraMenuBase
{
	protected const ResourceName TILES_LAYOUT = "{818240FE79AF0D49}UI/layouts/Menus/GMMenu/MainMenuTile_VerticalLarge.layout";
	protected const ResourceName CONFIG = "{CA59D3A983A1BBAB}Configs/GMMenu/GMMenuEntries.conf";
	
	protected SCR_GalleryComponent m_Gallery;
	protected WorkshopApi m_WorkshopAPI;
	protected SCR_GMMenuTileComponent m_CurrentTile;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_Gallery = SCR_GalleryComponent.GetGalleryComponent("Gallery", GetRootWidget());
		m_WorkshopAPI = GetGame().GetBackendApi().GetWorkshop();

		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, GetRootWidget());
		if (back)
			back.m_OnActivated.Insert(OnBack);

		bool disableContent = GetGame().GetGameInstallStatus() != 1.0;
		PrepareTiles(disableContent);

		if (m_Gallery)
			m_Gallery.SetFocusedItem(0, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		if (m_CurrentTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_CurrentTile.GetRootWidget());

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareTiles(bool isContentDisabled = false)
	{
		Resource resource = BaseContainerTools.LoadContainer(CONFIG);
		if (!resource)
			return;

		BaseContainer container = resource.GetResource().ToBaseContainer();
		SCR_GMMenuConfiguration menuConf = SCR_GMMenuConfiguration.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!menuConf)
			return;
		
		Widget root = GetRootWidget().FindAnyWidget("ScenariosHorizontalLayout");

		array<ResourceName> missions = menuConf.GetScenarios();

		foreach (ResourceName mission : missions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(TILES_LAYOUT, root);
			if (!w)
				continue;

			SCR_MainMenuTileComponent tile = SCR_MainMenuTileComponent.Cast(w.GetChildren().FindHandler(SCR_MainMenuTileComponent));
			if (!tile)
				continue;

			tile.ShowMission(SCR_ScenarioUICommon.GetInGameScenario(mission));
			if (isContentDisabled)
				tile.DisableTile();
		}
		
		Widget focusWidget = root.GetChildren().GetChildren();
		if (focusWidget)
			GetGame().GetWorkspace().SetFocusedWidget(focusWidget);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocused(SCR_GMMenuTileComponent tile)
	{
		m_CurrentTile = tile;
	}
}
