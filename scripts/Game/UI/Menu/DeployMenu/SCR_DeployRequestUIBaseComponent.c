// Base class for faction, loadout, group and spawn point request handlers
class SCR_DeployRequestUIBaseComponent : ScriptedWidgetComponent
{
	[Attribute("ExpandButton")]
	protected string m_sExpandButton;
	protected Widget m_wExpandButton;

	[Attribute("ExpandButtonText")]
	protected string m_sExpandButtonName;
	protected TextWidget m_wExpandButtonName;
	
	[Attribute("Freq")]
	protected string m_sExpandButtonNameFreq;
	protected TextWidget m_wExpandButtonNameFreq;

	[Attribute("ExpandButtonIcon")]
	protected string m_sExpandButtonIcon;
	protected ImageWidget m_wExpandButtonIcon;
	
	[Attribute("0")]
	protected bool m_bUseListFromButton;

	protected bool m_bEnabled = true;
	protected bool m_bLocked = false;
	protected ref array<SCR_DeployButtonBase> m_aButtons = {};

	protected Widget m_wRoot;

	protected ref ScriptInvoker m_OnButtonFocused = new ScriptInvoker();
	protected ref ScriptInvoker m_OnMouseLeft = new ScriptInvoker();
	protected static ref ScriptInvoker<SCR_DeployRequestUIBaseComponent, bool> s_OnListExpand;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		if (m_bLocked)
		{
			foreach (SCR_DeployButtonBase button : m_aButtons)
			{
				button.Update(dt);
			}
		}
	}

	//! Lock this component's buttons from processing any further requests. Unlock() should be called once you receive a response.
	protected void Lock(SCR_DeployButtonBase btn)
	{
		foreach (SCR_DeployButtonBase button : m_aButtons)
		{
			button.Lock(button == btn);
		}

		m_bLocked = true;
	}

	//! Unlock this component's buttons.
	void Unlock()
	{
		foreach (SCR_DeployButtonBase button : m_aButtons)
		{
			if (button)
				button.Unlock();
		}

		m_bLocked = false;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	/*!
	Gets first available button from the m_aButtons list.
	\return button instance.
	*/
	SCR_DeployButtonBase GetFirstValidButton()
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			if (btn)
				return btn;
		}
		
		return null;
	}
	
	protected void ToggleCollapsed();
	void SetExpanded(bool expanded);
	bool IsExpanded();

	protected void OnListExpand(SCR_DeployRequestUIBaseComponent component, bool expanded)
	{
		if (component == this)
			return;
		
		SetExpanded(false);
	}

	//! Marks which widget should be used as a root for creating buttons.
	void SetListWidget(Widget list)
	{
	}

	/*! Gets current list widget.
	\return current list
	*/
	Widget GetListWidget()
	{
	}

	protected void OnMouseLeft()
	{
		m_OnMouseLeft.Invoke();
	}

	ScriptInvoker GetOnButtonFocused()
	{
		return m_OnButtonFocused;
	}

	ScriptInvoker GetOnMouseLeft()
	{
		return m_OnMouseLeft;
	}
	
	static ScriptInvoker GetOnListCollapse()
	{
		if (!s_OnListExpand)
			s_OnListExpand = new ScriptInvoker();
		
		return s_OnListExpand;
	}
};

//------------------------------------------------------------------------------------------------
// Base class for faction, loadout and group request buttons
class SCR_DeployButtonBase : SCR_ButtonImageComponent
{
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorSelected;

	[Attribute("0.947 0.056 0.056 0.62", UIWidgets.ColorPicker)]
	protected ref Color m_ColorWarning;

	[Attribute("Elements")]
	protected string m_sElements;
	protected Widget m_wElements;

	[Attribute("List")]
	protected string m_sList;
	protected Widget m_wList;
	
	[Attribute("GridList")]
	protected string m_sGridList;
	protected GridLayoutWidget m_wGridList;
	
	[Attribute("BackgroundHighlight")]
	protected string m_sBackgroundHighlight;
	protected Widget m_wBackgroundHighlight;

	protected static SCR_DeployButtonBase s_ExpandedList;

	protected SCR_DeployRequestUIBaseComponent m_ParentHandler;

	protected ButtonWidget m_wRootButton;
	protected Widget m_wLoading;
	protected SCR_LoadingSpinner m_Loading;
	
	protected SCR_ScriptedWidgetTooltip m_ControlsTooltip;
	
	protected bool m_bCanBeUnlocked = true;
	protected bool m_bShowTooltip = true;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wRootButton = ButtonWidget.Cast(w);
		m_wLoading = w.FindAnyWidget("Loading");
		if (m_wLoading)
			m_Loading = SCR_LoadingSpinner.Cast(m_wLoading.FindHandler(SCR_LoadingSpinner));
		
		m_wBackgroundHighlight = w.FindAnyWidget(m_sBackgroundHighlight);

		if (w.GetParent())
		{
			m_wGridList = GridLayoutWidget.Cast(w.GetParent().FindAnyWidget(m_sGridList));
			m_wList = w.GetParent().FindAnyWidget(m_sList);
		}
		else
		{
			m_wGridList = GridLayoutWidget.Cast(w.FindAnyWidget(m_sGridList));
			m_wList = w.FindAnyWidget(m_sList);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
		return super.OnFocusLost(w, x, y);
	}
	
	Widget GetList()
	{
		return m_wList;
	}

	GridLayoutWidget GetGridList()
	{
		return m_wGridList;
	}

	void ExpandList()
	{
		if (!m_wList)
			return;

		if (s_ExpandedList)
			s_ExpandedList.CollapseList();

		m_wList.SetVisible(true);
		s_ExpandedList = this;
	}

	void CollapseList()
	{
		if (m_wList)
			m_wList.SetVisible(false);
	}

	void SetParentHandler(SCR_DeployRequestUIBaseComponent parent)
	{
		m_ParentHandler = parent;
	}

	void HideTooltip()
	{
		if (m_ControlsTooltip)
			m_ControlsTooltip.ForceHidden();
	}

	void Update(float dt)
	{
		if (m_Loading && m_wLoading.IsVisible())
		{
			m_Loading.Update(dt);
		}
	}

	//! Lock the button (eg. during a request)
	void Lock(bool loadingAnim = false)
	{
		if (m_wLoading)
			m_wLoading.SetVisible(loadingAnim);

		SetEnabled(false);
	}

	//! Unlock the button (eg. after receiving a request response)
	void Unlock()
	{
		if (m_wLoading)
			m_wLoading.SetVisible(false);
		if (ShouldUnlock())
			SetEnabled(true);
	}

	//! Visually set the button as selected.
	void SetSelected(bool selected)
	{
		Color color = Color.FromInt(Color.WHITE);
		
		if (selected)
			color = Color.FromInt(m_ColorSelected.PackToInt());
		
		if (m_wElements)
			m_wElements.SetColor(color);
		
		if (m_wBackgroundHighlight)
			m_wBackgroundHighlight.SetVisible(selected);
	}

	//! Set the button focused.
	void SetFocused()
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wRootButton);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_wRoot.IsEnabled())
			return false;
		
		return super.OnClick(w, x, y, button);
	}

	override void SetEnabled(bool enabled, bool animate = true)
	{
		m_wRoot.SetEnabled(enabled);

		if (enabled)
			OnEnabled(animate);
		else
			OnDisabled(animate);
	}

	//! Return true when the button should be unlocked when calling the Unlock() method.
	protected bool ShouldUnlock()
	{
		return m_bCanBeUnlocked;
	}

	//! Marks if the buttons should be unlocked when calling the Unlock() method.
	void SetShouldUnlock(bool shouldUnlock)
	{
		m_bCanBeUnlocked = shouldUnlock;
	}

	//! Get if tooltip should be shown.
	protected bool ShouldShowTooltip()
	{
		return m_bShowTooltip;
	}

	//! Set if tooltip should be shown.
	void SetTooltipAvailable(bool available)
	{
		m_bShowTooltip = available;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		if (!tooltip.IsValid("Join") && !tooltip.IsValid("JoinGroup"))
			return;
		
		if (!m_bShowTooltip)
			tooltip.ForceHidden();
		
		m_ControlsTooltip = tooltip;
		
		SCR_ActionHintScriptedWidgetTooltip content = SCR_ActionHintScriptedWidgetTooltip.Cast(tooltip.GetContent());
		if (!content)
			return;
		
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
			content.SetAction("MenuSelectMouse");
		else
			content.ResetAction();
	}
}