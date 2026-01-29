/*!
Class for centralized features that should simplify using menus from script
*/

class SCR_MenuToolsLib
{
	protected static const int MENU_CLOSE_FALLBACK = 10;
	
	protected static MenuManager m_MenuMgr;
	
	//------------------------------------------------------------------------------------------------
	// Invokers
	//------------------------------------------------------------------------------------------------
	
	protected static ref ScriptInvoker<> Event_OnAllMenuClosed;

	//------------------------------------------------------------------------------------------------
	protected static void InvokeEventOnAllMenuClosed()
	{
		if (Event_OnAllMenuClosed)
			Event_OnAllMenuClosed.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetEventOnAllMenuClosed()
	{
		if (!Event_OnAllMenuClosed)
			Event_OnAllMenuClosed = new ScriptInvoker();

		return Event_OnAllMenuClosed;
	}
	
	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Close all menus that are not of selected class
	static void CloseAllMenus(array<typename> menus)
	{
		if (!m_MenuMgr)
			m_MenuMgr = GetGame().GetMenuManager();
		
		CloseTopMenu(menus, MENU_CLOSE_FALLBACK);
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected static void CloseTopMenu(array<typename> menus, int calls)
	{
		//MainMenuUI mainMenu = MainMenuUI.Cast(GetGame().GetMenuManager().GetTopMenu());
		//ContentBrowserUI workshopUI = ContentBrowserUI.Cast(GetGame().GetMenuManager().GetTopMenu());
			
		bool excluded = false;
		
		// Is top menu of excluded type? 
		for (int i = 0, count = menus.Count(); i < count; i++)
		{
			typename menuType = m_MenuMgr.GetTopMenu().Type();
			
			if (menuType == menus[i])
			{
				excluded = true;
				break;
			}
		}
		
		// Current excluded = All non-excluded close
		if (excluded)
		{
			GetGame().GetCallqueue().CallLater(AllMenuClosed);
			//InvokeEventOnAllMenuClosed();
			return;
		}
		
		m_MenuMgr.GetTopMenu().Close();
		
		calls--;
		GetGame().GetCallqueue().CallLater(CloseTopMenu, 0, false, menus, calls);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void AllMenuClosed()
	{
		InvokeEventOnAllMenuClosed();
	}
}