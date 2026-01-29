class SCR_MissionHeaderSequence : SCR_MissionHeaderCombatOps
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Which scenario (header) should be loaded after the current one has been finished.", "conf")]
	ResourceName m_sNextScenario;

	[Attribute("", desc: "Variable name in SCR_ScenarioSequenceProgress specific for this sequence.")]
	string m_sProgressVariableName;

	[Attribute("0", desc: "This scenario's index in the sequence. If the sequence progress is lower than this index, this scenario is locked.")]
	int m_iScenarioIndex;

	//------------------------------------------------------------------------------------------------
	void UpdateProgress()
	{
		BaseContainer progressData = GetGame().GetGameUserSettings().GetModule("SCR_ScenarioSequenceProgress");

		if (!progressData)
			return;

		int progress;
		progressData.Get(m_sProgressVariableName, progress);

		if (m_iScenarioIndex < progress)
			return;

		// If finishing this scenario furthers the progress, increase the saved index
		progressData.Set(m_sProgressVariableName, m_iScenarioIndex + 1);
		GetGame().SaveUserSettings();
		GetGame().UserSettingsChanged();
	}
}
