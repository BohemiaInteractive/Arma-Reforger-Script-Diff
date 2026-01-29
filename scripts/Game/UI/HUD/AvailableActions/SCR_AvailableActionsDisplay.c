//------------------------------------------------------------------------------------------------
class SCR_AvailableActionsWidget
{
	protected Widget m_wRootWidget;
	protected OverlayWidget m_wOverlayWidget;
	
	protected ButtonWidget m_wButtonWidget;
	protected SCR_InputButtonComponent m_NavigationButton;

	protected bool m_bMustUpdate;
	
	protected const string BUTTON_WIDGET_NAME = "NavigationButton1";

	//------------------------------------------------------------------------------------------------
	//! Method used to inform this hint that it should update its content
	void SetForcedUpdate()
	{
		m_bMustUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text, string name, EInputDeviceType currentInputDevice = EInputDeviceType.KEYBOARD)
	{
		if (!m_NavigationButton)
			return;

		if (m_NavigationButton.SetAction(text, currentInputDevice, m_bMustUpdate))
			m_NavigationButton.SetLabel(name);
		else
			SetVisible(false);

		if (m_bMustUpdate)
			m_bMustUpdate = false;
	}

	//------------------------------------------------------------------------------------------------
	void SetSize(int size)
	{
		if (m_NavigationButton)
			m_NavigationButton.SetSize(size);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(bool isVisible)
	{
		if (m_wRootWidget)
			m_wRootWidget.SetVisible(isVisible);
	}
	
	
	void SetPadding(float padding)
	{
		if (!m_wOverlayWidget)
			return;
		
		float left, top, right, bottom;
		OverlaySlot.GetPadding(m_wOverlayWidget, left, top, right, bottom);
		OverlaySlot.SetPadding(m_wOverlayWidget, left, padding, right, padding);
	}

	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		if (m_wRootWidget)
			return m_wRootWidget.IsVisible();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AvailableActionsWidget CreateActionsWidget(string layout, Widget parent, string text = "", string name = "")
	{
		if (layout == string.Empty)
			return null;

		if (!parent)
			return null;

		SCR_AvailableActionsWidget instance = new SCR_AvailableActionsWidget();
		if (!instance)
			return null;

		Widget root = GetGame().GetWorkspace().CreateWidgets(layout, parent);
		instance.m_wRootWidget = root;
		if (root)
		{			
			instance.m_wButtonWidget = ButtonWidget.Cast(root.FindAnyWidget(instance.BUTTON_WIDGET_NAME));
			instance.m_NavigationButton = SCR_InputButtonComponent.FindComponent(instance.m_wButtonWidget);
			
			if (instance.m_NavigationButton)
				instance.m_NavigationButton.SetClickSoundDisabled(true);
		}
		instance.SetText(text, name);

		return instance;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget() { return m_wRootWidget; }

	//------------------------------------------------------------------------------------------------
	private void SCR_AvailableActionsWidget() {}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AvailableActionsWidget()
	{
		if (m_wRootWidget)
			m_wRootWidget.RemoveFromHierarchy();
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AvailableActionContextTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sAction");
		if (index == -1)
			return false;

		// Tag string
		string tag = "";
		source.Get("m_sTag", tag);

		source.Get("m_sAction", title);

		// Enabled string
		bool enabled;
		source.Get("m_bEnabled", enabled);

		string enabledStr = "x";
		if (enabled)
			enabledStr = "on";

		// Setup title string
		title = "(" + title + ") " + tag + " - " + enabledStr;
		
		return true;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_AvailableActionContextTitle()]
class SCR_AvailableActionContext
{
	[Attribute("true")]
	protected bool m_bEnabled;

	[Attribute("0", UIWidgets.EditBox, "Amount of milliseconds to hide this hint after it was shown, 0 means no hiding", params: "0 inf")]
	protected int m_iTimeForHide;

	[Attribute("0", UIWidgets.EditBox, "Amount of milliseconds that have to pass while conditions are met in oder for this hint to be shown", params: "0 inf")]
	protected int m_iTimeToShow;

	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_AvailableActionCondition> m_aConditions;

	[Attribute("DefaultTag", UIWidgets.EditBox, "Tag for quick search and recongnition. Change default tag to allow easier search.")]
	protected string m_sTag;

	[Attribute("", UIWidgets.EditBox, "Name of the action in action manager")]
	protected string m_sAction;

	[Attribute("", UIWidgets.EditBox, "Name of the action in to be displayed in UI")]
	protected string m_sName;

	protected const string MARKUP_FORMAT = "<action name=\"%1\"/>";

	protected float m_fShowCountdown = m_iTimeToShow;
	protected float m_fHideCountdown = m_iTimeForHide;

	//------------------------------------------------------------------------------------------------
	string ToString(bool forceText = true)
	{
		string text = "";
		if (forceText)
			text = string.Format(MARKUP_FORMAT, m_sAction);
		else
			text = string.Format(MARKUP_FORMAT, m_sAction);

		return string.Format("%1", text);
	}

	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sAction;
	}

	//------------------------------------------------------------------------------------------------
	// TODO: This should possibly be setup somewhere in the actions manager,
	// so it can be reused instead of having to rely on setting it up manually in this object?
	string GetUIName()
	{
		if (m_sName == string.Empty)
			return m_sAction;

		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAvailable(SCR_AvailableActionsConditionData data, float timeSlice)
	{
		if (!m_bEnabled)
			return false;
		
		bool isOk = true;
		foreach (SCR_AvailableActionCondition cond : m_aConditions)
		{
			if (!cond.IsEnabled())
				continue;

			if (!cond.IsAvailable(data))
			{
				isOk = false;
				break;
			}
		}

		if (m_iTimeToShow > 0)
		{// Show only when conditions were met for long enough
			if (!isOk)
			{
				m_fShowCountdown = m_iTimeToShow;
				m_fHideCountdown = m_iTimeForHide;
			}
			else if (m_fShowCountdown > 0)
			{
				m_fShowCountdown -= timeSlice * 1000;
			}

			isOk = isOk && m_fShowCountdown <= 0;
		}

		if (m_iTimeForHide > 0 && m_fShowCountdown <= 0)
		{// Hide if time is over
			if (!isOk)
				m_fHideCountdown = m_iTimeForHide;
			else if (m_fHideCountdown > 0)
				m_fHideCountdown -= timeSlice * 1000;

			isOk = isOk && m_fHideCountdown > 0;
		}

		return isOk;
	}

	//------------------------------------------------------------------------------------------------
	//! returns true if action display is currently counting down twords its delayed fade in or out
	bool IsDelayed()
	{
		return m_fShowCountdown < m_iTimeToShow || m_fHideCountdown < m_iTimeForHide;
	}

	//------------------------------------------------------------------------------------------------
	//! Reset fade in and fade out timers to their default values
	void ResetTimers()
	{
		m_fShowCountdown = m_iTimeToShow;
		m_fHideCountdown = m_iTimeForHide;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_AvailableActionsDisplay : SCR_InfoDisplayExtended
{
	//! List of all actions to process at any given moment, these are filtered and available ones are displayed
	[Attribute("", UIWidgets.Object, "List of all actions to process at any given moment, these are filtered and available ones are displayed")]
	protected ref array<ref SCR_AvailableActionContext> m_aActions;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout used for individual action widgets", params: "layout")]
	protected ResourceName m_sChildLayout;

	[Attribute()]
	protected float m_fDefaultOffsetY;

	protected float m_fOffsetY;
	protected float m_fAdditionalOffsetY = 0;

	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref AvailableActionLayoutBehaviorBase> m_aBehaviors;
	
	[Attribute("40", desc: "Size of the small button which will be used when there are too many bigger buttons. (Height in px)")]
	protected int m_iButtonSizeSmall;
	
	[Attribute("44", desc: "Size of the medium button which will be used when there are too many bigger buttons. (Height in px)")]
	protected int m_iButtonSizeMedium;
	
	[Attribute("48", desc: "Size of the large button. This is the maximum size of the shown button. If there are too many one of the above will be used instead. (Height in px)")]
	protected int m_iButtonSizeLarge;

	//! Count of maximum elements that will be pre-cached
	protected const int PRELOADED_WIDGETS_COUNT = 16;

	//! List of available action widget containers
	protected ref array<ref SCR_AvailableActionsWidget> m_aWidgets = new array<ref SCR_AvailableActionsWidget>();

	//! Layout widget in root or null if new
	protected Widget m_wLayoutWidget;

	//! Amount of previously shown widgets
	protected int m_iLastCount;

	//! Game settings
	protected bool m_bIsEnabledSettings;

	//! If user changed keybing then it should force update of hints
	protected bool m_bForceUpdate;

	//!
	protected EInputDeviceType m_eCurrentInputDevice;

	//!
	protected ref SCR_AvailableActionsConditionData m_data;
	protected SCR_InfoDisplaySlotHandler m_slotHandler;
	protected SCR_HUDSlotUIComponent m_HUDSlotComponent;

	//! Timer for fetching data limitation
	protected float m_fDataFetchTimer;
	
	//! Default values for how many large / medium buttons will be displayed before they switch to a smaller one. (This gets overritten by the SlotSize calculation)
	protected int m_iMaxActionsBig = 5;
	protected int m_iMaxActionsMedium = 7;

	//! This values are used to determine how many actions fit in the available space. (Space / devider = MaxActions)
	const int HEIGHT_DIVIDER_BIG = 70;
	const int HEIGHT_DIVIDER_MEDIUM = 50;	
	
	const int HINT_SIZE_Y = 34;
	const int DEFAULT_FONT_SIZE = 20;
	const int MIN_FONT_SIZE = 16;	

	//protected ref array<SCR_AvailableActionContext> availableActions;
	//protected int actionsCount;

	//------------------------------------------------------------------------------------------------
	//! Go through the list of passed in action names of inActions
	//! and populate the outActions list with actions that are currently active (available)
	//! Returns count of available actions or -1 in case of error
	protected int GetAvailableActions(SCR_AvailableActionsConditionData data, array<ref SCR_AvailableActionContext> inActions, out array<SCR_AvailableActionContext> outActions, float timeSlice)
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return 0;

		InputManager inputManager = game.GetInputManager();
		if (!inputManager)
			return 0;

		outActions.Clear();
		int count = 0;
		foreach (SCR_AvailableActionContext action : inActions)
		{
			string actionName = action.GetActionName();
			
			if (actionName == string.Empty)
				continue;

			if (!inputManager.IsActionActive(actionName))
			{
				if (action.IsDelayed())
					action.ResetTimers();

				continue;
			}

			if (!action.IsAvailable(data, timeSlice))
				continue;
			
			outActions.Insert(action);
			count++;
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanBeShown(IEntity controlledEntity)
	{
		if (SCR_EditorManagerEntity.IsOpenedInstance())
			return true;

		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		ECharacterLifeState lifeState = controller.GetLifeState();
		if (lifeState == ECharacterLifeState.ALIVE)
			return true;
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateIsEnabled()
	{
		// In case settings are not found, fallback to enabled
		m_bIsEnabledSettings = true;

		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (gameplaySettings)
			gameplaySettings.Get("m_bControlHints", m_bIsEnabledSettings);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return; // Mandatory
		
		//Check if the SlotUIComponent is still valid otherwise change to the new one
		if (m_HUDSlotComponent != m_slotHandler.GetSlotUIComponent())
		{
			if (m_HUDSlotComponent)
				m_HUDSlotComponent.GetOnResize().Remove(OnSlotUIResize);
			
			m_HUDSlotComponent = m_slotHandler.GetSlotUIComponent();
			if (!m_HUDSlotComponent)
				return;
			
			m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
		}

		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		
		//~ Can show checks if controlledEntity is valid or if editor is open
		if (m_bIsEnabledSettings && CanBeShown(controlledEntity))
			m_wRoot.SetVisible(true);
		else
		{
			m_wRoot.SetVisible(false);
			return;
		}

		if (!m_data)
			m_data = new SCR_AvailableActionsConditionData();

		m_fDataFetchTimer += timeSlice;

		if (m_fDataFetchTimer >= 0.25)
		{
			m_data.FetchData(controlledEntity, m_fDataFetchTimer);
			DisplayWidgetsUpdate(m_fDataFetchTimer);

			m_fDataFetchTimer = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	private void DisplayWidgetsUpdate(float timeSlice)
	{
		array<SCR_AvailableActionContext> availableActions = new array<SCR_AvailableActionContext>();
		int actionsCount = GetAvailableActions(m_data, m_aActions, availableActions, timeSlice);

		int updateCount = Math.Max(m_iLastCount, actionsCount);
		bool shouldBeVisible;
		SCR_AvailableActionContext availableAction;
		foreach (int i, SCR_AvailableActionsWidget widget : m_aWidgets)
		{
			if (i >= updateCount)
				break;

			if (!widget)
				continue;

			shouldBeVisible = i < actionsCount;
			if (widget.IsVisible() != shouldBeVisible)
			{
				if (shouldBeVisible)
					DisplayHint(widget.GetRootWidget(), UIConstants.FADE_RATE_SUPER_FAST, UIConstants.FADE_RATE_SUPER_FAST);
				else
					HintFadeOut(widget.GetRootWidget(), UIConstants.FADE_RATE_DEFAULT, UIConstants.FADE_RATE_FAST);
			}

			if (actionsCount > m_iMaxActionsMedium)
				widget.SetSize(m_iButtonSizeSmall);
			else if (actionsCount > m_iMaxActionsBig)
				widget.SetSize(m_iButtonSizeMedium);
			else
				widget.SetSize(m_iButtonSizeLarge);

			if (!shouldBeVisible)
				continue;

			if (m_bForceUpdate)
				widget.SetForcedUpdate();

			if (!availableActions.IsIndexValid(i))
				continue;

			availableAction = availableActions[i];
			if (availableAction)
				widget.SetText(availableAction.GetActionName(), availableAction.GetUIName(), m_eCurrentInputDevice);
		}

		m_bForceUpdate = false;

		// Acknowledge new count
		m_iLastCount = actionsCount;

		ApplyLayoutBehavior();
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayHint(Widget widget, float delayFade, float delayShrink)
	{
		widget.SetOpacity(0);
		VerticalLayoutSlot.SetPadding(widget, 0, -HINT_SIZE_Y, 0, 0);
		widget.SetVisible(true);

		// Clear hiding
		GetGame().GetCallqueue().Remove(HintShrink);
		GetGame().GetCallqueue().Remove(HintHide);

		// Animations
		float padding[4] = {0, 0, 0, 0};
		AnimateWidget.Padding(widget, padding, delayShrink);
		AnimateWidget.Opacity(widget, 1, delayFade);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout
	protected void HintFadeOut(Widget widget, float delayFade, float delayShrink)
	{
		/*GetGame().GetCallqueue().Remove(HintShrink);
		GetGame().GetCallqueue().Remove(HintHide);*/

		AnimateWidget.Opacity(widget, 0, delayFade);
		GetGame().GetCallqueue().CallLater(HintShrink, 1000 / delayFade, false, widget, delayShrink);
		//HintShrink(widget, delayShrink);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout
	protected void HintShrink(Widget widget, float delayShrink)
	{
		float padding[4] = {0, -HINT_SIZE_Y, 0, 0};
		AnimateWidget.Padding(widget, padding, delayShrink);
		GetGame().GetCallqueue().CallLater(HintHide, 1000 / delayShrink, false, widget);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout
	protected void HintHide(Widget widget)
	{
		widget.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Set available actions layout y position
	void SetOffsetY(float offset = -1)
	{
		return;
		// Default position
		if (offset == -1)
			offset = m_fDefaultOffsetY;

		m_fOffsetY = offset;

		// Set layout position
		//ApplyOffsets();
	}

	//------------------------------------------------------------------------------------------------
	void SetAdditionalOffsetY(float offset)
	{
		return;
		m_fAdditionalOffsetY = offset;
		//ApplyOffsets();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyOffsets()
	{
		return;
		float sum = (-m_fOffsetY) + (-m_fAdditionalOffsetY);

		if (m_wLayoutWidget)
			FrameSlot.SetPosY(m_wLayoutWidget, sum);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayStartDraw(IEntity owner)
	{
		if (m_wRoot)
			m_wLayoutWidget = m_wRoot.FindAnyWidget("VerticalLayout");

		// Layout setup
		SetOffsetY();

		int count = m_aWidgets.Count();
		int toGenerate = PRELOADED_WIDGETS_COUNT - count;
		for (int i = 0; i < toGenerate; i++)
		{
			if (i < toGenerate)
			{
				SCR_AvailableActionsWidget widgetContainer = SCR_AvailableActionsWidget.CreateActionsWidget(m_sChildLayout, m_wLayoutWidget, "");
				if (widgetContainer)
					widgetContainer.SetVisible(false);

				m_aWidgets.Insert(widgetContainer);
			}

			if (m_aWidgets[i])
				m_aWidgets[i].SetVisible(false);
		}
		
		m_slotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if(!m_slotHandler)
			return;
		
		m_HUDSlotComponent = m_slotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;
		
		m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayStopDraw(IEntity owner)
	{
		m_aWidgets.Clear();
		m_wLayoutWidget = null;
		m_iLastCount = 0;
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayInit(IEntity owner)
	{		
		PlayerController playerController = PlayerController.Cast(owner);
		if (!playerController)
		{
			Print("SCR_AvailableActionsDisplay is not an object in HUDManagerComponent attached onto a PlayerController entity! May result in undefined behaviour.", LogLevel.WARNING);
		}

		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
			return;

		OnGamepadDeviceChanged(!inputMgr.IsUsingMouseAndKeyboard());

		UpdateIsEnabled();
		GetGame().OnUserSettingsChangedInvoker().Insert(UpdateIsEnabled);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnGamepadDeviceChanged);
		SCR_MenuHelper.GetOnMenuClose().Insert(OnSettingsMenuClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method that is triggered when some menu is closed
	//! \param[in] menu that was closed
	protected void OnSettingsMenuClosed(ChimeraMenuBase menu)
	{
		if (SCR_SettingsSuperMenu.Cast(menu))
			m_bForceUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method that is triggered when player switches to or from using gamepad as his input device
	//! \param[in] isUsingGamepad
	protected void OnGamepadDeviceChanged(bool isUsingGamepad)
	{
		if (isUsingGamepad)
			m_eCurrentInputDevice = EInputDeviceType.GAMEPAD;
		else
			m_eCurrentInputDevice = EInputDeviceType.KEYBOARD;

		m_bForceUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSlotUIResize()
	{
		// Assign it again in case the SlotUIComponent has changed
		m_HUDSlotComponent = m_slotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;
		
		int height = m_HUDSlotComponent.GetHeight();
		
		m_iMaxActionsBig = (int)height / HEIGHT_DIVIDER_BIG;
		m_iMaxActionsMedium =  (int)height / HEIGHT_DIVIDER_MEDIUM;
		if (m_iMaxActionsMedium < 1)
			m_iMaxActionsMedium = 1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyLayoutBehavior()
	{
		AvailableActionLayoutBehaviorBase selectedBehavior;

		// Go through each behavior
		foreach (AvailableActionLayoutBehaviorBase beh : m_aBehaviors)
		{
			if (beh.ConditionsChecked(this))
			{
				// Default
				if (!selectedBehavior)
				{
					selectedBehavior = beh;
					continue;
				}

				// Select this behavior if it has bigger priority
				if (selectedBehavior.m_iPriority < beh.m_iPriority)
					selectedBehavior = beh;
			}
		}

		if (selectedBehavior)
			selectedBehavior.ApplyBehavior(this);
		else
			SetOffsetY();
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class AvailableActionLayoutBehaviorBase
{
	[Attribute("")]
	float m_fOffsetY;

	[Attribute("0")]
	int m_iPriority;

	//------------------------------------------------------------------------------------------------
	bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void ApplyBehavior(notnull SCR_AvailableActionsDisplay display)
	{
		display.SetOffsetY(m_fOffsetY);
	}
};

//------------------------------------------------------------------------------------------------
//! Variables that should be applied on available actions layout whenever given HUD is active
[BaseContainerProps()]
class AvailableActionLayoutBehavior : AvailableActionLayoutBehaviorBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	ResourceName m_sCheckHUD;

	[Attribute("0")]
	protected bool m_bOffsetFromDisplay;

	[Attribute("0")]
	protected float m_fAddOffset;

	protected float m_fAutoOffset;

	//------------------------------------------------------------------------------------------------
	override bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		SCR_HUDManagerComponent HUDManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!HUDManager)
			return false;

		Widget hud = HUDManager.FindLayoutByResourceName(m_sCheckHUD);
		if (!hud)
			return false;

		// Callculate auto offset
		if (m_bOffsetFromDisplay)
		{
			float w;

			SCR_InfoDisplay hudCmp = HUDManager.FindInfoDisplayByResourceName(m_sCheckHUD);
			if (hudCmp)
				hudCmp.GetDimensions(w, m_fAutoOffset);
		}

		return (!SCR_EditorManagerEntity.IsOpenedInstance() && hud && hud.IsEnabled());
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	override void ApplyBehavior(notnull SCR_AvailableActionsDisplay display)
	{
		// Manual offset
		if (!m_bOffsetFromDisplay)
		{
			super.ApplyBehavior(display);
			return;
		}

		// Auto offset from check HUD
		display.SetOffsetY(m_fAutoOffset + m_fAddOffset);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class AvailableActionMenuLayoutBehavior : AvailableActionLayoutBehaviorBase
{
	[Attribute("0", UIWidgets.ComboBox, "Is this menu active", "", ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	ChimeraMenuPreset m_ActiveMenu;

	//------------------------------------------------------------------------------------------------
	override bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		return (GetGame().GetMenuManager().FindMenuByPreset(m_ActiveMenu) != null);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class AvailableActionEditorLayoutBehavior : AvailableActionLayoutBehaviorBase
{
	[Attribute("1", UIWidgets.ComboBox, "In which mode should be this behavior applied", "", ParamEnumArray.FromEnum(EEditorMode))]
	EEditorMode m_eEditorMode;

	[Attribute("0", desc: "Additional offset applied when editor is no legal in given scenario.")]
	protected float m_fIllegalEditorOffset;

	//------------------------------------------------------------------------------------------------
	override bool ConditionsChecked(SCR_AvailableActionsDisplay display)
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return false;

		EEditorMode mode = editorManagerEntity.GetCurrentMode();

		return (SCR_EditorManagerEntity.IsOpenedInstance() && mode == m_eEditorMode);
	}
	override void ApplyBehavior(notnull SCR_AvailableActionsDisplay display)
	{
		float offset = m_fOffsetY;

		if (m_fIllegalEditorOffset != 0)
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
			{
				SCR_EditorSettingsEntity editorSettings = core.GetSettingsEntity();
				if (editorSettings && !editorSettings.IsUnlimitedEditorLegal())
					offset += m_fIllegalEditorOffset;
			}
		}

		display.SetOffsetY(offset);
	}
};
