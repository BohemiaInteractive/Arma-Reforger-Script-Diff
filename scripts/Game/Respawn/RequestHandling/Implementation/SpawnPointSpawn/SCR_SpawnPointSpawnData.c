//------------------------------------------------------------------------------------------------
class SCR_SpawnPointSpawnData : SCR_SpawnData
{
	protected ResourceName m_Prefab;
	protected RplId m_RplId;
	protected SCR_SpawnPoint m_SpawnPoint;

	override ResourceName GetPrefab() 
	{ 
		return m_Prefab;
	}
	override vector GetPosition() 
	{ 
		return m_SpawnPoint.GetOrigin();
	} 
	override vector GetAngles() 
	{ 
		return m_SpawnPoint.GetAngles();
	}
	
	RplId GetRplId()
	{ 
		return m_RplId;
	}
	
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}
	
	void SCR_SpawnPointSpawnData(ResourceName prefab, RplId rplId)
	{
		m_Prefab = prefab;
		m_RplId = rplId;
		m_SpawnPoint = SCR_SpawnPoint.GetSpawnPointByRplId(rplId);
	}
};
