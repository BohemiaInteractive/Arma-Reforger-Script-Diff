class SCR_MilitaryBaseLogicComponentClass : ScriptComponentClass
{
}

class SCR_MilitaryBaseLogicComponent : ScriptComponent
{
	protected ref array<SCR_MilitaryBaseComponent> m_aBases = {};

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] base
	void RegisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (m_aBases.Contains(base))
			return;

		m_aBases.Insert(base);
		OnBaseRegistered(base);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		if (base.GetOwner() == GetOwner())
			return;

		if (SCR_GameModeCampaign.Cast(GetGame().GetGameMode()))
		{
			if (SCR_CampaignMilitaryBaseComponent.Cast(base))
				OnBaseFactionChanged(base.GetFaction());
		}
		else if (m_aBases.Count() == 1)
		{
			OnBaseFactionChanged(base.GetFaction());
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] base
	void UnregisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (!m_aBases.Contains(base))
			return;

		m_aBases.RemoveItem(base);
		OnBaseUnregistered(base);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	void OnBaseUnregistered(notnull SCR_MilitaryBaseComponent base);

	//------------------------------------------------------------------------------------------------
	//! \param[out] bases
	//! \return
	int GetBases(out array<SCR_MilitaryBaseComponent> bases)
	{
		if (bases)
			return bases.Copy(m_aBases);
		else
			return m_aBases.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsControlledByFaction(notnull Faction faction)
	{
		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (base.GetFaction() == faction)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void OnBaseFactionChanged(Faction faction)
	{
		FactionAffiliationComponent factionControl = FactionAffiliationComponent.Cast(GetOwner().FindComponent(FactionAffiliationComponent));

		if (!factionControl || factionControl.GetAffiliatedFaction() == faction)
			return;

		factionControl.SetAffiliatedFaction(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCapturingFactionChanged(FactionKey faction);

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		// Check for play mode again in case init event was set from outside of this class
		if (!GetGame().InPlayMode())
			return;

		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (baseManager)
			baseManager.RegisterLogicComponent(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_MilitaryBaseLogicComponent()
	{
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		
		if (baseManager)
			baseManager.UnregisterLogicComponent(this);

		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (base)
				base.UnregisterLogicComponent(this);
		}
	}
}
