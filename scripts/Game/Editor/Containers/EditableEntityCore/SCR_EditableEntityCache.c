class SCR_EditableEntityCache
{
	protected ResourceName m_sPrefabResource;
	protected ref set<int> m_aExtendedPrefabIDs = new set<int>();

	ResourceName GetPrefab()
	{
		return m_sPrefabResource;
	}

	void SetPrefab(ResourceName prefabResource)
	{
		m_sPrefabResource = prefabResource;
	}

	int GetExtendedEntities(notnull out set<int> prefabIDs)
	{
		prefabIDs.Clear();

		foreach (int prefabID : m_aExtendedPrefabIDs)
		{
			prefabIDs.Insert(prefabID);
		}

		prefabIDs.Compact();
		return prefabIDs.Count();
	}

	void SetExtendedEntities(notnull set<int> prefabIDs)
	{
		m_aExtendedPrefabIDs.Clear();

		foreach (int prefabID : prefabIDs)
		{
			m_aExtendedPrefabIDs.Insert(prefabID);
		}

		m_aExtendedPrefabIDs.Compact();
	}
}
