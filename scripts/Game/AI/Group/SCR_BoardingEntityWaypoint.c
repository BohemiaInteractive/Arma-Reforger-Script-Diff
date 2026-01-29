class SCR_BoardingEntityWaypointClass: SCR_BoardingWaypointClass
{
};


class SCR_BoardingEntityWaypoint : SCR_BoardingWaypoint
{
	[Attribute("", UIWidgets.Object, "Related entity")]
	ref SCR_AIEntityWaypointParameters m_EntityWaypointParameters;
	
	//------------------------------------------------------------------------------------------------
	string GetEntityName()
	{
		if (m_EntityWaypointParameters)
			return m_EntityWaypointParameters.GetEntityName();
		return "";
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntityName(string entityName)
	{
		if (m_EntityWaypointParameters)
			return m_EntityWaypointParameters.SetEntityName(entityName);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntity(IEntity entity)
	{
		if (m_EntityWaypointParameters)
			return m_EntityWaypointParameters.SetEntity(entity);
	}	
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntity()
	{
		if (m_EntityWaypointParameters)
		{
			return m_EntityWaypointParameters.GetEntity();
		}
		return null;
	}
};

