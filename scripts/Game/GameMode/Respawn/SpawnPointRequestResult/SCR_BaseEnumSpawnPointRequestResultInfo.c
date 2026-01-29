[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ESpawnResult, "m_eSpawnResult", "Request Result: %1")]
class SCR_BaseEnumSpawnPointRequestResultInfo : SCR_BaseSpawnPointRequestResultInfo
{
	[Attribute(desc: "Spawn result required to show given Result Info", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ESpawnResult))]
	protected SCR_ESpawnResult m_eSpawnResult;
	
	//------------------------------------------------------------------------------------------------
	override bool IsValidRequestResultInfo(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data)
	{
		return response == m_eSpawnResult;
	}
}
