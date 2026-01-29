//! Holds an array of effects that are all executed when this Refund effect is chosen
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Multi Execute Entries", "(Disabled) Multi Execute Entries", 1)]
class SCR_MultiExecuteListArsenalRefundEffect : SCR_BaseArsenalRefundEffect
{	
	[Attribute(desc: "An array of effects. These will all be executed if the Refund effect is chosen in order of the array.")]
	protected ref array<ref SCR_BaseArsenalRefundEffect> m_aMultiExecuteEffectEntries;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ArsenalRefundEffectComponentClass arsenalRefundEffectClass)
	{
		//~ Remove any disabled
		for (int i = m_aMultiExecuteEffectEntries.Count() - 1; i >= 0; i--)
		{
			m_aMultiExecuteEffectEntries[i].Init(arsenalRefundEffectClass);
			
			if (!m_aMultiExecuteEffectEntries[i].m_bEnabled)
				m_aMultiExecuteEffectEntries.RemoveOrdered(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		if (!super.IsValid(item, playerController, arsenal))
			return false;
		
		//~ Check if there are any entries
		return !m_aMultiExecuteEffectEntries.IsEmpty() && HasValidEntries(item, playerController, arsenal);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ExecuteRefundEffect(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		bool executedSuccessfullyAtLeastOnce;
		
		//~ Get valid entries
		array<SCR_BaseArsenalRefundEffect> validEntries = {};
		GetValidEntries(validEntries, item, playerController, arsenal);
		
		//~ Execute all effects
		foreach(SCR_BaseArsenalRefundEffect entry : validEntries)
		{
			if (entry.ExecuteRefundEffect(item, playerController, arsenal))
				executedSuccessfullyAtLeastOnce = true;
		}
		
		//~ Return if successfully executed at least one effect
		return executedSuccessfullyAtLeastOnce;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Check if there is at least one valid entry
	protected bool HasValidEntries(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		//~ Check if there is at least one valid entry
		foreach(SCR_BaseArsenalRefundEffect entry : m_aMultiExecuteEffectEntries)
		{
			if (entry.IsValid(item, playerController, arsenal))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get all valid entries
	protected int GetValidEntries(notnull inout array<SCR_BaseArsenalRefundEffect> validEntries, notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		validEntries.Clear();
		
		//~ Get all valid entries
		foreach(SCR_BaseArsenalRefundEffect entry : m_aMultiExecuteEffectEntries)
		{
			if (entry.IsValid(item, playerController, arsenal))
				validEntries.Insert(entry);
		}
		
		return validEntries.Count();
	}
}