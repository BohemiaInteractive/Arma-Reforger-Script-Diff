[ComponentEditorProps(category: "GameScripted/Inventory", description: "")]
class SCR_ArsenalRefundEffectComponentClass : ScriptComponentClass
{
	[Attribute()]
	protected ref array<ref SCR_BaseArsenalRefundEffect> m_aArsenalRefundEffects;
	
	protected ref array<ref SCR_BaseArsenalRefundEffect> m_aOrderedArsenalRefundEffects;
	
	//------------------------------------------------------------------------------------------------
	void SCR_ArsenalRefundEffectComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		InitClass(componentSource, parentSource, prefabSource);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		//~ No need to init in Workbench or if the ordered list already exists
		if (SCR_Global.IsEditMode() || m_aOrderedArsenalRefundEffects)
			return;
		
		//~ Init the effects
		foreach(SCR_BaseArsenalRefundEffect effect : m_aArsenalRefundEffects)
		{
			effect.Init(this);
		}
		
		bool addedToArray;
		m_aOrderedArsenalRefundEffects = {};
		
		//~ Remove disabled effects and order the array on priority
		foreach(SCR_BaseArsenalRefundEffect effect : m_aArsenalRefundEffects)
		{			
			//~ Ignore disabled
			if (!effect.m_bEnabled)
				continue;
			
			addedToArray = false;
			
			foreach(int index, SCR_BaseArsenalRefundEffect orderedEffect : m_aOrderedArsenalRefundEffects)
			{
				//~ Order on priority
				if (effect.m_iPriority > orderedEffect.m_iPriority)
				{
					m_aOrderedArsenalRefundEffects.InsertAt(effect, index);
					addedToArray = true;
					break;
				}
			}
			
			//~ No higher priority found so add at the end
			if (!addedToArray)
				m_aOrderedArsenalRefundEffects.Insert(effect);
		}
		
		//~ Clear initial array
		m_aArsenalRefundEffects = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all refund effects in order. Weither they are valid or not
	//! \param[inout] refundEffects All refund effects
	//! \return Refund effect array count
	int GetOrderedRefundEffects(notnull inout array<SCR_BaseArsenalRefundEffect> refundEffects)
	{		
		refundEffects.Clear();
		
		if (!m_aOrderedArsenalRefundEffects)
			return 0;
		
		foreach(SCR_BaseArsenalRefundEffect orderedEffect : m_aOrderedArsenalRefundEffects)
		{
			refundEffects.Insert(orderedEffect);
		}
		
		return refundEffects.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns first valid refund effect
	//! \param[in] item Item that is being refunded
	//! \param[in] playerController Player controller of player that refunds the item
	//! \param[in] arsenal Arsenal the item was refunded at (Can potentially be null)
	//! \return Refund effect (Can be null)
	SCR_BaseArsenalRefundEffect GetValidEffect(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		if (!m_aOrderedArsenalRefundEffects || m_aOrderedArsenalRefundEffects.IsEmpty())
			return null;
		
		foreach(SCR_BaseArsenalRefundEffect orderedEffect : m_aOrderedArsenalRefundEffects)
		{
			//~ Check if valid
			if (!orderedEffect.IsValid(item, playerController, arsenal))
				continue;
			
			//~ Return valid refund effect
			return orderedEffect;
		}
		
		//~ No valid refund effect
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Executes the first valid refund effect
	//! \param[in] item Item that is being refunded
	//! \param[in] playerController Player controller of player that refunds the item
	//! \param[in] arsenal Arsenal the item was refunded at (Can potentially be null)
	//! \return Returns the effect that was executed
	SCR_BaseArsenalRefundEffect ExecuteFirstValidRefundEffect(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{		
		//~ Find a valid effect
		SCR_BaseArsenalRefundEffect validRefundEffect = GetValidEffect(item, playerController, arsenal);
		if (!validRefundEffect)
			return null;
		
		//~ Try to execute the effect
		if (!validRefundEffect.ExecuteRefundEffect(item, playerController, arsenal))
			return null;
		
		//~ Effect was successfully executed
		return validRefundEffect;
	}
}

class SCR_ArsenalRefundEffectComponent : ScriptComponent
{
	
}



