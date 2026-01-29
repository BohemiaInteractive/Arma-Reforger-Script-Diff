//! Map tool menu entry data class
class SCR_MapToolEntry : Managed
{
	protected bool m_bToolActive;
	protected bool m_bIsEnabled = true;
	protected bool m_bIsVisible = true;
	protected bool m_bIsExclusiveEntry = false;

	bool m_bButtonSoundsDisabled;
	int m_iSortPriority;
	ResourceName m_sImageSet;
	string m_sIconQuad;
	SCR_ToolMenuButtonComponent m_ButtonComp;
	SCR_MapToolMenuUI m_OwnerMenu;

	ref ScriptInvoker m_OnClick = new ScriptInvoker();
	ref ScriptInvokerVoid m_OnDisableMapUIComponent;
	static protected ref ScriptInvoker<SCR_MapToolEntry> s_OnEntryToggled = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvoker GetOnEntryToggledInvoker()
	{
		return s_OnEntryToggled;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnDisableMapUIInvoker()
	{
		if (!m_OnDisableMapUIComponent)
			m_OnDisableMapUIComponent = new ScriptInvokerVoid();

		return m_OnDisableMapUIComponent;
	}

	//------------------------------------------------------------------------------------------------
	void OnDisableMapUIComponent()
	{
		if (m_OnDisableMapUIComponent)
			m_OnDisableMapUIComponent.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! Activation behavior, ON/OFF if entry is active (visual distinction)
	void SetActive(bool toolActive)
	{
		m_bToolActive = toolActive;
		UpdateVisual();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsEntryActive()
	{
		return m_bToolActive;
	}

	//------------------------------------------------------------------------------------------------
	//! Disabled behavior, ON/OFF determines whether can be activated (clicked)
	void SetEnabled(bool isEnabled)
	{
		m_bIsEnabled = isEnabled;
		UpdateVisual();
	}

	//------------------------------------------------------------------------------------------------
	//! Whether entry can be activated
	bool IsEntryEnabled()
	{
		return m_bIsEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Visibility, determines whether the entry is visible with the menu
	void SetVisible(bool state)
	{
		m_bIsVisible = state;
		UpdateVisual();
	}

	//------------------------------------------------------------------------------------------------
	//! Enable button sounds
	void SetButtonSoundsDisabled(bool state)
	{
		m_bButtonSoundsDisabled = state;
	}

	//------------------------------------------------------------------------------------------------
	//! Update visual based on current state
	void UpdateVisual()
	{
		if (!m_ButtonComp)	// handler might not be updated yet when the set is attempted
			return;

		if (m_bToolActive)
		{
			SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
			SetBackgroundColor(Color.FromInt(UIColors.CONTRAST_DEFAULT.PackToInt()));
		}
		else
		{
			SetColor(Color.FromInt(UIColors.WHITE_DISABLED.PackToInt()));
			SetBackgroundColor(Color.FromInt(Color.Black.PackToInt()));
		}

		if (m_bIsEnabled)
			SetBorderColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
		else
			SetBorderColor(Color.FromInt(UIColors.IDLE_DISABLED.PackToInt()));

		m_ButtonComp.GetRootWidget().SetVisible(m_bIsVisible);
	}

	//------------------------------------------------------------------------------------------------
	//! Set entry image color
	//! \param[in] color is target color
	protected void SetColor(notnull Color color)
	{
		m_ButtonComp.m_wImage.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! Set border color
	//! \param[in] color is target color
	protected void SetBorderColor(notnull Color color)
	{
		m_ButtonComp.m_wBorder.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! Set background color
	//! \param[in] color is target color
	protected void SetBackgroundColor(notnull Color color)
	{
		m_ButtonComp.SetBackgroundColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetImageSet()
	{
		return m_sIconQuad;
	}

	//------------------------------------------------------------------------------------------------
	//! On click callback
	protected void OnClick()
	{
		if (!m_bButtonSoundsDisabled)
		{
			if (m_bToolActive)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_HIDE);
			else
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_SHOW);
		}

		s_OnEntryToggled.Invoke(this);
		m_OwnerMenu.DisableInactiveMapUIEntries(this);

		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			m_OwnerMenu.SetToolMenuFocused(false);
	}

	//------------------------------------------------------------------------------------------------
	bool IsExclusiveEntry()
	{
		return m_bIsExclusiveEntry;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] menu
	//! \param[in] imageset
	//! \param[in] icon
	//! \param[in] sortPriority
	//! \param[in] isExclusive
	void SCR_MapToolEntry(SCR_MapToolMenuUI menu, ResourceName imageset, string icon, int sortPriority = 0, bool isExclusive = 0)
	{
		m_OwnerMenu = menu;
		m_sImageSet = imageset;
		m_sIconQuad = icon;
		m_bIsExclusiveEntry = isExclusive;

		if (sortPriority > 0)
			m_iSortPriority = sortPriority;
		else
			m_iSortPriority = 0;

		m_OnClick.Insert(OnClick);
	}
}

//! Map tool menu
class SCR_MapToolMenuUI : SCR_MapUIBaseComponent
{
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourceNamePicker, "Menu icons imageset", "imageset")]
	protected ResourceName m_sToolMenuIcons;

	[Attribute("{47C1A2A23B9CAC97}UI/layouts/Map/MapToolButton.layout", UIWidgets.ResourceNamePicker, "Entry button prefab", "layout")]
	protected ResourceName m_sButtonResource;

	[Attribute("ToolMenu", UIWidgets.EditBox, desc: "Root frame widget name")]
	protected string m_sToolMenuRootName;

	[Attribute("ToolMenuHoriz", UIWidgets.EditBox, desc: "Tool menu widget name")]
	protected string m_sToolBarName;

	[Attribute("ToolMenuButton", UIWidgets.EditBox, desc: "Default name for generated button widgets")]
	protected string m_sButtonDefaultName;

	[Attribute("MenuBackground", UIWidgets.EditBox, desc: "Tool menu background widget name")]
	protected string m_sBackgroundName;

	static ResourceName s_sToolMenuIcons;

	protected bool m_bIsVisible;
	protected bool m_bIsMenuFocused;
	protected Widget m_wBackground;
	protected Widget m_wToolMenuRoot;
	protected Widget m_wToolMenuBar;
	protected SCR_MapCursorModule m_CursorModule;

	protected ref array<ref SCR_MapToolEntry> m_aMenuEntries = {};

	//------------------------------------------------------------------------------------------------
	//! Returns default button name. Bear in mind that actual buttons have added index number to its end in PopulateToolMenu()
	string GetDefaultButtonName()
	{
		return m_sButtonDefaultName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_MapToolEntry> GetMenuEntries()
	{
		return m_aMenuEntries;
	}

	//------------------------------------------------------------------------------------------------
	//! Register menu entry
	//! \param[in] imageset is source imageset
	//! \param[in] icon is quad from the provided imageset
	//! \param[in] sortPriority is disply priority of the icon within the menu, lower value means higher priority
	//! \param[in] isExclusive
	SCR_MapToolEntry RegisterToolMenuEntry(ResourceName imageset, string icon, int sortPriority, bool isExclusive = false)
	{
		SCR_MapToolEntry entry = new SCR_MapToolEntry(this, imageset, icon, sortPriority, isExclusive);
		m_aMenuEntries.Insert(entry);

		if (!m_wToolMenuRoot)
			m_wToolMenuRoot = m_MapEntity.GetMapMenuRoot().FindAnyWidget(m_sToolMenuRootName);

		if (m_wToolMenuRoot)
		{
			m_wToolMenuBar = m_wToolMenuRoot.FindAnyWidget(m_sToolBarName);

			if (!m_wBackground)
				m_wBackground = m_wToolMenuRoot.FindAnyWidget(m_sBackgroundName);
		}

		return entry;
	}

	//------------------------------------------------------------------------------------------------
	//! Add custom inherited entry
	//! \param[in] customEntry is the subject
	void RegisterEntryCustom(SCR_MapToolEntry customEntry)
	{
		m_aMenuEntries.Insert(customEntry);
	}

	//------------------------------------------------------------------------------------------------
	//! Disables all other exclusive map ui entries if activeEntry is also exclusive
	//! \param[in] activeEntry
	void DisableInactiveMapUIEntries(notnull SCR_MapToolEntry activeEntry)
	{
		// We only want to disable other entries when active entry is exclusive
		if (!activeEntry.IsExclusiveEntry())
			return;

		foreach (SCR_MapToolEntry entry : m_aMenuEntries)
		{
			// We only disable exclusive entries other than active entry
			if (entry == activeEntry || !entry.IsExclusiveEntry())
				continue;

			entry.OnDisableMapUIComponent();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Build entries
	protected void PopulateToolMenu()
	{
		Widget button;
		SCR_ToolMenuButtonComponent buttonComp;

		bool sorted = false;
		int count = m_aMenuEntries.Count() - 1;

		while (!sorted)		// sort by prio
		{
			sorted = true;

			for (int i = 0; i < count; i++)
			{
				if (m_aMenuEntries[i + 1].m_iSortPriority < m_aMenuEntries[i].m_iSortPriority)
				{
					m_aMenuEntries.SwapItems(i, i + 1);
					sorted = false;
				}
			}
		}

		foreach (int i, SCR_MapToolEntry entry : m_aMenuEntries)	// use the cached entry data to create layouts and find button handlers
		{
			button = GetGame().GetWorkspace().CreateWidgets(m_sButtonResource, m_wToolMenuBar);
			button.SetName(m_sButtonDefaultName + i);

			buttonComp = SCR_ToolMenuButtonComponent.Cast(button.FindHandler(SCR_ToolMenuButtonComponent));
			if (buttonComp)
			{
				buttonComp.m_OnClicked = entry.m_OnClick;
				buttonComp.SetImage(entry.m_sImageSet, entry.m_sIconQuad);

				entry.m_ButtonComp = buttonComp;

				entry.UpdateVisual();
			}
		}

		SetUIVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Focus menu event when using controller
	protected void OnFocusToolMenu(float value, EActionTrigger reason)
	{
		SetToolMenuFocused(!m_wToolMenuBar.IsEnabled());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFreeCursor(float value, EActionTrigger reason)
	{
		SetToolMenuFocused(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapPan(float x, float y, bool adjusted)
	{
		SetToolMenuFocused(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Set tool menu visibility
	//! \param[in] state is target visibility
	protected void SetUIVisible(bool state)
	{
		m_bIsVisible = state;
		m_wToolMenuRoot.SetVisible(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Set focused state
	//! \param[in] state
	void SetToolMenuFocused(bool state)
	{
		if (state == m_bIsMenuFocused || m_aMenuEntries.IsEmpty())
			return;

		if (state)
		{
			if (!m_CursorModule.HandleSubMenu(true))
				return;

			m_wToolMenuBar.SetEnabled(true);
			GetGame().GetWorkspace().SetFocusedWidget(m_aMenuEntries[0].m_ButtonComp.GetRootWidget());
			m_bIsMenuFocused = true;
		}
		else 	//  Disable tool menu besides root, so its buttons arent being focused when unwanted
		{
			m_wToolMenuBar.SetEnabled(false);
			GetGame().GetWorkspace().SetFocusedWidget(null);
			m_CursorModule.HandleSubMenu(false);
			m_bIsMenuFocused = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	override event bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_wToolMenuRoot && w == m_wToolMenuRoot && !m_wToolMenuBar.IsEnabled())	 // menu is disabled
			SetToolMenuFocused(true);

		GetGame().GetWorkspace().SetFocusedWidget(w);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override event bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		Widget parent = enterW;
		while (parent)
		{
			if (parent == m_wToolMenuRoot)
				break;

			parent = parent.GetParent();
		}

		if (parent != m_wToolMenuRoot && w.IsEnabled())	// disable menu
			SetToolMenuFocused(false);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		m_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));

		if (!m_wToolMenuRoot)
			m_wToolMenuRoot = m_MapEntity.GetMapMenuRoot().FindAnyWidget(m_sToolMenuRootName);

		if (!m_wToolMenuBar)
			m_wToolMenuBar = m_wToolMenuRoot.FindAnyWidget(m_sToolBarName);

		m_wToolMenuRoot.AddHandler(this);

		InputManager inputMgr = GetGame().GetInputManager();
		if (inputMgr)
		{
			inputMgr.AddActionListener("MapToolMenuFocus", EActionTrigger.DOWN, OnFocusToolMenu);
			inputMgr.AddActionListener("MapGamepadCursorX", EActionTrigger.DOWN, OnFreeCursor);
			inputMgr.AddActionListener("MapGamepadCursorY", EActionTrigger.DOWN, OnFreeCursor);
		}

		m_MapEntity.GetOnMapPan().Insert(OnMapPan);

		PopulateToolMenu();

		m_wToolMenuBar.SetEnabled(false); // by default not interactable unless enabled by entering with mouse/activating through input
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		if (m_wToolMenuRoot)
			m_wToolMenuRoot.RemoveHandler(this);

		InputManager inputMgr = GetGame().GetInputManager();
		if (inputMgr)
		{
			inputMgr.RemoveActionListener("MapToolMenuFocus", EActionTrigger.DOWN, OnFocusToolMenu);
			inputMgr.RemoveActionListener("MapGamepadCursorX", EActionTrigger.DOWN, OnFreeCursor);
			inputMgr.RemoveActionListener("MapGamepadCursorY", EActionTrigger.DOWN, OnFreeCursor);
		}

		m_MapEntity.GetOnMapPan().Remove(OnMapPan);
	}

	//------------------------------------------------------------------------------------------------
	bool IsFocused()
	{
		return m_bIsMenuFocused;
	}

	//------------------------------------------------------------------------------------------------
	//! \return menu background
	Widget GetBackgroundWidget()
	{
		if (!m_wBackground)
		{
			m_wToolMenuRoot = m_MapEntity.GetMapMenuRoot().FindAnyWidget(m_sToolMenuRootName);
			m_wBackground = m_wToolMenuRoot.FindAnyWidget(m_sBackgroundName);
		}

		return m_wBackground;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MapToolMenuUI()
	{
		s_sToolMenuIcons = m_sToolMenuIcons;
	}
}
