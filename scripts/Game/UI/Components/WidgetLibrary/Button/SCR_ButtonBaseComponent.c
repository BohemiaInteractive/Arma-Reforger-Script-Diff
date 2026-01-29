//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Several issues: missing events, and duplicate OnClick (one already in a parent class), incomplete handling of interaction states, it's looks are too tied to old Reforger style, and setting them in component's attributes makes mantaining a consistent look across the whole project an absolute nightmare. Holding refernces to parent menu or submenu is very limiting: it's not the button's business to change based on parent menu state, it's whatever holds the button that should notify it of important changes, because there's no guarantee this button will only be in a menu or submenu. 

//! Base class for any button, regardless its own content
//------------------------------------------------------------------------------------------------
class SCR_ButtonBaseComponent : SCR_WLibComponentBase 
{	
	[Attribute("", UIWidgets.CheckBox, "Can the button be only clicked, or also toggled?")]
	bool m_bCanBeToggled;
	
	[Attribute()]
	protected bool m_bIsToggled;
	
	[Attribute("true", UIWidgets.CheckBox, "Use in built button colorization or handle it on your own")]
	bool m_bUseColorization;

	[Attribute("1 1 1 0.1", UIWidgets.ColorPicker)]
	ref Color m_BackgroundDefault;
	
	[Attribute("1 1 1 0.3", UIWidgets.ColorPicker)]
	ref Color m_BackgroundHovered;
	
	[Attribute("0.760 0.392 0.08 0.3", UIWidgets.ColorPicker)]
	ref Color m_BackgroundSelected;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	ref Color m_BackgroundSelectedHovered;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	ref Color m_BackgroundClicked;
	
	[Attribute("true")]
	bool m_bShowBorderOnFocus;
	
	[Attribute("false")]
	bool m_bShowBackgroundOnFocus;	
	
	[Attribute("false")]
	bool m_bNoBorderAnimation;
	
	[Attribute("false")]
	bool m_bNoBackgroundAnimation;
	
	[Attribute()]
	bool m_bShowBorderOnHover;

	Widget m_wBackground;
	Widget m_wBorder;
	
	// Arguments passed: SCR_ButtonBaseComponent
	ref ScriptInvoker m_OnClicked = new ScriptInvoker();
	// Arguments passed: SCR_ButtonBaseComponent, bool (toggled)
	ref ScriptInvoker m_OnToggled = new ScriptInvoker();
	
	// TODO: Rewrite - do not pass widget //Why? This is super useful!
	ref ScriptInvoker<Widget> m_OnFocus = new ScriptInvoker<Widget>();
	ref ScriptInvoker<Widget> m_OnFocusLost = new ScriptInvoker<Widget>();
		
	// Arguments passed: SCR_ButtonBaseComponent, bool border shown
	ref ScriptInvoker m_OnShowBorder = new ScriptInvoker();
	
	protected MenuBase m_ParentMenu; // Pointer back to menu which owns this button, initialized at FindParentMenu
	protected SCR_SubMenuBase m_ParentSubMenu;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wBackground = w.FindAnyWidget("Background");
		m_wBorder = w.FindAnyWidget("Border");
		
		if (m_wBorder && m_bUseColorization)
			m_wBorder.SetOpacity(0);
		
		if (m_wBackground && m_bShowBackgroundOnFocus)
			m_wBackground.SetOpacity(0);
		
		ColorizeBackground(false);
		
		if (!m_wRoot.IsEnabled())
			OnDisabled(false);
		
		// Find parent menu. Due to init order, menu component is not attached to menu root widget right now,
		// so it must be done through call queue.
		GetGame().GetCallqueue().CallLater(FindParentMenu, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		if (m_bCanBeToggled)
			SetToggled(!m_bIsToggled);
		m_OnClicked.Invoke(this);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
				
		if (m_bMouseOverToFocus)
			return false;
		
		if (m_bShowBorderOnHover)
		{
			ShowBorder(true, !m_bNoBorderAnimation);
		}

		if (!m_bUseColorization)
			return false;
			
		if (!m_wBackground)
			return false;
		
		if (m_bIsToggled)
			AnimateWidget.Color(m_wBackground, m_BackgroundSelectedHovered, m_fAnimationRate);
		else
			AnimateWidget.Color(m_wBackground, m_BackgroundHovered, m_fAnimationRate);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
				
		if (m_bMouseOverToFocus)
			return false;
		
		if (m_bUseColorization && m_wBackground)
		{
			if (m_bIsToggled)
				AnimateWidget.Color(m_wBackground, m_BackgroundSelected, m_fAnimationRate);
			else
				AnimateWidget.Color(m_wBackground, m_BackgroundDefault, m_fAnimationRate);
		}
		
		if (m_bShowBorderOnHover)
		{
			ShowBorder(false, !m_bNoBorderAnimation);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		
		if(m_bShowBorderOnFocus)
		{
			ShowBorder(true, !m_bNoBorderAnimation);
		}
		
		if (m_bShowBackgroundOnFocus && m_wBackground)
		{
			if (m_bNoBackgroundAnimation)
				m_wBackground.SetOpacity(1);
			else
				AnimateWidget.Opacity(m_wBackground, 1, m_fAnimationRate, true);
		}
		
		m_OnFocus.Invoke(m_wRoot);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		m_OnFocusLost.Invoke(w);
		
		if (m_bShowBackgroundOnFocus && m_wBackground)
		{
			if (m_bNoBackgroundAnimation)
				m_wBackground.SetOpacity(0);
			else
				AnimateWidget.Opacity(m_wBackground, 0, m_fAnimationRate, true);	
		}
		
		if(m_bShowBorderOnFocus)
		{
			ShowBorder(false, !m_bNoBorderAnimation);
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuSelect()
	{
		if (!m_wRoot.IsEnabled())
			return;
		
		MenuSelectBase();
		
		m_OnClicked.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MenuSelectBase()
	{
		if (m_bCanBeToggled)
		{
			SetToggled(!m_bIsToggled);
		}
		else if (m_bUseColorization && m_wBackground)
		{
			AnimateWidget.Color(m_wBackground, m_BackgroundClicked, m_fAnimationRate);
			GetGame().GetCallqueue().CallLater(ColorizeBackground, m_fAnimationTime * 500 + 1, false, true);
		}
	}
	
	// Public API
	//------------------------------------------------------------------------------------------------
	void SetToggled(bool toggled, bool animate = true, bool invokeChange = true, bool instant = false)
	{
		if (!m_bCanBeToggled)
			return;
		
		m_bIsToggled = toggled;
		ColorizeBackground(animate);
		if (invokeChange)
			m_OnToggled.Invoke(this, m_bIsToggled);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsToggled()
	{
		return m_bIsToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetToggleable(bool togglable)
	{
		m_bCanBeToggled = togglable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetToggleable()
	{
		return m_bCanBeToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowBorder(bool show, bool animate = true)
	{		
		if (m_wBorder)
		{
			if (animate)
				AnimateWidget.Opacity(m_wBorder, show, m_fAnimationRate, true);
			else if (m_wBorder)
				m_wBorder.SetOpacity(show);
		}
		
		m_OnShowBorder.Invoke(this, show);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBorderShown()
	{
		if (!m_wBorder)
			return false;
		return m_wBorder.GetOpacity() != 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Alternate way of setting button background colors through script
	void SetBackgroundColors(Color defColor = null, Color hoverColor = null, Color selectColor = null, Color selectHoverColor = null, Color clickColor = null)
	{
		if (defColor)
			m_BackgroundDefault = defColor;
		
		if (hoverColor)
			m_BackgroundHovered = hoverColor;
		
		if (selectColor)
			m_BackgroundSelected = selectColor;
		
		if (selectHoverColor)
			m_BackgroundSelectedHovered = selectHoverColor;
		
		if (clickColor)
			m_BackgroundClicked = clickColor;
	}
	
	//------------------------------------------------------------------------------------------------
	void ColorizeBackground(bool animate = true)
	{
		if (!m_bUseColorization || !m_wBackground)
			return;
		
		Color color;
		bool isHovered = WidgetManager.GetWidgetUnderCursor() == m_wRoot;
		if (m_bIsToggled && m_bCanBeToggled)
		{
			if (isHovered)
				color = m_BackgroundSelectedHovered;
			else
				color = m_BackgroundSelected;
		}
		else
		{
			if (isHovered)
				color = m_BackgroundHovered;
			else
				color = m_BackgroundDefault;
		}
		
		if (animate)
			AnimateWidget.Color(m_wBackground, color, m_fAnimationRate);
		else
		{
			AnimateWidget.StopAnimation(m_wBackground, WidgetAnimationColor);
			m_wBackground.SetColor(color);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ButtonBaseComponent GetButtonBase(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ButtonBaseComponent.Cast(
				SCR_WLibComponentBase.GetComponent(SCR_ButtonBaseComponent, name, parent, searchAllChildren)
			);
		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds the parent menu of this component
	protected void FindParentMenu()
	{
		// Find parent menu
		MenuBase parentMenu = GetGame().GetMenuManager().GetOwnerMenu(m_wRoot);
		m_ParentMenu = parentMenu;
		
		// Find parent submenu
		Widget wParent = m_wRoot.GetParent();
		while (wParent)
		{
			ScriptedWidgetEventHandler subMenuBase = wParent.FindHandler(SCR_SubMenuBase);
			if (subMenuBase)
			{
				m_ParentSubMenu = SCR_SubMenuBase.Cast(subMenuBase);
				break;
			}
			wParent = wParent.GetParent();
		}
		
		// Warning disabled since we have quite many places where a button is not in a menu
		//if (!parentMenu)
		//	Print(string.Format("Failed to find parent menu of SCR_ButtonBaseComponent %1 on widget %2", this, this.GetRootWidget().GetName()), LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if parent menu is focused, or if there is no parent menu.
	protected bool IsParentMenuFocused()
	{
		if (m_ParentSubMenu)
			return m_ParentSubMenu.GetShown();
		else if(m_ParentMenu)
			return m_ParentMenu.IsFocused();
		else
			return true;
	}
};

//---- REFACTOR NOTE END ----