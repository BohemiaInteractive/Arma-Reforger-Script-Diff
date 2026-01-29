class SCR_TourniquetUserAction : SCR_HealingUserAction
{
	
	//------------------------------------------------------------------------------------------------
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		if (controller.GetLifeState() != ECharacterLifeState.ALIVE)
			return;
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(character);
		if (consumableComponent)
			consumableComponent.SetAlternativeModel(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (IsTourniquetted())
			return true;
		
		if (!super.CanBeShownScript(user))
			return false;
		
		// It is not allowed to perform healing useraction on self
		if (!user || user == GetOwner())
			return false;
		
		// Target character
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!targetCharacter)
			return false;
		
		SCR_CharacterDamageManagerComponent charDamMan = SCR_CharacterDamageManagerComponent.Cast(targetCharacter.GetDamageManager());
		if (!charDamMan || charDamMan.GetGroupDamageOverTime(m_eHitZoneGroup, EDamageType.BLEEDING) == 0)
		{
			SetCannotPerformReason(m_sNotBleeding);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsTourniquetted()
	{
		// Target character
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!targetCharacter)
			return false;

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(targetCharacter.GetDamageManager());
		if (!damageMan)
			return false;

		return damageMan.GetGroupTourniquetted(m_eHitZoneGroup);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (IsTourniquetted())
			return true;

		// Target character
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!targetCharacter)
			return false;
		
		// Medic character
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (!userCharacter)
			return false;

		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent)
			return false;
		
		SCR_ConsumableTourniquet consumableTQ = SCR_ConsumableTourniquet.Cast(consumableComponent.GetConsumableEffect());
		if (!consumableTQ)
			return false;
		
		int reason;
		if (consumableTQ.CanApplyEffectToHZ(targetCharacter, userCharacter, m_eHitZoneGroup, reason))
			return true;

		if (reason == SCR_EConsumableFailReason.ALREADY_APPLIED)
			SetCannotPerformReason(m_sAlreadyApplied);
		else if (reason == SCR_EConsumableFailReason.NOT_BLEEDING)
			SetCannotPerformReason(m_sNotBleeding);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsTourniquetted())
		{
			super.PerformAction(pOwnerEntity, pUserEntity);
			return;
		}

		SCR_TourniquetStorageComponent tourniquetStorage = SCR_TourniquetStorageComponent.Cast(pOwnerEntity.FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorage)
			return;

		if (tourniquetStorage.RemoveTourniquetFromSlot(m_eHitZoneGroup, pUserEntity))
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_OPEN);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (IsTourniquetted())
		{
			outName = ("#AR-RemoveTourniquetAction");
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool HealingAllowedFromSeat(ChimeraCharacter char)
	{
		return false;
	}
};
