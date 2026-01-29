[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sEntityName")]
class SCR_TutorialCoursePrefabInfo
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prefab resource name", "et")]
	ResourceName m_sPrefabResourceName;
	
	[Attribute(desc: "Name to be set to entity", category: "Entity")]
	string m_sEntityName;
	
	[Attribute(desc: "Position to spawn entity at", category: "Entity")]
	string m_sSpawnPosName;
	
	[Attribute(desc: "If true, entity will be despawned after end of the course", category: "Despawn")]
	bool m_bDespawnAfterCourse;
	
	[Attribute(desc: "Respawn entity, even if present in world", category: "Respawn")]
	bool m_bRespawn;
	
	[Attribute(desc: "Entity must be damaged for respawn", category: "Respawn")]
	bool m_bRespawnDamaged;
	
	[Attribute(desc: "Entity must be out of minimum distance from its spawn position to be spawned", category: "Respawn")]
	bool m_bRespawnDistance;
	
	[Attribute(defvalue: "5", desc: "Respawn only if distance from spawn position is bigger than this value", category: "Respawn")]
	float m_fRespawnMinimumDistance;
}