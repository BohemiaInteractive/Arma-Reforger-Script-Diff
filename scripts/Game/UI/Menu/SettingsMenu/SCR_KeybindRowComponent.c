//------------------------------------------------------------------------------------------------
class SCR_KeybindRowComponent : SCR_ScriptedWidgetComponent
{
	protected const string WIDGET_BIND_PC = "PCBind";
	protected const string WIDGET_BIND_CONTROLLER = "ControllerBind";
	
	static const string BUTTON_ADVANCED_KEYBIND = "AdvancedKeybind";
	static const string BUTTON_RESET = "Reset";
	static const string BUTTON_UNBIND = "Unbind";
	
	protected const string ACTION_DISPLAY_ROOT = "KeybindActionDisplayRow";
	
	protected bool m_bFocused;
	protected bool m_bIsDialogOpen;
	
	protected ref InputBinding m_Binding;
	protected ref SCR_KeyBindingEntry m_Entry;
	protected SCR_SettingsManagerKeybindModule m_SettingsKeybindModule;

	protected string m_sActionName;
	protected string m_sActionPreset;
	protected string m_sGamepadAltActionName;
	protected string m_sGamepadAltActionPreset;
	protected string m_sActionDisplayName;

	protected SCR_EActionPrefixType m_eActionPrefixType;
	
	protected bool m_bOnInlineMouseButton;
	
	protected Widget m_wInlineButtonsWrapper;
	
	protected SCR_ModularButtonComponent m_BindButtonKeyboard;
	protected SCR_ModularButtonComponent m_BindButtonController;
	
	protected SCR_KeybindActionDisplayRowComponent m_ActionDisplayKeyboard;
	protected SCR_KeybindActionDisplayRowComponent m_ActionDisplayController;
	
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocus;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocusLost;
	
	protected ref ScriptInvokerVoid m_OnKeyCaptured;
	protected ref ScriptInvokerString m_OnInlineButton;

	//------------------------------------------------------------------------------------------------
	override event bool OnFocus(Widget w, int x, int y)
	{
		m_bFocused = true;
		UpdateInlineButtons();
		UpdateBindButtons();
		UpdateActionDisplays();

		if (m_OnFocus)
			m_OnFocus.Invoke(this);
		
		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool OnFocusLost(Widget w, int x, int y)
	{	
		m_bFocused = false;
		UpdateInlineButtons();
		UpdateBindButtons();
		UpdateActionDisplays();

		if (m_OnFocusLost)
			m_OnFocusLost.Invoke(this);
		
		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		SCR_MenuHelper.GetOnDialogClose().Remove(OnDialogClose);
	}
	
	// Init
	//------------------------------------------------------------------------------------------------
	void Init(notnull InputBinding binding, SCR_KeyBindingEntry entry)
	{
		// Setup
		m_sActionDisplayName = entry.m_sDisplayName;
		if (m_sActionDisplayName.Length() == 0)
			m_sActionDisplayName = "<#AR-Settings_Keybind_MissingName>" + entry.m_sActionName;
	
		m_sActionPreset = entry.m_sPreset;
		m_sActionName = entry.m_sActionName;
		m_sGamepadAltActionName = entry.m_sActionNameGamepadOptional;
		m_sGamepadAltActionPreset = entry.m_sPresetGamepadOptional;
		m_Binding = binding;
		m_Entry = entry;
		m_eActionPrefixType = entry.m_ePrefixType;
		
		m_SettingsKeybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!m_SettingsKeybindModule)
			return;
		
		RichTextWidget textBox = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Name"));
		textBox.SetText(m_sActionDisplayName);
		textBox.ElideText(1, 1.0, "...");
	
		// Bindings buttons
		// Keyboard
		Widget pcBind = m_wRoot.FindAnyWidget(WIDGET_BIND_PC);
		if (pcBind)
		{
			m_BindButtonKeyboard = SCR_ModularButtonComponent.FindComponent(pcBind);
			m_ActionDisplayKeyboard = SCR_KeybindActionDisplayRowComponent.FindComponentInHierarchy(pcBind);
			if (m_ActionDisplayKeyboard)
				m_ActionDisplayKeyboard.GetOnAdditionalBindsClicked().Insert(OnAdvancedKeybind);
		}
		
		if (m_BindButtonKeyboard)
		{
			if (GetGame().IsPlatformGameConsole() && !GetGame().GetHasKeyboard())
				m_BindButtonKeyboard.SetVisible(false);
			else
				SetRichTextAction(EInputDeviceType.KEYBOARD);
			
			m_BindButtonKeyboard.m_OnClicked.Insert(OnBindKeyboardClicked);
			m_BindButtonKeyboard.m_OnMouseEnter.Insert(OnInlineMouseButtonEnter);
			m_BindButtonKeyboard.m_OnMouseLeave.Insert(OnInlineMouseButtonLeave);
		}
	
		// Controller
		Widget controllerBind = m_wRoot.FindAnyWidget(WIDGET_BIND_CONTROLLER);
		if (controllerBind)
		{
			m_BindButtonController = SCR_ModularButtonComponent.FindComponent(controllerBind);
			m_ActionDisplayController = SCR_KeybindActionDisplayRowComponent.FindComponentInHierarchy(controllerBind);
		}
		
		if (m_BindButtonController)
		{
			SetRichTextAction(EInputDeviceType.GAMEPAD);
			
			m_BindButtonController.m_OnClicked.Insert(OnBindControllerClicked);
			m_BindButtonController.m_OnMouseEnter.Insert(OnInlineMouseButtonEnter);
			m_BindButtonController.m_OnMouseLeave.Insert(OnInlineMouseButtonLeave);
		}

		// Main button
		SCR_ModularButtonComponent button = SCR_ModularButtonComponent.FindComponent(m_wRoot);
		if (button)
			button.m_OnClicked.Insert(OnButtonClick);
		
		// Inline buttons
		m_wInlineButtonsWrapper = m_wRoot.FindAnyWidget("InlineButtons");
		if (m_wInlineButtonsWrapper)
		{
			array<ref Widget> children = {};
			SCR_WidgetHelper.GetAllChildren(m_wInlineButtonsWrapper, children);
			SCR_ModularButtonComponent comp;
			foreach (Widget child : children)
			{
				comp = SCR_ModularButtonComponent.FindComponent(child);
				if (!comp)
					continue;
				
				switch (child.GetName())
				{
					case BUTTON_ADVANCED_KEYBIND: 	comp.m_OnClicked.Insert(OnAdvancedKeybind); break;
					case BUTTON_RESET: 				comp.m_OnClicked.Insert(OnReset); break;
					case BUTTON_UNBIND: 			comp.m_OnClicked.Insert(OnUnbind); break;					
				}
				
				comp.m_OnMouseEnter.Insert(OnInlineMouseButtonEnter);
				comp.m_OnMouseLeave.Insert(OnInlineMouseButtonLeave);
			}
		}
		
		UpdateInlineButtons();
		UpdateBindButtons();
		UpdateActionDisplays();

		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChange);
		SCR_MenuHelper.GetOnDialogOpen().Insert(OnDialogOpen);
		SCR_MenuHelper.GetOnDialogClose().Insert(OnDialogClose);
	}
	
	// Protected
	//------------------------------------------------------------------------------------------------
	protected void OnButtonClick()
	{
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();
		
		if (m_bOnInlineMouseButton && device == EInputDeviceType.MOUSE)
			return;
		
		if (device == EInputDeviceType.MOUSE || device == EInputDeviceType.KEYBOARD)
			CaptureKeyboard();
		
		else if (device == EInputDeviceType.GAMEPAD)
			CaptureGamepad();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CaptureKeyboard()
	{
		if (m_sActionPreset.IsEmpty())
			return;
		
		string finalPreset = m_SettingsKeybindModule.GetPrimaryPresetPrefix() + m_sActionPreset;
		
		// Enfusion supports capturing mouse and keyboard at once under the EInputDeviceType.KEYBOARD
		m_SettingsKeybindModule.StartCaptureForAction(m_sActionName, finalPreset, EInputDeviceType.KEYBOARD, false);
		
		// Dialog
		SCR_SimpleKeybindDialogUI dialog = SCR_KeybindDialogs.CreateSimpleKeybindDialog(
			m_Entry,
			m_sActionDisplayName,
			m_sActionName,
			m_Binding, 
			EInputDeviceType.KEYBOARD
		);
		
		if (!dialog)
			return;

		dialog.GetOnKeyCaptured().Insert(OnKeyCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CaptureGamepad()
	{
		if (m_sActionPreset.IsEmpty() && m_sGamepadAltActionPreset.IsEmpty())
			return;
		
		string finalPreset;
		
		if (m_sGamepadAltActionPreset.IsEmpty())
			finalPreset = m_SettingsKeybindModule.GetGamepadPresetPrefix() + m_sActionPreset;
		else
			finalPreset = m_SettingsKeybindModule.GetGamepadPresetPrefix() + m_sGamepadAltActionPreset;
		
		string actionName = m_sActionName;
		if (!m_sGamepadAltActionName.IsEmpty())
			actionName = m_sGamepadAltActionName;

		m_SettingsKeybindModule.StartCaptureForAction(actionName, finalPreset, EInputDeviceType.GAMEPAD, false);
		
		// Dialog
		SCR_SimpleKeybindDialogUI dialog = SCR_KeybindDialogs.CreateSimpleKeybindDialog(
			m_Entry,
			m_sActionDisplayName,
			actionName,
			m_Binding, 
			EInputDeviceType.GAMEPAD
		);
		
		if (!dialog)
			return;

		dialog.GetOnKeyCaptured().Insert(OnKeyCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInlineMouseButtonEnter(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		m_bOnInlineMouseButton = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInlineMouseButtonLeave(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		m_bOnInlineMouseButton = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBindKeyboardClicked(SCR_ModularButtonComponent comp)
	{
		if (!comp.GetToggled())
			CaptureKeyboard();	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBindControllerClicked(SCR_ModularButtonComponent comp)
	{
		if (!comp.GetToggled())
			CaptureGamepad();	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAdvancedKeybind()
	{
		if (m_OnInlineButton)
			m_OnInlineButton.Invoke(BUTTON_ADVANCED_KEYBIND);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnReset()
	{
		if (m_OnInlineButton)
			m_OnInlineButton.Invoke(BUTTON_RESET);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUnbind()
	{
		if (m_OnInlineButton)
			m_OnInlineButton.Invoke(BUTTON_UNBIND);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnKeyCaptured()
	{
		if (m_OnKeyCaptured)
			m_OnKeyCaptured.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceChange(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		UpdateInlineButtons();
		UpdateBindButtons();
		UpdateActionDisplays();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDialogOpen(DialogUI dialog)
	{
		m_bIsDialogOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDialogClose(DialogUI dialog)
	{
		m_bIsDialogOpen = false;
		UpdateBindButtons();
		UpdateActionDisplays();
	}
	
	//------------------------------------------------------------------------------------------------
	// Currently supports either KEYBOARD or GAMEPAD
	protected void SetRichTextAction(EInputDeviceType device)
	{
		if (device == EInputDeviceType.MOUSE)
			device = EInputDeviceType.KEYBOARD;
		
		if (m_ActionDisplayKeyboard && device != EInputDeviceType.GAMEPAD)	
			m_ActionDisplayKeyboard.CreateActionDisplays(m_Entry, m_SettingsKeybindModule, device);
		else if (m_ActionDisplayController)
			m_ActionDisplayController.CreateActionDisplays(m_Entry, m_SettingsKeybindModule, device);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateActionDisplayScrolling(bool enable)
	{
		if (m_ActionDisplayKeyboard)	
			m_ActionDisplayKeyboard.UpdateScrolling(enable);
		
		if (m_ActionDisplayController)
			m_ActionDisplayController.UpdateScrolling(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateInlineButtons()
	{
		bool visible = m_bFocused && GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE;
		
		if (m_wInlineButtonsWrapper)
			m_wInlineButtonsWrapper.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateBindButtons()
	{
		if (m_bIsDialogOpen)
			return;
		
		bool isController = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD;
		bool keyboardBindEnabled = !isController && (!GetGame().IsPlatformGameConsole() || GetGame().GetHasKeyboard());
		
		// Toggling is used as an alternative to disabling, so that they may still react to mouse inputs
		if (m_BindButtonKeyboard)
			m_BindButtonKeyboard.SetToggled(!keyboardBindEnabled || m_sActionPreset.IsEmpty(), false);
		
		if (m_BindButtonController)
			m_BindButtonController.SetToggled(!isController || m_sActionPreset.IsEmpty(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateActionDisplays()
	{
		bool isController = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD;
		SCR_EActionDisplayState stateKeyboard = SCR_EActionDisplayState.DEFAULT;
		SCR_EActionDisplayState stateGamepad = SCR_EActionDisplayState.DEFAULT;
		
		if (isController)
			stateKeyboard = SCR_EActionDisplayState.DISABLED;
		else
			stateGamepad = SCR_EActionDisplayState.DISABLED;
		
		if (m_ActionDisplayKeyboard)
			m_ActionDisplayKeyboard.SetAllActionDisplayStates(stateKeyboard);
		
		if (m_ActionDisplayController)
			m_ActionDisplayController.SetAllActionDisplayStates(stateGamepad);
	}
	
	// Public
	//------------------------------------------------------------------------------------------------
	void ResetAction()
	{
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();
		
		string actionName = m_sActionName;
		if (device == EInputDeviceType.GAMEPAD && !m_sGamepadAltActionName.IsEmpty())
			actionName = m_sGamepadAltActionName;

		m_SettingsKeybindModule.ResetAction(actionName, GetFinalActionPreset(), device);
		
		SetRichTextAction(device);
		UpdateActionDisplayScrolling(false);
		
		SCR_AnalyticsApplication.GetInstance().ChangeKeybind(m_sActionName, m_sActionPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unbind()
	{
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();

		// KEYBOARD is also used for mouse
		if (device == EInputDeviceType.MOUSE)
			device = EInputDeviceType.KEYBOARD;
		
		string actionName = m_sActionName;
		if (device == EInputDeviceType.GAMEPAD && !m_sGamepadAltActionName.IsEmpty())
			actionName = m_sGamepadAltActionName;

		m_SettingsKeybindModule.UnbindAction(actionName, GetFinalActionPreset(), device);
		
		SetRichTextAction(device);
		UpdateActionDisplayScrolling(false);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_KeyBindingEntry GetEntry()
	{
		return m_Entry;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sActionName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		return m_sActionDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionPreset()
	{
		return m_sActionPreset;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFinalActionPreset()
	{
		string finalPreset;
		string devicePrefix;
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();

		if (deviceType == EInputDeviceType.GAMEPAD)
			devicePrefix = m_SettingsKeybindModule.GetGamepadPresetPrefix();
		else
			devicePrefix = m_SettingsKeybindModule.GetPrimaryPresetPrefix();
		
		if (deviceType == EInputDeviceType.GAMEPAD && !m_sGamepadAltActionPreset.IsEmpty())
			finalPreset = devicePrefix + m_sGamepadAltActionPreset;
		else if (!m_sActionPreset.IsEmpty())
			finalPreset = devicePrefix + m_sActionPreset;
		
		return finalPreset;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EActionPrefixType GetActionPrefixType()
	{
		return m_eActionPrefixType;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocus()
	{
		if (!m_OnFocus)
			m_OnFocus = new ScriptInvokerScriptedWidgetComponent();
		
		return m_OnFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocusLost()
	{
		if (!m_OnFocusLost)
			m_OnFocusLost = new ScriptInvokerScriptedWidgetComponent();
		
		return m_OnFocusLost;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnKeyCaptured()
	{
		if (!m_OnKeyCaptured)
			m_OnKeyCaptured = new ScriptInvokerVoid();
		
		return m_OnKeyCaptured;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerString GetOnInlineButton()
	{
		if (!m_OnInlineButton)
			m_OnInlineButton = new ScriptInvokerString();
		
		return m_OnInlineButton;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_KeybindRowComponent FindComponent(Widget w)
	{
		return SCR_KeybindRowComponent.Cast(w.FindHandler(SCR_KeybindRowComponent));
	}
}