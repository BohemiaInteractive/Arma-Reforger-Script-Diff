class SCR_JumpOutAction : SCR_CompartmentUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (controller && controller.IsUnconscious())
			return;

		BaseCompartmentSlot targetCompartment = GetCompartmentSlot();
		if (!targetCompartment)
			return;
		
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return;
		
		if (!compartmentAccess.GetOutVehicle(EGetOutType.ANIMATED, GetRelevantDoorIndex(pUserEntity), ECloseDoorAfterActions.LEAVE_OPEN, false))
			return;
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (controller && controller.IsUnconscious())
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;
		
		if (!compartmentAccess.IsInCompartment())
			return false;
		
		if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
			return false;
		
		BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
		if (!compartment)
			return false;

		BaseCompartmentSlot thisCompartment = GetCompartmentSlot();
		return thisCompartment == compartment;
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformed(user);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{		
		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;
		
		outName = actionInfo.GetName();
		
		return true;
	}	
};