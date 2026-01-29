//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// This class was a workaround for the lack of a scripted MenuManager

//! This class is a collection of useful invokers for menu and dialog related events
void ScriptInvokerActiveWidgetInteractionMethod(bool isActive, int delay);
typedef func ScriptInvokerActiveWidgetInteractionMethod;
typedef ScriptInvokerBase<ScriptInvokerActiveWidgetInteractionMethod> ScriptInvokerActiveWidgetInteraction;

void ScriptInvokerDialogMethod(DialogUI dialog);
typedef func ScriptInvokerDialogMethod;
typedef ScriptInvokerBase<ScriptInvokerDialogMethod> ScriptInvokerDialog;

void ScriptInvokerMenuMethod(ChimeraMenuBase menu);
typedef func ScriptInvokerMenuMethod;
typedef ScriptInvokerBase<ScriptInvokerMenuMethod> ScriptInvokerMenu;

//------------------------------------------------------------------------------------------------
class SCR_MenuHelper
{
	protected static ref ScriptInvokerActiveWidgetInteraction m_OnActiveWidgetInteraction;
	protected static ref ScriptInvokerDialog m_OnDialogOpen;
	protected static ref ScriptInvokerDialog m_OnDialogClose;
	protected static ref ScriptInvokerMenu m_OnMenuOpen;
	protected static ref ScriptInvokerMenu m_OnMenuOpened;
	protected static ref ScriptInvokerMenu m_OnMenuClose;
	protected static ref ScriptInvokerMenu m_OnMenuFocusGained;
	protected static ref ScriptInvokerMenu m_OnMenuFocusLost;
	protected static ref ScriptInvokerMenu m_OnMenuShow;
	protected static ref ScriptInvokerMenu m_OnMenuHide;
	protected static ref ScriptInvokerTabView m_OnTabChange;

	// ---- Invokers ----
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerActiveWidgetInteraction GetOnActiveWidgetInteraction()
	{
		if (!m_OnActiveWidgetInteraction)
			m_OnActiveWidgetInteraction = new ScriptInvokerActiveWidgetInteraction();

		return m_OnActiveWidgetInteraction;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerDialog GetOnDialogOpen()
	{
		if (!m_OnDialogOpen)
			m_OnDialogOpen = new ScriptInvokerDialog();

		return m_OnDialogOpen;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerDialog GetOnDialogClose()
	{
		if (!m_OnDialogClose)
			m_OnDialogClose = new ScriptInvokerDialog();

		return m_OnDialogClose;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerMenu GetOnMenuOpen()
	{
		if (!m_OnMenuOpen)
			m_OnMenuOpen = new ScriptInvokerMenu();

		return m_OnMenuOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerMenu GetOnMenuOpened()
	{
		if (!m_OnMenuOpened)
			m_OnMenuOpened = new ScriptInvokerMenu();

		return m_OnMenuOpened;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerMenu GetOnMenuClose()
	{
		if (!m_OnMenuClose)
			m_OnMenuClose = new ScriptInvokerMenu();

		return m_OnMenuClose;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerMenu GetOnMenuFocusGained()
	{
		if (!m_OnMenuFocusGained)
			m_OnMenuFocusGained = new ScriptInvokerMenu();

		return m_OnMenuFocusGained;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerMenu GetOnMenuFocusLost()
	{
		if (!m_OnMenuFocusLost)
			m_OnMenuFocusLost = new ScriptInvokerMenu();

		return m_OnMenuFocusLost;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerMenu GetOnMenuShow()
	{
		if (!m_OnMenuShow)
			m_OnMenuShow = new ScriptInvokerMenu();

		return m_OnMenuShow;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerMenu GetOnMenuHide()
	{
		if (!m_OnMenuHide)
			m_OnMenuHide = new ScriptInvokerMenu();

		return m_OnMenuHide;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerTabView GetOnTabChange()
	{
		if (!m_OnTabChange)
			m_OnTabChange = new ScriptInvokerTabView();

		return m_OnTabChange;
	}

	// ---- Methods ----
	//------------------------------------------------------------------------------------------------
	//! Navigation buttons are bound to this delegate to disable themselves during edit/combo boxes interaction and prevent reenabling
	//! Edit boxes and dropdown menus call this method as a substitute for the lack of events when starting and finishing txt editing / dropdown interaction
	static void SetActiveWidgetInteractionState(bool isActive, int delay = 0)
	{
		if (m_OnActiveWidgetInteraction)
			m_OnActiveWidgetInteraction.Invoke(isActive, delay);
	}
	
	//------------------------------------------------------------------------------------------------
	//! NB: This will return false if the widget is in a DIALOG on top of the MENU!!!
	static bool IsInTopMenu(notnull Widget componentRoot)
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return false;
		
		return menuManager.GetOwnerMenu(componentRoot) == menuManager.GetTopMenu();
	}
	
	// -- Calls --
	//------------------------------------------------------------------------------------------------
	//! Methods to keep track of current dialogs.
	//! The MenuManager does not keep track of dialogs: GetTopMenu() only returns the last menu opened with OpenMenu(), not those opened with OpenDialog()
	//! Navigation buttons bind themselves to m_OnDialogOpen and m_OnDialogClose
	//! Called by DialogUI
	static void OnDialogOpen(DialogUI dialog)
	{
		if (m_OnDialogOpen)
			m_OnDialogOpen.Invoke(dialog);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by DialogUI.
	static void OnDialogClose(DialogUI dialog)
	{
		if (m_OnDialogClose)
			m_OnDialogClose.Invoke(dialog);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by ChimeraMenuBase
	//! SCR_ScriptedWidgetTooltip uses this to make sure the tooltip is removed when the menu changes
	static void OnMenuOpen(ChimeraMenuBase menu)
	{
		if (m_OnMenuOpen)
			m_OnMenuOpen.Invoke(menu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by ChimeraMenuBase
	static void OnMenuOpened(ChimeraMenuBase menu)
	{
		if (m_OnMenuOpened)
			m_OnMenuOpened.Invoke(menu);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by ChimeraMenuBase.
	static void OnMenuClose(ChimeraMenuBase menu)
	{
		if (m_OnMenuClose)
			m_OnMenuClose.Invoke(menu);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by ChimeraMenuBase
	static void OnMenuFocusGained(ChimeraMenuBase menu)
	{
		if (m_OnMenuFocusGained)
			m_OnMenuFocusGained.Invoke(menu);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by ChimeraMenuBase
	static void OnMenuFocusLost(ChimeraMenuBase menu)
	{
		if (m_OnMenuFocusLost)
			m_OnMenuFocusLost.Invoke(menu);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by ChimeraMenuBase
	static void OnMenuShow(ChimeraMenuBase menu)
	{
		if (m_OnMenuShow)
			m_OnMenuShow.Invoke(menu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by ChimeraMenuBase
	static void OnMenuHide(ChimeraMenuBase menu)
	{
		if (m_OnMenuHide)
			m_OnMenuHide.Invoke(menu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_TabViewComponent.
	static void OnTabChange(SCR_TabViewComponent tabView, Widget widget)
	{
		if (m_OnTabChange)
			m_OnTabChange.Invoke(tabView, widget);
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_EListMenuWidgetFocus
{
	LIST = 0,
	FILTERING = 1,
	SORTING = 2,
	FALLBACK = 3,
	NULL = 4
}

//---- REFACTOR NOTE END ----
