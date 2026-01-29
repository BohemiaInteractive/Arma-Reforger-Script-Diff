[BaseContainerProps()]
class SCR_ResourceContainerQueueBase : Managed
{
	static const int FIRST_CONTAINER_INDEX		= 0;
	static const int INVALID_CONTAINER_INDEX	= -1;
	static const int INVALID_CONTAINER_COUNT	= -1;
	
	protected SCR_ResourceComponent	m_ResourceComponent;
	protected ref array<SCR_ResourceContainer> m_aRegisteredContainers = new array<SCR_ResourceContainer>();
	protected float	m_fAggregatedResourceValue;
	protected float	m_fAggregatedMaxResourceValue;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the current value of the aggregated resources pre-calculated by adding the current 
		resource values of the containers registered into the container queue.
	
	\return The current value of aggregated resources.
	*/
	float GetAggregatedResourceValue()
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (m_fAggregatedResourceValue < SCR_ResourceActor.RESOURCES_LOWER_LIMIT)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_fAggregatedResourceValue is lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.", "The current aggregated resources can not be lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.");
		
			return SCR_ResourceActor.RESOURCES_LOWER_LIMIT;
		}
		
		return m_fAggregatedResourceValue;
	#else
		return m_fAggregatedResourceValue;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the current maximum value of the aggregated resources pre-calculated by adding the maximum
		resource values of the containers registered into the container queue.
	
	\return The current maximum value of aggregated resources.
	*/
	float GetAggregatedMaxResourceValue()
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (m_fAggregatedMaxResourceValue < SCR_ResourceActor.RESOURCES_LOWER_LIMIT)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_fAggregatedMaxResourceValue is lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.", "The current maximum aggregated resources can not be lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.");
		
			return SCR_ResourceActor.RESOURCES_LOWER_LIMIT;
		}
		
		return m_fAggregatedMaxResourceValue;
	#else
		return m_fAggregatedMaxResourceValue;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the first resource container in the queue.
	
	\warning 
		Note that it is the first one in the queue in terms of order/priority and not the first 
			container to be registered so do not cache this value as a pointer to the first container.
		Note that this method should only be called if and only if the queue is not empty.
	\return The first resource container in the queue.
	*/
	SCR_ResourceContainer GetFirstContainer()
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
			
			return null;
		}
		
		if (m_aRegisteredContainers.IsEmpty())
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is empty.", "The method should not be called if the array of registered containers is empty.");
		
			return null;
		}
		
		SCR_ResourceContainer container = m_aRegisteredContainers[SCR_ResourceContainerQueueBase.FIRST_CONTAINER_INDEX];
			
		if (!container)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::GetFirstContainer found null instead of a resource container.", "There should never be null values in the registered containers array.");
		
			return null;
		}
		
		return container;
	#else
		return m_aRegisteredContainers[SCR_ResourceContainerQueueBase.FIRST_CONTAINER_INDEX];
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Copies the registered resource containers into a set of weak references of resource containers.
	//! 
	//! \param[inout] containers The set to copy into.
	void CopyContainers(inout notnull set<SCR_ResourceContainer> containers)
	{
		foreach (SCR_ResourceContainer container: m_aRegisteredContainers)
		{
			containers.Insert(container);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Copies the registered resource containers into an array of weak references of resource containers.
	//! 
	//! \param[inout] containers The array to copy into.
	void CopyContainers(inout notnull array<SCR_ResourceContainer> containers)
	{
		containers.Copy(m_aRegisteredContainers);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns a resource container located at the provided index/position in the queue.
	
	\warning Note that this method should only be called if and only if the queue has an element at 
		the desired index.
	\param index Index/position of the desired container in the queue,
	\return A resource container located at the provided index/position in the queue.
	*/
	SCR_ResourceContainer GetContainerAt(int index)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
			
			return null;
		}
		
		if (m_aRegisteredContainers.IsEmpty())
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is empty.", "The method should not be called if the array of registered containers is empty.");
		
			return null;
		}
		
		if (!m_aRegisteredContainers.IsIndexValid(index))
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers has no element at the provided index.", "The method should not be called if the array of registered containers does not contain an element at the provided index.");
		
			return null;
		}
		
		SCR_ResourceContainer container = m_aRegisteredContainers[index];
		
		if (!container)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::GetContainerAt found null instead of a resource container.", "There should never be null values in the registered containers array.");
		
			return null;
		}
		
		return container;
	#else
		return m_aRegisteredContainers[index];
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the number of registered containers in the queue.
	
	\return The number of registered containers in the queue.
	*/
	int GetContainerCount()
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
		
			return SCR_ResourceContainerQueueBase.INVALID_CONTAINER_COUNT;
		}
		
		return m_aRegisteredContainers.Count();
	#else
		return m_aRegisteredContainers.Count();
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Registers a container into the queue.
	
	\warning An existence check is not performed so the method should not be called if the container 
		is already registered in the queue.
	\param container The container to be registered into the queue.
	\return The position of the container in the queue.
	*/
	int RegisterContainer(notnull SCR_ResourceContainer container)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
			
			return SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX;
		}
		
		if (m_aRegisteredContainers.Contains(container))
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::RegisterContainer tried to register an already registered resource container.", "A resource container should not be registered more than once.");
			
			return SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX;
		}
		
		return m_aRegisteredContainers.Insert(container);
	#else
		return m_aRegisteredContainers.Insert(container);
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Finds the position of a specified container in the queue and returns said position.
	
	\param container The container to ne found in the queue.
	\return The position of the container in the queue or a representation of a invalid index in the
		queue in case of failure.
	*/
	int FindContainer(notnull SCR_ResourceContainer container)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
		
			return SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX;
		}
		
		return m_aRegisteredContainers.Find(container);
	#else
		return m_aRegisteredContainers.Find(container);
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Removes from the queue the first container and returns it.
	
	\warning Note that this method should only be called if and only if the queue is not empty.
	\return The container that was at the first position in the queue.
	*/
	SCR_ResourceContainer PopFirstContainer()
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
			
			return null;
		}
		
		if (m_aRegisteredContainers.IsEmpty())
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is empty.", "The method should not be called if the array of registered containers is empty.");
		
			return null;
		}
		
		SCR_ResourceContainer container = m_aRegisteredContainers[SCR_ResourceContainerQueueBase.FIRST_CONTAINER_INDEX];
		
		m_aRegisteredContainers.RemoveOrdered(SCR_ResourceContainerQueueBase.FIRST_CONTAINER_INDEX);
		
		return container;
	#else
		SCR_ResourceContainer container = m_aRegisteredContainers[SCR_ResourceContainerQueueBase.FIRST_CONTAINER_INDEX];
		
		m_aRegisteredContainers.RemoveOrdered(SCR_ResourceContainerQueueBase.FIRST_CONTAINER_INDEX);
		
		return container;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Removes from the queue the container at the specified index/position and returns it.
	
	\warning Note that this method should only be called if and only if the queue has an element at 
		the desired index.
	\param index Index/position of the desired container in the queue,
	\return A resource container that was located at the provided index/position in the queue.
	*/
	SCR_ResourceContainer PopContainerAt(int index)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
			
			return null;
		}
		
		if (m_aRegisteredContainers.IsEmpty())
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is empty.", "The method should not be called if the array of registered containers is empty.");
		
			return null;
		}
		
		if (!m_aRegisteredContainers.IsIndexValid(index))
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers has no element at the provided index.", "The method should not be called if the array of registered containers does not contain an element at the provided index.");
		
			return null;
		}
		
		SCR_ResourceContainer container = m_aRegisteredContainers[index];
		
		if (!container)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::PopContainerAt found null instead of a resource container.", "There should never be null values in the registered containers array.");
		
			return null;
		}
		
		m_aRegisteredContainers.RemoveOrdered(index);
		
		return container;
	#else
		if (!m_aRegisteredContainers.IsIndexValid(index))
			return null;
		
		SCR_ResourceContainer container = m_aRegisteredContainers[index];
		
		m_aRegisteredContainers.RemoveOrdered(index);
		
		return container;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Assigns the current value of the aggregated resources to a specific value.
	
	\warning Use with care, assigning an arbitrary value with this could destabilize the entire queue
		 aggregated resource value processing.
	\param value The desired value to be assigned for the current value of the aggregated resources.
	*/
	void SetAggregatedResourceValue(float value)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (value < SCR_ResourceActor.RESOURCES_LOWER_LIMIT)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::SetAggregatedResourceValue The value is lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.", "The current aggregated resources can not be lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.");
		
			return;
		}
		
		if (value > m_fAggregatedMaxResourceValue)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::SetAggregatedResourceValue The value is greater than the current maximum aggregated resources value.", "The current aggregated resources can not be greater than than the current maximum aggregated resources value.");
		
			return;
		}
		
		m_fAggregatedResourceValue = value;
	#else
		m_fAggregatedResourceValue = value;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Assigns the current maximum value of the aggregated resources to a specific value.
	
	\warning Use with care, assigning an arbitrary value with this could destabilize the entire queue
		 maximum aggregated resource value processing.
	\param value The desired value to be assigned for the current maximum value of the aggregated 
		resources.
	*/
	void SetAggregatedMaxResourceValue(float value)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (value < SCR_ResourceActor.RESOURCES_LOWER_LIMIT)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::SetAggregatedMaxResourceValue The value is lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.", "The current maximum aggregated resources can not be lower than SCR_ResourceActor.RESOURCES_LOWER_LIMIT.");
		
			return;
		}
		
		if (value < m_fAggregatedResourceValue)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::SetAggregatedMaxResourceValue The value is lower than the current aggregated resources value.", "The current maximum aggregated resources can not be lower than than the current aggregated resources value.");
		
			return;
		}
		
		m_fAggregatedMaxResourceValue = value;
	#else
		m_fAggregatedMaxResourceValue = value;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Increases the current value of the aggregated resources with a specific increment.
	
	\warning Use with care, increasing to an arbitrary increment with this could destabilize the 
		entire queue aggregated resource value processing.
	\param increment The desired value to be incremented on the current value of the aggregated 
		resources.
	*/
	void IncreaseAggregatedResourceValue(float increment)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (increment < 0)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::IncreaseAggregatedResourceValue The increment is lower than 0.0.", "The increment has to be a positive number.");
		
			return;
		}
		
		SetAggregatedResourceValue(m_fAggregatedResourceValue + increment);
	#else
		m_fAggregatedResourceValue += increment;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Decreases the current value of the aggregated resources with a specific decrement.
	
	\warning Use with care, decreasing to an arbitrary decrement with this could destabilize the 
		entire queue aggregated resource value processing.
	\param decrement The desired value to be decremented on the current value of the aggregated 
		resources.
	*/
	void DecreaseAggregatedResourceValue(float decrement)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (decrement < 0)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::DecreaseAggregatedResourceValue The decrement is lower than 0.0.", "The decrement has to be a positive number.");
		
			return;
		}
		
		SetAggregatedResourceValue(m_fAggregatedResourceValue - decrement);
	#else
		m_fAggregatedResourceValue -= decrement;
	#endif

	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Increases the current maximum value of the aggregated resources with a specific increment.
	
	\warning Use with care, increasing to an arbitrary increment with this could destabilize the 
		entire queue maximum aggregated resource value processing.
	\param increment The desired value to be incremented on the current maximum value of the 
		aggregated resources.
	*/
	void IncreaseAggregatedMaxResourceValue(float increment)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (increment < 0)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::IncreaseAggregatedMaxResourceValue The increment is lower than 0.0.", "The increment has to be a positive number.");
		
			return;
		}
		
		SetAggregatedMaxResourceValue(m_fAggregatedResourceValue + increment);
	#else
		m_fAggregatedMaxResourceValue += increment;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Decreases the current maximum value of the aggregated resources with a specific decrement.
	
	\warning Use with care, decreasing to an arbitrary decrement with this could destabilize the 
		entire queue maximum aggregated resource value processing.
	\param decrement The desired value to be decremented on the current maximum value of the 
		aggregated resources.
	*/
	void DecreaseAggregatedMaxResourceValue(float decrement)
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		if (decrement < 0)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::DecreaseAggregatedMaxResourceValue The decrement is lower than 0.0.", "The decrement has to be a positive number.");
		
			return;
		}
		
		SetAggregatedMaxResourceValue(m_fAggregatedResourceValue - decrement);
	#else
		m_fAggregatedMaxResourceValue -= decrement;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Clears and resets the queue.
	*/
	array<SCR_ResourceContainer> Clear()
	{
	#ifdef SANDBOX_RESOURCES_DEBUG
		array<SCR_ResourceContainer> clearedContainers = {};
		
		if (!m_aRegisteredContainers)
		{
			Debug.Error2("SCR_ResourceContainerQueueBase::m_aRegisteredContainers is null.", "The array of registered containers has to be present.");
			
			return clearedContainers;
		}
		
		for (int index = m_aRegisteredContainers.Count() - 1; index >= 0; --index)
		{
			if (!m_aRegisteredContainers[index])
			{
				Debug.Error2("SCR_ResourceContainerQueueBase::Clear found null instead of a resource container.", "There should never be null values in the registered containers array.");
			
				return clearedContainers;
			}
			
			//! TODO: Optimize this.
			clearedContainers.Insert(PopContainerAt(index));
		}
		
		m_aRegisteredContainers.Clear();
		SetAggregatedResourceValue(SCR_ResourceActor.RESOURCES_LOWER_LIMIT);
		SetAggregatedMaxResourceValue(SCR_ResourceActor.RESOURCES_LOWER_LIMIT);
		
		return clearedContainers;
	#else
		array<SCR_ResourceContainer> clearedContainers = {};
		
		for (int index = m_aRegisteredContainers.Count() - 1; index >= 0; --index)
		{
			//! TODO: Optimize this.
			clearedContainers.Insert(PopContainerAt(index));
		}
		
		m_aRegisteredContainers.Clear();
		
		m_fAggregatedResourceValue		= SCR_ResourceActor.RESOURCES_LOWER_LIMIT;
		m_fAggregatedMaxResourceValue	= SCR_ResourceActor.RESOURCES_LOWER_LIMIT;
		
		return clearedContainers;
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void PerformSorting();
	
	//------------------------------------------------------------------------------------------------
	void Initialize(notnull SCR_ResourceInteractor interactor)
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
	void DebugDraw()
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
	float UpdateContainerResourceValue(float currentValue, float previousValue)
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
	float UpdateContainerMaxResourceValue(float currentValue, float previousValue)
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_ResourceContainerQueueBase()
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Destructor
	void ~SCR_ResourceContainerQueueBase()
	{
		Clear();
	}
}

[BaseContainerProps()]
class SCR_ResourceContainerQueue<Class ResourceInteractorType> : SCR_ResourceContainerQueueBase
{
	//! The interactor that owns the queue.
	protected ResourceInteractorType m_Interactor;
	
	[Attribute(uiwidget: UIWidgets.Object)]
	protected ref array<ref SCR_ResourceStoragePolicyBase<ResourceInteractorType>> m_StoragePolicies;
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns the number of storage policies that are registered.
	protected int GetStoragePolicyCount()
	{
		return m_StoragePolicies.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param storageType The container storage type to return number of containers of.
	//! \return Returns the number of containers of a specified storageType.
	int GetStorageTypeCount(EResourceContainerStorageType storageType)
	{
		foreach (SCR_ResourceStoragePolicyBase<ResourceInteractorType> policy : m_StoragePolicies)
		{
			if (policy.IsStorageTypeValid(storageType))
				return policy.GetStorageQueue().GetContainerCount();
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers a container into the queue.
	//! \param container The container to be registered into the queue.
	//! \return Returns the position of the container in the queue.
	override int RegisterContainer(notnull SCR_ResourceContainer container)
	{
		int position = SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX;
		float resourceValue;
		bool shouldIncrementOffset;
		SCR_ResourceContainerStorageQueue<ResourceInteractorType> storageQueue;
		
		EResourceContainerStorageType storageType = container.GetStorageType();
		
		foreach (SCR_ResourceStoragePolicyBase<ResourceInteractorType> policy: m_StoragePolicies)
		{
			storageQueue = policy.GetStorageQueue();
		
			if (shouldIncrementOffset)
			{
				storageQueue.IncrementOffset(1);
				continue;
			}
			
			resourceValue = container.GetResourceValue();
			
			if (!policy.IsStorageTypeValid(storageType))
				continue;
			
			m_fAggregatedResourceValue		+= resourceValue;
			m_fAggregatedMaxResourceValue	+= container.GetMaxResourceValue();
			
			shouldIncrementOffset = true;
			position = storageQueue.RegisterContainer(container, m_Interactor);
		}
		
		return position;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_ResourceContainer PopFirstContainer()
	{
		bool shouldDecrementOffset;
		SCR_ResourceContainerStorageQueue<ResourceInteractorType> storageQueue;
		SCR_ResourceContainer container;
		
		foreach (SCR_ResourceStoragePolicyBase<ResourceInteractorType> policy: m_StoragePolicies)
		{
			storageQueue = policy.GetStorageQueue();
			
			if (storageQueue.IsEmpty())
				continue;
			
			if (shouldDecrementOffset)
			{
				storageQueue.DecrementOffset(1);
				continue;
			}
			
			container = m_aRegisteredContainers[0];
			shouldDecrementOffset = true;
			
			if (container)
			{
				m_fAggregatedResourceValue		-= container.GetResourceValue();
				m_fAggregatedMaxResourceValue	-= container.GetMaxResourceValue();
				
				m_Interactor.OnContainerUnregistered(container);
			}
			
			storageQueue.UnregisterFirstContainer();
		}
		
		return container;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_ResourceContainer PopContainerAt(int index)
	{
		if (!m_aRegisteredContainers.IsIndexValid(index))
			return null;
		
		int offsetPosition;
		bool shouldDecrementOffset;
		SCR_ResourceContainerStorageQueue<ResourceInteractorType> storageQueue;
		SCR_ResourceContainer container;
		
		foreach (SCR_ResourceStoragePolicyBase<ResourceInteractorType> policy: m_StoragePolicies)
		{
			storageQueue = policy.GetStorageQueue();
			
			if (storageQueue.IsEmpty())
				continue;
			
			if (shouldDecrementOffset)
			{
				storageQueue.DecrementOffset(1);
				continue;
			}
			
			offsetPosition = storageQueue.GetOffsetPosition();
			
			if (index < offsetPosition || index > offsetPosition + storageQueue.GetContainerCount())
				continue;
			
			container = m_aRegisteredContainers[index];
			shouldDecrementOffset = true;
			
			storageQueue.UnregisterContainerAt(index - offsetPosition);
			
			if (container)
			{
				m_fAggregatedResourceValue		-= container.GetResourceValue();
				m_fAggregatedMaxResourceValue	-= container.GetMaxResourceValue();
				
				m_Interactor.OnContainerUnregistered(container);
			}
		}
		
		return container;
	}
	
	override void PerformSorting()
	{
		array<SCR_ResourceContainer> registeredContainers = Clear();
		
		foreach (SCR_ResourceContainer container: registeredContainers)
		{
			if (!container)
				continue;
			
			m_Interactor.RegisterContainer(container);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clears the queue.
	override array<SCR_ResourceContainer> Clear()
	{
		array<SCR_ResourceContainer> clearedContainers = {};
		clearedContainers.Copy(m_aRegisteredContainers);
		
		for (int index = m_aRegisteredContainers.Count() - 1; index >= 0; --index)
		{
			m_Interactor.UnregisterContainer(index);
		}
		
		m_aRegisteredContainers.Clear();
		
		m_fAggregatedResourceValue		= SCR_ResourceActor.RESOURCES_LOWER_LIMIT;
		m_fAggregatedMaxResourceValue	= SCR_ResourceActor.RESOURCES_LOWER_LIMIT;
		
		foreach (SCR_ResourceStoragePolicyBase<ResourceInteractorType> policy: m_StoragePolicies)
		{
			policy.ResetStorageQueue();
		}
		
		return clearedContainers;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Initialize(notnull SCR_ResourceInteractor interactor)
	{
		SCR_ResourceContainerStorageQueue<ResourceInteractorType> storageQueue;
		m_Interactor = ResourceInteractorType.Cast(interactor);
		
		foreach (int idx, SCR_ResourceStoragePolicyBase<ResourceInteractorType> policy: m_StoragePolicies)
		{
			storageQueue = policy.GetStorageQueue();
			
			if (!storageQueue)
			{
				// TODO: Do proper default behavior or just block progress perhaps?.
				storageQueue = new SCR_ResourceContainerStorageQueue<ResourceInteractorType>();
				
				policy.SetStorageQueue(storageQueue);	
				
				Print(string.Format("%1:\n\tPolicy at index %2 does not have a defined storage queue.\n\tUsing default storage queue instead.", Type().ToString(), idx), LogLevel.WARNING);
			}
			
			storageQueue.Initialize(m_aRegisteredContainers);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override float UpdateContainerResourceValue(float currentValue, float previousValue)
	{
		float previousAggregatedValue	= m_fAggregatedResourceValue;
		m_fAggregatedResourceValue		+= currentValue - previousValue;
		
		return previousAggregatedValue;
	}
	
	//------------------------------------------------------------------------------------------------
	override float UpdateContainerMaxResourceValue(float currentValue, float previousValue)
	{
		float previousAggregatedMaxValue	= m_fAggregatedMaxResourceValue;
		m_fAggregatedMaxResourceValue		+= currentValue - previousValue;
		
		return previousAggregatedMaxValue;
	}
};