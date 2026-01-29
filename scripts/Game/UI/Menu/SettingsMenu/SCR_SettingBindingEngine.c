class SCR_SettingBindingEngine : SCR_SettingsBindingBase
{
	void SCR_SettingBindingEngine(string module, string name, string widgetName, string subModule = string.Empty)
	{
		if (!SetBaseContainer())
			return;

		m_iItem = m_Setting.GetVarIndex(m_sName);
		if (m_iItem < 0)
			return;
	}
};
