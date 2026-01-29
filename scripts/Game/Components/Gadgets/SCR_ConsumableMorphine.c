//! Morphine effect
[BaseContainerProps()]
class SCR_ConsumableMorphine : SCR_ConsumableEffectHealthItems
{
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return false;
		
		array<ref SCR_PersistentDamageEffect> effects = damageMgr.GetAllPersistentEffectsOfType(SCR_MorphineDamageEffect);
		if (!effects.IsEmpty())
		{
			failReason = SCR_EConsumableFailReason.ALREADY_APPLIED;
			return false;
		}
		
		array<HitZone> hitZones = {};
		damageMgr.GetPhysicalHitZones(hitZones);
		SCR_RegeneratingHitZone regenHitZone;
		
		foreach (HitZone hitZone : hitZones)
		{
			regenHitZone = SCR_RegeneratingHitZone.Cast(hitZone);
			if (!regenHitZone)
				continue;
			
			if (regenHitZone.GetDamageState() != EDamageState.UNDAMAGED)
				return true;
		}

		failReason = SCR_EConsumableFailReason.UNDAMAGED;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		return CanApplyEffect(target, user, failReason);
	}
	
	//------------------------------------------------------------------------------------------------
	EDamageType GetDefaultDamageType()
	{
		return EDamageType.HEALING;
	}
	
	override ItemUseParameters GetAnimationParameters(IEntity item, notnull IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ItemUseParameters itemUseParams = super.GetAnimationParameters(item, target, group);
		itemUseParams.SetAllowMovementDuringAction(true);
		return itemUseParams;
	}
		
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ConsumableMorphine()
	{
		m_eConsumableType = SCR_EConsumableType.MORPHINE;
	}
}
