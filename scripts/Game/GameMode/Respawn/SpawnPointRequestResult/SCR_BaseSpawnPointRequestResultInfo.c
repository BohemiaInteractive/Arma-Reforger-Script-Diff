[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_UIInfo")]
class SCR_BaseSpawnPointRequestResultInfo
{
	[Attribute(desc: "The lower the order the earlier this entry will be checked. EG: order 1 will be checked before order 100")]
	int m_iOrder;
	
	[Attribute("1", desc: "Disabled entries will be removed on init")]
	bool m_bEnabled;
	
	[Attribute("1", desc: "If true will show the respawn timer if any availible. This requires the UIInfo 'NameWithTimer' to be filled. %1 is the time left to respawn. If Respawn time is 0 then 'Name' is used instead")]
	protected bool m_bShowRespawnTime;
	
	[Attribute(desc: "UI info used to disaplay the spaw poimt Request result. If null then the entry will be removed from the list on init!")]
	protected ref SCR_UIInfoSpawnRequestResult m_UIInfo;
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns UI info of Disable reason
	SCR_UIInfoSpawnRequestResult GetUIInfo()
	{
		return m_UIInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return True if Respawn Time should be displayed if any is active
	bool ShowRespawnTime()
	{
		return m_bShowRespawnTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reason why the spawn point is disabled
	//! \param[in] requestComponent
	//! \param[in] response
	//! \param[in] data
	//! \return
	// Override this method in inherited classes to display the correct reason why a spawn point is disabled
	bool IsValidRequestResultInfo(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data);
}