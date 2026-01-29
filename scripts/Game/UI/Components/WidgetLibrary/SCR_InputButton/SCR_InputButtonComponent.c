class SCR_InputButtonComponent : SCR_ButtonBaseComponent
{
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Very bloated class, with a lot of functionality mixed with looks, and too many useless methods and attributes inherited from the parent.
	
	[Attribute("1", desc: "If enabled, buttons won't show the Text/Image to display the required input")]
	protected bool m_bKeybindActive;
	
	[Attribute("MenuTabLeft", desc: "Name of action from chimeraInputCommon")]
	protected string m_sActionName;

	[Attribute("Navigation Button")]
	protected string m_sLabel;

	[Attribute("0")]
	protected bool m_bIsMouseInput;

	[Attribute("2", desc: "Amount of keybinds for one action we display to prevent displaying 100 alternatives for one action")]
	protected int m_iMaxShownKeys;

	[Attribute(defvalue: "32", desc: "Define the Height in PX. Wight will be adjusted automatically")]
	int m_iHeightInPixel;

	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_COLOR), UIWidgets.ColorPicker)]
	ref Color m_ActionDefault;

	[Attribute(UIColors.GetColorAttribute(UIColors.HIGHLIGHTED), UIWidgets.ColorPicker)]
	ref Color m_ActionHovered;

	[Attribute(UIColors.GetColorAttribute(UIColors.IDLE_DISABLED), UIWidgets.ColorPicker)]
	ref Color m_ActionDisabled;

	[Attribute(UIColors.GetColorAttribute(GUIColors.DEFAULT_GLOW), UIWidgets.ColorPicker)]
	protected ref Color m_ActionUnbound;
	
	[Attribute("not-available")]
	protected string m_sActionUnboundIcon;
	
	[Attribute(UIColors.GetColorAttribute(Color.White), UIWidgets.ColorPicker)]
	ref Color m_LabelDefault;

	[Attribute("0.597421 0.061189 0.475868 1.000000", UIWidgets.ColorPicker)]
	ref Color m_ActionToggled;

	[Attribute("0", desc: "If enabled there will be no sound played when the button is pressed")]
	protected bool m_bDisableClickSound;

	[Attribute("0", desc: "Postion offset when button is not held")]
	float m_fHoldIndicatorDefaultPosition;

	[Attribute("5", desc: "Position offset when button is held")]
	float m_fHoldIndicatorHoldPosition;

	[Attribute("0.1", desc:"Time in sec who long the animation takes.")]
	float m_fHoldIndicatorAnimationTime;

	[Attribute("1", desc: "If false the button will never change to its disabled visual state.")]
	bool m_bCanBeDisabled;

	[Attribute("1", desc: "If false the Label does not change its color to disabled when button is disabled.")]
	bool m_bChangeLabelColorOnDisabled;

	[Attribute("InputButtonDisplayRoot")]
	protected string m_sButtonWidgetName;

	[Attribute("HorizontalLayout0")]
	protected string m_sHorizontalLayoutWidget;

	[Attribute("TextHint")]
	protected string m_sTextHintWidget;

	[Attribute("{B7C8B633428B153D}UI/layouts/WidgetLibrary/Buttons/WLib_InputButtonDisplay.layout")]
	protected ResourceName m_sInputButtonWidget;

	[Attribute("{983DFCCEB578C1C5}UI/layouts/InputButtons/InputButton_ComboIndicator.layout")]
	protected ResourceName m_sComboIndicatorWidget;

	[Attribute(UIConstants.ICONS_IMAGE_SET)]
	protected ResourceName m_sComboIndicatorImageSet;

	[Attribute(UIConstants.ICONS_GLOW_IMAGE_SET)]
	protected ResourceName m_sComboIndicatorImageSetGlow;
	
	[Attribute(desc: "If enabled Text size is controlled by set size Inside the RichText widget")]
	protected bool m_bOverrideTextSize;

	[Attribute("-0.05", desc: "Time that will the HoldTime will be reduced by to make the Animation the right length. Time in seconds")]
	protected float m_fHoldTimeReduction;

	[Attribute()]
	protected bool m_bDebugSimulateController;

	protected const string COMBO_INDICATOR_IMAGE_NAME = "ComboIndicatorImg";
	protected const string COMBO_INDICATOR_SHADOW_NAME = "ComboIndicatorShadow";
	protected const string COMBO_INDICATOR_DIVIDER_NAME = "keybind_divider";
	protected const string COMBO_INDICATOR_COMBO_NAME = "keybind_combo";
	protected const string COMBO_INDICATOR_KEY = " + ";
	protected const string COMBO_DIVIDER_KEY = " | ";

	protected const float COMBO_INDICATOR_SIZE_MULTIPLIER = 0.5;
	
	// TODO: fix textures having improper offset: we should not need different padding scaling
	protected const float COMBO_INDICATOR_PADDING_MULTIPLIER_LEFT = 0.20;
	protected const float COMBO_INDICATOR_PADDING_MULTIPLIER_RIGHT = 0.15;
	
	protected const float MIN_FONTSIZE_MULTIPLIER = 0.5;

	protected ref SCR_InputButtonDisplay m_ButtonDisplay;
	protected Widget m_wHorizontalLayout;
	protected RichTextWidget m_wTextHint;

	protected ref array<SizeLayoutWidget> m_aComboIndicators = {};
	protected ref array<Widget> m_aComboWidgets = {};
	protected ref array<BaseContainer> m_aFilterStack = {};
	protected ref array<int> m_aFilterStackIndexRemover = {};
	protected ref array<string> m_aKeyStackArray = {};

	InputManager m_InputManager = GetGame().GetInputManager();

	protected bool m_bIsComboInput;
	protected bool m_bIsAlternativeInput;
	protected bool m_bIsHoldAction;
	protected bool m_bIsDoubleTapAction;
	protected bool m_bPressedInput;
	protected bool m_bIsContinuous;

	protected bool m_bForceDisabled;
	protected bool m_bShouldBeEnabled;
	protected bool m_bIsInteractionActive;
	
	//! If user changed keybing then it should force update of hints
	protected bool m_bForceUpdate;

	protected string m_sOldActionName;

	protected bool m_bIsHoldingButton;
	bool m_bIsHovered;
	bool m_bIsDoubleTapStated;

	protected float m_fDefaultHoldTime;
	protected float m_fDefaultClickTime;
	float m_fMaxHoldtime;

	int m_iDoubleTapThreshold;

	protected EInputDeviceType m_eCurrentInputDevice;

	protected ref ScriptInvokerVoid m_OnAnimateHover;
	protected ref ScriptInvokerVoid m_OnUpdateEnableColor;
	protected ref ScriptInvokerVoid m_OnHoldAnimComplete;

	ref ScriptInvoker m_OnActivated = new ScriptInvoker();
	
//---- REFACTOR NOTE END ----	
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Quick and dirty way of disabling the buttons when they're not in a top dialog. Can definitely be done better, as the buttons have no business knowing all displayed dialogs
	
	protected ref array<DialogUI> m_aDialogs = {};
	
//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wHorizontalLayout = HorizontalLayoutWidget.Cast(w.FindAnyWidget(m_sHorizontalLayoutWidget));
		m_wTextHint = RichTextWidget.Cast(w.FindAnyWidget(m_sTextHintWidget));

		Widget buttonWidget = w.FindAnyWidget(m_sButtonWidgetName);
		if (!buttonWidget)
			return;

		m_ButtonDisplay = SCR_InputButtonDisplay.Cast(buttonWidget.FindHandler(SCR_InputButtonDisplay));
		if (!m_ButtonDisplay)
			return;

		//! Get the default hold time defined in Project settings
		GameProject.GetModuleConfig("InputManagerSettings").Get("HoldDuration", m_fDefaultHoldTime);
		GameProject.GetModuleConfig("InputManagerSettings").Get("ClickDuration", m_fDefaultClickTime);
		if (m_fDefaultClickTime)
		{
			m_fDefaultClickTime /= 1000;
			m_fHoldTimeReduction += m_fDefaultClickTime;
		}

		if (m_wTextHint)
		{
			if (!m_sLabel)
				m_wTextHint.SetVisible(false);

			m_wTextHint.SetText(m_sLabel);
			m_wTextHint.SetColor(m_LabelDefault);
		}

		if (GetGame().InPlayMode())
		{
			SCR_MenuHelper.GetOnActiveWidgetInteraction().Insert(OnActiveWidgetInteraction);
			SCR_MenuHelper.GetOnDialogOpen().Insert(OnDialogOpen);
			SCR_MenuHelper.GetOnDialogClose().Insert(OnDialogClose);
			SCR_MenuHelper.GetOnMenuFocusGained().Insert(OnMenuFocusGained);
			SCR_MenuHelper.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
			SCR_MenuHelper.GetOnMenuClose().Insert(OnSettingsMenuClosed);
		}

		m_bShouldBeEnabled = IsEnabled();

		m_ButtonDisplay.Init(m_wRoot);

		EInputDeviceType currentDevice = EInputDeviceType.KEYBOARD;
		if (!m_InputManager.IsUsingMouseAndKeyboard())
			currentDevice = EInputDeviceType.GAMEPAD;

		ChangeInputDevice(currentDevice, false);

		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceUserChanged);

		if (m_bDebugSimulateController)
			OnInputDeviceUserChanged(true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		//! Remove old listener
		if (!m_InputManager)
			return;

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceUserChanged);

		if (GetGame().InPlayMode())
		{
			SCR_MenuHelper.GetOnActiveWidgetInteraction().Remove(OnActiveWidgetInteraction);
			SCR_MenuHelper.GetOnDialogOpen().Remove(OnDialogOpen);
			SCR_MenuHelper.GetOnDialogClose().Remove(OnDialogClose);
			SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuFocusGained);
			SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
			SCR_MenuHelper.GetOnMenuClose().Remove(OnSettingsMenuClosed);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_wRoot.IsVisible() || !m_wRoot.IsEnabled())
			return false;

		if (m_bIsDoubleTapAction)
			return false;

		m_bIsHoldingButton = true;

		OnInput();

		super.OnClick(w, x, y, button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[inout] keyStack stack for keys. Can be: '+' for input combo (binary node followed by two child nodes) or '|' for input sum/alternative (binary node followed by two child nodes) otherwise contains key code (leaf node)
	//! \return true when there is a keybind avialable for current input device
	bool IsKeybindAvailable(inout notnull array<string> keyStack)
	{
		if (m_sActionName.IsEmpty())
			return false;

		if (m_aFilterStack.IsEmpty())
			ProcessFilterStack();

		return m_InputManager && m_InputManager.GetActionKeybinding(m_sActionName, keyStack, m_aFilterStack, m_eCurrentInputDevice);
	}

	//------------------------------------------------------------------------------------------------
	override void SetEnabled(bool enabled, bool animate = true)
	{
		m_bShouldBeEnabled = enabled;
		SetEnabled_Internal(enabled, animate);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEnabled_Internal(bool enabled, bool animate = true)
	{
		if (!m_bForceDisabled)
		{
			m_wRoot.SetEnabled(enabled);
			if (enabled)
				OnEnabled(animate);
			else
				OnDisabled(animate);
		}
		else
		{
			m_wRoot.SetEnabled(false);
			OnDisabled(animate);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! React on switching between input methods
	protected void OnInputDeviceUserChanged(bool isGamepad)
	{
		if (isGamepad)
			ChangeInputDevice(EInputDeviceType.GAMEPAD, false);
		else
			ChangeInputDevice(EInputDeviceType.KEYBOARD, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Check for last used input device and get the correct keybind for it.
	//! \param[in] EInputDeviceType type of InputDevice
	//! \param[in] bool is the realted button focused by mouse
	//! \param[in] bool reset Button override if it was enabled (false by default)
	protected bool ChangeInputDevice(EInputDeviceType inputDevice, bool hasFocus, bool resetOverride = false)
	{
		if (inputDevice == m_eCurrentInputDevice && m_sActionName == m_sOldActionName && !resetOverride && !m_bForceUpdate)
			return true;
		
		m_bForceUpdate = false;

		if (inputDevice == EInputDeviceType.MOUSE && hasFocus && m_bIsMouseInput)
		{
			m_eCurrentInputDevice = inputDevice;
		}
		else
		{
			if (inputDevice == EInputDeviceType.GAMEPAD)
				m_eCurrentInputDevice = inputDevice;
			else
				m_eCurrentInputDevice = EInputDeviceType.KEYBOARD;
		}

		if (inputDevice == EInputDeviceType.INVALID)
			return false;

		array<string> keyStack = {};
		//! Get keybind for selected input action and input device
		if (m_InputManager.GetActionKeybinding(m_sActionName, keyStack, m_aFilterStack, m_eCurrentInputDevice))
		{
			m_aFilterStackIndexRemover.Clear();
			m_aKeyStackArray.Clear();
			int index = keyStack.Count() - 1;
			ProcessKeybindStack(index, keyStack);

			if (keyStack.IsEmpty() || !m_aKeyStackArray || m_aKeyStackArray.IsEmpty())
				return false;

			string keyCode = m_aKeyStackArray[0];
			BaseContainer data;
			if (m_InputManager.GetKeyUIMapping(keyCode))
				data = m_InputManager.GetKeyUIMapping(keyCode).GetObject("Data");

			if (!data)
			{
				Print(string.Format("No data found for %1 !! Check 'chimeraMapping.conf' and add data if necessary! Provided Actioname: %2.", keyCode, m_sActionName), LogLevel.ERROR);
				return false;
			}

			//Check is there is a valid filter applied
			GetInputFilterSettings(m_aFilterStack[0]);

			//Process filter stack for visualisation
			ProcessFilterStack();

			if (m_ButtonDisplay)
			{
				m_ButtonDisplay.SetAction(data, m_aFilterStack[0]);
				
				if (!m_bKeybindActive)
					m_ButtonDisplay.ChangeInputVisibility(false);
			}

			if (!m_bOverrideTextSize && m_wTextHint && m_ButtonDisplay)
			{
				int textSize = m_iHeightInPixel / m_ButtonDisplay.GetTextSizeModifier();
				m_wTextHint.SetDesiredFontSize(textSize);
				m_wTextHint.SetMinFontSize(textSize * MIN_FONTSIZE_MULTIPLIER);
			}

			DeleteComboWidget();
			if (m_bIsComboInput || m_bIsAlternativeInput)
				CreateComboWidget();

			if (inputDevice != m_eCurrentInputDevice || m_sActionName != m_sOldActionName)
				SetInputAction();
		}
		else
		{
			if (m_ButtonDisplay)
				m_ButtonDisplay.OverrideTexture(UIConstants.ICONS_IMAGE_SET, m_sActionUnboundIcon, m_ActionUnbound, SCR_EButtonSize.KEYBOARD_SQUARE, false);
			
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check which filter is applied to the action to change the buttons behavior
	protected bool GetInputFilterSettings(BaseContainer filter)
	{

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Flags
		
		m_bIsHoldAction = false;
		m_bIsDoubleTapAction = false;
		m_bPressedInput = false;
		m_bCanBeToggled = false;
		m_bIsContinuous = false;

//---- REFACTOR NOTE END ----
		
		if (!filter)
			return false;

		switch (filter.GetClassName())
		{
			case "InputFilterHold":
			{
				filter.Get("HoldDuration", m_fMaxHoldtime);
				if (m_fMaxHoldtime == -1 || m_fMaxHoldtime == 0)
					m_fMaxHoldtime = m_fDefaultHoldTime;

				m_fMaxHoldtime /= 1000;
				m_fMaxHoldtime -= m_fHoldTimeReduction;
				if (m_fMaxHoldtime <= 0)
					m_fMaxHoldtime += m_fHoldTimeReduction;

				m_bIsHoldAction = true;
				break;
			}
			case "InputFilterHoldOnce":
			{
				filter.Get("HoldDuration", m_fMaxHoldtime);
				if (m_fMaxHoldtime == -1 || m_fMaxHoldtime == 0)
					m_fMaxHoldtime = m_fDefaultHoldTime;

				m_fMaxHoldtime /= 1000;
				m_fMaxHoldtime -= m_fHoldTimeReduction;
				if (m_fMaxHoldtime <= 0)
					m_fMaxHoldtime += m_fHoldTimeReduction;

				m_bIsHoldAction = true;
				break;
			}
			case "InputFilterDoubleClick":
			{
				m_bIsDoubleTapAction = true;
				filter.Get("Threshold", m_iDoubleTapThreshold);
				if (m_iDoubleTapThreshold == -1)
					GameProject.GetModuleConfig("InputManagerSettings").Get("DoubleClickDuration", m_iDoubleTapThreshold);

				m_iDoubleTapThreshold /= 1000;
				break;
			}
			case "InputFilterPressed":
			{
				m_bPressedInput = true;
				break;
			}
			case "InputFilterToggle":
			{
				m_bCanBeToggled = true;
				break;
			}
			case "InputFilterValue":
			{
				m_bIsContinuous = true;
				break;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a second InputButton widget to display combos
	protected void CreateComboWidget()
	{
		if (m_ButtonDisplay.GetIsOverwritten())
			return;

		int comboCount;
		int filterIndex = 1;
		Widget button;
		SizeLayoutWidget comboIndicatorContainer;
		ImageWidget comboIndicator;
		ImageWidget comboIndicatorShadow;
		SCR_InputButtonDisplay display;
		foreach (int i, string key : m_aKeyStackArray)
		{
			if (i == 0)
				continue;//skip first button info to not duplicate it

			if (key == COMBO_INDICATOR_KEY || key == COMBO_DIVIDER_KEY)
			{
				if (comboCount >= m_iMaxShownKeys - 1 && key == COMBO_DIVIDER_KEY)
					break;

				comboIndicatorContainer = SizeLayoutWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sComboIndicatorWidget, m_wHorizontalLayout));
				if (!comboIndicatorContainer)
				{
					Print(string.Format("Unable to create Widget! %1 or %2 are null!", m_sComboIndicatorWidget, m_wHorizontalLayout), LogLevel.ERROR);
					return;
				}

				comboIndicator = ImageWidget.Cast(comboIndicatorContainer.FindAnyWidget(COMBO_INDICATOR_IMAGE_NAME));
				comboIndicatorShadow = ImageWidget.Cast(comboIndicatorContainer.FindAnyWidget(COMBO_INDICATOR_SHADOW_NAME));

				if (comboIndicator && comboIndicatorShadow)
				{
					if (key == COMBO_INDICATOR_KEY)
					{
						comboIndicator.LoadImageFromSet(0, m_sComboIndicatorImageSet, COMBO_INDICATOR_COMBO_NAME);
						comboIndicatorShadow.LoadImageFromSet(0, m_sComboIndicatorImageSetGlow, COMBO_INDICATOR_COMBO_NAME);
					}
					else if (key == COMBO_DIVIDER_KEY)
					{
						comboIndicator.LoadImageFromSet(0, m_sComboIndicatorImageSet, COMBO_INDICATOR_DIVIDER_NAME);
						comboIndicatorShadow.LoadImageFromSet(0, m_sComboIndicatorImageSetGlow, COMBO_INDICATOR_DIVIDER_NAME);

						comboCount++;
					}
				}

				m_aComboIndicators.Insert(comboIndicatorContainer);
				m_aComboWidgets.Insert(comboIndicatorContainer);
				continue;
			}

			BaseContainer keyData = m_InputManager.GetKeyUIMapping(m_aKeyStackArray[i]).GetObject("Data");
			if (!keyData)
			{
				Print(string.Format("No data found for %1 !! Check 'chimeraMapping.conf' and add data if necessary!", m_aKeyStackArray[i]), LogLevel.ERROR);
				return;
			}

			//! Create InputButton widget
			Widget comboWidget = GetGame().GetWorkspace().CreateWidgets(m_sInputButtonWidget, m_wHorizontalLayout);

			if (!comboWidget)
			{
				Print(string.Format("Unable to create Widget! %1 or %2 are null!", m_sInputButtonWidget, m_wHorizontalLayout), LogLevel.ERROR);
				return;
			}

			SCR_InputButtonDisplay component = SCR_InputButtonDisplay.Cast(comboWidget.FindHandler(SCR_InputButtonDisplay));

			if (!component)
			{
				Print(string.Format("%1 has no 'SCR_InputButtonDisplay' component attached!", component), LogLevel.ERROR);
				return;
			}

			UpdateComboIndicatorSizes();
			
			component.Init(m_wRoot);
			component.SetAction(keyData, m_aFilterStack[filterIndex]);
			filterIndex++;
			
			if (!m_bKeybindActive)
				component.ChangeInputVisibility(false);

			m_aComboWidgets.Insert(comboWidget);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Delete secound InputButton widget if there is one
	protected void DeleteComboWidget()
	{
		if (m_aComboWidgets.IsEmpty())
			return;

		foreach (Widget widget : m_aComboWidgets)
		{
			if (!widget)
				continue;

			widget.RemoveFromHierarchy();
		}

		m_aComboWidgets.Clear();
		m_aComboIndicators.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Add action listeners for selected input action
	protected void SetInputAction()
	{
		if (!m_sOldActionName.IsEmpty())
		{
			m_InputManager.RemoveActionListener(m_sOldActionName, EActionTrigger.DOWN, OnButtonPressed);
			m_InputManager.RemoveActionListener(m_sOldActionName, EActionTrigger.VALUE, OnButtonHold);
			m_InputManager.RemoveActionListener(m_sOldActionName, EActionTrigger.UP, ActionReleased);
		}

		if (m_sActionName.IsEmpty())
			return;

		if (m_bIsHoldAction)
			m_InputManager.AddActionListener(m_sActionName, EActionTrigger.VALUE, OnButtonHold);

		if (m_bIsContinuous)
			m_InputManager.AddActionListener(m_sActionName, EActionTrigger.UP, ActionReleased);

		m_InputManager.AddActionListener(m_sActionName, EActionTrigger.DOWN, OnButtonPressed);

		m_sOldActionName = m_sActionName;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonPressed(float value, EActionTrigger reason)
	{
		if (!m_bKeybindActive)
			return;

		PlaySoundClicked();
		OnInput();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (m_bIsDoubleTapAction)
			super.OnDoubleClick(w, x, y, button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		m_bIsHovered = true;
		if (m_OnAnimateHover)
			m_OnAnimateHover.Invoke(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		m_bIsHovered = false;

		if (m_OnAnimateHover)
			m_OnAnimateHover.Invoke(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDisabled(bool animate)
	{
		if (m_OnUpdateEnableColor)
			m_OnUpdateEnableColor.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnabled(bool animate)
	{
		super.OnEnabled(animate);

		if (m_OnUpdateEnableColor)
			m_OnUpdateEnableColor.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when button is held.
	//! \param Float 0-1. State of button press.
	void OnButtonHold(float value)
	{
		if (!m_wRoot)
			return;

		if (m_bCanBeDisabled && (!m_wRoot.IsVisibleInHierarchy() || !m_wRoot.IsEnabledInHierarchy()))
			return;

		// Bail if attached to menu but menu is not focused
		if (!IsParentMenuFocused())
			return;

		if (value == 0)
		{
			if (m_bIsHoldingButton && m_ButtonDisplay)
			{
				m_bIsHoldingButton = false;
				ActionReleased();
			}

			return;
		}

		if (value < m_fDefaultClickTime)
			return;

		if (!m_bIsHoldingButton)
		{
			m_bIsHoldingButton = true;
			ActionPressed(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	void PlaySoundClicked()
	{
		if (m_bDisableClickSound)
			return;
		
		if (m_bCanBeDisabled && (!m_wRoot.IsVisibleInHierarchy() || !m_wRoot.IsEnabledInHierarchy()))
			return;

		PlaySound(m_sSoundClicked);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when Button is clicked / key is presses
	protected void OnInput()
	{
		if (!m_wRoot)
			return;

		if (m_bCanBeDisabled && (!m_wRoot.IsVisibleInHierarchy() || !m_wRoot.IsEnabledInHierarchy()))
			return;

		// Bail if attached to menu but menu is not focused
		if (!IsParentMenuFocused())
			return;

		//--- If a modal widget is active, check if the button is in its hierarchy
		Widget modal = GetGame().GetWorkspace().GetModal();
		if (modal && !SCR_WidgetTools.InHierarchy(m_wRoot, modal))
			return;

		m_OnActivated.Invoke(this, m_sActionName);
		if (m_bIsHoldAction)
			return;

		ActionPressed();
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionPressed(bool isHoldAction = false)
	{
		if (m_ButtonDisplay)
			m_ButtonDisplay.ActionPressed(isHoldAction);

		foreach (Widget inputButton : m_aComboWidgets)
		{
			SCR_InputButtonDisplay component = SCR_InputButtonDisplay.Cast(inputButton.FindHandler(SCR_InputButtonDisplay));
			if (!component)
				continue;

			component.ActionPressed(isHoldAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionReleased()
	{

		if (!m_bIsHoldAction && !m_bIsContinuous)
			return;

		if (m_ButtonDisplay)
			m_ButtonDisplay.ActionReleased();

		foreach (Widget inputButton : m_aComboWidgets)
		{
			SCR_InputButtonDisplay component = SCR_InputButtonDisplay.Cast(inputButton.FindHandler(SCR_InputButtonDisplay));
			if (!component)
				continue;

			component.ActionReleased();
		}
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsHoldingButton()
	{
		return m_bIsHoldingButton;
	}

	//------------------------------------------------------------------------------------------------
	void SetIsHoldingButton(bool isHolding)
	{
		m_bIsHoldingButton = isHolding;
	}

	//------------------------------------------------------------------------------------------------
	float GetAnimationRate()
	{
		return m_fAnimationRate;
	}

	//------------------------------------------------------------------------------------------------
	float GetAnimationTime()
	{
		return m_fAnimationTime;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsInputCombo()
	{
		return m_bIsComboInput;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAnimateHover()
	{
		if (!m_OnAnimateHover)
			m_OnAnimateHover = new ScriptInvokerVoid();

		return m_OnAnimateHover;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnUpdateEnableColor()
	{
		if (!m_OnUpdateEnableColor)
			m_OnUpdateEnableColor = new ScriptInvokerVoid();

		return m_OnUpdateEnableColor;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnHoldAnimComplete()
	{
		if (!m_OnHoldAnimComplete)
			m_OnHoldAnimComplete = new ScriptInvokerVoid();

		return m_OnHoldAnimComplete;
	}

	//------------------------------------------------------------------------------------------------
	//! Iterate trough key bind stack
	//! \param[in,out] index gets decremented  by one
	void ProcessKeybindStack(inout int index, notnull array<string> keyStack)
	{
		if (!keyStack.IsIndexValid(index))
			return;

		// pop back
		string key = keyStack[index];
		index--;

		switch (key)
		{
			case "+": // combo
			{
				m_aFilterStackIndexRemover.Insert(index + 1);
				ProcessKeybindStack(index, keyStack);
				m_aKeyStackArray.Insert(COMBO_INDICATOR_KEY);
				ProcessKeybindStack(index, keyStack);
				m_bIsComboInput = true;
				break;
			}
			case "|": // sum/alternative
			{
				m_aFilterStackIndexRemover.Insert(index + 1);
				ProcessKeybindStack(index, keyStack);
				m_aKeyStackArray.Insert(COMBO_DIVIDER_KEY);
				ProcessKeybindStack(index, keyStack);
				m_bIsAlternativeInput = true;
				break;
			}

			default: // key value
				m_aKeyStackArray.Insert(key);

		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Delete not needed entries from m_aFilterStack and bring it into correct order
	!*/
	void ProcessFilterStack()
	{
		if (m_aFilterStack.IsEmpty())
			return;

		foreach (int index : m_aFilterStackIndexRemover)
		{
			m_aFilterStack.RemoveOrdered(index);
		}

		array<BaseContainer> filteredm_aFilterStack = {};
		int count = m_aFilterStack.Count() - 1;
		for (int i = count; i >= 0; i--)
		{
			BaseContainer m_aFilterStackIndex = m_aFilterStack[i];
			filteredm_aFilterStack.Insert(m_aFilterStackIndex);

			if (!m_aFilterStackIndex)
				continue;

			if (m_aFilterStackIndex.GetClassName() == "InputFilterHold" || m_aFilterStackIndex.GetClassName() == "InputFilterHoldOnce")
			{
				m_aFilterStackIndex.Get("HoldDuration", m_fMaxHoldtime);
				if (m_fMaxHoldtime == -1 || m_fMaxHoldtime == 0)
					m_fMaxHoldtime = m_fDefaultHoldTime;

				m_fMaxHoldtime /= 1000;
				m_fMaxHoldtime -= m_fHoldTimeReduction;
				if (m_fMaxHoldtime <= 0)
					m_fMaxHoldtime += m_fHoldTimeReduction;

				m_bIsHoldAction = true;
			}
		}
		m_aFilterStack.Clear();
		m_aFilterStack = filteredm_aFilterStack;
	}

	///PUBLIC API\\\
	//------------------------------------------------------------------------------------------------
	//! Change the Text displayed in the label
	void SetLabel(string label)
	{
		if (!m_wTextHint)
			return;

		m_wTextHint.SetText(label);
		m_wTextHint.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Change the color of the Label
	void SetLabelColor(notnull Color color)
	{
		if (!m_wTextHint)
			return;

		m_wTextHint.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! Override action. This changes the visuals of the button based on the new action.
	//! \param[in] action name defined in chimeraInputCommon.conf
	//! \param[in] currentInputDevice for what input device type this hint should load the data
	//! \param[in] forceUpdate if this hint should ignore if it is already configured for the same action and try to fetch that data again in order to refresh it
	bool SetAction(string action, EInputDeviceType currentInputDevice = -1, bool forceUpdate = false)
	{
		if (forceUpdate)
			m_sOldActionName = string.Empty;
		else if (m_sActionName == action)
			return true;

		m_sActionName = action;
		if (currentInputDevice < 0)
		{
			if (m_InputManager.IsUsingMouseAndKeyboard())
				currentInputDevice = EInputDeviceType.KEYBOARD;
			else
				currentInputDevice = EInputDeviceType.GAMEPAD;
		}

		if (ChangeInputDevice(currentInputDevice, false))
			return true;

		if (!forceUpdate)
			m_sActionName = string.Empty;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	int GetSize()
	{
		return m_iHeightInPixel;
	}

	//------------------------------------------------------------------------------------------------
	//! Change the size of the button
	//! \param Int - Height in px. Width will be calculated automatically
	void SetSize(int size)
	{
		m_iHeightInPixel = size;

		if (!m_bOverrideTextSize && m_wTextHint && m_ButtonDisplay)
		{
			int textSize = m_iHeightInPixel / m_ButtonDisplay.GetTextSizeModifier();
			m_wTextHint.SetDesiredFontSize(textSize);
			m_wTextHint.SetMinFontSize(textSize * MIN_FONTSIZE_MULTIPLIER);
		}

		UpdateComboIndicatorSizes();

		m_ButtonDisplay.Resize();
	}

	//------------------------------------------------------------------------------------------------
	bool IsClickSoundDisabled()
	{
		return m_bDisableClickSound;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable / disable the click sound played when button is pressed
	void SetClickSoundDisabled(bool isEnabled)
	{
		m_bDisableClickSound = isEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Override texture in Button
	//! \param ImagePath
	//! \param Image name if using ImageSet. Default: null
	//! \param Color of the Image
	void SetTexture(string imagePath, string image = string.Empty, Color color = Color.FromInt(Color.WHITE), SCR_EButtonSize buttonType = SCR_EButtonSize.KEYBOARD_MEDIUM)
	{
		if (!m_ButtonDisplay)
			return;

		DeleteComboWidget();
		m_ButtonDisplay.OverrideTexture(imagePath, image, color, buttonType);
	}

	//------------------------------------------------------------------------------------------------
	//! Restet Button back to it's default texutre
	//! Use after SetTexture() to undo the override
	void ResetTexture()
	{
		if (!m_ButtonDisplay)
			return;

		m_ButtonDisplay.SetIsOverwritten(false);

		EInputDeviceType currentDevice = EInputDeviceType.KEYBOARD;
		if (!m_InputManager.IsUsingMouseAndKeyboard())
			currentDevice = EInputDeviceType.GAMEPAD;

		ChangeInputDevice(currentDevice, false, true);
	}

	//------------------------------------------------------------------------------------------------
	void SetColorActionDisabled(Color newColor)
	{
		m_ActionDisabled = newColor;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change the visibility of the Button Image & Text
	//! \param[in] visibility
	void ChangeInputVisibility(bool visible)
	{
		if (!m_ButtonDisplay)
			return;
		
		m_bKeybindActive = visible;
		
		m_ButtonDisplay.ChangeInputVisibility(visible);
		
		if (m_aComboWidgets.IsEmpty())
			return;
		
		SCR_InputButtonDisplay component;
		foreach (Widget inputButton : m_aComboWidgets)
		{
			component = SCR_InputButtonDisplay.Cast(inputButton.FindHandler(SCR_InputButtonDisplay));
			if (!component)
				continue;

			component.ChangeInputVisibility(visible);
		}
	}

	//------------------------------------------------------------------------------------------------
	static SCR_InputButtonComponent FindComponent(notnull Widget w)
	{
		return SCR_InputButtonComponent.Cast(w.FindHandler(SCR_InputButtonComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool IsHovering()
	{
		return m_bIsHovered;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetForceDisabled(bool forceDisabled, bool animate = true)
	{
		GetGame().GetCallqueue().Remove(SetForceDisabled); //Stop SetForceDisabledWithDelay

		m_bForceDisabled = forceDisabled;

		if (forceDisabled)
			SetEnabled_Internal(false);
		else
			SetEnabled_Internal(m_bShouldBeEnabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetForceDisabledWithDelay(bool forceDisabled, bool animate = true, int delay = 0)
	{
		m_bForceDisabled = forceDisabled;

		if (delay == 0)
		{
			SetForceDisabled(forceDisabled, animate);
			return;
		}

		GetGame().GetCallqueue().Remove(SetForceDisabled);
		GetGame().GetCallqueue().CallLater(SetForceDisabled, delay, false, forceDisabled, animate);
	}

	//Handling Input through SCR_MenuHelper, ei: edit and combo boxes call this to disable buttons during interaction
	//------------------------------------------------------------------------------------------------
	void OnActiveWidgetInteraction(bool isInteractionActive, int delay)
	{
		m_bIsInteractionActive = isInteractionActive;

		if (IsInTopMenu())
			SetForceDisabledWithDelay(isInteractionActive, true, delay);
	}

	//------------------------------------------------------------------------------------------------
	void OnDialogOpen(DialogUI dialog)
	{
		m_aDialogs.Insert(dialog);

		SetForceDisabled(!IsInTopMenu() || m_bIsInteractionActive);
	}

	//------------------------------------------------------------------------------------------------
	void OnDialogClose(DialogUI dialog)
	{
		m_aDialogs.RemoveItem(dialog);

		SetForceDisabled(!IsInTopMenu() || m_bIsInteractionActive);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusGained(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot))
			SetForceDisabled(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot))
			SetForceDisabled(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback method that is triggered when some menu is closed
	//! \param[in] menu that was closed
	protected void OnSettingsMenuClosed(ChimeraMenuBase menu)
	{
		if (!SCR_SettingsSuperMenu.Cast(menu))
			return;
		
		EInputDeviceType currentDevice = EInputDeviceType.KEYBOARD;
		if (!m_InputManager.IsUsingMouseAndKeyboard())
			currentDevice = EInputDeviceType.GAMEPAD;

		m_bForceUpdate = true;
		
		ChangeInputDevice(currentDevice, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if there are no dailogs or if this button is in the last dialog opened
	bool IsInTopMenu()
	{
		return m_aDialogs.IsEmpty() || m_aDialogs[m_aDialogs.Count() - 1] == GetGame().GetMenuManager().GetOwnerMenu(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if there if this button is in the last dialog opened
	bool IsInTopDialog()
	{
		return !m_aDialogs.IsEmpty() && m_aDialogs[m_aDialogs.Count() - 1] == GetGame().GetMenuManager().GetOwnerMenu(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	bool GetForceDisabled()
	{
		return m_bForceDisabled;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateComboIndicatorSizes()
	{
		float size = m_iHeightInPixel * COMBO_INDICATOR_SIZE_MULTIPLIER;
		float paddingLeft = m_iHeightInPixel * COMBO_INDICATOR_PADDING_MULTIPLIER_LEFT * -1;
		float paddingRight = m_iHeightInPixel * COMBO_INDICATOR_PADDING_MULTIPLIER_RIGHT * -1;
		
		foreach (SizeLayoutWidget w : m_aComboIndicators)
		{
			w.SetWidthOverride(size);
			w.SetHeightOverride(size);
			LayoutSlot.SetPadding(w, paddingLeft, 0, paddingRight, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_InputButtonComponent GetInputButtonComponent(string name, notnull Widget parent, bool searchAllChildren = true)
	{
		SCR_InputButtonComponent comp = SCR_InputButtonComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_InputButtonComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
}
