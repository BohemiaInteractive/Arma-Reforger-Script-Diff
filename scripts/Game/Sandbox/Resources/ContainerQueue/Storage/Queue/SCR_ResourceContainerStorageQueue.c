[BaseContainerProps()]
class SCR_ResourceContainerStorageQueue<Class ResourceActorType> : Managed
{
	protected int m_iOffsetPosition;
	protected int m_iContainerCount;
	private array<SCR_ResourceContainer> m_aContainers;
	
	//------------------------------------------------------------------------------------------------
	int GetContainerCount()
	{
		return m_iContainerCount;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetOffsetPosition()
	{
		return m_iOffsetPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetUpperOffsetPosition()
	{
		return m_iOffsetPosition + m_iContainerCount;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceContainer GetContainerRelativeAt(int index)
	{
		return m_aContainers[m_iOffsetPosition + index];
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceContainer GetContainerAt(int index)
	{
		if (m_aContainers.IsIndexValid(index))
			return m_aContainers[index];
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return m_iContainerCount == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ShouldContainerChangeCauseUpdate()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers a container into the queue.
	//! \param container The container to be registered into the queue.
	//! \return Returns the position of the container in the queue.
	int RegisterContainer(notnull SCR_ResourceContainer container, notnull ResourceActorType actor)
	{
		return InsertContainerAt(container, ComputePosition(container, actor));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unegisters the first container in the queue.
	void UnregisterFirstContainer()
	{
		if (!m_iContainerCount)
			return;
		
		m_aContainers.RemoveOrdered(m_iOffsetPosition);
		m_iContainerCount--;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unegisters the container at a specific position in the queue.
	void UnregisterContainerAt(int position)
	{
		if (!m_iContainerCount)
			return;
		
		m_aContainers.RemoveOrdered(m_iOffsetPosition + position);
		m_iContainerCount--;
	}
	
	//------------------------------------------------------------------------------------------------
	int ComputePosition(SCR_ResourceContainer container, ResourceActorType actor)
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int InsertContainerAt(SCR_ResourceContainer container, int index)
	{
		if (index == SCR_ResourceContainerQueue.INVALID_CONTAINER_INDEX)
			return SCR_ResourceContainerQueue.INVALID_CONTAINER_INDEX;
		
		int position = m_iOffsetPosition + index;
		
		m_aContainers.InsertAt(container, position);
		m_iContainerCount++;
		
		return position;
	}
	
	//------------------------------------------------------------------------------------------------
	int InsertContainerAtEx(SCR_ResourceContainer container, int index)
	{
		if (index == SCR_ResourceContainerQueue.INVALID_CONTAINER_INDEX)
			return SCR_ResourceContainerQueue.INVALID_CONTAINER_INDEX;
		
		int position = m_iOffsetPosition + index;
		
		m_aContainers.InsertAt(container, position);
		
		return position;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOffsetPosition(int offset)
	{
		m_iOffsetPosition = offset;
	}
	
	//------------------------------------------------------------------------------------------------
	void IncrementOffset(int increment)
	{
		m_iOffsetPosition += increment;
	}
	
	//------------------------------------------------------------------------------------------------
	void DecrementOffset(int decrement)
	{
		m_iOffsetPosition -= decrement;
	}
	
	//------------------------------------------------------------------------------------------------
	void Initialize(array<SCR_ResourceContainer> containers)
	{
		Reset();
		m_aContainers = containers;
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset(int offsetPosition = 0)
	{
		m_iOffsetPosition = offsetPosition;
		m_iContainerCount = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void DebugPrint()
	{
		string content = string.Format("Storage Container Queue of [ %1 ] has: %2 containers", ToString(), m_iContainerCount);
		
		for (int i = 0; i < m_iContainerCount; i++)
		{
			content += string.Format("\n\tContainer %1", GetContainerRelativeAt(i).m_sDebugName);
		}
		
		content += "\n";
		
		Print(string.ToString(content, false, false, false));
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ResourceContainerStorage()
	{
		m_iOffsetPosition = 0;
	}
};