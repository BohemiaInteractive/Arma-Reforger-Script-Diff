class SCR_JoystickBindingRowComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(defvalue: EInputDeviceType.JOYSTICK.ToString(), desc: "Input Device", uiwidget: UIWidgets.ComboBox, enumType: EInputDeviceType)]
	protected EInputDeviceType m_eInputDevice;

	protected SCR_SettingsManagerKeybindModule m_SettingsKeybindModule;
	protected SCR_ModularButtonComponent m_BindButton;
	protected SCR_KeybindActionDisplayRowComponent m_ActionDisplay;
	protected string m_sActionName;
	protected string m_sActionPreset;
	protected string m_sActionDisplayName;
	protected string m_sDevicePrefix;
	protected SCR_EActionPrefixType m_eActionPrefixType;
	protected ref InputBinding m_Binding;
	protected ref SCR_KeyBindingEntry m_Entry;
	protected Widget m_wInlineButtonsWrapper;
	protected bool m_bFocused;
	protected bool m_bOnInlineMouseButton;

	protected ref ScriptInvokerVoid m_OnKeyCaptured;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocus;

	protected const string BUTTON_BINDING = "Binding";
	protected const string BUTTON_ADVANCED_KEYBIND = "AdvancedKeybind";
	protected const string BUTTON_UNBIND = "Unbind";
	protected const string WIDGET_ACTION_NAME = "Name";
	protected const string WIDGET_INLINE_BUTTONS_HOLDER = "InlineButtons";
	protected const string TEXT_OVERFLOW_INDICATOR = "...";

	//------------------------------------------------------------------------------------------------
	void Init(notnull InputBinding binding, SCR_KeyBindingEntry entry)
	{
		m_sActionDisplayName = entry.m_sDisplayName;
		m_eActionPrefixType = entry.m_ePrefixType;
		m_sActionPreset = entry.m_sPreset;
		m_sActionName = entry.m_sActionName;
		m_Binding = binding;
		m_Entry = entry;
		
		m_SettingsKeybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!m_SettingsKeybindModule)
			return;

		m_sDevicePrefix = m_SettingsKeybindModule.GetGamepadPresetPrefix();
		RichTextWidget textBox = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_ACTION_NAME));
		if (textBox)
		{
			if (!m_sActionDisplayName.IsEmpty())
				textBox.SetText(m_sActionDisplayName);

			textBox.ElideText(1, 1, TEXT_OVERFLOW_INDICATOR);
		}

		Widget bindButton = m_wRoot.FindAnyWidget(BUTTON_BINDING);
		if (!binding)
			return;

		m_BindButton = SCR_ModularButtonComponent.FindComponent(bindButton);
		m_ActionDisplay = SCR_KeybindActionDisplayRowComponent.FindComponentInHierarchy(bindButton);
		if (m_BindButton)
		{
			if (m_ActionDisplay)
			{
				m_ActionDisplay.GetOnAdditionalBindsClicked().Insert(OnAdvancedKeybind);
				m_ActionDisplay.CreateActionDisplays(m_Entry, m_SettingsKeybindModule, m_eInputDevice);
			}

			m_BindButton.m_OnClicked.Insert(OnBindClicked);
			m_BindButton.m_OnMouseEnter.Insert(OnInlineMouseButtonEnter);
			m_BindButton.m_OnMouseLeave.Insert(OnInlineMouseButtonLeave);
		}

		SCR_ModularButtonComponent button = SCR_ModularButtonComponent.FindComponent(m_wRoot);
		if (button)
			button.m_OnClicked.Insert(OnButtonClick);

		m_wInlineButtonsWrapper = m_wRoot.FindAnyWidget(WIDGET_INLINE_BUTTONS_HOLDER);
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
					case BUTTON_UNBIND: 			comp.m_OnClicked.Insert(OnUnbind); break;					
				}
				
				comp.m_OnMouseEnter.Insert(OnInlineMouseButtonEnter);
				comp.m_OnMouseLeave.Insert(OnInlineMouseButtonLeave);
			}
		}
		
		UpdateInlineButtons();
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChange);;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateInlineButtons()
	{
		bool visible = m_bFocused && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD;
		if (m_wInlineButtonsWrapper)
			m_wInlineButtonsWrapper.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	override event bool OnFocus(Widget w, int x, int y)
	{
		m_bFocused = true;
		UpdateInlineButtons();
		if (m_OnFocus)
			m_OnFocus.Invoke(this);

		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool OnFocusLost(Widget w, int x, int y)
	{	
		m_bFocused = false;
		UpdateInlineButtons();
		
		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_JoystickBindSettingsSubMenu)]
	protected void OnAdvancedKeybind()
	{
		AdvancedKeybindButtonClick(m_eInputDevice);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBindClicked(SCR_ModularButtonComponent comp)
	{
		if (!comp.GetToggled())
			CaptureInput(m_eInputDevice);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUnbind()
	{
		Unbind();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnKeyCaptured()
	{
		if (m_OnKeyCaptured)
			m_OnKeyCaptured.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonClick()
	{
		CaptureInput(m_eInputDevice);
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
	protected void CaptureInput(EInputDeviceType deviceType)
	{
		if (m_sActionPreset.IsEmpty())
			return;

		// Enfusion supports capturing mouse and keyboard at once under the EInputDeviceType.KEYBOARD
		m_SettingsKeybindModule.StartCaptureForAction(m_sActionName, GetFinalActionPreset(), deviceType, false);
		
		// Dialog
		SCR_SimpleKeybindDialogUI dialog = SCR_KeybindDialogs.CreateSimpleKeybindDialog(
			m_Entry,
			m_sActionDisplayName,
			m_sActionName,
			m_Binding,
			deviceType
		);
		
		if (!dialog)
			return;

		dialog.GetOnKeyCaptured().Insert(OnKeyCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnKeyCaptured()
	{
		if (!m_OnKeyCaptured)
			m_OnKeyCaptured = new ScriptInvokerVoid();
		
		return m_OnKeyCaptured;
	}

	//------------------------------------------------------------------------------------------------
	protected void AdvancedKeybindButtonClick(EInputDeviceType deviceType)
	{
		SCR_AdvancedKeybindDialogUI keybindDialog = SCR_KeybindDialogs.CreateAdvancedKeybindDialog(
			m_Entry,
			m_sActionDisplayName,
			m_sActionName, 
			GetFinalActionPreset(),
			SCR_EActionPrefixType.AMBIGUOUS,
			m_eInputDevice
		);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unbind()
	{
		m_SettingsKeybindModule.UnbindAction(m_sActionName, GetFinalActionPreset(), m_eInputDevice);
		
		SetRichTextAction(m_eInputDevice);
		UpdateActionDisplayScrolling(false);
		
		SCR_AnalyticsApplication analytics = SCR_AnalyticsApplication.GetInstance();
		if (analytics)
			analytics.ChangeKeybind(m_sActionName, GetFinalActionPreset());
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFinalActionPreset()
	{
		string finalPreset;
		if (!m_sActionPreset.IsEmpty())
			finalPreset = m_SettingsKeybindModule.GetJoystickPresetPrefix() + m_sActionPreset;

		return finalPreset;
	}
	
	//------------------------------------------------------------------------------------------------
	// Currently supports either KEYBOARD or GAMEPAD
	protected void SetRichTextAction(EInputDeviceType device)
	{
		if (m_ActionDisplay)	
			m_ActionDisplay.CreateActionDisplays(m_Entry, m_SettingsKeybindModule, m_eInputDevice);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateActionDisplayScrolling(bool enable)
	{
		if (m_ActionDisplay)	
			m_ActionDisplay.UpdateScrolling(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceChange(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		UpdateInlineButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocus()
	{
		if (!m_OnFocus)
			m_OnFocus = new ScriptInvokerScriptedWidgetComponent();
		
		return m_OnFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionPreset()
	{
		return m_sActionPreset;
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
}