//------------------------------------------------------------------------------------------------
class SCR_FreeSpawnData : SCR_SpawnData
{
	protected ResourceName m_Prefab;
	protected vector m_Position;
	protected vector m_Angles;

	override ResourceName GetPrefab()
	{
		return m_Prefab;
	}
	override vector GetPosition()
	{
		return m_Position;
	}
	override vector GetAngles()
	{
		return m_Angles;
	}

	void SCR_FreeSpawnData(ResourceName prefab, vector position, vector angles = "0 0 0")
	{
		m_Prefab = prefab;
		m_Position = position;
		m_Angles = angles;
	}
};
