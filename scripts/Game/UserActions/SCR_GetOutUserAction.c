class SCR_GetOutAction : SCR_CompartmentUserAction
{
	protected const float MAX_GETOUT_SPEED_METER_PER_SEC_SQ = 17.36138889;
	protected const float MAX_GETOUT_ALTITUDE_AGL_METERS = 3;

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
		
		if (!compartmentAccess.GetOutVehicle(EGetOutType.ANIMATED, GetRelevantDoorIndex(pUserEntity), ECloseDoorAfterActions.RETURN_TO_PREVIOUS_STATE, false))
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
		
		// Do not allow plain GetOut with speeds higher than 15 km/phys
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		
		if (vehicle)
		{
			Physics phys = vehicle.GetPhysics();
			
			if (phys) 
			{
				vector velocity = phys.GetVelocity();

				if ((velocity.LengthSq()) > MAX_GETOUT_SPEED_METER_PER_SEC_SQ)
					return false;
			}

			// Disallow GetOut when flying is above 3 meters
			HelicopterControllerComponent helicopterController = HelicopterControllerComponent.Cast(vehicle.GetVehicleController());
			if (helicopterController)
			{
				VehicleHelicopterSimulation simulation = VehicleHelicopterSimulation.Cast(helicopterController.GetBaseSimulation());
				if (simulation && simulation.GetAltitudeAGL() > MAX_GETOUT_ALTITUDE_AGL_METERS)
					return false;
			}
		}
		
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
