//! Categories of prefabs and/or nested categories of prefabs.
[BaseContainerProps()]
class SCR_PrefabsCategory : SCR_BasePrefabCategory
{
	[Attribute("", UIWidgets.EditBox, "World Editor display name, has no real usage")]
    protected string m_sName;
	
	[Attribute("", UIWidgets.Object, "Categories of prefabs and/or nested categories of prefabs")]
	protected ref array<ref SCR_BasePrefabCategory> m_aPrefabsCategories;
	
	//------------------------------------------------------------------------------------------------
	override void Spawn(int numberOfPrefabsToSpawn, SCR_PrefabsSpawner prefabsSpawner)
	{
		prefabsSpawner.SpawnPrefabsCategories(numberOfPrefabsToSpawn, m_aPrefabsCategories);
	}
}
