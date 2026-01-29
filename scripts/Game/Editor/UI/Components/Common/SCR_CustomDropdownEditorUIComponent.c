class SCR_CustomDropdownEditorUIComponent : ScriptedWidgetComponent
{
	//TODO: Check how to close the drop down if anything else is clicked (How does the combobox work?)
	[Attribute("ItemHolder")]
	protected string m_sItemHolderName;
	
	[Attribute("ModeSelection_List0")]
	protected string m_sListWidgetName;
	
	[Attribute("Stripe")]
	protected string m_sListWidgetStripeName;
	
	[Attribute("ModeSelection_Mode0")]
	protected string m_sDropdownButtonName;
	
	[Attribute("Content")]
	protected string m_DropdownTextName;
	
	[Attribute("PcArrow")]
	protected string m_sArrowWidgetName;
	
	[Attribute("GamepadHint")]
	protected string m_sGamepadHintWidgetName;
	
	[Attribute()]
	protected ResourceName m_sItemPrefab;
	
	[Attribute("0.025", "Fade delay in seconds. For every new button that is added the delay increases slightly")]
	protected float m_fFadeDelayNextButton;
	
	[Attribute("4", "How fast each button appears")]
	protected float m_fButtonFadeSpeed;
	
	[Attribute("-1", desc: "How much will the list of controls hints be offset vertically.\nKeep -1 to leave it unaffected.")]
	protected int m_iAvailableActionsOffsetY;
	
	//Refs
	protected Widget m_Root;
	protected Widget m_ItemHolder;
	protected Widget m_ListWidget;
	protected SCR_FadeUIComponent m_ListWidgetStripeFadeComponent;
	protected TextWidget m_DropdownText;
	protected ImageWidget m_ArrowWidget;
	protected SCR_ButtonImageComponent m_DropdownButton;
	protected ref array<SCR_ButtonImageComponent> m_aItemButtons = {};
	protected ref array<SCR_EditorModeUIInfo> m_aModeUIInfo = {};
	
	//States
	protected bool m_bIsOpened;
	protected int m_iSelectedItem;
	protected bool m_bHovered;
	
	protected ref ScriptInvoker Event_OnDropdownOpened;
	protected ref ScriptInvoker Event_OnDropdownClosed;
	protected ref ScriptInvoker Event_OnChanged;
	
	protected int m_iCloseOnIndex;
	protected bool m_bIsEnabled = true;
	
	//------------------------------------------------------------------------------------------------
	//! If enabled shows the dropdown arrow else it looks like a static header
	//! \param[in] enabled to set enabled or disabled
	void SetDropdownEnable(bool enable)
	{
		m_bIsEnabled = enable;
		
		if (m_bIsOpened)
			CloseDropdown();
		
		m_Root.SetEnabled(enable);
		
		if (enable)
		{
			OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		}
		else 
		{
			m_ArrowWidget.SetVisible(false);
		
			Widget gamePadhintWidget = m_Root.FindAnyWidget(m_sGamepadHintWidgetName);
			if (gamePadhintWidget)
				gamePadhintWidget.SetVisible(false);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDropDownClicked(SCR_ButtonImageComponent button)
	{
		if (m_bIsOpened)
			CloseDropdown();
		else
			OpenDropdown();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemClicked(SCR_ButtonImageComponent button)
	{
		int index = m_aItemButtons.Find(button);
		
		if (index < 0)
			return;
		
		SetCurrentItem(index, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDropDownFocus()
	{
		if (!m_bIsOpened)
			OpenDropdown();
		
		GetGame().GetWorkspace().SetFocusedWidget(m_aItemButtons[0].GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuActionLeft()
	{
		CloseDropdown();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] uiInfo
	//! \param[in] color
	void AddItem(SCR_EditorModeUIInfo uiInfo, Color color)
	{
		if (!m_ItemHolder)
			return;
		
		Widget itemWidget = GetGame().GetWorkspace().CreateWidgets(m_sItemPrefab, m_ItemHolder);
		if (!itemWidget)
			return;
		
		SCR_ButtonImageComponent newItem = SCR_ButtonImageComponent.Cast(itemWidget.FindHandler(SCR_ButtonImageComponent));
		
		if (newItem)
		{			
			newItem.m_OnClicked.Insert(OnItemClicked);
			m_aItemButtons.Insert(newItem);
			m_aModeUIInfo.Insert(uiInfo);
			
			uiInfo.SetIconTo(newItem.GetImageWidget());
			newItem.GetImageWidget().SetVisible(true);
			
			TextWidget textWidget = TextWidget.Cast(newItem.GetRootWidget().FindAnyWidget("Text"));
			if (textWidget)
				uiInfo.SetNameTo(textWidget);
			
			Widget sideBar = newItem.GetRootWidget().FindAnyWidget("SideBar");
			
			if (sideBar)
				sideBar.SetColor(uiInfo.GetModeColor());
			
			SCR_LinkTooltipTargetEditorUIComponent tooltip = SCR_LinkTooltipTargetEditorUIComponent.Cast(itemWidget.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
			if (tooltip)
				tooltip.SetInfo(uiInfo);
		}
		else
		{
			itemWidget.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! If dropdown is open
	//! \return bool if dropdown is open
	//!
	bool IsOpened()
	{
		return m_bIsOpened;
	}
	
	//---- REFACTOR NOTE START: 90 and 270 rotation is understandable but it might be hard to find in case of tweaking
	
	//------------------------------------------------------------------------------------------------
	//! Open the dropdown
	//! \param[in] focusIndex set which button should be focused. Ignored if -1
	void OpenDropdown(int focusIndex =-1)
	{
		float fadeDelay = 0;
		SCR_FadeUIComponent fadeComponent
		foreach (SCR_ButtonImageComponent button: m_aItemButtons)
		{
			if (!button.GetRootWidget().IsVisible())
				continue;
			
			fadeComponent = SCR_FadeUIComponent.Cast(button.GetRootWidget().FindHandler(SCR_FadeUIComponent));
			if (fadeComponent)
			{
				fadeComponent.SetFadeInSpeed(m_fButtonFadeSpeed);
				fadeComponent.DelayedFadeIn(fadeDelay * 1000);
				fadeDelay += m_fFadeDelayNextButton;
			}
		}
		
		if (m_ListWidgetStripeFadeComponent)
			m_ListWidgetStripeFadeComponent.DelayedFadeIn(fadeDelay * 1000);
		
		m_ListWidget.SetVisible(true);
		
		// Set arrow image angle
		if (m_ArrowWidget)
			m_ArrowWidget.SetRotation(90);
		
		if (m_iAvailableActionsOffsetY != -1)
		{
			SCR_AvailableActionsDisplay availableActionsDisplay = SCR_AvailableActionsDisplay.Cast(GetGame().GetHUDManager().FindInfoDisplay(SCR_AvailableActionsDisplay));
			if (availableActionsDisplay)
				availableActionsDisplay.SetAdditionalOffsetY(m_iAvailableActionsOffsetY);
		}
		
		//Rotate arrow
		
		m_bIsOpened = true;
		if (Event_OnDropdownOpened)
			Event_OnDropdownOpened.Invoke(this);
		
		if (focusIndex > -1)
		{
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			if (!workspace) 
				return;
		
			workspace.SetFocusedWidget(m_aItemButtons[focusIndex].GetRootWidget());
		}		
		
		// Setup action listeners
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnMenuActionLeft);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Close the dropdown
	void CloseDropdown()
	{
		m_ListWidget.SetVisible(false);
		
		// Set arrow image angle
		if (m_ArrowWidget)
			m_ArrowWidget.SetRotation(270);
		
		if (m_iAvailableActionsOffsetY != -1)
		{
			SCR_AvailableActionsDisplay availableActionsDisplay = SCR_AvailableActionsDisplay.Cast(GetGame().GetHUDManager().FindInfoDisplay(SCR_AvailableActionsDisplay));
			if (availableActionsDisplay)
				availableActionsDisplay.SetAdditionalOffsetY(0);
		}
		
		m_bIsOpened = false;
		if (Event_OnDropdownClosed)
			Event_OnDropdownClosed.Invoke(this);
		
		SCR_FadeUIComponent fadeComponent;
		foreach (SCR_ButtonImageComponent button: m_aItemButtons)
		{
			fadeComponent = SCR_FadeUIComponent.Cast(button.GetRootWidget().FindHandler(SCR_FadeUIComponent));
			if (fadeComponent)
				fadeComponent.CancelFade(false);
		}

		if (m_ListWidgetStripeFadeComponent)
			m_ListWidgetStripeFadeComponent.CancelFade(false);
		
		// Remove action listeners
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnMenuActionLeft);
	}
	
	//---- REFACTOR NOTE END ----
	
	protected void OnLMB()
	{
		if (m_bIsOpened && !m_bHovered)
			CloseDropdown();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set current item selected in dropdown
	//! \param[in] index index of item
	//! \param[in] callOnChanged if the Event_OnChanged event should be called
	void SetCurrentItem(int index, bool callOnChanged)
	{
		m_iSelectedItem = index;
		
		if (callOnChanged && Event_OnChanged)
			Event_OnChanged.Invoke(this, index);
		
		if (!m_aModeUIInfo.IsIndexValid(index))
			return;
		
		SCR_EditorModeUIInfo uiInfo = m_aModeUIInfo[index];
		
		if (m_DropdownText)
			uiInfo.SetNameTo(m_DropdownText);
		
		if (m_DropdownButton)
			uiInfo.SetIconTo(m_DropdownButton.GetImageWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get selected index
	//! \return int get selected index
	int GetSelectedIndex()
	{
		return m_iSelectedItem;
	}

//	//------------------------------------------------------------------------------------------------
//	//! Clear all elements
//	void ClearAll()
//	{
//	}
	
	//------------------------------------------------------------------------------------------------
	//! \return root widget
	Widget GetRootWidget()
	{
		return m_Root;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set item enabled
	//! \param[in] index index of item
	//! \param[in] enabled enabled state
	void SetItemEnabled(int index, bool enabled)
	{		
		if (index < 0 || index >= m_aItemButtons.Count())
			return;
	
		m_aItemButtons[index].GetRootWidget().SetEnabled(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set item visible
	//! \param[in] index index of item
	//! \param[in] visible visible state
	//!
	void SetItemVisible(int index, bool visible)
	{
		if (index < 0 || index >= m_aItemButtons.Count())
			return;
		
		m_aItemButtons[index].GetRootWidget().SetVisible(visible);
	}
	
	////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! \return On Change script invoker
	ScriptInvoker GetOnChanged()
	{
		if (!Event_OnChanged)
			Event_OnChanged = new ScriptInvoker();

		return Event_OnChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return On Opened script invoker
	ScriptInvoker GetOnOpened()
	{
		if (!Event_OnDropdownOpened)
			Event_OnDropdownOpened = new ScriptInvoker();

		return Event_OnDropdownOpened;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return On Closed script invoker
	ScriptInvoker GetOnClosed()
	{
		if (!Event_OnDropdownClosed)
			Event_OnDropdownClosed = new ScriptInvoker();

		return Event_OnDropdownClosed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{	
		if (!m_bIsEnabled)
			return;
		
		m_ArrowWidget.SetVisible(!isGamepad);
		
		Widget gamePadhintWidget = m_Root.FindAnyWidget(m_sGamepadHintWidgetName);
		if (gamePadhintWidget)
			gamePadhintWidget.SetVisible(isGamepad);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bHovered = true;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bHovered = false;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		GetGame().GetInputManager().AddActionListener("MouseLeft", EActionTrigger.DOWN, OnLMB);
		m_Root = w;
		m_ItemHolder = w.FindAnyWidget(m_sItemHolderName);
		m_ListWidget = w.FindAnyWidget(m_sListWidgetName);
		m_ArrowWidget = ImageWidget.Cast(w.FindAnyWidget(m_sArrowWidgetName));
		
		if (m_ListWidget)
		{
			Widget listWidgetStripe = m_ListWidget.FindAnyWidget(m_sListWidgetStripeName);
			
			if (listWidgetStripe)
			{
				m_ListWidgetStripeFadeComponent = SCR_FadeUIComponent.Cast(listWidgetStripe.FindHandler(SCR_FadeUIComponent));
				
				if (m_ListWidgetStripeFadeComponent)
					m_ListWidgetStripeFadeComponent.SetFadeInSpeed(m_fButtonFadeSpeed);
			}
		}
			
		
		Widget dropDownWidget = w.FindAnyWidget(m_sDropdownButtonName);
		if (!dropDownWidget)
			return;
		
		m_DropdownText = TextWidget.Cast(w.FindAnyWidget(m_DropdownTextName));
		if (!m_DropdownText)
			return;
		
		m_DropdownButton = SCR_ButtonImageComponent.Cast(dropDownWidget.FindHandler(SCR_ButtonImageComponent));
		m_DropdownButton.m_OnClicked.Insert(OnDropDownClicked);
		m_DropdownButton.m_OnFocus.Insert(OnDropDownFocus);
		
		ScriptInvoker invoker = GetGame().OnInputDeviceIsGamepadInvoker();
		if (invoker)
			invoker.Insert(OnInputDeviceIsGamepad);

		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		ScriptInvoker invoker = GetGame().OnInputDeviceIsGamepadInvoker();
		if (invoker)
			invoker.Remove(OnInputDeviceIsGamepad);
		
		GetGame().GetInputManager().RemoveActionListener("MouseLeft", EActionTrigger.DOWN, OnLMB);
	}
}
