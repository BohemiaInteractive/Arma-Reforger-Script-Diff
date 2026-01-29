/*!
Utility class that aids on budgeting or batching the processing of a specified container
	of CONTAINER_TYPE that holds items of ITEM_TYPE type.
*/
class SCR_ContainerBudgetManager<Class CONTAINER_TYPE, Class ITEM_TYPE>
{
	protected CONTAINER_TYPE m_Container;
	protected int m_iBudgetSize;
	protected int m_iPivot;
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return The container instance.
	*/
	CONTAINER_TYPE GetContainer()
	{
		return m_Container;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets the container instance.
	
	\param container The container instance to set into the manager.
	*/
	void SetContainer(CONTAINER_TYPE container)
	{
		m_Container = container;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Processes the next batch based on the defined budget, and returns a new array with the batched
		items.
	
	\return The newly created batch or null if the container is null.
	*/
	array<ITEM_TYPE> ProcessNextBatch()
	{
		if (!m_Container)
			return null;
		
		ITEM_TYPE item;
		int containerCount = m_Container.Count();
		array<ITEM_TYPE> batch = {};
		
		batch.Reserve(m_iBudgetSize);
		
		if (containerCount == 0)
			return batch;
		
		for (int i = 0; (i < m_iBudgetSize && batch.Count() < m_iBudgetSize); ++i)
		{
			if (m_iPivot >= containerCount)
				m_iPivot = 0;
				
			item = m_Container[m_iPivot++];
			
			batch.Insert(item);
		}
		
		return batch;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ContainerBudgetManager(CONTAINER_TYPE container, int budgetSize)
	{
		m_Container = container;
		m_iBudgetSize = budgetSize;
	}
}