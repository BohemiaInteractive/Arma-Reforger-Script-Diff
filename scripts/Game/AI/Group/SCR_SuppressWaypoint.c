class SCR_SuppressWaypointClass : SCR_TimedWaypointClass
{
}

class SCR_SuppressWaypoint : SCR_TimedWaypoint
{
	[Attribute("1", UIWidgets.EditBox, desc: "Height of suppression volume above waypoint center")]
	protected float m_fSuppressionHeight;
	
	float GetSuppressionHeight()
	{
		return m_fSuppressionHeight;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] height of suppression volume above waypoint center
	void SetSuppressionHeight(float height)
	{
		m_fSuppressionHeight = height;
	}
	
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_SuppressWaypointState(groupUtilityComp, this);
	}
}

class SCR_SuppressWaypointState : SCR_AIWaypointState
{
	protected SCR_AISuppressActivity m_SuppressActivity;
	
	//--------------------------------
	override void OnSelected()
	{
		super.OnSelected();
		
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(m_Waypoint);
		if (wp)
		{
			wp.GetOnWaypointPropertiesChanged().Remove(OnWaypointPropertiesChanged);
			wp.GetOnWaypointPropertiesChanged().Insert(OnWaypointPropertiesChanged);
		}
		
		TryCancelSuppressActivity();
		AddSuppressActivity();
	}
	
	//--------------------------------	
	override void OnDeselected()
	{
		super.OnDeselected();
		
		TryCancelSuppressActivity();
	}
	
	//--------------------------------
	void OnWaypointPropertiesChanged()
	{
		TryCancelSuppressActivity();
		AddSuppressActivity();
	}
	
	//--------------------------------	
	protected void AddSuppressActivity()
	{
		float suppressHeight = 1.0;
		SCR_SuppressWaypoint suppressWp = SCR_SuppressWaypoint.Cast(m_Waypoint);
		if (suppressWp)
			suppressHeight = suppressWp.GetSuppressionHeight();
		
		SCR_AISuppressionVolumeWaypoint volume = new SCR_AISuppressionVolumeWaypoint(vector.Zero, vector.Zero);
		volume.SetWaypoint(m_Waypoint, suppressHeight);
		
		float priorityLevel = 0;
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(m_Waypoint);
		if (wp)
			priorityLevel = wp.GetPriorityLevel();
		
		SCR_AISuppressActivity activity = new SCR_AISuppressActivity(m_Utility, m_Waypoint, volume, priorityLevel: priorityLevel);
		m_Utility.AddAction(activity);
		
		m_SuppressActivity = activity;
	}
	
	//--------------------------------
	protected void TryCancelSuppressActivity()
	{
		if (!m_SuppressActivity)
			return;
		
		if (m_SuppressActivity.GetActionState() == EAIActionState.FAILED)
			return;
		
		m_SuppressActivity.SetFailReason(EAIActionFailReason.CANCELLED);
		m_SuppressActivity.Fail();
	}
}