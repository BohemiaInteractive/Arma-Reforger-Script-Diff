class SCR_SettingSpinBoxUserData : Managed
{
	protected string m_sSettingName;

	//------------------------------------------------------------------------------------------------
	//! Set the name for the setting
	//! \param[in] setting name
	void SetSettingName(string settingName)
	{
		m_sSettingName = settingName;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the defined setting name
	//! \return setting name. string.Empty if not defined
	string GetSettingName()
	{
		if (m_sSettingName.IsEmpty())
			return string.Empty;

		return m_sSettingName;
	}
}
