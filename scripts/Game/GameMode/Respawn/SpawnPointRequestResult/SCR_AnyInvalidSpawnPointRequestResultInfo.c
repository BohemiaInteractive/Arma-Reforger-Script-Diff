//! Used as a catch all for when players cannot spawn at spawn point and all other checks were invalid
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_UIInfo")]
class SCR_AnyInvalidSpawnPointRequestResultInfo : SCR_BaseSpawnPointRequestResultInfo
{
	//------------------------------------------------------------------------------------------------
	override bool IsValidRequestResultInfo(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data)
	{
		return response != SCR_ESpawnResult.OK;
	}
}
