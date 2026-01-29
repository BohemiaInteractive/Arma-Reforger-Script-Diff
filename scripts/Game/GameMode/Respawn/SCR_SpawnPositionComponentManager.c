[EntityEditorProps(insertable: false)]
class SCR_SpawnPositionComponentManagerClass : GenericEntityClass
{
}

class SCR_SpawnPositionComponentManager : GenericEntity
{
	protected static SCR_SpawnPositionComponentManager s_Instance;
	protected ref array<SCR_SpawnPositionComponent> m_aSpawnPositions;

	//------------------------------------------------------------------------------------------------
	//! \param[in] createNew
	//! \return
	static SCR_SpawnPositionComponentManager GetInstance(bool createNew = true)
	{
		if (!s_Instance && createNew)
			s_Instance = SCR_SpawnPositionComponentManager.Cast(GetGame().SpawnEntity(SCR_SpawnPositionComponentManager, GetGame().GetWorld()));

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] positionComp
	void AddSpawnPosition(SCR_SpawnPositionComponent positionComp)
	{
		if (!m_aSpawnPositions)
			m_aSpawnPositions = {};

		if (!m_aSpawnPositions.Contains(positionComp))
			m_aSpawnPositions.Insert(positionComp);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] positionComp
	void DeleteSpawnPosition(SCR_SpawnPositionComponent positionComp)
	{
		if (!m_aSpawnPositions)
			return;

		m_aSpawnPositions.RemoveItem(positionComp);

		if (m_aSpawnPositions.IsEmpty())
			m_aSpawnPositions = null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] center
	//! \param[in] range
	//! \param[out] positions
	//! \return
	int GetSpawnPositionsInRange(vector center, float range, out array<SCR_SpawnPositionComponent> positions)
	{
		if (!m_aSpawnPositions)
			return 0;

		float rangeSq = range * range;
		foreach (SCR_SpawnPositionComponent position : m_aSpawnPositions)
		{
			if (vector.DistanceSq(position.GetOwner().GetOrigin(), center) <= rangeSq)
				positions.Insert(position);
		}

		return positions.Count();
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_SpawnPositionComponentManager()
	{
		if (s_Instance == this)
			s_Instance = null;
	}
}
