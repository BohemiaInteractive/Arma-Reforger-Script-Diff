class SCR_TimedWaypointClass : SCR_EntityWaypointClass
{
};

[BaseContainerProps()]
class SCR_AITimedWaypointParameters
{
	[Attribute("", UIWidgets.EditBox, "Minimal time to hold the waypoint before it completes")]
	float m_holdingTime;
	
	//------------------------------------------------------------------------------------------------
	float GetParameters()
	{
		return m_holdingTime;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParameters(float holdingTime)
	{
		m_holdingTime = holdingTime;
	}
};

class SCR_TimedWaypoint : SCR_AIWaypoint
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