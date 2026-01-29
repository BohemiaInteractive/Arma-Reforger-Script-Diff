
class ContentBrowserDetailsMenu : SCR_SuperMenuBase
{
	protected SCR_InputButtonComponent m_NavBack;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		// Setup the 'back' nav button
		m_NavBack = m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK);
		if (m_NavBack)	
			m_NavBack.m_OnActivated.Insert(OnNavButtonClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavButtonClose()
	{
		// This menu might be not focused, because another details menu is currently open
		if (IsFocused())
			Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWorkshopItem(SCR_WorkshopItem item)
	{
		if (!item)
		{
			Close();
			GetGame().GetCallqueue().Call(SCR_CommonDialogs.CreateRequestErrorDialog);
			return;
		}

		SCR_ModDetailsSuperMenuComponent detailsSuperMenu = SCR_ModDetailsSuperMenuComponent.Cast(m_SuperMenuComponent);
		if (detailsSuperMenu)
			detailsSuperMenu.SetWorkshopItem(item);
		
		// Header
		SCR_CoreMenuHeaderComponent header = SCR_CoreMenuHeaderComponent.FindComponentInHierarchy(GetRootWidget());
		if (header)
			header.SetTitle(item.GetName());
	}
	
	// Public
	//------------------------------------------------------------------------------------------------
	//! Opens the menu for a given workshop item
	static ContentBrowserDetailsMenu OpenForWorkshopItem(SCR_WorkshopItem item)
	{
		ContentBrowserDetailsMenu detailsMenu;
		WorldSaveItem saveItem = WorldSaveItem.Cast(item.GetWorkshopItem());
		
		if (saveItem)
		{
			// World save
			detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowserDetailsMenuSave));
		}
		else
		{
			// Addon
			detailsMenu = ContentBrowserDetailsMenu.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowserDetailsMenu));
		}
		
		
		if (detailsMenu)
			detailsMenu.SetWorkshopItem(item);
		
		return detailsMenu;
	}
}
