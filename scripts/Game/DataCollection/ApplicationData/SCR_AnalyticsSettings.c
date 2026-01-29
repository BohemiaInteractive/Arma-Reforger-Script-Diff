class SCR_AnalyticsSettings
{
	protected static const string UNCHANGED = "Unchanged";
	protected ref SCR_AnalyticsTimer m_Timer = new SCR_AnalyticsTimer();
	protected ref map<string /*category*/, ref set<string> /*changes*/> m_ChangedSettings = new map<string, ref set<string>>();
	protected bool m_WasResetAllKeysUsed = false;
	protected string m_QualityPreset = UNCHANGED;
	protected string m_HudVisibility = UNCHANGED;

	//------------------------------------------------------------------------------------------------
	//! Start tracking changed settings. This CLEARS all previous data.
	void Start()
	{
		m_Timer.Start();
		m_ChangedSettings.Clear();
		m_WasResetAllKeysUsed = false;
		m_QualityPreset = UNCHANGED;
		m_HudVisibility = UNCHANGED;
	}

	//------------------------------------------------------------------------------------------------
	//! Set a different hud visibility
	//! \param[in] index from combobox
	void SetHudVisibility(int index)
	{
		array<string> options = {"HUD_Visiblity_Hide_all", "HUD_Visiblity_Show_all", "HUD_Visiblity_Custom"};

		if (options.IsIndexValid(index))
			m_HudVisibility = options[index];
		else
			m_HudVisibility = "HUD_Visiblity_" + index.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! Set a different quality preset
	//! \param[in] index
	void SetQualityPreset(int index)
	{
		array<string> options = {"QualityPreset_Low", "QualityPreset_Medium", "QualityPreset_High", "QualityPreset_Ultra", "QualityPreset_Custom"};

		if (options.IsIndexValid(index))
			m_QualityPreset = options[index];
		else
			m_QualityPreset = "QualityPreset_" + index.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! Use a reset all button for keybinds
	void UseResetAllButton()
	{
		m_WasResetAllKeysUsed = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Change some setting in some category
	//! \param[in] category
	//! \param[in] action
	void ChangeSetting(string category, string action)
	{
		if (!m_ChangedSettings.Contains(category))
			m_ChangedSettings.Insert(category, new set<string>());

		m_ChangedSettings.Get(category).Insert(action);
	}

	//------------------------------------------------------------------------------------------------
	//! \return time spent in seconds
	int GetTimeSpent()
	{
		return m_Timer.GetTimeSpent();
	}

	//------------------------------------------------------------------------------------------------
	//! \return JSON-like formatted string of all changes
	string GetSettingsChanged()
	{
		string result = "{";

		foreach (string category, set<string> changes : m_ChangedSettings)
			result += string.Format("'%1': %2,", category, SerializeChanges(changes));

		int resetAll = m_WasResetAllKeysUsed;
		result += string.Format("'ResetAll':%1,'QualityPreset':'%2','HudVisibility':'%3'}", resetAll, m_QualityPreset, m_HudVisibility);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected string SerializeChanges(set<string> changes)
	{
		string result = "{";

		bool firstElement = true;
		foreach (string change : changes)
		{
			if (firstElement)
				result += string.Format("'%1'", change);
			else
				result += string.Format(",'%1'", change);

			firstElement = false;
		}

		return result + "}";
	}
}
