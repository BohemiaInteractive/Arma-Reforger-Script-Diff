class SCR_MorphineUserAction : SCR_HealingUserAction
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
	override bool CanBePerformedScript(IEntity user)
	{
		// Medic character
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (!userCharacter)
			return false;
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent)
			return false;
		
		int reason;
		if (!consumableComponent.GetConsumableEffect().CanApplyEffect(GetOwner(), userCharacter, reason))
		{
			if (reason == SCR_EConsumableFailReason.UNDAMAGED)
				SetCannotPerformReason(m_sNotDamaged);		
			else if (reason == SCR_EConsumableFailReason.ALREADY_APPLIED)
				SetCannotPerformReason(m_sAlreadyApplied);
			
			return false;
		}
		
		return true;
	}
};
