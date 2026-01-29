[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_BaseFactionTriggerEntityClass : SCR_BaseTriggerEntityClass
{
}

class SCR_BaseFactionTriggerEntity : SCR_BaseTriggerEntity
{
	[Attribute(desc: "Factions which are used for area control calculation.", category: "Faction Trigger")]
	protected ref array<FactionKey> m_aOwnerFactionKeys;

	//------------------------------------------------------------------------------------------------
	void AddOwnerFaction(FactionKey factionKey)
	{
		if (!m_aOwnerFactionKeys)
			m_aOwnerFactionKeys = {};
		
		m_aOwnerFactionKeys.Insert(factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (!m_aOwnerFactionKeys || m_aOwnerFactionKeys.IsEmpty() || !IsAlive(ent))
			return false;
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		return factionAffiliation && m_aOwnerFactionKeys.Contains(factionAffiliation.GetAffiliatedFaction().GetFactionKey());
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BaseFactionTriggerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
}