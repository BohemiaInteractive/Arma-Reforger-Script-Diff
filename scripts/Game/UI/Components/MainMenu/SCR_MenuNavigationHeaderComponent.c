[BaseContainerProps()]
class SCR_MenuNavigationItem
{
	[Attribute("0")]
	bool m_bIsSelected;
	
	[Attribute("")]
	string m_sTitle;
	
	[Attribute("-1", UIWidgets.SearchComboBox, "", "", ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	ChimeraMenuPreset m_eMenuPreset;
}

class SCR_MenuNavigationHeaderComponent: SCR_ScriptedWidgetComponent
{
	protected static const ResourceName ENTRY_LAYOUT = "{04A28F3B36FE0F9A}UI/layouts/Menus/ContentBrowser/ScenariosMenu/HeaderNavigationEntry.layout";
	
	protected ref SCR_HeaderNavigationWidgets m_Widgets = new SCR_HeaderNavigationWidgets;
	
	[Attribute()]
	protected ref array<ref SCR_MenuNavigationItem> m_aItems;
	
	protected int m_iSelectedId;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Widgets.Init(w);
		
		// create widget
		Widget itemWidget;
		SCR_MenuNavigationItemComponent NavItemComp;
		SCR_ButtonTextComponent NavButtonComp;
		foreach(int id, SCR_MenuNavigationItem item: m_aItems)
		{
			itemWidget = GetGame().GetWorkspace().CreateWidgets(ENTRY_LAYOUT, m_Widgets.m_wHorizontalItems);
			NavItemComp = SCR_MenuNavigationItemComponent.Cast(itemWidget.FindHandler(SCR_MenuNavigationItemComponent));
			NavItemComp.Init(item.m_eMenuPreset, item.m_sTitle, item.m_bIsSelected);
			
			if (!item.m_bIsSelected)
			{
				NavButtonComp = SCR_ButtonTextComponent.Cast(itemWidget.FindHandler(SCR_ButtonTextComponent));
				NavButtonComp.m_OnClicked.Insert(OnClickedEntry);
			}
			else
				m_iSelectedId = id;
		}
		
		// if its the first entry
		if (m_iSelectedId == 0)
			DisableNavButton(m_Widgets.m_NavButtonLeftComponent);
		else
			m_Widgets.m_NavButtonLeftComponent.m_OnActivated.Insert(OnPrevMenu);
		
		// if its the last entry
		if (m_iSelectedId == m_aItems.Count() - 1)
			DisableNavButton(m_Widgets.m_NavButtonRightComponent);
		else
			m_Widgets.m_NavButtonRightComponent.m_OnActivated.Insert(OnNextMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClickedEntry(SCR_ButtonTextComponent comp)
	{
		Widget root = comp.GetRootWidget();
		SCR_MenuNavigationItemComponent NavItemComp = SCR_MenuNavigationItemComponent.Cast(root.FindHandler(SCR_MenuNavigationItemComponent));
		OnOpenMenu(NavItemComp.GetMenuPreset());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNextMenu()
	{
		SCR_MenuNavigationItem item = m_aItems[m_iSelectedId + 1];
		OnOpenMenu(item.m_eMenuPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPrevMenu()
	{
		SCR_MenuNavigationItem item = m_aItems[m_iSelectedId - 1];
		OnOpenMenu(item.m_eMenuPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnOpenMenu(ChimeraMenuPreset menuToOpen)
	{
		GetGame().GetMenuManager().CloseMenu(GetGame().GetMenuManager().GetTopMenu());
		GetGame().GetMenuManager().OpenMenu(menuToOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisableNavButton(notnull SCR_InputButtonComponent comp)
	{
		Widget root = comp.GetRootWidget();
		
		comp.SetEnabled(false);
		root.SetEnabled(false);
		root.SetOpacity(UIConstants.DISABLED_WIDGET_OPACITY);
	}
}