class SCR_BandageUserAction : SCR_HealingUserAction
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
		if (!super.CanBeShownScript(user))
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
};
