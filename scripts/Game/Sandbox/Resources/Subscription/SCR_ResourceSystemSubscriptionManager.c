class SCR_ResourceSystemSubscriptionManager
{
	static const int REPLICATION_MAX_FRAME_BUDGET		= 100;
	static const int GRACEFUL_HANDLES_MAX_FRAME_BUDGET	= 10;
	static const int GRACEFUL_HANDLES_POKED_THRESHOLD	= 10000;
	
	protected int m_iGracefulHandlesPivot;
	protected int m_iReplicateListenersPivot;
	
	protected ref array<ref SCR_ResourceSystemSubscriptionListing> m_aListings				= {};
	protected ref array<SCR_ResourceSystemSubscriptionHandleBase> m_aHandles				= {};
	protected ref array<ref SCR_ResourceSystemSubscriptionHandleBase> m_aGracefulHandles	= {};
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceSystemSubscriptionListing GetListing(notnull SCR_ResourceInteractor interactor)
	{
		foreach (SCR_ResourceSystemSubscriptionListing listing : m_aListings)
		{
			if (listing.GetInteractor() == interactor)
				return listing;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceSystemSubscriptionHandleBase GetHandle(RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		foreach (SCR_ResourceSystemSubscriptionHandleBase handle : m_aHandles)
		{
			if (handle.GetResourceComponentRplId() == resourceComponentRplId
			&&	handle.GetInteractorType() == interactorType
			&&	handle.GetResourceType() == resourceType
			&&	handle.GetResourceIdentifier() == resourceIdentifier)
				return handle;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SubscribeListener(RplId listener, notnull SCR_ResourceInteractor interactor)
	{
		if (!listener.IsValid())
			return false;
		
		SCR_ResourceSystemSubscriptionListing listing = GetListing(interactor);
		
		if (listing)
		{
			listing.SubscribeListener(listener);
			
			return true;
		}
		
		listing = new SCR_ResourceSystemSubscriptionListing(interactor);
		
		m_aListings.Insert(listing);
		listing.SubscribeListener(listener);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool UnsubscribeListener(RplId listener, notnull SCR_ResourceInteractor interactor)
	{
		SCR_ResourceSystemSubscriptionListing listing = GetListing(interactor);
		
		if (!listing)
			return false;
		
		listing.UnsubscribeListener(listener);
		
		if (listing.IsEmpty())
			m_aListings.RemoveItem(listing);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void UnsubscribeListenerCompletely(RplId listener)
	{
		SCR_ResourceSystemSubscriptionListing listing;
		
		for (int i = m_aListings.Count() - 1; i >= 0; --i)
		{
			listing = m_aListings[i];
			
			if (!listing)
			{
				m_aListings.RemoveOrdered(i);
				
				continue;
			}
			
			listing.UnsubscribeListener(listener)
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcessGracefulHandles()
	{
		const WorldTimestamp currentTime = GetGame().GetWorld().GetTimestamp();
		SCR_ResourceSystemSubscriptionHandleBase handle;
		
		// Clear out null graceful handles.
		m_aGracefulHandles.RemoveItem(null);
		
		for (int i = 0; i < SCR_ResourceSystemSubscriptionManager.GRACEFUL_HANDLES_MAX_FRAME_BUDGET && !m_aGracefulHandles.IsEmpty(); ++i)
		{
			handle = m_aGracefulHandles[m_iGracefulHandlesPivot];
			
			if (currentTime.DiffMilliseconds(handle.GetLastPokedAt()) >= SCR_ResourceSystemSubscriptionManager.GRACEFUL_HANDLES_POKED_THRESHOLD)
			{
				m_aGracefulHandles.RemoveItem(handle);
				
				--m_iGracefulHandlesPivot;
			}
			
			m_iGracefulHandlesPivot = ++m_iGracefulHandlesPivot % Math.MaxInt(m_aGracefulHandles.Count(), 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ReplicateListeners()
	{
		SCR_ResourceSystemSubscriptionListing listing;
		
		// Clear out null listings.
		m_aListings.RemoveItem(null);
		
		for (int i = 0; i < SCR_ResourceSystemSubscriptionManager.REPLICATION_MAX_FRAME_BUDGET && !m_aListings.IsEmpty(); ++i)
		{
			listing = m_aListings[Math.Repeat(m_iReplicateListenersPivot++, m_aListings.Count())];
			
			if (listing)
				listing.Replicate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceSystemSubscriptionHandleBase RequestSubscriptionListenerHandle(notnull SCR_ResourceInteractor interactor, RplId ownerRplId)
	{
		if (!ownerRplId.IsValid())
			return null;
		
		RplId resourceComponentRplId = Replication.FindItemId(interactor.GetComponent());
		
		if (!resourceComponentRplId.IsValid())
			return null;
		
		typename interactorType = interactor.Type();
		EResourceType resourceType = interactor.GetResourceType();
		EResourceGeneratorID resourceIdentifier = interactor.GetIdentifier();
		
		SCR_ResourceSystemSubscriptionHandleBase handle = GetHandle(resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (handle)
			return handle;	
		
		handle = SCR_ResourceSystemSubscriptionHandleBase.CreateHandle(this, ownerRplId, resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (!handle)
			return null;
		
		m_aHandles.Insert(handle);
		
		return handle;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceSystemSubscriptionHandleBase RequestSubscriptionListenerHandleGraceful(notnull SCR_ResourceInteractor interactor, RplId ownerRplId)
	{
		if (!ownerRplId.IsValid())
			return null;
		
		RplId resourceComponentRplId = Replication.FindItemId(interactor.GetComponent());
		
		if (!resourceComponentRplId.IsValid())
			return null;
		
		typename interactorType = interactor.Type();
		EResourceType resourceType = interactor.GetResourceType();
		EResourceGeneratorID resourceIdentifier = interactor.GetIdentifier();
		
		SCR_ResourceSystemSubscriptionHandleBase handle = GetHandle(resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (handle)
		{
			handle.Poke();
			
			return handle;
		}
		
		handle = SCR_ResourceSystemSubscriptionHandleBase.CreateHandle(this, ownerRplId, resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (!handle)
			return null;
		
		m_aHandles.Insert(handle);
		m_aGracefulHandles.Insert(handle);
		handle.Poke();
		
		return handle;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSubscriptionListenerHandleDeleted(notnull SCR_ResourceSystemSubscriptionHandleBase handle)
	{
		m_aHandles.RemoveOrdered(m_aHandles.Find(handle));
		
		if (m_aGracefulHandles.Contains(handle))
			m_aGracefulHandles.RemoveItem(handle);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnResourceInteractorDeleted(notnull SCR_ResourceInteractor interactor)
	{
		SCR_ResourceSystemSubscriptionListing listing = GetListing(interactor);
		
		if (!listing)
			return;
		
		m_aListings.RemoveItem(listing);
	}
}