class SCR_BoardingTimedWaypointClass: SCR_BoardingWaypointClass
{
};


class SCR_BoardingTimedWaypoint : SCR_BoardingWaypoint
{
	[Attribute("", UIWidgets.Object, "Waypoint parameters")]
	ref SCR_AITimedWaypointParameters m_TimedWaypointParameters;
	
	//------------------------------------------------------------------------------------------------
	float GetHoldingTime()
	{
		if (m_TimedWaypointParameters)
			return m_TimedWaypointParameters.GetParameters();
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHoldingTime(float holdingTime)
	{
		if (m_TimedWaypointParameters)
			m_TimedWaypointParameters.SetParameters(holdingTime);
	}
};

