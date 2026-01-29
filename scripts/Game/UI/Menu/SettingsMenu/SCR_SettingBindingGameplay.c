class SCR_SettingBindingGameplay : SCR_SettingsBindingBase
{
	void SCR_SettingBindingGameplay(string module, string name, string widgetName, string subModule = string.Empty)
	{
		m_bUseGameUserSettings = true;
		if (!SetBaseContainer())
			return;

		m_iItem = m_Setting.GetVarIndex(m_sName);
		if (m_iItem < 0)
			return;
	}
};
