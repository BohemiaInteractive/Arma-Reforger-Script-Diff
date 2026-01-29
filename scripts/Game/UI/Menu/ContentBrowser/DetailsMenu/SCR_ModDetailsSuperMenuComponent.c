enum SCR_EModDetailsMenuTabs
{
	OVERVIEW = 0,
	DEPENDENCY,
	DEPENDENT
}

class SCR_ModDetailsSuperMenuComponent : SCR_SuperMenuComponent
{
	protected SCR_WorkshopItem m_WorkshopItem;
	protected ref array<ref SCR_WorkshopItem> m_aDependencies = {};
	protected ref array<ref SCR_WorkshopItem> m_aDependent = {}; 
	
	//------------------------------------------------------------------------------------------------
	override SCR_SubMenuBase OnTabCreate(SCR_TabViewComponent comp, Widget w, int index)
	{
		SCR_SubMenuBase subMenu = super.OnTabCreate(comp, w, index);
		
		// Dependencies tab is created only on click		
		if (index == SCR_EModDetailsMenuTabs.DEPENDENCY)
			InitDependenciesTab();
		
		if (index == SCR_EModDetailsMenuTabs.DEPENDENT)
			InitDependentTab();
		
		return subMenu;
	}
 	
	//------------------------------------------------------------------------------------------------
	override SCR_SubMenuBase OnTabShow(SCR_TabViewComponent comp, Widget w)
	{
		SCR_SubMenuBase subMenu = super.OnTabShow(comp, w);
		SCR_AnalyticsApplication.GetInstance().ModDetailsSetTab(subMenu.GetIndex());
		return subMenu;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		SCR_AnalyticsApplication.GetInstance().OpenModDetails();	
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		if (m_WorkshopItem)
			m_WorkshopItem.m_OnDependenciesLoaded.Remove(OnDetailsLoaded);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		
		SCR_AnalyticsApplication.GetInstance().CloseModDetails();	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWorkshopItem(SCR_WorkshopItem item)
	{
		if (!item)
			return;
		
		if (m_WorkshopItem)
			m_WorkshopItem.m_OnDependenciesLoaded.Remove(OnDetailsLoaded);
		
		m_WorkshopItem = item;
		m_WorkshopItem.m_OnDependenciesLoaded.Insert(OnDetailsLoaded);
		
		LoadItemDetails();
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadItemDetails()
	{
		if (!m_WorkshopItem)
			return;
		
		if (!m_WorkshopItem.GetDetailsLoaded())
			m_WorkshopItem.LoadDetails();
		
		OnDetailsLoaded(m_WorkshopItem);	
	}
	
	//------------------------------------------------------------------------------------------------
	// Called when we have received dependencies
	protected void OnDetailsLoaded(SCR_WorkshopItem item)
	{
		// Details tab - Created at start
		SCR_ContentBrowserDetails_OverviewSubMenu detailsSubMenu = SCR_ContentBrowserDetails_OverviewSubMenu.Cast(GetSubMenu(SCR_EModDetailsMenuTabs.OVERVIEW));
		if (detailsSubMenu)
			detailsSubMenu.SetWorkshopItem(item);
		
		// Dependencies tab - Created on tab button click
		m_aDependencies = item.GetLatestDependencies();
		if (!m_aDependencies.IsEmpty())
			GetTabView().EnableTab(SCR_EModDetailsMenuTabs.DEPENDENCY, true);
		
		// Dependent tab
		m_aDependent = SCR_WorkshopUiCommon.GetDownloadedDependentAddons(item);
		if (!m_aDependent.IsEmpty())
			GetTabView().EnableTab(SCR_EModDetailsMenuTabs.DEPENDENT, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitDependenciesTab()
	{
		if (m_aDependencies.IsEmpty())
			return;

		SCR_ContentBrowser_AddonsSubMenu dependencySubMenu = SCR_ContentBrowser_AddonsSubMenu.Cast(GetSubMenu(SCR_EModDetailsMenuTabs.DEPENDENCY));
		if (dependencySubMenu)
			dependencySubMenu.SetWorkshopItems(m_aDependencies);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitDependentTab()
	{
		if (m_aDependent.IsEmpty())
			return;
		
		SCR_ContentBrowser_AddonsSubMenu dependentSubMenu = SCR_ContentBrowser_AddonsSubMenu.Cast(GetSubMenu(SCR_EModDetailsMenuTabs.DEPENDENT));
		if (dependentSubMenu)
			dependentSubMenu.SetWorkshopItems(m_aDependent);
	}
}