class SCR_AdditionalDeployablePart : ScriptAndConfig
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prefab which is going to be required in order to deploy the main entity", params: "et")]
	protected ResourceName m_sPrefab;

	[Attribute(defvalue: "1", desc: "How many instances of this prefab should be spawned", params: "1 inf")]
	protected int m_iNumberOfPrefabs;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetPrefab()
	{
		return m_sPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to spawn additional prefabs
	//! \param[in] authorPlayerId id of the player who initalized this procedure
	//! \param[in] user who initialized this procedure
	//! \param[in] params used for spawning of the prefab
	bool SpawnPrefabs(int authorPlayerId, IEntity user, notnull EntitySpawnParams params)
	{
		Resource resource = Resource.Load(m_sPrefab);
		if (!resource.IsValid())
		{
			Print("[SCR_AdditionalDeployablePart.SpawnPrefabs] => Failed to load the resource from m_sPrefab = " + m_sPrefab, LogLevel.ERROR);
			return false;
		}

		vector spawnedMat[4];
		IEntity spawnedEntity;
		InventoryItemComponent spawnedIIC;
		int numberOfSuccesfullySpawnedEnts;
		for(int i; i < m_iNumberOfPrefabs; i++)
		{
			spawnedEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
			if (!spawnedEntity)
				continue;

			numberOfSuccesfullySpawnedEnts++;
			PostPrefabSpawn(spawnedEntity);
			SCR_MultiPartDeployableItemComponent.SetAuthor(spawnedEntity, authorPlayerId);

			spawnedIIC = InventoryItemComponent.Cast(spawnedEntity.FindComponent(InventoryItemComponent));
			if (spawnedIIC)
				spawnedIIC.PlaceOnGround(user);
		}

		return numberOfSuccesfullySpawnedEnts == m_iNumberOfPrefabs;
	}

	//------------------------------------------------------------------------------------------------
	//! Override if you would like to do something to the entity that was spawned after deployment
	//! \param[in] spawnedEntity
	void PostPrefabSpawn(notnull IEntity spawnedEntity);
}