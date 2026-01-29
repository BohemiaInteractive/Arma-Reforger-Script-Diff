class SCR_ScenarioSequenceProgress : ModuleGameSettings
{
	[Attribute("0")]
	int m_iOperationOmega;

	//------------------------------------------------------------------------------------------------
	static bool IsScenarioLocked(notnull MissionWorkshopItem item)
	{
		SCR_MissionHeaderSequence sequenceHeader = SCR_MissionHeaderSequence.Cast(MissionHeader.ReadMissionHeader(item.Id()));

		if (!sequenceHeader)
			return false;

		BaseContainer progressData = GetGame().GetGameUserSettings().GetModule("SCR_ScenarioSequenceProgress");

		if (!progressData)
			return false;

		int progress;
		progressData.Get(sequenceHeader.m_sProgressVariableName, progress);

		return (progress < sequenceHeader.m_iScenarioIndex);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsNextScenarioLocked(notnull MissionWorkshopItem item)
	{
		SCR_MissionHeaderSequence sequenceHeader = SCR_MissionHeaderSequence.Cast(MissionHeader.ReadMissionHeader(item.Id()));

		if (!sequenceHeader)
			return false;

		BaseContainer progressData = GetGame().GetGameUserSettings().GetModule("SCR_ScenarioSequenceProgress");

		if (!progressData)
			return false;

		int progress;
		progressData.Get(sequenceHeader.m_sProgressVariableName, progress);

		return (progress > sequenceHeader.m_iScenarioIndex);
	}
}
