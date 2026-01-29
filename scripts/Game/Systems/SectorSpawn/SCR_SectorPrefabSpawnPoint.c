[EntityEditorProps(style: "cylinder", category: "GameScripted/GameMode/PrefabsSpawn", description: "Prefab Spawn Point Entity", sizeMin: "-0.25 0 -0.25", color: "64 0 64 255")]
class SCR_SectorPrefabSpawnPointClass: SCR_PrefabSpawnPointClass
{
};

class SCR_SectorPrefabSpawnPoint : SCR_PrefabSpawnPoint
{
	[Attribute("{8F21EDDBADC889FE}entities/Survival/Apple.et", UIWidgets.ResourceNamePicker, desc: "Spawned prefab", params: "et")]
	ResourceName m_sLoadoutResource;
};