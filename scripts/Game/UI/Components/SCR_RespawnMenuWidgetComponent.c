//------------------------------------------------------------------------------------------------
class SCR_RespawnMenuWidgetHandler : ScriptedWidgetComponent
{
	[Attribute("DeployMenuRight", desc: "Input Action to listen to")]
	protected string m_sRightMenuAction;
	
	[Attribute("DeployMenuLeft", desc: "Input Action to listen to")]
	protected string m_sToolMenuAction;
	
	[Attribute("ExpandButton", desc: "Widget to be initially focused")]
	protected string m_sDefaultButtonName;
	
	[Attribute("ToolMenu")]
	protected string m_sMapToolMenu;
	
	[Attribute("Selectors")]
	protected string m_sSelectorsWidgetName;
	
	[Attribute("ControlHints")]
	protected string m_sControlHintsWidgetName;
	
	protected Widget m_wOwner;
	protected Widget m_wRoot;
	protected Widget m_wDefaultFocusWidget;
	protected Widget m_wDpadActionWidgetRightMenu;
	protected Widget m_wDpadActionWidgetToolMenu;
	protected Widget m_wDpadActionWidgetBackground;
	protected Widget m_wMapToolWidget;
	protected Widget m_wLoadoutSelector;
	protected SCR_ButtonBaseComponent m_LoadoutSelectorBtnLeft, m_LoadoutSelectorBtnRight;
	protected SCR_MapToolMenuUI m_MapToolMenuUI;
	protected SCR_DeployMenuMain m_DeployMenu;
	
	protected ref array<SCR_DeployRequestUIBaseComponent> m_aSelectors = {};
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadoudSelectorBtnFocused(Widget w)
	{
		Widget focusTo = m_wOwner.FindAnyWidget("ExpandButton");
		if (!focusTo)
			return;
		
		GetGame().GetWorkspace().SetFocusedWidget(focusTo);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_DeployRequestUIBaseComponent GetOpenedSelector()
	{
		foreach(SCR_DeployRequestUIBaseComponent selector : m_aSelectors)
		{
			if (selector.IsExpanded())
				return selector;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSelectors()
	{
		Widget selectorsWidget = m_wOwner.FindAnyWidget(m_sSelectorsWidgetName);
		if (!selectorsWidget)
		{
			Print("SCR_RespawnMenuWidgetComponent: Couldn't set up selectors in the respawn menu widget component because the widget with name " + m_sSelectorsWidgetName + " was not found", LogLevel.WARNING);
			return;
		}
		
		SCR_DeployRequestUIBaseComponent deployReqComponent;
		Widget selector = selectorsWidget.GetChildren();
		while (selector)
		{
			deployReqComponent = SCR_DeployRequestUIBaseComponent.Cast(selector.FindHandler(SCR_DeployRequestUIBaseComponent));
			if (deployReqComponent)
				m_aSelectors.Insert(deployReqComponent);
			
			selector = selector.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowControlHints(Widget w, bool show)
	{
		Widget hintsWidget = w.GetParent().FindAnyWidget(m_sControlHintsWidgetName);
		if (!hintsWidget)
			return;
		
		hintsWidget.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowDpadWidgets(bool show)
	{
		if (m_wDpadActionWidgetRightMenu)
			m_wDpadActionWidgetRightMenu.SetVisible(show);
		
		if (m_wDpadActionWidgetToolMenu)
			m_wDpadActionWidgetToolMenu.SetVisible(show);
		
		if (m_wDpadActionWidgetBackground)
			m_wDpadActionWidgetBackground.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableWidget(notnull Widget w, bool enable)
	{
		w.SetEnabled(enable);
		Widget child = w.GetChildren();
		while (child)
		{
			EnableWidget(child, enable);
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		SCR_LoadoutGallery gallery = SCR_LoadoutGallery.Cast(m_wLoadoutSelector.FindHandler(SCR_LoadoutGallery));
		if (!gallery)
			return;
		
		m_LoadoutSelectorBtnLeft.m_OnFocus.Remove(OnLoadoudSelectorBtnFocused);
		m_LoadoutSelectorBtnRight.m_OnFocus.Remove(OnLoadoudSelectorBtnFocused);
		
		//Disable all widgets and have them enabled through DPAD interactions, if on gamepad.
		if (isGamepad)
		{
			gallery.EnablePagingInputListeners(false);
			gallery.SetGalleryFocused(false);
			gallery.GetOnFocusChange().Insert(gallery.EnablePagingInputListeners);
			
			m_LoadoutSelectorBtnLeft.m_OnFocus.Insert(OnLoadoudSelectorBtnFocused);
			m_LoadoutSelectorBtnRight.m_OnFocus.Insert(OnLoadoudSelectorBtnFocused);
			
			EnableWidget(m_wMapToolWidget, !isGamepad);
			return;
		}
		
		gallery.GetOnFocusChange().Remove(gallery.EnablePagingInputListeners);
		gallery.EnablePagingInputListeners(true);
		
		GetGame().GetWorkspace().SetFocusedWidget(null);
		EnableWidget(m_wMapToolWidget, true);
		ShowControlHints(m_wOwner, false);
		ShowControlHints(m_wMapToolWidget, false);
		
		if (m_DeployMenu)
			m_DeployMenu.AllowMapContext(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupMap()
	{
		if (!m_DeployMenu)
			m_DeployMenu = SCR_DeployMenuMain.GetDeployMenu();
		
		m_wMapToolWidget = m_wRoot.FindAnyWidget(m_sMapToolMenu);
		if (!m_wMapToolWidget)
			return;
		
		Widget widget = m_wRoot.FindAnyWidget("LoadoutSelector");
		if (!widget)
			return;
		
		m_wLoadoutSelector = widget.FindAnyWidget("Selector");
		
		m_wDpadActionWidgetToolMenu = m_wMapToolWidget.FindAnyWidget("Action");
		m_wDpadActionWidgetBackground = m_wMapToolWidget.FindAnyWidget("ActionBG");
		m_MapToolMenuUI = SCR_MapToolMenuUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapToolMenuUI));
		
		widget = m_wRoot.FindAnyWidget("PagingLeft");
		if (widget)
			m_LoadoutSelectorBtnLeft = SCR_ButtonBaseComponent.Cast(widget.FindHandler(SCR_PagingButtonComponent));
		
		widget = m_wRoot.FindAnyWidget("PagingRight");
		if (widget)
			m_LoadoutSelectorBtnRight = SCR_ButtonBaseComponent.Cast(widget.FindHandler(SCR_PagingButtonComponent));
		
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		SetupSelectors();
		AddActionListeners();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;

		m_wOwner = w;
		m_wRoot = SCR_WidgetHelper.GetRootWidget(m_wOwner);
		m_wDpadActionWidgetRightMenu = w.FindAnyWidget("Control-Hint-Loadout");
		m_wDefaultFocusWidget = m_wOwner.FindAnyWidget(m_sDefaultButtonName);
		
		//Delayed caching of other widgets, as they are initialised later
		GetGame().GetCallqueue().CallLater(SetupMap, 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddActionListeners()
	{
		GetGame().GetInputManager().AddActionListener(m_sRightMenuAction, EActionTrigger.DOWN, SetFocusRightMenu);
		GetGame().GetInputManager().AddActionListener(m_sToolMenuAction, EActionTrigger.DOWN, SetFocusToolMenu);

	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		GetGame().GetInputManager().RemoveActionListener(m_sRightMenuAction, EActionTrigger.DOWN, SetFocusRightMenu);
		GetGame().GetInputManager().RemoveActionListener(m_sToolMenuAction, EActionTrigger.DOWN, SetFocusToolMenu);

	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFocusRightMenu()
	{
		if (!m_wDefaultFocusWidget)
			return;

		ShowDpadWidgets(false);
		ShowControlHints(m_wOwner, true);
		
		GetGame().GetWorkspace().SetFocusedWidget(m_wDefaultFocusWidget);
		
		RemoveActionListeners();
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, DisableFocusRightMenu);
		
		if (m_DeployMenu)
			m_DeployMenu.AllowMapContext(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFocusToolMenu()
	{
		if (!m_MapToolMenuUI)
			return;
		
		ShowDpadWidgets(false);
		EnableWidget(m_wMapToolWidget, true);
		ShowControlHints(m_wMapToolWidget, true);
		m_MapToolMenuUI.SetToolMenuFocused(true);
		
		RemoveActionListeners();
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, DisableFocusToolMenu);
		
		if (m_DeployMenu)
			m_DeployMenu.AllowMapContext(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisableFocusRightMenu()
	{
		/*
		SCR_DeployRequestUIBaseComponent selector = GetOpenedSelector();
		if (selector && !GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			selector.SetExpanded(false);
			GetGame().GetWorkspace().SetFocusedWidget(m_wDefaultFocusWidget);
			return;
		}
		*/
		
		GetGame().GetWorkspace().SetFocusedWidget(null);

		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, DisableFocusRightMenu);
		AddActionListeners();
		
		ShowDpadWidgets(true);
		ShowControlHints(m_wOwner, false);
		
		if (m_DeployMenu)
			m_DeployMenu.AllowMapContext(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisableFocusToolMenu()
	{	
		if (!m_MapToolMenuUI)
			return;
		
		if (IsAnyEntryActive())
		{
			CloseAllMapTools();
			
			m_MapToolMenuUI.SetToolMenuFocused(true);
			return;
		}
		
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, DisableFocusToolMenu);
		m_MapToolMenuUI.SetToolMenuFocused(false);
		AddActionListeners();
		ShowDpadWidgets(true);
		ShowControlHints(m_wMapToolWidget, false);
		
		if (m_DeployMenu)
			m_DeployMenu.AllowMapContext(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsAnyEntryActive()
	{
		array<ref SCR_MapToolEntry> entries = m_MapToolMenuUI.GetMenuEntries();
		if (!entries || entries.IsEmpty())
			return false;
		
		foreach (ref SCR_MapToolEntry entry : entries)
		{
			//TODO: This is far from ideal, on the contrary, this is terrible.
			if ((entry.GetImageSet() == "ruler") || (entry.GetImageSet() == "compass") || (entry.GetImageSet() == "watch") || (entry.GetImageSet() == "editor"))
				continue;
			
			if (entry.IsEntryActive())
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CloseAllMapTools()
	{
		array<ref SCR_MapToolEntry> entries = m_MapToolMenuUI.GetMenuEntries();
		if (!entries || entries.IsEmpty())
			return;
		
		foreach (ref SCR_MapToolEntry entry : entries)
		{
			if ((entry.GetImageSet() == "ruler") || (entry.GetImageSet() == "compass") || (entry.GetImageSet() == "watch") || (entry.GetImageSet() == "editor"))
				continue;
			
			if (entry.IsEntryActive())
				entry.SetActive(false);
		}
	}
}
