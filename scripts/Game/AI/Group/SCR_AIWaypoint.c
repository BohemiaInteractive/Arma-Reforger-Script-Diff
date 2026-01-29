class SCR_AIWaypointClass: AIWaypointClass
{
};

class SCR_AIWaypoint : AIWaypoint
{
	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
    private float m_fPriorityLevel;
	
	[Attribute(desc: "Settings which are valid while this waypoint is active.")]
	protected ref array<ref SCR_AISettingBase> m_aSettings;
	
	protected ref ScriptInvoker m_OnWaypointPropertiesChanged;
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIWaypoint(IEntitySource src, IEntity parent)
	{
		foreach (auto s : m_aSettings)
			s.Internal_ConstructedAtProperty(SCR_EAISettingOrigin.WAYPOINT, SCR_EAISettingFlags.WAYPOINT);
	}

	//------------------------------------------------------------------------------------------------
	float GetPriorityLevel()
	{
		return m_fPriorityLevel;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPriorityLevel(float priority)
	{
		m_fPriorityLevel = priority;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsWithinCompletionRadius(vector pos)
	{
		return vector.DistanceXZ(GetOrigin(), pos) < GetCompletionRadius();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnWaypointPropertiesChanged()
	{
		if (!m_OnWaypointPropertiesChanged)
			m_OnWaypointPropertiesChanged = new ScriptInvoker();
		return m_OnWaypointPropertiesChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTransformResetImpl(TransformResetParams params)
	{
		if (m_OnWaypointPropertiesChanged)
			m_OnWaypointPropertiesChanged.Invoke();
		super.OnTransformResetImpl(params);
	}
	
	//------------------------------------------------------------------------------------------------
	// Override this to create a custom waypoint state object.
	SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_AIWaypointState(groupUtilityComp, this);
	}
	
	
	//----------------------------------------------------------------------------------------------------------------
	// Operations with settings
	
	//------------------------------------------------------------------------------------------------
	//! Adds setting. It is applied only while group has this waypoint as current one.
	//! Settings must be added to waypoint before waypoint is added to the group.
	//! Settings added to waypoint while it is current for some group, will not be added to that group.
	void AddSetting(notnull SCR_AISettingBase s)
	{
		if (m_aSettings.Contains(s))
			return;
		
		s.Internal_SetFlag(SCR_EAISettingFlags.WAYPOINT);
		m_aSettings.Insert(s);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetSettings(notnull array<SCR_AISettingBase> outSettings)
	{
		outSettings.Clear();
		
		foreach (auto s : m_aSettings)
			outSettings.Insert(s);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveSetting(notnull SCR_AISettingBase s)
	{
		m_aSettings.RemoveItem(s);
	}
};