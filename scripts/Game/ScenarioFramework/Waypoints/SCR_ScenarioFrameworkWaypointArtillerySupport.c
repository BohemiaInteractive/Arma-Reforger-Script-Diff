//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointArtillerySupport : SCR_ScenarioFrameworkWaypointScripted
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in m", params: "0 inf")]
	float m_fCompletionRadius;

	[Attribute(defvalue: "{C524700A27CFECDD}Prefabs/AI/Waypoints/AIWaypoint_ArtillerySupport.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: EAIWaypointCompletionType.All.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", enumType: EAIWaypointCompletionType)]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("0", UIWidgets.Slider, "Waypoint priority level", "0 2000 100")]
	float m_fPriorityLevel;

	[Attribute("0", "Fast init - units will be spawned on their defensive locations")]
	bool m_bFastInit;
	
	[Attribute("-1", UIWidgets.EditBox, desc: "Total amount of shots which must be fired at the target. -1 means infinite firing.", params: "-1 inf")]
	int m_iTargetShotCount;
	
	[Attribute(typename.EnumToString(SCR_EAIArtilleryAmmoType, SCR_EAIArtilleryAmmoType.HIGH_EXPLOSIVE), UIWidgets.ComboBox, desc: "Ammo type", enumType: SCR_EAIArtilleryAmmoType)]
	protected SCR_EAIArtilleryAmmoType m_eAmmoType;
	
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_AIWaypointArtillerySupport waypointArtillerySupport = SCR_AIWaypointArtillerySupport.Cast(waypointEntity);
		if (!waypointArtillerySupport)
			return;

		waypointArtillerySupport.SetTargetShotCount(m_iTargetShotCount);
		waypointArtillerySupport.SetAmmoType(m_eAmmoType);
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointCompletionRadius(float radius)
	{
		m_fCompletionRadius = radius;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWaypointCompletionRadius()
	{
		return m_fCompletionRadius;
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointCompletionType(EAIWaypointCompletionType type)
	{
		m_eAIWaypointCompletionType = type;
	}

	//------------------------------------------------------------------------------------------------
	override EAIWaypointCompletionType GetWaypointCompletionType()
	{
		return m_eAIWaypointCompletionType;
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointPriorityLevel(float priority)
	{
		m_fPriorityLevel = priority;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWaypointPriorityLevel()
	{
		return m_fPriorityLevel;
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointPrefab(ResourceName prefab)
	{
		m_sWaypointPrefab = prefab;
	}

	//------------------------------------------------------------------------------------------------
	override ResourceName GetWaypointPrefab()
	{
		return m_sWaypointPrefab;
	}
}