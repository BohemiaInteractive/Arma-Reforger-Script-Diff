class SCR_HealingUserAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Which hitzone group will be checked for conditions", enums:ParamEnumArray.FromEnum(ECharacterHitZoneGroup) )]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;	
	
	[Attribute("0", UIWidgets.ComboBox, "Which consumabletype will be checked for conditions", enums:ParamEnumArray.FromEnum(SCR_EConsumableType) )]
	protected SCR_EConsumableType m_eConsumableType;
	
    [Attribute("#AR-FailReason_AlreadyApplied", UIWidgets.EditBox, "String for already applied healing gadgets")]
	protected LocalizedString m_sAlreadyApplied;    
	
	[Attribute("#AR-FailReason_NotBleeding", UIWidgets.EditBox, "String for when target hitzone isn't bleeding")]
	protected LocalizedString m_sNotBleeding;	
	
	[Attribute("#AR-FailReason_NoBloodLoss", UIWidgets.EditBox, "String for when blood hitzone isn't damaged")]
	protected LocalizedString m_sNoBloodLoss;	
	
	[Attribute("#AR-FailReason_NotDamaged", UIWidgets.EditBox, "String for when target hitzone isn't damaged")]
	protected LocalizedString m_sNotDamaged;

	//------------------------------------------------------------------------------------------------
	protected SCR_ConsumableItemComponent GetConsumableComponent(notnull ChimeraCharacter userChar)
	{
		CharacterControllerComponent controller = userChar.GetCharacterController();
		if (!controller)
			return null;

		IEntity item = controller.GetAttachedGadgetAtLeftHandSlot();
		if (!item)
			return null;
		
		return SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool CanBeShownScript(IEntity user)
	{
		// It is not allowed to perform healing useraction on self
		if (!user || user == GetOwner())
			return false;
		
		// Target character
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!targetCharacter)
			return false;
		
		// Medic character
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (!userCharacter)
			return false;
		
		// Medics' item use ability check
		CharacterControllerComponent userController = userCharacter.GetCharacterController();
		if (!userController || userController.IsUsingItem())
			return false;
		
		// Check if character is in a vehicle and if healing is allowed from seat, if so
		if (userCharacter.IsInVehicle() && !HealingAllowedFromSeat(userCharacter))
			return false;

		// Can only see healing useractions when holding respective consumable
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent || consumableComponent.GetConsumableType() != m_eConsumableType)
			return false;
		
		// Cannot see healing useractions on dead people
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(targetCharacter.GetDamageManager());
		if (!damageMan || damageMan.GetState() == EDamageState.DESTROYED)
			return false;
		
		if (damageMan.GetGroupIsBeingHealed(m_eHitZoneGroup))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Target character
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!targetCharacter)
			return;
		
		// Medic character
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(pUserEntity);
		if (!userCharacter)
			return;
		
		SCR_CharacterControllerComponent userController = SCR_CharacterControllerComponent.Cast(userCharacter.GetCharacterController());
		if (!userController)
			return;
		
		IEntity item = userController.GetAttachedGadgetAtLeftHandSlot();
		if (!item)
			return;
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent)
			return;
		
		consumableComponent.SetTargetCharacter(targetCharacter);
		SCR_ConsumableEffectHealthItems consumableEffect = SCR_ConsumableEffectHealthItems.Cast(consumableComponent.GetConsumableEffect());
		if (!consumableEffect)
			return;
			
		TAnimGraphCommand desiredCmd;
		if (pOwnerEntity == pUserEntity)
			desiredCmd = consumableEffect.GetApplyToSelfAnimCmnd(pOwnerEntity);
		else
			desiredCmd = consumableEffect.GetApplyToOtherAnimCmnd(pOwnerEntity);

		SCR_CharacterControllerComponent targetController = SCR_CharacterControllerComponent.Cast(targetCharacter.GetCharacterController());
		if (targetController && targetController.IsUnconscious())
			desiredCmd = consumableEffect.GetReviveAnimCmnd(pOwnerEntity);
	
		SCR_CharacterDamageManagerComponent targetDamageMan = SCR_CharacterDamageManagerComponent.Cast(targetCharacter.GetDamageManager());
		if (!targetDamageMan)
			return;
		
		ItemUseParameters params = ItemUseParameters();
		params.SetEntity(item);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(desiredCmd);
		params.SetCommandIntArg(1);
		params.SetCommandFloatArg(0.0);
		params.SetMaxAnimLength(consumableEffect.GetApplyToOtherDuraction());
		params.SetIntParam(targetDamageMan.FindAssociatedBandagingBodyPart(m_eHitZoneGroup));
		
		consumableEffect.ActivateEffect(pOwnerEntity, pUserEntity, item, params);
	}
	
	//------------------------------------------------------------------------------------------------
	bool HealingAllowedFromSeat(ChimeraCharacter char)
	{
		if (!char)
			return false;
		
		CompartmentAccessComponent compartmentAccess = char.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;
		
		SCR_DoctorCompartmentSlot doctorSlot = SCR_DoctorCompartmentSlot.Cast(compartmentAccess.GetCompartment());
		if (!doctorSlot)
			return false;
		
		return doctorSlot.AllowHealingFromCompartment();
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetUserActionGroup()
	{
		return m_eHitZoneGroup;
	}
};
