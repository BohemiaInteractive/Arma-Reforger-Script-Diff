//! Base class for Prefab Spawning.
[BaseContainerProps()]
class SCR_BasePrefabCategory
{
	[Attribute("", UIWidgets.EditBox, "Ratio of how many prefabs to spawn in comparison to others prefab categories", "0 100")]
    protected int m_iRatio;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRatio()
	{
		return m_iRatio;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Interface for spawning prefabs, used in SCR_BasePrefabCategory Subclasses.
	//! \param[in] numberOfPrefabsToSpawn
	//! \param[in] prefabsSpawner
	void Spawn(int numberOfPrefabsToSpawn, SCR_PrefabsSpawner prefabsSpawner)
	{
		Debug.Error("SpawnPrefab in SCR_BasePrefabCategory called. This is interface usage only, call it in SCR_BasePrefabCategory sublasses only. ");
	}
}
