class SCR_EntitySpawnerCharacterSlotComponentClass : SCR_EntitySpawnerSlotComponentClass
{
}

//! Specifies slot to be used with nearby Entity Spawner Components
class SCR_EntitySpawnerCharacterSlotComponent : SCR_EntitySpawnerSlotComponent
{
	[Attribute("{FFF9518F73279473}PrefabsEditable/Auto/AI/Waypoints/E_AIWaypoint_Move.et", UIWidgets.ResourceNamePicker, "Defend waypoint prefab", "et")]
	protected ResourceName m_sDefaultWaypointPrefab;
	
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, params: "inf inf purpose=coords space=entity")]
	protected vector m_vDefaultWaypointPosition;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetDefaultWaypointPrefab()
	{
		return m_sDefaultWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetDefaultWaypointPosition()
	{
		return m_vDefaultWaypointPosition;
	}
}
