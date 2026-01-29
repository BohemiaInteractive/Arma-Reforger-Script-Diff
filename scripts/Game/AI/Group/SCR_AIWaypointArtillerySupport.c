class SCR_AIWaypointArtillerySupportClass: SCR_TimedWaypointClass
{
};

class SCR_AIWaypointArtillerySupport : SCR_AIWaypoint
{
	[Attribute("-1", UIWidgets.EditBox, desc: "Total amount of shots which must be fired at the target. -1 means infinite firing.")]
	protected int m_iTargetShotCount;
	
	[Attribute(typename.EnumToString(SCR_EAIArtilleryAmmoType, SCR_EAIArtilleryAmmoType.HIGH_EXPLOSIVE), UIWidgets.ComboBox, desc: "Ammo type", enumType: SCR_EAIArtilleryAmmoType)]
	protected SCR_EAIArtilleryAmmoType m_eAmmoType;
	
	[Attribute("1", UIWidgets.CheckBox, desc: "When true, the AI group will start firing at given position instantly when the waypoint is selected.")];
	protected bool m_bActive;
	
	//----------------------------------------------------------------------------------------
	void SetTargetShotCount(int value, bool invokeEvent = true)
	{
		m_iTargetShotCount = value;
		
		if (m_OnWaypointPropertiesChanged && invokeEvent)
			m_OnWaypointPropertiesChanged.Invoke();
	}
	
	//----------------------------------------------------------------------------------------
	void SetActive(bool value, bool invokeEvent = true)
	{
		m_bActive = value;
		
		if (m_OnWaypointPropertiesChanged && invokeEvent)
			m_OnWaypointPropertiesChanged.Invoke();
	}
	
	//----------------------------------------------------------------------------------------
	void SetAmmoType(SCR_EAIArtilleryAmmoType value, bool invokeEvent = true)
	{
		m_eAmmoType = value;
		
		if (m_OnWaypointPropertiesChanged && invokeEvent)
			m_OnWaypointPropertiesChanged.Invoke();
	}
	
	//----------------------------------------------------------------------------------------
	int GetTargetShotCount()
	{
		return m_iTargetShotCount;
	}
	
	//----------------------------------------------------------------------------------------
	bool IsActive()
	{
		return m_bActive;
	}
	
	//----------------------------------------------------------------------------------------
	SCR_EAIArtilleryAmmoType GetAmmoType()
	{
		return m_eAmmoType;
	}
	
	//----------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		SCR_AIWaypointArtillerySupportState wpState = new SCR_AIWaypointArtillerySupportState(groupUtilityComp, this);
		return wpState;
	}

};

class SCR_AIWaypointArtillerySupportState : SCR_AIWaypointState
{
	protected SCR_AIStaticArtilleryActivity m_ArtilleryActivity;
	protected SCR_AIWaypointArtillerySupport m_ArtilleryWaypoint;
	
	//--------------------------------
	void SCR_AIWaypointArtillerySupportState(notnull SCR_AIGroupUtilityComponent utility, SCR_AIWaypoint waypoint)
	{
		m_ArtilleryWaypoint = SCR_AIWaypointArtillerySupport.Cast(waypoint);
	}
	
	//--------------------------------
	override void OnSelected()
	{
		super.OnSelected();
		
		if (m_ArtilleryWaypoint.IsActive())
			AddArtilleryActivity();
		
		m_ArtilleryWaypoint.GetOnWaypointPropertiesChanged().Insert(OnWaypointPropertiesChange);
	}
	
	//--------------------------------
	override void OnDeselected()
	{
		super.OnDeselected();
		
		if (m_ArtilleryActivity)
			m_ArtilleryActivity.Complete();
	}
	
	//--------------------------------
	void OnWaypointPropertiesChange()
	{
		// Cancel previous activity, create a new one
		
		if (m_ArtilleryActivity)
			m_ArtilleryActivity.Complete(); // Complete instead of failing. Because activity fail results in waypoint deselection.
		
		if (m_ArtilleryWaypoint.IsActive())
			AddArtilleryActivity();
	}
	
	//--------------------------------
	protected void AddArtilleryActivity()
	{
		int targetShotCount = m_ArtilleryWaypoint.GetTargetShotCount();
		SCR_EAIArtilleryAmmoType ammoType = m_ArtilleryWaypoint.GetAmmoType();
		SCR_AIStaticArtilleryActivity activity = new SCR_AIStaticArtilleryActivity(m_Utility, m_Waypoint, m_Waypoint.GetOrigin(), ammoType, targetShotCount, priorityLevel: m_ArtilleryWaypoint.GetPriorityLevel());
		m_Utility.AddAction(activity);
		m_ArtilleryActivity = activity;
	}
}