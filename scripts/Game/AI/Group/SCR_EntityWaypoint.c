class SCR_EntityWaypointClass: SCR_AIWaypointClass
{
};

[BaseContainerProps()]
class SCR_AIEntityWaypointParameters
{
	[Attribute("", UIWidgets.EditBox, "Related entity")]
	private string	m_sEntityName;
	private IEntity m_Entity;
	
	//------------------------------------------------------------------------------------------------
	string GetEntityName()
	{
		return m_sEntityName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntityName(string entityName)
	{
		 m_sEntityName = entityName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntity()
	{
		if (m_Entity)
		{
			return m_Entity;
		}
		
		SetEntity(GetGame().GetWorld().FindEntityByName(m_sEntityName));
		return m_Entity;
	}
};

class SCR_EntityWaypoint : SCR_AIWaypoint
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