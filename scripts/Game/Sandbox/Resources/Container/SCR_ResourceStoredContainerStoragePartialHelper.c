class SCR_ResourceStoredContainerStoragePartialHelper
{
	protected SCR_ResourceContainer m_ContainerFrom;
	protected SCR_ResourceContainer m_ContainerToCached;
	protected bool m_bHasDetectedContainers;
	protected float m_fMaxStoredResourceValue;
	
	//------------------------------------------------------------------------------------------------
	float GetMaxStoredResourceValue()
	{
		if (m_bHasDetectedContainers)
			return Math.Min(m_fMaxStoredResourceValue, m_ContainerFrom.GetResourceValue());
		
		return -1.0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsSourceValid(notnull IEntity entity)
	{
		if (entity == m_ContainerFrom.GetOwner())
			return false;
		
		SCR_ResourceComponent component = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		
		if (!component)
			return false;
		// TODO MARIO: Fix the whole method for the container query.
		//m_ContainerToCached = component.GetContainer();
		
		if (!m_ContainerToCached)
			return false;
		
		if (!m_bHasDetectedContainers && m_ContainerToCached.IsAllowedToStoreResource())
			m_bHasDetectedContainers = true;
		
		return m_ContainerToCached.CanStoreResource();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ProcessStorage(notnull IEntity entity)
	{
		m_bHasDetectedContainers	= true;
		m_fMaxStoredResourceValue		+= m_ContainerToCached.ComputeResourceDifference();
		
		return m_fMaxStoredResourceValue < m_ContainerFrom.GetResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ResourceStoredContainerStoragePartialHelper(notnull SCR_ResourceContainer container)
	{
		m_ContainerFrom = container;
	}
};