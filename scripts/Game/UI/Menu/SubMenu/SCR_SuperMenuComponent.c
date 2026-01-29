/*!
Handles tab menus. Requires: 
	- TabView component, to handle displaying tabs
	- Each tab layout to have a SCR_SubMenuBase component, to take care of the tab's needs
	- A manager/brain class (like SCR_SuperMenuBase or SCR_TabDialog) to call Init() and the main events

Also determines which button layouts are used in the sub menus, so they are consistent across al tabs
*/

class SCR_SuperMenuComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/WLib_NavigationButton.layout")]
	protected ResourceName m_sNavigationButtonLayout;
	
	[Attribute("TabView", UIWidgets.EditBox, desc: "Name of tabview widget to find tab view component.")]
	protected string m_sTabViewName;

	protected SCR_TabViewComponent m_TabViewComponent;
	
	protected SCR_SubMenuBase m_OpenedSubmenu;
	
	// The index might not be the same as the tab's Elements, as the tabs might be created later or not have a SubMenu component
	protected ref array<SCR_SubMenuBase> m_aSubMenus = {};
	
	Widget m_wMenuRoot;
	
	// Menu requests
	protected ref ScriptInvokerVoid m_OnRequestCloseMenu;

	//------------------------------------------------------------------------------------------------
	// --- Main events, needing to be called by manager classes ---
	//------------------------------------------------------------------------------------------------
	// As parameter, pass a widget the sub menus can use to grab whatever they need from the menu layout (eg the SCR_DynamicFooterComponent)
	void Init(Widget menuRoot)
	{	
		m_wMenuRoot = menuRoot;
		
		// Tab view 
		Widget tabView = m_wRoot.FindAnyWidget(m_sTabViewName); 
		if (tabView)
			m_TabViewComponent = SCR_TabViewComponent.Cast(tabView.FindHandler(SCR_TabViewComponent));
	
		if (!m_TabViewComponent)
			return;
		
		m_TabViewComponent.GetOnContentCreate().Insert(OnTabCreate);
		m_TabViewComponent.GetOnContentShow().Insert(OnTabShow);
		m_TabViewComponent.GetOnContentHide().Insert(OnTabHide);
		m_TabViewComponent.GetOnContentRemove().Insert(OnTabRemove);
		m_TabViewComponent.GetOnTabChange().Insert(OnTabChange);
		
		// Init tabs
		if (m_TabViewComponent.m_bManualInit)
		{
			// Init tab view and rely on it's events
			m_TabViewComponent.Init();
		}
		else
		{
			// Tab view has already created tabs, manually init opened content
			array<ref SCR_TabViewContent> contents = m_TabViewComponent.GetContents();
			
			foreach (int i, SCR_TabViewContent content : contents)
			{
				if (!content.m_wTab)
					continue;
				
				OnTabCreate(m_TabViewComponent, content.m_wTab, i);
				
				if (i == m_TabViewComponent.m_iSelectedTab)
					OnTabShow(m_TabViewComponent, content.m_wTab);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuShow()
	{
		if (m_OpenedSubmenu && m_OpenedSubmenu.GetRootWidget().IsVisible() && m_OpenedSubmenu.GetShown())
			m_OpenedSubmenu.OnMenuShow();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuHide()
	{
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuHide();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuUpdate(float tDelta)
	{
		if (m_OpenedSubmenu && m_OpenedSubmenu.GetRootWidget().IsVisible() && m_OpenedSubmenu.GetShown())
			m_OpenedSubmenu.OnMenuUpdate(tDelta);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuFocusGained()
	{
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuFocusGained();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuFocusLost()
	{
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuFocusLost();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuClose()
	{
		foreach (SCR_SubMenuBase subMenu : m_aSubMenus)
		{
			if (!subMenu)
				continue;
			
			subMenu.OnTabHide();
			subMenu.OnTabRemove();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// --- Sub menus handling ---
	//------------------------------------------------------------------------------------------------
	// Might be called after OnMenuOpen, given that tabs have the option to be created only when they are selected 
	SCR_SubMenuBase OnTabCreate(SCR_TabViewComponent comp, Widget w, int index)
	{
		if (!w)
			return null;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return null;

		subMenu.GetOnRequestCloseMenu().Insert(OnRequestCloseMenu);
		subMenu.GetOnRequestTabChange().Insert(OnRequestTabChange);
		
		// Init sub menu
		subMenu.OnTabCreate(m_wMenuRoot, m_sNavigationButtonLayout, index);
		
		m_aSubMenus.Insert(subMenu);
		
		return subMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SubMenuBase OnTabShow(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return null;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return null;
		
		subMenu.OnTabShow();
		m_OpenedSubmenu = subMenu;
		
		return subMenu;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SubMenuBase OnTabHide(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return null;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return null;
		
		subMenu.OnTabHide();
		
		return subMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SubMenuBase OnTabRemove(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return null;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return null;
		
		subMenu.OnTabRemove();
		m_aSubMenus.RemoveItem(subMenu);
		
		return subMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SubMenuBase OnTabChange(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return null;

		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return null;

		subMenu.OnTabChange();
		
		return subMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRequestCloseMenu()
	{
		if (m_OnRequestCloseMenu)
			m_OnRequestCloseMenu.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRequestTabChange(int newTab, int oldTab)
	{
		m_TabViewComponent.ShowTab(newTab);
	}
	
	//------------------------------------------------------------------------------------------------
	//  --- Public ---
	//------------------------------------------------------------------------------------------------
	SCR_TabViewComponent GetTabView()
	{
		return m_TabViewComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSelectedTab()
	{
		return m_TabViewComponent.m_iSelectedTab;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SubMenuBase GetOpenedSubMenu()
	{
		return m_OpenedSubmenu;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns a sub menu based TabView's Elements, meaning it might return null if the tab's content hasn't been created yet
	SCR_SubMenuBase GetSubMenu(int index)
	{
		Widget content = m_TabViewComponent.GetContentWidget(index);
		if (!content)
			return null;
		
		return SCR_SubMenuBase.Cast(content.FindHandler(SCR_SubMenuBase));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_SuperMenuComponent FindComponent(notnull Widget w)
	{
		return SCR_SuperMenuComponent.Cast(w.FindHandler(SCR_SuperMenuComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//  --- Invokers ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnRequestCloseMenu()
	{
		if (!m_OnRequestCloseMenu)
			m_OnRequestCloseMenu = new ScriptInvokerVoid();
		
		return m_OnRequestCloseMenu;
	}
}