/* 
Component for SCR_SuperMenuComponent tab.
Relies on a SCR_DynamicFooterComponent somewhere in the layout under the menuRoot widget passed on creation to handle input buttons of the tab
*/

class SCR_SubMenuBase : SCR_ScriptedWidgetComponent
{
	protected ResourceName m_sNavigationButtonLayout;
	
	protected SCR_DynamicFooterComponent m_DynamicFooter;
	protected ref array<SCR_InputButtonComponent> m_aNavigationButtons = {};
	protected ref map<SCR_InputButtonComponent, bool> m_aNavigationButtonsVisibilityFlags = new map<SCR_InputButtonComponent, bool>();
	
	protected Widget m_wMenuRoot;
	protected int m_iIndex;
	
	protected bool m_bShown;
	protected bool m_bFocused;
	
	// Menu requests
	protected ref ScriptInvokerVoid m_OnRequestCloseMenu;
	protected ref ScriptInvokerInt2 m_OnRequestTabChange;

	//------------------------------------------------------------------------------------------------
	// --- Menu events ---
	//------------------------------------------------------------------------------------------------
	void OnMenuFocusGained()
	{
		m_bFocused = true;
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuFocusLost()
	{
		m_bFocused = false;
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuShow();
	void OnMenuHide();
	void OnMenuUpdate(float tDelta);
	
	//------------------------------------------------------------------------------------------------
	// --- Tab events ---
	//------------------------------------------------------------------------------------------------
	// Tab initialization, depending on SCR_TabViewComponent settings it can happen before or after OnMenuOpen, or even when the tab is first selected 
	void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		m_iIndex = index;
		m_wMenuRoot = menuRoot;
		m_DynamicFooter = SCR_DynamicFooterComponent.FindComponentInHierarchy(menuRoot);
		m_sNavigationButtonLayout = buttonsLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	// Tab has actively been selected, or it's the default one after creation
	void OnTabShow()
	{
		ShowNavigationButtons(true);
		m_bShown = true;
	}

	//------------------------------------------------------------------------------------------------
	// Another tab has been selected
	void OnTabHide()
	{
		ShowNavigationButtons(false);
		m_bShown = false;
	}

	//------------------------------------------------------------------------------------------------
	// Tab destruction
	void OnTabRemove()
	{
		foreach (SCR_InputButtonComponent comp : m_aNavigationButtons)
		{
			Widget w = comp.GetRootWidget();
			if (!w)
				continue;

			w.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Generic, happens anytime a tab is selected
	void OnTabChange();

	//------------------------------------------------------------------------------------------------
	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	protected void ShowNavigationButtons(bool show)
	{
		bool visible;
		
		foreach (SCR_InputButtonComponent comp : m_aNavigationButtons)
		{
			if (m_aNavigationButtonsVisibilityFlags.Contains(comp))
				visible = m_aNavigationButtonsVisibilityFlags.Get(comp);
			
			visible = visible && show;
			
			comp.SetVisible(visible, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Flags a button to be set visible the next time ShowNavigationButtons() is called, like on TabShow
	protected void FlagNavigationButtonVisibility(SCR_InputButtonComponent button, bool show)
	{
		if (!button)
			return;
		
		if (!m_aNavigationButtonsVisibilityFlags.Contains(button) || !m_aNavigationButtons.Contains(button))
		{
			PrintFormat(
				"SCR_SubMenuBase - FlagNavigationButtonVisibility() - button %1, %2 is not a sub menu footer button as it was not created with CreateNavigationButton() method", 
				button,
				button.GetRootWidget().GetName()
			);
			
			return;
		}
		
		m_aNavigationButtonsVisibilityFlags.Set(button, show);
	}
	
	//------------------------------------------------------------------------------------------------
	// Use this to control the visibility of buttons created with CreateNavigationButton() method
	protected void SetNavigationButtonVisibile(SCR_InputButtonComponent button, bool show, bool animate = false)
	{
		if (!button)
			return;
		
		if (!m_aNavigationButtonsVisibilityFlags.Contains(button) || !m_aNavigationButtons.Contains(button))
		{
			PrintFormat(
				"SCR_SubMenuBase - SetFooterButtonVisibile() - button %1, %2 is not a sub menu footer button as it was not created with CreateNavigationButton() method", 
				button,
				button.GetRootWidget().GetName()
			);
			
			return;
		}
		
		FlagNavigationButtonVisibility(button, show);
		button.SetVisible(m_bShown && show, animate);
	}
	
	//------------------------------------------------------------------------------------------------
	// If used on tab show, make sure to avoid creating duplicates
 	protected SCR_InputButtonComponent CreateNavigationButton(string actionName, string label, bool rightFooter = false, bool show = true)
	{
		if (!m_DynamicFooter)
			return null;

		SCR_EDynamicFooterButtonAlignment alignment = SCR_EDynamicFooterButtonAlignment.LEFT;
		if (rightFooter)
			alignment = SCR_EDynamicFooterButtonAlignment.RIGHT;

		SCR_InputButtonComponent comp = m_DynamicFooter.CreateButton(m_sNavigationButtonLayout, label + m_iIndex, label, actionName, alignment, show);

		if (!comp)
			return null;

		m_aNavigationButtons.Insert(comp);
		m_aNavigationButtonsVisibilityFlags.Insert(comp, show);
		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RequestCloseMenu()
	{
		if (m_OnRequestCloseMenu)
			m_OnRequestCloseMenu.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	// Provides the desired tab and the current index
	protected void RequestTabChange(int newTabIndex)
	{
		if (m_OnRequestTabChange)
			m_OnRequestTabChange.Invoke(newTabIndex, m_iIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- Public ---
	//------------------------------------------------------------------------------------------------
	bool GetShown()
	{
		return m_bShown;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIndex()
	{
		return m_iIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnRequestCloseMenu()
	{
		if (!m_OnRequestCloseMenu)
			m_OnRequestCloseMenu = new ScriptInvokerVoid();
		
		return m_OnRequestCloseMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt2 GetOnRequestTabChange()
	{
		if (!m_OnRequestTabChange)
			m_OnRequestTabChange = new ScriptInvokerInt2();
		
		return m_OnRequestTabChange;
	}
}