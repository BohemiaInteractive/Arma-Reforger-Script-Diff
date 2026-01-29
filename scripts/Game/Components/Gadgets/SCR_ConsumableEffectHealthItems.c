[BaseContainerProps()]
class SCR_ConsumableEffectHealthItems : SCR_ConsumableEffectBase
{
	[Attribute("0", UIWidgets.EditBox, "Regeneration speed of related hitZone when consuming this item", category: "Regeneration")]
	protected float m_fItemRegenerationSpeed;
	
	[Attribute("0", UIWidgets.EditBox, "Regeneration duration of related hitZone when consuming this item in seconds", category: "Regeneration")]
	protected float m_fItemRegenerationDuration;	
	
	[Attribute("0", UIWidgets.EditBox, "Total amount of regeneration that will be applied to the related hitZone. Will be ignored if m_fItemRegenerationDuration > 0", category: "Regeneration")]
	protected float m_fItemAbsoluteRegenerationAmount;
	
	[Attribute(desc: "DamageEffects to add when the effect is applied")]
	protected ref array<ref SCR_DamageEffect> m_aDamageEffectsToLoad;
	
	//Cached to remember which bodypart to remove effect from
	protected ECharacterHitZoneGroup m_eTargetHZGroup;
	
	//------------------------------------------------------------------------------------------------	
	override bool ActivateEffect(IEntity target, IEntity user, IEntity item, ItemUseParameters animParams = null)
	{
		ItemUseParameters localAnimParams = animParams;
		if (!localAnimParams)
		{
			//user-held healthitem needs to get data from target to perform anim
			localAnimParams = GetAnimationParameters(item, target);
		}
		
		if (!localAnimParams)
			return false;
		
		if (!super.ActivateEffect(target, user, item, localAnimParams))
			return false;		

		ChimeraCharacter character = ChimeraCharacter.Cast(target);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
			
		if (localAnimParams.GetIntParam() == EBandagingAnimationBodyParts.LeftLeg || localAnimParams.GetIntParam() == EBandagingAnimationBodyParts.RightLeg)
		{
			if (controller.GetStance() == ECharacterStance.STAND)
				controller.SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOCROUCH);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
 	 	if (itemComp)
 	 		itemComp.RequestUserLock(user, false);
		
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return;
		
		AddConsumableDamageEffects(char, user);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddConsumableDamageEffects(notnull ChimeraCharacter char, IEntity instigator)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		SCR_DotDamageEffect dotClone;
		
		array<HitZone> hitZones = {};
		damageMgr.GetHitZonesOfGroup(GetTargetHitZoneGroup(), hitZones);
		if (hitZones.IsEmpty())
			hitZones.Insert(damageMgr.GetDefaultHitZone());
		
		foreach (SCR_DamageEffect effect : m_aDamageEffectsToLoad)
		{
			effect.SetInstigator(Instigator.CreateInstigator(instigator));
			effect.SetDamageType(effect.GetDefaultDamageType());
			effect.SetAffectedHitZone(hitZones[0]);
			
			dotClone = SCR_DotDamageEffect.Cast(effect);
			if (dotClone)
			{
				dotClone.SetDPS(-GetItemRegenSpeed());
				dotClone.SetMaxDuration(m_fItemRegenerationDuration);
			}
			
			damageMgr.AddDamageEffect(effect);
		}	
	};
	
	//------------------------------------------------------------------------------------------------
	float GetItemRegenSpeed()
	{
		float itemRegenSpeed = 0;		
		if (m_fItemRegenerationSpeed != 0)	// If a regeneration time is set, regen will occur for given amount of time at the itemRegenerationSpeed
			itemRegenSpeed = m_fItemRegenerationSpeed;
		else if (m_fItemAbsoluteRegenerationAmount != 0)	// If an absolute regen amount is set instead of a duration, the regen will last until the amount of points has been distributed at the itemRegenerationSpeed
			itemRegenSpeed = m_fItemAbsoluteRegenerationAmount / m_fItemRegenerationDuration;	

		return itemRegenSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UpdateAnimationCommands(IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(user);
		if (!char)
			return false;
		
		CharacterAnimationComponent animationComponent = char.GetAnimationComponent();
		if (!animationComponent)
			return false;		
		
		m_iPlayerApplyToSelfCmdId = animationComponent.BindCommand("CMD_HealSelf");
		m_iPlayerApplyToOtherCmdId = animationComponent.BindCommand("CMD_HealOther");
		m_iPlayerReviveCmdId = animationComponent.BindCommand("CMD_Revive");
		if (m_iPlayerApplyToSelfCmdId < 0 || m_iPlayerApplyToOtherCmdId < 0)
		{
			Print("One or both healing animationCommands have incorrect ID's!!", LogLevel.ERROR);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Condition whether this effect can be applied to the specific hit zone
	//! \param[in] target is the character who is having the effect applied
	//! \param[in] user
	//! \param[in] group the hitzonegroup which is having the effect applied
	//! \param[in] failReason
	bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	//! \return
	TAnimGraphCommand GetReviveAnimCmnd(IEntity user)
	{
		UpdateAnimationCommands(user);
		return m_iPlayerReviveCmdId;
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetTargetHitZoneGroup()
	{
		return m_eTargetHZGroup;
	}
}
