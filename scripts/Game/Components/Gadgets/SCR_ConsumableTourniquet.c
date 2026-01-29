//! Tourniquet effect
[BaseContainerProps()]
class SCR_ConsumableTourniquet : SCR_ConsumableEffectHealthItems
{

	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;
		
		m_eTargetHZGroup = damageMgr.FindAssociatedHitZoneGroup(animParams.GetIntParam());

		SCR_TourniquetStorageComponent tqStorageComp = SCR_TourniquetStorageComponent.Cast(user.FindComponent(SCR_TourniquetStorageComponent));
		if (!tqStorageComp)
		{
			Print("SCR_TourniquetStorageComponent missing", LogLevel.ERROR);
			return;
		}
		
		tqStorageComp.AddTourniquetToSlot(target, m_eTargetHZGroup, item);
		
		super.ApplyEffect(target, user, item, animParams);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr || !damageMgr.IsBleeding())
			return false;
		
		return damageMgr.GetCharMostDOTHitzoneGroup(EDamageType.BLEEDING, true, true);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return false;
		
		array<ECharacterHitZoneGroup> limbs = {};
		damageMgr.GetAllExtremities(limbs);
		
		if (!limbs.Contains(group))
			return false;
		
		if (damageMgr.GetGroupTourniquetted(group))
		{
			failReason = SCR_EConsumableFailReason.ALREADY_APPLIED;
			return false;
		}
		
		if (damageMgr.GetGroupDamageOverTime(group, EDamageType.BLEEDING) > 0)
			return true;
		
		failReason = SCR_EConsumableFailReason.NOT_BLEEDING;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override ItemUseParameters GetAnimationParameters(IEntity item, notnull IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return null;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return null;

		EBandagingAnimationBodyParts bodyPartToBandage = EBandagingAnimationBodyParts.Invalid;
		
		if (group != ECharacterHitZoneGroup.VIRTUAL)
		{
			if (!damageMgr.GetGroupIsBeingHealed(group))
				bodyPartToBandage = damageMgr.FindAssociatedBandagingBodyPart(group);
		}
		else
		{
			group = damageMgr.GetCharMostDOTHitzoneGroup(EDamageType.BLEEDING, true, true, true);
			if (!group)
				return null;
			
			array<HitZone> groupHitZones = {};
			damageMgr.GetHitZonesOfGroup(group, groupHitZones);
			
			SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(groupHitZones[0]);
			if (!charHitZone)
				return null;
			
			bodyPartToBandage = charHitZone.GetBodyPartToHeal();
		}
		
		if (bodyPartToBandage == EBandagingAnimationBodyParts.Invalid)
				return null;
		
		bool allowMovement = bodyPartToBandage != EBandagingAnimationBodyParts.RightLeg && bodyPartToBandage != EBandagingAnimationBodyParts.LeftLeg;
		
		ItemUseParameters params = ItemUseParameters();
		params.SetEntity(item);
		params.SetAllowMovementDuringAction(allowMovement);
		params.SetKeepInHandAfterSuccess(false);
		params.SetCommandID(GetApplyToSelfAnimCmnd(target));
		params.SetCommandIntArg(1);
		params.SetCommandFloatArg(0.0);
		params.SetMaxAnimLength(m_fApplyToSelfDuration);
		params.SetIntParam(bodyPartToBandage);

		return params;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ConsumableTourniquet()
	{
		m_eConsumableType = SCR_EConsumableType.TOURNIQUET;
	}
}
