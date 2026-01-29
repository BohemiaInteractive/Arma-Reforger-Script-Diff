class SCR_ResourceSystemSubscriptionListing
{
	protected ref set<RplId> m_aListeners = new set<RplId>();
	protected SCR_ResourceInteractor m_Interactor;
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceInteractor GetInteractor()
	{
		return m_Interactor;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return m_aListeners.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	void SubscribeListener(RplId listener)
	{
		m_aListeners.Insert(listener);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnsubscribeListener(RplId listener)
	{
		m_aListeners.RemoveItem(listener);
	}
	
	//------------------------------------------------------------------------------------------------
	void Replicate()
	{
		if (m_Interactor)
			m_Interactor.ReplicateEx();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ResourceSystemSubscriptionListing(notnull SCR_ResourceInteractor interactor)
	{
		m_Interactor = interactor;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
     	
		if (!world)
			return;
		
		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
        
		if (!updateSystem)
			return;
		
		updateSystem.RegisterSubscribedInteractor(m_Interactor);
	}
	
	void ~SCR_ResourceSystemSubscriptionListing()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(game.GetWorld());
     	
		if (!world)
			return;
		
		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
        
		if (!updateSystem)
			return;
		
		updateSystem.UnregisterSubscribedInteractor(m_Interactor);
	}
}