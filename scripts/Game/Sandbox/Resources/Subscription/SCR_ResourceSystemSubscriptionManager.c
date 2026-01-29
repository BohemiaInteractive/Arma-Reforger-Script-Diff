class SCR_ResourceSystemSubscriptionManager
{
	static const int REPLICATION_MAX_FRAME_BUDGET		= 10;
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
		int higherLimitPosition = m_aListings.Count();
		
		if (higherLimitPosition == 0)
			return null;
		
		int position;
		SCR_ResourceSystemSubscriptionListing listing;
		
		while (position < higherLimitPosition)
		{
			if (GetNextListingCandidate(position, higherLimitPosition, listing, interactor))
				break;
		}
		
		if (listing 
		&&	position == m_aListings.Count()
		&&	listing.GetInteractor() != interactor)
			return null;
		
		return listing;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetNextListingCandidate(inout int position, inout int higherLimitPosition, inout SCR_ResourceSystemSubscriptionListing listing, notnull SCR_ResourceInteractor interactor)
	{
		int comparePosition	= position + ((higherLimitPosition - position) >> 1);
		listing				= m_aListings[comparePosition];
		
		if (!listing)
			return false;
		
		SCR_ResourceInteractor compareInteractor = listing.GetInteractor();
		
		if (interactor > compareInteractor)
			position = comparePosition + 1;
		else if (interactor < compareInteractor)
			higherLimitPosition = comparePosition;
		else 
		{
			listing = m_aListings[comparePosition];
			
			return true;
		}
		
		listing = null;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceSystemSubscriptionHandleBase GetHandle(RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		int higherLimitPosition = m_aHandles.Count();
		
		if (higherLimitPosition == 0)
			return null;
		
		int position;
		SCR_ResourceSystemSubscriptionHandleBase handle;
		
		while (position < higherLimitPosition)
		{
			if (GetNextHandleCandidate(position, higherLimitPosition, handle, resourceComponentRplId, interactorType, resourceType, resourceIdentifier))
				break;
		}
		
		if (handle 
		&&	position == m_aHandles.Count()
		&&	(handle.GetResourceComponentRplId() != resourceComponentRplId
		||	handle.GetInteractorType() != interactorType
		||	handle.GetResourceType() != resourceType
		||	handle.GetResourceIdentifier() != resourceIdentifier))
			return null;
		
		return handle;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetNextHandleCandidate(inout int position, inout int higherLimitPosition, inout SCR_ResourceSystemSubscriptionHandleBase handle, RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		int comparePosition	= position + ((higherLimitPosition - position) >> 1);
		handle				= m_aHandles[comparePosition];
		
		if (!handle)
			return false;
		
		RplId compareResourceComponentRplId = handle.GetResourceComponentRplId();
		typename compareInteractorType = handle.GetInteractorType();
		EResourceType compareResourceType = handle.GetResourceType();
		EResourceGeneratorID compareResourceIdentifier = handle.GetResourceIdentifier();
		
		//! Component rpl id.
		if (resourceComponentRplId > compareResourceComponentRplId)
			position = comparePosition + 1;
		else if (resourceComponentRplId < compareResourceComponentRplId)
			higherLimitPosition = comparePosition;
		
		//! Resource interactor type.
		else if (interactorType.ToString() > compareInteractorType.ToString())
			position = comparePosition + 1;
		else if (interactorType.ToString() < compareInteractorType.ToString())
			higherLimitPosition = comparePosition;
		
		//! Resource type.
		else if (resourceType > compareResourceType)
			position = comparePosition + 1;
		else if (resourceType < compareResourceType)
			higherLimitPosition = comparePosition;
		
		//! Resource identifier type
		else if (resourceIdentifier > compareResourceIdentifier)
			position = comparePosition + 1;
		else if (resourceIdentifier < compareResourceIdentifier)
			higherLimitPosition = comparePosition;
		
		//! Final result (Match was found).
		else 
		{
			handle = m_aHandles[comparePosition];
			
			return true;
		}
		
		handle = null;
		
		return false;
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
		
		int position;
		int comparePosition;
		int maxPosition = m_aListings.Count();
		SCR_ResourceInteractor compareInteractor;
		SCR_ResourceSystemSubscriptionListing compareListing;
		
		while (position < maxPosition)
		{
			comparePosition		= position + ((maxPosition - position) >> 1);
			compareListing		= m_aListings[comparePosition];
			compareInteractor	= compareListing.GetInteractor();
			
			if (interactor > compareInteractor)
				position = comparePosition + 1;
			else if (interactor < compareInteractor)
				maxPosition = comparePosition;
			
			else 
				break;
		}
		
		listing = new SCR_ResourceSystemSubscriptionListing(interactor);
		
		listing.SubscribeListener(listener);
		m_aListings.InsertAt(listing, position);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool UnsubscribeListener(RplId listener, notnull SCR_ResourceInteractor interactor)
	{
		if (!listener.IsValid())
			return false;
		
		SCR_ResourceSystemSubscriptionListing listing = GetListing(interactor);
		
		if (!listing)
			return false;
		
		listing.UnsubscribeListener(listener);
		
		if (listing.IsEmpty())
			m_aListings.RemoveOrdered(m_aListings.Find(listing));
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void UnsubscribeListenerCompletely(RplId listener)
	{
		if (!listener.IsValid())
			return;
		
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
			handle = m_aGracefulHandles[m_iGracefulHandlesPivot++ % m_aGracefulHandles.Count()];
			
			if (currentTime.DiffMilliseconds(handle.GetLastPokedAt()) >= SCR_ResourceSystemSubscriptionManager.GRACEFUL_HANDLES_POKED_THRESHOLD)
			{
				m_aGracefulHandles.RemoveItem(handle);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ReplicateListeners()
	{
		// Clear out null listings.
		m_aListings.RemoveItem(null);
		
		for (int i = 0; i < SCR_ResourceSystemSubscriptionManager.REPLICATION_MAX_FRAME_BUDGET && !m_aListings.IsEmpty(); ++i)
		{
			m_aListings[m_iReplicateListenersPivot++ % m_aListings.Count()].Replicate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceSystemSubscriptionHandleBase RequestSubscriptionListenerHandle(notnull SCR_ResourceInteractor interactor, RplId ownerRplId)
	{
		if (!ownerRplId.IsValid())
			return null;
		
		RplId resourceComponentRplId = Replication.FindId(interactor.GetComponent());
		
		if (!resourceComponentRplId.IsValid())
			return null;
		
		typename interactorType = interactor.Type();
		EResourceType resourceType = interactor.GetResourceType();
		EResourceGeneratorID resourceIdentifier = interactor.GetIdentifier();
		
		SCR_ResourceSystemSubscriptionHandleBase handle = GetHandle(resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (handle)
			return handle;
		
		int position;
		int comparePosition;
		int maxPosition = m_aHandles.Count();
		SCR_ResourceSystemSubscriptionHandleBase compareHandle;
		RplId compareResourceComponentRplId;
		typename compareInteractorType;
		EResourceType compareResourceType;
		EResourceGeneratorID compareResourceIdentifier;
		
		while (position < maxPosition)
		{
			comparePosition					= position + ((maxPosition - position) >> 1);
			compareHandle					= m_aHandles[comparePosition];
			compareResourceComponentRplId	= compareHandle.GetResourceComponentRplId();
			compareInteractorType			= compareHandle.GetInteractorType();
			compareResourceType				= compareHandle.GetResourceType();
			compareResourceIdentifier		= compareHandle.GetResourceIdentifier();
			
			//! Component rpl id.
			if (resourceComponentRplId > compareResourceComponentRplId)
				position = comparePosition + 1;
			else if (resourceComponentRplId < compareResourceComponentRplId)
				maxPosition = comparePosition;
			
			//! Resource interactor type.
			else if (interactorType.ToString() > compareInteractorType.ToString())
				position = comparePosition + 1;
			else if (interactorType.ToString() < compareInteractorType.ToString())
				maxPosition = comparePosition;
			
			//! Resource type.
			else if (resourceType > compareResourceType)
				position = comparePosition + 1;
			else if (resourceType < compareResourceType)
				maxPosition = comparePosition;
			
			//! Resource identifier type
			else if (resourceIdentifier > compareResourceIdentifier)
				position = comparePosition + 1;
			else if (resourceIdentifier < compareResourceIdentifier)
				maxPosition = comparePosition;
			
			else
				break;
		}		
		
		handle = SCR_ResourceSystemSubscriptionHandleBase.CreateHandle(this, ownerRplId, resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (!handle)
			return null;
		
		m_aHandles.InsertAt(handle, position);
		
		return handle;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceSystemSubscriptionHandleBase RequestSubscriptionListenerHandleGraceful(notnull SCR_ResourceInteractor interactor, RplId ownerRplId)
	{
		if (!ownerRplId.IsValid())
			return null;
		
		RplId resourceComponentRplId = Replication.FindId(interactor.GetComponent());
		
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
		
		int position;
		int comparePosition;
		int maxPosition = m_aHandles.Count();
		SCR_ResourceSystemSubscriptionHandleBase compareHandle;
		RplId compareResourceComponentRplId;
		typename compareInteractorType;
		EResourceType compareResourceType;
		EResourceGeneratorID compareResourceIdentifier;
		
		while (position < maxPosition)
		{
			comparePosition					= position + ((maxPosition - position) >> 1);
			compareHandle					= m_aHandles[comparePosition];
			compareResourceComponentRplId	= compareHandle.GetResourceComponentRplId();
			compareInteractorType			= compareHandle.GetInteractorType();
			compareResourceType				= compareHandle.GetResourceType();
			compareResourceIdentifier		= compareHandle.GetResourceIdentifier();
			
			//! Component rpl id.
			if (resourceComponentRplId > compareResourceComponentRplId)
				position = comparePosition + 1;
			else if (resourceComponentRplId < compareResourceComponentRplId)
				maxPosition = comparePosition;
			
			//! Resource interactor type.
			else if (interactorType.ToString() > compareInteractorType.ToString())
				position = comparePosition + 1;
			else if (interactorType.ToString() < compareInteractorType.ToString())
				maxPosition = comparePosition;
			
			//! Resource type.
			else if (resourceType > compareResourceType)
				position = comparePosition + 1;
			else if (resourceType < compareResourceType)
				maxPosition = comparePosition;
			
			//! Resource identifier type
			else if (resourceIdentifier > compareResourceIdentifier)
				position = comparePosition + 1;
			else if (resourceIdentifier < compareResourceIdentifier)
				maxPosition = comparePosition;
			
			else
				break;
		}		
		
		handle = SCR_ResourceSystemSubscriptionHandleBase.CreateHandle(this, ownerRplId, resourceComponentRplId, interactorType, resourceType, resourceIdentifier);
		
		if (!handle)
			return null;
		
		m_aHandles.InsertAt(handle, position);
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