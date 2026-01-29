class SCR_FactionAffiliationComponentClass : FactionAffiliationComponentClass
{
}

void OnFactionChangeDelegate(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction);
typedef func OnFactionChangeDelegate;
typedef ScriptInvokerBase<OnFactionChangeDelegate> OnFactionChangedInvoker;


class SCR_FactionAffiliationComponent : FactionAffiliationComponent
{
	// Local invokers
	private ref ScriptInvoker m_OnFactionUpdate;
	private ref OnFactionChangedInvoker m_OnFactionChangedInvoker;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnFactionChanged(Faction previous, Faction current)
	{
		if (m_OnFactionUpdate)
			m_OnFactionUpdate.Invoke(GetAffiliatedFaction());
		
		if (m_OnFactionChangedInvoker)
			m_OnFactionChangedInvoker.Invoke(this, previous, current);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	// may return to DefaultAffiliatedFaction in some cases - override this
	void ClearAffiliatedFaction()
	{
		SetAffiliatedFaction(null); 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set faction.
	//! \param[in] owner
	//! \param[in] faction the desired faction
	static void SetFaction(IEntity owner, Faction faction)
	{
		if (!owner || !faction )
			return;

		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (factionComponent)
			factionComponent.SetAffiliatedFaction(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	[Obsolete("Use the SCR_FactionAffiliationComponent.GetOnFactionChanged invoker instead!")]
	ScriptInvoker GetOnFactionUpdate()
	{
		if (!m_OnFactionUpdate)
			m_OnFactionUpdate = new ScriptInvoker();
		return m_OnFactionUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnFactionChangedInvoker GetOnFactionChanged()
	{
		if (!m_OnFactionChangedInvoker)
			m_OnFactionChangedInvoker = new OnFactionChangedInvoker();
		
		return m_OnFactionChangedInvoker;
	}
}
