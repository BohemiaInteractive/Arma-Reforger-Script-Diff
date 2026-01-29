//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointAnimation : SCR_ScenarioFrameworkWaypointScripted
{
	[Attribute(defvalue: "{4481F98AAFA79B1C}Prefabs/AI/Waypoints/AIWaypoint_Animation.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: EAIWaypointCompletionType.Any.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", "", ParamEnumArray.FromEnum(EAIWaypointCompletionType))]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Object, "Animation methods")]
	ref SCR_AIAnimation_Base m_AnimationClass;
	
	[Attribute("-1", UIWidgets.SpinBox, "For how long the animation should be performed. -1 means infinit.", params: "-1 inf")]
	float m_fAnimationDuration;
	
	[Attribute("", UIWidgets.Object, "Animation position and angles - use PointInfo")]
	ref PointInfo m_AnimationLocalTransform;
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_AIAnimationWaypoint animationWaypoint = SCR_AIAnimationWaypoint.Cast(waypointEntity);
		if (!animationWaypoint)
			return;
		
		vector vAnimationLocalTransform[4];
		if (m_AnimationLocalTransform)
			m_AnimationLocalTransform.GetLocalTransform(vAnimationLocalTransform);
		
		SCR_AIAnimationScript animationScript = new SCR_AIAnimationScript();
		SCR_AIAnimationWaypointParameters animationParameters = new SCR_AIAnimationWaypointParameters();

		animationParameters.SetParameters(vAnimationLocalTransform, m_fAnimationDuration, m_AnimationClass);		
		animationScript.AddAnimationWaypointParameter(animationParameters, 0);
		animationWaypoint.AddAnimationScript(animationScript);
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