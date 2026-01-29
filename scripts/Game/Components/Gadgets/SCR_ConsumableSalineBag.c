class LoadoutSalineBagArea : LoadoutAreaType
{
}

//! Saline bag effect
[BaseContainerProps()]
class SCR_ConsumableSalineBag : SCR_ConsumableEffectHealthItems
{
	
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
 	 	if (itemComp)
 	 		itemComp.RequestUserLock(user, false);

		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;
		
		SCR_CharacterBloodHitZone bloodHitZone = damageMgr.GetBloodHitZone();
		if (!bloodHitZone)
			return;
		
		SCR_InventoryStorageManagerComponent inventoryStorageComp = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryStorageComp)
			return;

		SCR_SalineStorageComponent salineStorageComp = SCR_SalineStorageComponent.Cast(user.FindComponent(SCR_SalineStorageComponent));
		if (!salineStorageComp)
			return;
		
		if (salineStorageComp.AddSalineBagToSlot(target, ECharacterHitZoneGroup.RIGHTARM, item, m_fItemRegenerationDuration))
			AddConsumableDamageEffects(char, user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user,out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		//Can only apply when SCR_CharacterBloodHitZone is damaged
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr || damageMgr.GetBloodHitZone().GetDamageState() == EDamageState.UNDAMAGED)
		{
			failReason = SCR_EConsumableFailReason.UNDAMAGED;
			return false;
		}

		//Character must have slot for the salinebag to move to
		SCR_SalineStorageComponent salineStorageMan = SCR_SalineStorageComponent.Cast(target.FindComponent(SCR_SalineStorageComponent));
		if (!salineStorageMan)
			return false;

		array<IEntity> items = {};
		salineStorageMan.GetAll(items);
		if (!items.IsEmpty())
		{
			failReason = SCR_EConsumableFailReason.ALREADY_APPLIED;
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		return CanApplyEffect(target, user, failReason);
	}
	
	override ItemUseParameters GetAnimationParameters(IEntity item, notnull IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ItemUseParameters itemUseParams = super.GetAnimationParameters(item, target, group);
		itemUseParams.SetAllowMovementDuringAction(true);
		return itemUseParams;
	}
	
	//------------------------------------------------------------------------------------------------
	EDamageType GetDefaultDamageType()
	{
		return EDamageType.HEALING;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ConsumableSalineBag()
	{
		m_eConsumableType = SCR_EConsumableType.SALINE;
	}
}
