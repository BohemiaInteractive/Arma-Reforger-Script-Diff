class SCR_ResourceSystemSubscriptionHandle<Class OWNER_TYPE> : SCR_ResourceSystemSubscriptionHandleBase
{
	protected OWNER_TYPE m_Owner;
	
	//------------------------------------------------------------------------------------------------
	OWNER_TYPE GetOwner()
	{
		return m_Owner;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnDelete()
	{
		super.OnDelete();
		
		if (m_Owner)
			m_Owner.RequestUnsubscription(m_ResourceComponentRplId, m_InteractorType, m_eResourceType, m_eResourceIdentifier);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInit(notnull SCR_ResourceSystemSubscriptionManager manager, RplId ownerRplId, RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		super.OnInit(manager, ownerRplId, resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		m_Owner = OWNER_TYPE.Cast(Replication.FindItem(ownerRplId));
		
		if (m_Owner)
			m_Owner.RequestSubscription(m_ResourceComponentRplId, m_InteractorType, m_eResourceType, m_eResourceIdentifier);
	}
}