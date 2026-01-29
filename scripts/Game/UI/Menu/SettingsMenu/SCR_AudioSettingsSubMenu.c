class SCR_AudioSettingsSubMenu: SCR_SettingsSubMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_aSettingsBindings.Clear();
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "Volume", "Master"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeSfx", "SFX"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeMusic", "Music"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeVoiceChat", "VOIP"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeDialog", "Dialogue"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "StereoMode", "ProcessingMode"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "DualsenseSpeakerEnabled", "DualsenseSpeakerEnabled"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AudioSettings", "m_fDynamicRange", "DynamicRange"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AudioSettings", "m_bGTinnitus", "EnableTinnitus"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AudioSettings", "m_bHQAnnouncer", "HQAnnouncer"));
		LoadSettings();
		
		Widget dualsenseSpeakerWidget = m_wRoot.FindAnyWidget("DualsenseSpeakerEnabled");
		if (dualsenseSpeakerWidget && System.GetPlatform() != EPlatform.PS5 && System.GetPlatform() != EPlatform.PS5_PRO)
			dualsenseSpeakerWidget.SetVisible(false);
	}
};