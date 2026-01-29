class SCR_DeploySmokeCoverWaypointClass : SCR_AIWaypointClass
{
}

class SCR_DeploySmokeCoverWaypoint : SCR_AIWaypoint
{	
	[Attribute("1", UIWidgets.EditBox, desc: "Max number of grenades soldiers can use to deploy cover")]
	protected int m_iMaxGrenadeCount;
	
	[Attribute("0", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_AIActivitySmokeCoverFeatureProperties))]
	protected SCR_AIActivitySmokeCoverFeatureProperties m_SmokeCoverProperties;
	
	//-------------------------------------------------------------------------------------
	int GetMaxGrenadeCount()
	{
		return m_iMaxGrenadeCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] count Max grenade count to be used
	void SetMaxGrenadeCount(float count)
	{
		m_iMaxGrenadeCount = count;
	}
	
	//-------------------------------------------------------------------------------------
	SCR_AIActivitySmokeCoverFeatureProperties GetSmokeCoverProperties()
	{
		return m_SmokeCoverProperties;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] smokeCoverProperties Sets smoke cover properties for AI activity smoke cover waypoint.
	void SetSmokeCoverProperties(SCR_AIActivitySmokeCoverFeatureProperties smokeCoverProperties)
	{
		m_SmokeCoverProperties = smokeCoverProperties;
	}
	
	//-------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_DeploySmokeCoverWaypointState(groupUtilityComp, this);
	}
}

class SCR_DeploySmokeCoverWaypointState : SCR_AIWaypointState
{
	override void OnSelected()
	{
		super.OnSelected();
	
		SCR_DeploySmokeCoverWaypoint wp = SCR_DeploySmokeCoverWaypoint.Cast(m_Waypoint);
		if (wp)
		{
			int maxGrenCount = wp.GetMaxGrenadeCount();
			SCR_AIActivitySmokeCoverFeatureProperties coverProps = wp.GetSmokeCoverProperties();
			
			vector position = m_Waypoint.GetOrigin();
			array<AIAgent> avoidAgents = {};
			array<AIAgent> excludeAgents = {};
			
			SCR_AIActivitySmokeCoverFeature smokeCoverFeature = new SCR_AIActivitySmokeCoverFeature();
			bool result = smokeCoverFeature.Execute(m_Utility, position, coverProps, avoidAgents, excludeAgents, maxGrenCount);
		}	
		
		// Complete waypoint, we do it either way
		if (m_Utility && m_Utility.m_Owner)
			m_Utility.m_Owner.CompleteWaypoint(m_Waypoint);	
	}
}