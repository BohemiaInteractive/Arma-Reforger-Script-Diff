class SCR_ResourceSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetUnique(true);
	}
	
	static const int SUBSCRIBED_INTERACTORS_MAX_FRAME_BUDGET	= 10;
	static const int CONTAINERS_MAX_FRAME_BUDGET				= 10;
	
	protected int m_iSubscribedInteractorsPivot;
	protected int m_iContainersPivot;
	
    protected ref array<SCR_ResourceInteractor> m_aSubscribedInteractors	= {};
    protected ref array<SCR_ResourceContainer> m_aContainers				= {};
	protected SCR_ResourceSystemSubscriptionManager m_ResourceSystemSubscriptionManager;
 	protected ref SCR_ResourceGrid m_ResourceGrid;
	
	//------------------------------------------------------------------------------------------------
	bool IsRegistered(notnull SCR_ResourceContainer container)
	{
		return m_aContainers.Contains(container);
	}
	
	//------------------------------------------------------------------------------------------------
    override protected void OnUpdatePoint(WorldUpdatePointArgs args)
    {
		if (args.GetPoint() != ESystemPoint.FixedFrame)
			return;
		
		// 1. Cleanup and setup.
		m_aContainers.RemoveItem(null);
		m_aSubscribedInteractors.RemoveItem(null);
		m_ResourceSystemSubscriptionManager.ProcessGracefulHandles();
		m_ResourceGrid.ProcessFlaggedItems();
		m_ResourceGrid.ResetFrameBudget();
		
		// 2. Update.
		m_ResourceGrid.Update();
		
		for (int i = 0; i < SCR_ResourceSystem.CONTAINERS_MAX_FRAME_BUDGET && !m_aContainers.IsEmpty(); ++i)
		{
			m_aContainers[m_iContainersPivot++ % m_aContainers.Count()].Update(GetWorld().GetTimestamp());
		}

		
		for (int i = 0; i < SCR_ResourceSystem.SUBSCRIBED_INTERACTORS_MAX_FRAME_BUDGET && !m_aSubscribedInteractors.IsEmpty(); ++i)
		{
			m_ResourceGrid.UpdateInteractor(m_aSubscribedInteractors[m_iSubscribedInteractorsPivot++ % m_aSubscribedInteractors.Count()], true);
			
			if (m_ResourceGrid.GetFrameBudget() <= 0)
				break;
		}
		
		// 3. Replicate.
		m_ResourceSystemSubscriptionManager.ReplicateListeners();
    }
 	
	//------------------------------------------------------------------------------------------------
    override protected void OnDiag(float timeSlice)
    {
        DbgUI.Begin(ClassName());
        DbgUI.Text("Containers: " + m_aContainers.Count());
        DbgUI.Text("Subscribed interactors: " + m_aSubscribedInteractors.Count());
 
        if (DbgUI.Button("Dump active components"))
        {
			foreach (SCR_ResourceContainer container : m_aContainers)
            {
                Print(container.GetOwner(), LogLevel.ERROR);
            }
			
			foreach (SCR_ResourceInteractor interactor : m_aSubscribedInteractors)
            {
                Print(interactor.GetOwner(), LogLevel.ERROR);
            }
        }
 
        DbgUI.End();
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterContainer(notnull SCR_ResourceContainer container)
    {
		const RplComponent rplComponent = container.GetComponent().GetReplicationComponent();
		
        if (rplComponent.Role() != RplRole.Authority || m_aContainers.Contains(container))
	        return;
	
		container.SetLastUpdateTimestamp(GetWorld().GetTimestamp());
		m_aContainers.Insert(container);
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterSubscribedInteractor(notnull SCR_ResourceInteractor interactor)
    {
        if (!m_aSubscribedInteractors.Contains(interactor))
            m_aSubscribedInteractors.Insert(interactor);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterContainer(notnull SCR_ResourceContainer container)
    {
        m_aContainers.RemoveItem(container);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterSubscribedInteractor(notnull SCR_ResourceInteractor interactor)
    {
        m_aSubscribedInteractors.RemoveItem(interactor);
    }
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnStarted()
	{
		m_ResourceGrid = GetGame().GetResourceGrid();
		m_ResourceSystemSubscriptionManager = GetGame().GetResourceSystemSubscriptionManager();
		
	}
}