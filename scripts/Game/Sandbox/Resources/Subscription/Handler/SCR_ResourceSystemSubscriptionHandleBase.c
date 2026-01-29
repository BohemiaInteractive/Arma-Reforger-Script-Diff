class SCR_ResourceSystemSubscriptionHandleBase
{
	protected RplId m_OwnerRplId;
	protected RplId m_ResourceComponentRplId;
	protected typename m_InteractorType;
	protected EResourceType m_eResourceType;
	protected EResourceGeneratorID m_eResourceIdentifier;
	protected SCR_ResourceSystemSubscriptionManager m_SubscriptionManager;
	protected WorldTimestamp m_fLastPokedAt;
	
	//------------------------------------------------------------------------------------------------
	static SCR_ResourceSystemSubscriptionHandleBase CreateHandle(notnull SCR_ResourceSystemSubscriptionManager manager, RplId ownerRplId, RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		Managed owner = Replication.FindItem(ownerRplId);
		
		if (!owner)
			return null;
		
		SCR_ResourceSystemSubscriptionHandleBase handle = manager.GetHandle(resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (handle)
			return handle;	
		
		string stringType = string.Format("SCR_ResourceSystemSubscriptionHandle<%1>", owner.Type());
		typename type = stringType.ToType();
		handle = SCR_ResourceSystemSubscriptionHandleBase.Cast(type.Spawn());
		
		if (!handle)
			return null;
		
		handle.OnInit(manager, ownerRplId, resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		return handle;
	}
	
	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetLastPokedAt()
	{
		return m_fLastPokedAt;
	}
	
	//------------------------------------------------------------------------------------------------
	RplId GetOwnerRplId()
	{
		return m_OwnerRplId;
	}
	
	//------------------------------------------------------------------------------------------------
	RplId GetResourceComponentRplId()
	{
		return m_ResourceComponentRplId;
	}
	
	//------------------------------------------------------------------------------------------------
	typename GetInteractorType()
	{
		return m_InteractorType;
	}
	
	//------------------------------------------------------------------------------------------------
	EResourceGeneratorID GetResourceIdentifier()
	{
		return m_eResourceIdentifier;
	}
	
	//------------------------------------------------------------------------------------------------
	EResourceType GetResourceType()
	{
		return m_eResourceType;
	}
	
	//------------------------------------------------------------------------------------------------
	void Poke()
	{
		m_fLastPokedAt = GetGame().GetWorld().GetTimestamp();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDelete()
	{
		if (m_SubscriptionManager)
			m_SubscriptionManager.OnSubscriptionListenerHandleDeleted(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInit(notnull SCR_ResourceSystemSubscriptionManager manager, RplId ownerRplId, RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		m_SubscriptionManager		= manager;
		m_OwnerRplId				= ownerRplId;
		m_ResourceComponentRplId	= resourceComponentRplId;
		m_InteractorType			= interactorType;
		m_eResourceType				= resourceType;
		m_eResourceIdentifier		= resourceIdentifier;
	}
	
	//------------------------------------------------------------------------------------------------
	sealed void SCR_ResourceSystemSubscriptionHandleBase(SCR_ResourceSystemSubscriptionManager manager, RplId ownerRplId, RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		if (manager)
			OnInit(manager, ownerRplId, resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
	}
	
	//------------------------------------------------------------------------------------------------
	sealed void ~SCR_ResourceSystemSubscriptionHandleBase()
	{
		OnDelete();
	}
}
