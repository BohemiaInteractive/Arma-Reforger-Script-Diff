[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Component handling Armory and ammo storage compositions.", color: "0 0 255 255")]
class SCR_ArmoryComponentClass : ScriptComponentClass
{
}

class SCR_ArmoryComponent : ScriptComponent
{
	protected ref array <SCR_FactionAffiliationComponent> m_aFactionAffiliationComponents = {};
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	void ChangeOwningFaction(notnull Faction faction)
	{
		if (IsProxy())
			return;
		
		FactionKey key = faction.GetFactionKey();
		
		foreach (SCR_FactionAffiliationComponent factionComp : m_aFactionAffiliationComponents)
		{
			factionComp.SetAffiliatedFactionByKey(key);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLinkedEntitiesSpawned(SCR_EditorLinkComponent link)
	{
		FindFactionComponentsInHiearchy(GetOwner());
		link.GetOnLinkedEntitiesSpawned().Remove(OnLinkedEntitiesSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes though entity children (recursively) and finds all faction control components
	//! \param[in] entity cannot be null
	protected void FindFactionComponentsInHiearchy(IEntity entity)
	{
		IEntity child = entity.GetChildren();
		SCR_FactionAffiliationComponent factionComp;
		
		while (child)
		{
			if (child.GetChildren())
				FindFactionComponentsInHiearchy(child);
			
			factionComp = SCR_FactionAffiliationComponent.Cast(child.FindComponent(SCR_FactionAffiliationComponent));
			if (factionComp)
				m_aFactionAffiliationComponents.Insert(factionComp);
			
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
		
		if (IsProxy())
			return;
		
		SCR_EditorLinkComponent link = SCR_EditorLinkComponent.Cast(owner.FindComponent(SCR_EditorLinkComponent));
		if (link)
			link.GetOnLinkedEntitiesSpawned().Insert(OnLinkedEntitiesSpawned);
		else
			FindFactionComponentsInHiearchy(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	// PostInit
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
}
