[EntityEditorProps(category: "GameScripted/Campaign", description: "A rack filled with weapons.", color: "0 0 255 255")]
class SCR_WeaponRackEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_WeaponRackEntity : GenericEntity
{
	// Simple timer setup
	[Attribute("10")]
	float m_fPeriodTime;
	
	protected SCR_ArsenalDisplayComponent m_ArsenalDisplayComponent;
	protected SCR_FactionAffiliationComponent m_FactionAffiliationComponent;
	
	//------------------------------------------------------------------------------------------------
	float GetPeriodTime()
	{
		return m_fPeriodTime;
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearWeapons()
	{
		m_ArsenalDisplayComponent.ClearArsenal();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called periodically to spawn weapons in the queue
	void PeriodicalSpawn(notnull SCR_Faction faction)
	{
		Faction assignedFaction;
		if (m_FactionAffiliationComponent)
		{
			assignedFaction = m_FactionAffiliationComponent.GetAffiliatedFaction();
			if (!assignedFaction || (assignedFaction && assignedFaction.GetFactionKey() != faction.GetFactionKey()))
			{
				m_FactionAffiliationComponent.SetAffiliatedFaction(faction);
				// Set new faction on component,FactionChanged callback refreshes items in ArsenalDisplayComponent
				return;
			}
		}
		
		m_ArsenalDisplayComponent.RefreshArsenal(false, faction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_ArsenalDisplayComponent = SCR_ArsenalDisplayComponent.Cast(owner.FindComponent(SCR_ArsenalDisplayComponent));
		
		if (!m_ArsenalDisplayComponent)
		{
			Print("Arsenal display component not found on WeaponRackEntity", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_WeaponRackEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_WeaponRackEntity()
	{
	}

}
