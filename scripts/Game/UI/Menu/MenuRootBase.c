/*!
Scripted menu base class.

Manages functionality of class MenuRootComponent.
*/

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// This class should be merged with ChimeraMenuBase

class MenuRootBase : ChimeraMenuBase
{
	
//---- REFACTOR NOTE END ----

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// This gives components access to their menu's events (they will register to the MenuRootComponent) through tight couplings: it is debatable if this is the right approach
	
	protected MenuRootComponent m_MenuRootComponent;

//---- REFACTOR NOTE END ----
	
	protected ref ScriptInvoker m_EventOnMenuFocusGained;
	protected ref ScriptInvoker m_EventOnMenuFocusLost;
	protected ref ScriptInvoker m_EventOnMenuShow;
	protected ref ScriptInvoker m_EventOnMenuHide;
	protected ref ScriptInvoker m_EventOnMenuItem;
	protected ref ScriptInvoker m_EventOnMenuUpdate;
	protected ref ScriptInvoker m_EventOnMenuOpen;
	protected ref ScriptInvoker m_EventOnMenuInit;
	protected ref ScriptInvoker m_EventOnMenuClose;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Old implementation, now we have HUD manager for this stuff
	
	protected SCR_ChatPanel m_ChatPanel;

//---- REFACTOR NOTE END ----
	
	protected SCR_DynamicFooterComponent m_DynamicFooter;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMenuFocusGained()
	{
		if (!m_EventOnMenuFocusGained)
			m_EventOnMenuFocusGained = new ScriptInvoker();
		return m_EventOnMenuFocusGained;
	}
	ScriptInvoker GetOnMenuFocusLost()
	{
		if (!m_EventOnMenuFocusLost)
			m_EventOnMenuFocusLost = new ScriptInvoker();
		return m_EventOnMenuFocusLost;
	}
	ScriptInvoker GetOnMenuShow()
	{
		if (!m_EventOnMenuShow)
			m_EventOnMenuShow = new ScriptInvoker();
		return m_EventOnMenuShow;
	}
	ScriptInvoker GetOnMenuHide()
	{
		if (!m_EventOnMenuHide)
			m_EventOnMenuHide = new ScriptInvoker();
		return m_EventOnMenuHide;
	}
	ScriptInvoker GetOnMenuItem()
	{
		if (!m_EventOnMenuItem)
			m_EventOnMenuItem = new ScriptInvoker();
		return m_EventOnMenuItem;
	}
	ScriptInvoker GetOnMenuUpdate()
	{
		if (!m_EventOnMenuUpdate)
			m_EventOnMenuUpdate = new ScriptInvoker();
		return m_EventOnMenuUpdate;
	}
	ScriptInvoker GetOnMenuOpen()
	{
		if (!m_EventOnMenuOpen)
			m_EventOnMenuOpen = new ScriptInvoker();
		return m_EventOnMenuOpen;
	}
	ScriptInvoker GetOnMenuInit()
	{
		if (!m_EventOnMenuInit)
			m_EventOnMenuInit = new ScriptInvoker();
		return m_EventOnMenuInit;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMenuClose()
	{
		if (!m_EventOnMenuClose)
			m_EventOnMenuClose = new ScriptInvoker();
		return m_EventOnMenuClose;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMenuClosed()
	{
		SCR_ChatPanelManager chatManager = SCR_ChatPanelManager.GetInstance();
		if (chatManager)
			GetGame().GetCallqueue().CallLater(chatManager.OnMenuClosed, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void InitChat()
	{
		// Check if this menu has chat
		Widget chatPanelWidget = GetRootWidget().FindAnyWidget("ChatPanel");
		if (!chatPanelWidget)
			return;
		
		m_ChatPanel = SCR_ChatPanel.Cast(chatPanelWidget.FindHandler(SCR_ChatPanel));
	}
	
	//------------------------------------------------------------------------------------------------
	// Chat related methods are here, as we assume any menu can have it's chat panel and therefore needs to implement these.
	void ShowChat()
	{
		SCR_ChatPanelManager chatManager = SCR_ChatPanelManager.GetInstance();
		if (!chatManager)
			return;
		
		// Check if this menu has chat
		Widget chatPanelWidget = GetRootWidget().FindAnyWidget("ChatPanel");
		if (!chatPanelWidget)
			return;
		
		SCR_ChatPanel chatPanel = SCR_ChatPanel.Cast(chatPanelWidget.FindHandler(SCR_ChatPanel));
		chatManager.HideAllChatPanels();
		chatManager.ShowChatPanel(chatPanel);
	}

	//------------------------------------------------------------------------------------------------
	MenuRootComponent GetRootComponent()
	{
		return m_MenuRootComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DynamicFooterComponent GetFooterComponent()
	{
		return m_DynamicFooter;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		ShowChat(); // If chat is part of this menu, it will be shown
		if (m_EventOnMenuFocusGained)
			m_EventOnMenuFocusGained.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		if (m_EventOnMenuFocusLost)
			m_EventOnMenuFocusLost.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		if (m_EventOnMenuShow)
			m_EventOnMenuShow.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		if (m_EventOnMenuHide)
			m_EventOnMenuHide.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuItem(string menuItemName, bool changed, bool finished)
	{
		super.OnMenuItem(menuItemName, changed, finished);
		if (m_EventOnMenuItem)
			m_EventOnMenuItem.Invoke(menuItemName, changed, finished);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);
		
		if (IsFocused() && m_EventOnMenuUpdate)
			m_EventOnMenuUpdate.Invoke(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		InitChat();
		ShowChat(); // If chat is part of this menu, it will be shown
		if (m_EventOnMenuOpen)
			m_EventOnMenuOpen.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		super.OnMenuInit();
		m_MenuRootComponent = MenuRootComponent.GetRootOf(this);
		m_DynamicFooter = SCR_DynamicFooterComponent.FindComponentInHierarchy(GetRootWidget());
		if (m_MenuRootComponent)
			m_MenuRootComponent.Init(this);
		if (m_EventOnMenuInit)
			m_EventOnMenuInit.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		OnMenuClosed();
		if (m_EventOnMenuClose)
			m_EventOnMenuClose.Invoke();
	}
};
