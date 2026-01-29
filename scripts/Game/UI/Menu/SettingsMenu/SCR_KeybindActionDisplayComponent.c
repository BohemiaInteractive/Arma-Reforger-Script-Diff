/*
Component that takes care of displaying action bindings
Additional text will display the number of extra bindings associated with the action
*/

class SCR_KeybindActionDisplayComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(UIConstants.ACTION_DISPLAY_ICON_SCALE_BIG)]
	float m_fActionIconScale;
	
	protected RichTextWidget m_wText;
	
	//TODO: cleanup: move the separator to the ActionDisplayRow
	protected Widget m_wSeparator;
	protected ref SCR_KeybindActionDisplayData m_ActionDisplayData;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Text"));
		m_wSeparator = m_wRoot.FindAnyWidget("Separator");
	}

	//------------------------------------------------------------------------------------------------
	void UpdateActionDisplay(SCR_KeybindActionDisplayData data)
	{
		if (!data || !data.m_Entry || !data.m_SettingsKeybindModule)
			return;

		SCR_KeyBindingEntry entry = data.m_Entry;
		
		string finalPreset;
		string deviceString;
		string actionName;
		string preset;

		// Devices
		if (data.m_eDevice == EInputDeviceType.GAMEPAD) // Gamepad setup
		{
			if (!entry.m_sActionNameGamepadOptional.IsEmpty())
				actionName = entry.m_sActionNameGamepadOptional;
			else
				actionName = entry.m_sActionName;
			
			if (!entry.m_sPresetGamepadOptional.IsEmpty())
				preset = entry.m_sPresetGamepadOptional;
			else
				preset = entry.m_sPreset;
			
			deviceString = UIConstants.DEVICE_GAMEPAD;

			if (!preset.IsEmpty())
				finalPreset = data.m_SettingsKeybindModule.GetGamepadPresetPrefix() + preset;
		}
		else if (data.m_eDevice == EInputDeviceType.KEYBOARD) // Keyboard & mouse setup
		{
			actionName = entry.m_sActionName;
			preset = entry.m_sPreset;
			deviceString = UIConstants.DEVICE_KEYBOARD;

			if (!preset.IsEmpty())
				finalPreset = data.m_SettingsKeybindModule.GetPrimaryPresetPrefix() + preset;
		}
		
		// Set Action text
		if (m_wText)
		{
			string binding = string.Format(
				"<action name='%1' preset='%2' device='%3' scale='%4' index='%5' state='%6'/>", 
				actionName, 
				finalPreset, 
				deviceString, 
				m_fActionIconScale.ToString(),
				data.m_iIndex.ToString(),
				UIConstants.GetActionDisplayStateAttribute(data.m_eState)
				);
			
			m_wText.SetText(binding);
		}
		
		// Store data, should we need to modify only one of them
		m_ActionDisplayData = data;
		
		if (m_wSeparator)
			m_wSeparator.SetVisible(data.m_iIndex < data.m_iLastIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_KeybindActionDisplayData GetActionDisplayData()
	{
		return m_ActionDisplayData;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_KeybindActionDisplayComponent FindComponent(Widget w)
	{
		return SCR_KeybindActionDisplayComponent.Cast(w.FindHandler(SCR_KeybindActionDisplayComponent));
	}
}

class SCR_KeybindActionDisplayData
{
	SCR_KeyBindingEntry m_Entry;
	SCR_SettingsManagerKeybindModule m_SettingsKeybindModule;
	EInputDeviceType m_eDevice;
	SCR_EActionDisplayState m_eState;
	int m_iIndex;
	int m_iLastIndex;

	//------------------------------------------------------------------------------------------------
	void SCR_KeybindActionDisplayData(SCR_KeyBindingEntry entry, SCR_SettingsManagerKeybindModule settingsKeybindModule, EInputDeviceType device, SCR_EActionDisplayState state, int index = 0, int lastIndex = 0)
	{
		m_Entry = entry;
		m_SettingsKeybindModule = settingsKeybindModule;
		m_eDevice = device;
		m_eState = state;
		m_iIndex = index;
		m_iLastIndex = lastIndex;
	}
}