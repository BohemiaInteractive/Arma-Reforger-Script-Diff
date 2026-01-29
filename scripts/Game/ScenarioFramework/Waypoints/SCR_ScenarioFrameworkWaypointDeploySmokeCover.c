//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointDeploySmokeCover : SCR_ScenarioFrameworkWaypointScripted
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in m")]
	float m_fCompletionRadius;

	[Attribute(defvalue: "{CE97215CE55CF734}Prefabs/AI/Waypoints/AIWaypoint_DeploySmokeCover.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: EAIWaypointCompletionType.All.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", "", ParamEnumArray.FromEnum(EAIWaypointCompletionType))]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("2000", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
	float m_fPriorityLevel;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Max number of grenades soldiers can use to deploy cover")]
	int m_iMaxGrenadeCount;
	
	[Attribute("0", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_AIActivitySmokeCoverFeatureProperties))]
	SCR_AIActivitySmokeCoverFeatureProperties m_SmokeCoverProperties;
	
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_DeploySmokeCoverWaypoint waypointDeploySmokeCover = SCR_DeploySmokeCoverWaypoint.Cast(waypointEntity);
		if (!waypointDeploySmokeCover)
			return;

		waypointDeploySmokeCover.SetMaxGrenadeCount(m_iMaxGrenadeCount);
		waypointDeploySmokeCover.SetSmokeCoverProperties(m_SmokeCoverProperties);
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