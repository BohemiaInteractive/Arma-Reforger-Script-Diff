//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointGetIn : SCR_ScenarioFrameworkWaypointBoardingEntity
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in m")]
	float m_fCompletionRadius;

	[Attribute(defvalue: "{712F4795CF8B91C7}Prefabs/AI/Waypoints/AIWaypoint_GetIn.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: EAIWaypointCompletionType.All.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", "", ParamEnumArray.FromEnum(EAIWaypointCompletionType))]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
	float m_fPriorityLevel;
	
	[Attribute("1", UIWidgets.CheckBox, "Occupy driver")]
	bool m_bDriverAllowed;

	[Attribute("1", UIWidgets.CheckBox, "Occupy gunner")]
	bool m_bGunnerAllowed;

	[Attribute("1", UIWidgets.CheckBox, "Occupy cargo")]
	bool m_bCargoAllowed;

	//------------------------------------------------------------------------------------------------
	override void SetDriverAllowed(bool enabled)
	{
		m_bDriverAllowed = enabled;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetDriverAllowed()
	{
		return m_bDriverAllowed;
	}

	//------------------------------------------------------------------------------------------------
	override void SetGunnerAllowed(bool enabled)
	{
		m_bGunnerAllowed = enabled;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetGunnerAllowed()
	{
		return m_bGunnerAllowed;
	}

	//------------------------------------------------------------------------------------------------
	override void SetCargoAllowed(bool enabled)
	{
		m_bCargoAllowed = enabled;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetCargoAllowed()
	{
		return m_bCargoAllowed;
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
