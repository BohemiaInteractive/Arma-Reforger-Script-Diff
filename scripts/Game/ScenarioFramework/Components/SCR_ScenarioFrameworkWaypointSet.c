//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointSet
{
	[Attribute(defvalue: "", desc: "Name of a Layer that contains SlotWaypoints or directly the name of the SlotWaypoint. It will be added into the queue of waypoints for AIs attached to this Slot.")]
	ref array<string> m_aLayerName;

	//------------------------------------------------------------------------------------------------
	void Init (AIGroup group, IEntity entity)
	{

	}
}

//------------------------------------------------------------------------------------------------
//! This will remain here for a while until we switch all the waypoints to the SCR_ScenarioFrameworkWaypointSet
[BaseContainerProps()]
class SCR_WaypointSet
{
	[Attribute("Name")]
	string m_sName;

	[Attribute("Use random order")]
	bool m_bUseRandomOrder;

	[Attribute("Cycle the waypoints")]
	bool m_bCycleWaypoints;
}