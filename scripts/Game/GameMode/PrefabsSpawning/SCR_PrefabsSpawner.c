//! Base class for Prefabs Spawning.
[BaseContainerProps()]
class SCR_PrefabsSpawner
{
	[Attribute("0", UIWidgets.EditComboBox, "Defines which spawner uses this Spawn Point", "", ParamEnumArray.FromEnum(EPrefabSpawnType))]
	protected EPrefabSpawnType m_eType;
	
	[Attribute("0", uiwidget: UIWidgets.Flags, "Game Flags required to spawn contained prefabs.", "", ParamEnumArray.FromEnum(EGameFlags))]
	protected EGameFlags m_eRequiredGameFlags;
	
	[Attribute("50", UIWidgets.Slider, "Percentage of how many prefabs to spawn based on how many SCR_PrefabSpawnPoints(with defined type) exists in Mission.", "0 100")]
    protected int m_iTotalPrefabs;
	
	[Attribute("", UIWidgets.Object, "Add categories of prefabs and/or specific prefabs to spawn.")]
	protected ref array<ref SCR_BasePrefabCategory> m_aPrefabsCategories;

	//! List of all SCR_PrefabSpawnPoints for specific EPrefabSpawnType
	protected ref array<SCR_PrefabSpawnPoint> m_aPrefabSpawnPoints;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EPrefabSpawnType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this to trigger spawn logic for this spawner
	//! \param[in] prefabSpawnPoints
	void Spawn(array<SCR_PrefabSpawnPoint> prefabSpawnPoints)
	{
		if (m_eRequiredGameFlags != 0 && !GetGame().AreGameFlagsSet(m_eRequiredGameFlags))
			return;
		
		if (prefabSpawnPoints == null)
		{
			Debug.Error("Can't spawn items of type: " + m_eType.ToString() + " because no spawn points are available.");
			return;
		}
		
		m_aPrefabSpawnPoints = prefabSpawnPoints;
		
		int numberOfPrefabsToSpawn = m_aPrefabSpawnPoints.Count() * m_iTotalPrefabs * 0.01;
		SpawnPrefabsCategories(numberOfPrefabsToSpawn, m_aPrefabsCategories);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Triggers spawning for Categories of prefabs and/or nested categories of prefabs.
	//! \param[in] numberOfPrefabsToSpawn
	//! \param[in] prefabsCategories
	void SpawnPrefabsCategories(int numberOfPrefabsToSpawn, array<ref SCR_BasePrefabCategory> prefabsCategories)
	{
		int totalRatio = GetTotalRatio(prefabsCategories);
		
		if (totalRatio == 0)
			return;
		
		foreach (SCR_BasePrefabCategory prefabCategory : prefabsCategories)
		{
			prefabCategory.Spawn(numberOfPrefabsToSpawn * prefabCategory.GetRatio() / totalRatio, this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn prefabs on random spawnpoints.
	//! \param[in] totalprefabs
	//! \param[in] prefab
	void SpawnPrefabCategory(int totalprefabs, ResourceName prefab)
	{
		if (totalprefabs <= 0 || m_aPrefabSpawnPoints == null)
			return;
		
		for (int i = 0; i < totalprefabs; i++)
		{
			SCR_PrefabSpawnPoint prefabSpawnPoint = m_aPrefabSpawnPoints.GetRandomElement();
			m_aPrefabSpawnPoints.RemoveItem(prefabSpawnPoint);
			
			vector mat[4];
			prefabSpawnPoint.GetWorldTransform(mat);
			
			if (prefabSpawnPoint.ShouldSnapToGround())
			{
				vector position = mat[3];
				position[1] = prefabSpawnPoint.GetWorld().GetSurfaceY(position[0], position[2]);
				mat[3] = position;
			}
				
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			
			Resource resource = Resource.Load(prefab);
			if (resource.IsValid())
				GetGame().SpawnEntityPrefab(resource, null, spawnParams);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Total Ratio for prefab/s categories on same level of hierarchy.
	//! \param[in] prefabsCategories
	//! \return
	protected int GetTotalRatio(array<ref SCR_BasePrefabCategory> prefabsCategories)
	{
		int totalWeight = 0;
		foreach (SCR_BasePrefabCategory prefabCategory : prefabsCategories)
		{
			totalWeight += prefabCategory.GetRatio();
		}
		
		return totalWeight;
	}
}
