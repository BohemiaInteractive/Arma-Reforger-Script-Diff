/*!
Workshop sub menu for displaying page of saves 

!! This is prototype implementation to display saves separatelly. With proper implementation it should be possible to load both mod and save items.
*/
class SCR_ContentBrowser_SavesSubMenu : SCR_ContentBrowser_AddonsSubMenu
{
	protected int m_iSavesPageCount;
	
	protected SCR_ContentBrowserDetails_SaveOverviewSubMenu m_OpenedSaveDetOverview;
	
	//------------------------------------------------------------------------------------------------
	// Inits workshop API according to current mode
	override protected void InitWorkshopApi()
	{
		super.InitWorkshopApi();
		
		m_bClearCacheAtNextRequest = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PackTagFilterData(SCR_ContentBrowser_GetAssetListParams params, bool defaultValues)
	{
		// Preventing behavior as no tags are used
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void RequestOnlinePage(int pageId)
	{	
		if (!m_GetAssetListParams)
			m_GetAssetListParams = new SCR_ContentBrowser_GetAssetListParams(this);
		
		super.RequestOnlinePage(pageId);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnTileClick(SCR_ScriptedWidgetComponent baseComp)
	{
		super.OnTileClick(baseComp);
		
		ContentBrowserDetailsMenu detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.ContentBrowserDetailsMenuSave));
		if (!detailsMenu)
			return;
		
		SCR_SuperMenuComponent superMenuComp = detailsMenu.GetSuperMenu();
		if (!superMenuComp)
			return;
		
		SCR_ContentBrowserDetails_SaveOverviewSubMenu saveOverview = SCR_ContentBrowserDetails_SaveOverviewSubMenu.Cast(superMenuComp.GetSubMenu(0));
		if (!saveOverview)
			return;
		
		
		m_OpenedSaveDetOverview = saveOverview;
		
		m_OpenedSaveDetOverview.GetItemDeleted().Insert(OnSaveItemDeleted);
		detailsMenu.GetOnMenuClose().Insert(OnDetailsMenuClosed);
		//m_OpenedSaveDetOverview.GetOnRequestCloseMenu().Insert(OnSavePreviewClose);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void Callback_OnRequestPageGetAssets()
	{
		super.Callback_OnRequestPageGetAssets();
		
		if (m_iOnlineFilteredItems == 0)
			SetPanelsMode(false, messagePresetTag: MESSAGE_TAG_NO_SAVES_FOUND, forceFiltersList: true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSaveItemDeleted(WorkshopItem item)
	{
		RequestPage(m_iCurrentPage);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDetailsMenuClosed()
	{
		m_OpenedSaveDetOverview.GetItemDeleted().Remove(OnSaveItemDeleted);
		m_OpenedSaveDetOverview = null;
		
		ContentBrowserDetailsMenu detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.ContentBrowserDetailsMenuSave));
		if (!detailsMenu)
			return;
		
		detailsMenu.GetOnMenuClose().Insert(OnDetailsMenuClosed);
	}
}