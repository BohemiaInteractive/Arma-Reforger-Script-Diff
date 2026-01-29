class SCR_ResourceGrid : AABGridMap
{
	static const int MAX_FRAME_BUDGET = 20;
	
	protected ref array<SCR_ResourceComponent> m_aFlaggedItems = {};
	protected ref set<SCR_ResourceContainer> m_aQueriedContainers = new set<SCR_ResourceContainer>();
	protected int m_iGridUpdateId = int.MIN;
	protected int m_iFrameBudget;
	
	//------------------------------------------------------------------------------------------------
	//! \return The latest update ID of the grid.
	int GetGridUpdateId()
	{
		return m_iGridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The current frame budget remaining for the amount of containers to process in the
	//! iteration.
	int GetFrameBudget()
	{
		return m_iFrameBudget;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetFrameBudget()
	{
		m_iFrameBudget = SCR_ResourceGrid.MAX_FRAME_BUDGET;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increases the update ID of the grid by 1.
	//! 
	//! \return The latest update ID of the grid.
	int IncreaseGridUpdateId()
	{
		if (++m_iGridUpdateId == int.MAX)
			Debug.Error2(ClassName(), "Maximum grid update id has been reached.");
		
		return m_iGridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Flags a resource component (Considered items in this context) to be registered into the grid.
	//! 
	//! \param[in] item The resource component to flag for registration.
	void FlagResourceItem(notnull SCR_ResourceComponent item)
	{
		m_aFlaggedItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unflags a resource component (Considered items in this context) from being registered into the grid.
	//! 
	//! \param[in] item The resource component to unflag from registration.
	void UnflagResourceItem(notnull SCR_ResourceComponent item)
	{
		m_aFlaggedItems.RemoveItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers all resource components that are flagged into the grid. 
	void ProcessFlaggedItems()
	{
		if (m_aFlaggedItems.IsEmpty())
			return;
		
		IncreaseGridUpdateId();
		
		// Clear out null flagged items.
		m_aFlaggedItems.RemoveItem(null);
		
		foreach (SCR_ResourceComponent item: m_aFlaggedItems)
		{
			ProcessResourceItem(item);
		}
		
		m_aFlaggedItems.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates the state of a resource component (Considered items in this context) in the grid.
	//! 
	//! \param[in] item The resource component to update.
	void UpdateResourceItem(notnull SCR_ResourceComponent item)
	{
		const IEntity owner = item.GetOwner();
		
		if (!owner)
			return;
		
		vector boundsMins, boundsMaxs;
		
		item.SetGridUpdateId(m_iGridUpdateId);
		item.GetGridContainersBoundingBox(boundsMins, boundsMaxs);
		UpdatePosition(owner, owner.GetOrigin());
		UpdateAABB(owner, boundsMins, boundsMaxs);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers a resource component (Considered items in this context) in the grid.
	//! 
	//! \param[in] item The resource component to register.
	protected bool ProcessResourceItem(notnull SCR_ResourceComponent item)
	{
		const IEntity owner = item.GetOwner();
		
		if (!owner)
			return false;
		
		vector boundsMins, boundsMaxs;
		const Physics phys = owner.GetPhysics();
		
		item.GetGridContainersBoundingBox(boundsMins, boundsMaxs);
		
		if (phys)
			Insert(owner, boundsMins, boundsMaxs, phys.IsDynamic());
		else
			Insert(owner, boundsMins, boundsMaxs, false);
		
		item.SetGridUpdateId(m_iGridUpdateId);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregisters a resource component (Considered items in this context) from the grid.
	//! 
	//! \param[in] item The resource component to unregister.
	void UnregisterResourceItem(notnull SCR_ResourceComponent item)
	{
		item.SetGridUpdateId(int.MIN);
		
		const IEntity owner = item.GetOwner();
		
		if (!owner)
			return;
		
		Remove(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Attempts to update the state of a resource interactor, basically will try to perform a query
	//! on the grid for containers it can reach with it's detection radius and then perform a check
	//! to see if it can be registered as valid container for the interactor. It will also recheck the
	//! current queue of containers to see if anything should be kept.
	//! 
	//! \param[in] interactor The resource interactor to update.
	//! \param[in] useFrameBudget If the frame budgeting should be used.
	void UpdateInteractor(notnull SCR_ResourceInteractor interactor, bool useFrameBudget = false)
	{	
		// This check is to prevent nested calls of UpdateInteractor function
		if (!m_aQueriedContainers.IsEmpty())
			return;
		
		const vector interactorOrigin = interactor.GetOwnerOrigin();
		
		if	(vector.DistanceSq(interactorOrigin, interactor.GetLastPosition()) <= SCR_ResourceComponent.UPDATE_DISTANCE_TRESHOLD_SQUARE 
		&&	interactor.GetGridUpdateId() == m_iGridUpdateId 
		||	interactor.IsIsolated())
			return;
		
		array<IEntity> foundEntities = {};
		const float resourceGridRange = interactor.GetResourceGridRange();
		SCR_ResourceContainer resourceContainer;
		const SCR_ResourceContainerQueueBase containerQueue = interactor.GetContainerQueue();
		
		FindEntitiesInRange(foundEntities, interactorOrigin, resourceGridRange);
		containerQueue.CopyContainers(m_aQueriedContainers);
		
		foreach (IEntity entity : foundEntities)
		{
			m_aQueriedContainers.Insert(SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent)).GetContainer(interactor.GetResourceType()));
		}
		
		m_aQueriedContainers.RemoveItem(null);
		
		const int containerCount = m_aQueriedContainers.Count();
		
		if (useFrameBudget)
			m_iFrameBudget -= containerCount;
		
		for (int idx = containerCount - 1; idx >= 0; --idx)
		{
			resourceContainer = m_aQueriedContainers[idx];
			
			if (resourceContainer.IsIsolated())
				continue;
			
			if (interactor.CanInteractWith(resourceContainer) && resourceContainer.IsInRange(interactorOrigin, resourceGridRange))
			{
				if (!resourceContainer.IsInteractorLinked(interactor))
					interactor.RegisterContainerForced(resourceContainer);
			}
			else
				interactor.UnregisterContainer(resourceContainer);
		}
		
		interactor.OnResourceGridUpdated(this);
		m_aQueriedContainers.Clear();
	}
}