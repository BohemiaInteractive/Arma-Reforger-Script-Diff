//! Holds an array of effects that are executed using a weighted randomization
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Weighted Entries", "(Disabled) Weighted Entries", 1)]
class SCR_WeightedListArsenalRefundEffect : SCR_BaseArsenalRefundEffect
{	
	[Attribute(desc: "An array of effects. It will weighted randomize which effects will be executed any effect disabled will never be randomized")]
	protected ref array<ref SCR_BaseWeightedArsenalRefundEffectEntry> m_aWeightedEffectEntries;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ArsenalRefundEffectComponentClass arsenalRefundEffectClass)
	{
		//~ Remove any disabled
		for (int i = m_aWeightedEffectEntries.Count() - 1; i >= 0; i--)
		{
			m_aWeightedEffectEntries[i].Init(arsenalRefundEffectClass);
			
			if (!m_aWeightedEffectEntries[i].m_bEnabled)
				m_aWeightedEffectEntries.RemoveOrdered(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		if (!super.IsValid(item, playerController, arsenal))
			return false;
		
		//~ Check if there are any entries
		return !m_aWeightedEffectEntries.IsEmpty() && HasValidEntries(item, playerController, arsenal);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ExecuteRefundEffect(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		//~ Get valid entries
		array<SCR_BaseWeightedArsenalRefundEffectEntry> validEntries = {};
		GetValidEntries(validEntries, item, playerController, arsenal);
		
		if (validEntries.IsEmpty())
			return false;
		
		SCR_WeightedArray<SCR_BaseWeightedArsenalRefundEffectEntry> weightedArrayEffectEntries = new SCR_WeightedArray<SCR_BaseWeightedArsenalRefundEffectEntry>();
		
		//~ Add to weighted array
		foreach (SCR_BaseWeightedArsenalRefundEffectEntry entry : validEntries)
		{
			if (!entry.m_bEnabled)
				continue;
			
			weightedArrayEffectEntries.Insert(entry, entry.m_iWeight);
		}
		
		//~ Get weighted random entry
		SCR_BaseWeightedArsenalRefundEffectEntry entry;
		weightedArrayEffectEntries.GetRandomValue(entry);
		
		//~ Execute the effect
		return entry && entry.ExecuteRefundEffect(item, playerController, arsenal);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Check if there is at least one valid entry
	protected bool HasValidEntries(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		//~ Check if there is at least one valid entry
		foreach(SCR_BaseWeightedArsenalRefundEffectEntry entry : m_aWeightedEffectEntries)
		{
			if (entry.IsValid(item, playerController, arsenal))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get all valid entries
	protected int GetValidEntries(notnull inout array<SCR_BaseWeightedArsenalRefundEffectEntry> validEntries, notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		validEntries.Clear();
		
		//~ Get all valid entries
		foreach(SCR_BaseWeightedArsenalRefundEffectEntry entry : m_aWeightedEffectEntries)
		{
			if (entry.IsValid(item, playerController, arsenal))
				validEntries.Insert(entry);
		}
		
		return validEntries.Count();
	}
}