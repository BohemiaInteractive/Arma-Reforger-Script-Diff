[BaseContainerProps(configRoot: true)]
class SCR_SpawnPointRequestResultInfoConfig
{	
	[Attribute(desc: "When a spawn point is disabled the system will go over each disable reason in order and will return the first valid disable reason")]
	protected ref array<ref SCR_BaseSpawnPointRequestResultInfo> m_aSpawnPointRequestResultInfos;
	
	//~ Holds a sorted list that orders the spawn point not allowed reason depending on the given order
	protected ref SCR_SortedArray<SCR_BaseSpawnPointRequestResultInfo> m_aSortedSpawnPointRequestResultInfos = new SCR_SortedArray<SCR_BaseSpawnPointRequestResultInfo>();
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_SpawnPointRequestResultInfoConfig()
	{
		if (SCR_Global.IsEditMode())
			return;
		
		int count = m_aSpawnPointRequestResultInfos.Count();
		
		SCR_BaseSpawnPointRequestResultInfo foundRequestResult;
		
		//~ Init ordered list
		for (int i = count - 1; i >= 0; i--)
		{
			foundRequestResult = m_aSpawnPointRequestResultInfos[i];
			
			if (!foundRequestResult || !foundRequestResult.m_bEnabled)
			{
				m_aSpawnPointRequestResultInfos.RemoveOrdered(i);
				continue;
			}
			
			if (!foundRequestResult.GetUIInfo())
			{
				m_aSpawnPointRequestResultInfos.RemoveOrdered(i);
				Print("'SCR_SpawnPointDisabledReasonConfig' init, entry: '" + i + "' has no UIInfo assigned and thus is removed.", LogLevel.ERROR);
				continue;
			}
			
			m_aSortedSpawnPointRequestResultInfos.Insert(foundRequestResult.m_iOrder, foundRequestResult);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] requestComponent
	//! \param[in] response
	//! \param[in] data
	//! \return
	SCR_BaseSpawnPointRequestResultInfo GetFirstValidRequestResultInfo(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data)
	{
		SCR_BaseSpawnPointRequestResultInfo requestResultInfo;
		
		int count = m_aSortedSpawnPointRequestResultInfos.Count();
		for(int i = 0; i < count; ++i)
		{
			requestResultInfo = m_aSortedSpawnPointRequestResultInfos.Get(i);
			if (!requestResultInfo || !requestResultInfo.IsValidRequestResultInfo(requestComponent, response, data))
				continue;
			
			//~ Is the first valid not allowed reason
			return requestResultInfo;
		}
		
		return null;
	}
}
