//! Prefab to be spawned by SCR_PrefabsSpawnerManager.
[BaseContainerProps()]
class SCR_PrefabCategory : SCR_BasePrefabCategory
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Contained Prefab", "et")]
	ResourceName m_Prefab;
	
	//------------------------------------------------------------------------------------------------
	override void Spawn(int numberOfPrefabsToSpawn, SCR_PrefabsSpawner prefabsSpawner)
	{
		prefabsSpawner.SpawnPrefabCategory(numberOfPrefabsToSpawn, m_Prefab);
	}
}
