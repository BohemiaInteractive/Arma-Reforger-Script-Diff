class SCR_SwitchSeatAction : SCR_GetInUserAction
{
	[Attribute("0")]
	protected bool m_bPilotOnly;
	
	[Attribute("0")]
	protected bool m_bCargoOnly;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character && !character.IsInVehicle())
			return false;

		CharacterControllerComponent characterController = character.GetCharacterController();
		if (!characterController)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;
		
		if (compartmentAccess.IsSwitchingSeatsAnim())
			return false;

		BaseCompartmentSlot characterCompartment = compartmentAccess.GetCompartment();
		if (!characterCompartment)
			return false;

		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		if (characterCompartment == compartment)
			return false;

		// Restrict switching to this compartment in case the section does not match
		if (characterCompartment.GetCompartmentSection() != compartment.GetCompartmentSection())
			return false;

		auto commandHandler = character.GetAnimationComponent().GetCommandHandler();
		if (commandHandler && commandHandler.IsVehicleSwitchingSeats())
			return false;

		// Prevents switching seats within different vehicles,
		if (characterCompartment.GetOwner().GetRootParent() != GetOwner().GetRootParent())
			return false;

		// Check if the position isn't locked
		if (m_pLockComp && m_pLockComp.IsLocked(user, compartment))
		{
			SetCannotPerformReason(m_pLockComp.GetCannotPerformReason(user));
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!user)
			return false;

		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		if (compartment.GetOccupant())
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character && !character.IsInVehicle())
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
			return false;

		BaseCompartmentSlot characterCompartment = compartmentAccess.GetCompartment();
		if (!characterCompartment)
			return false;

		if (characterCompartment == compartment)
			return false;

		// Restrict switching to this compartment in case the section does not match
		if (characterCompartment.GetCompartmentSection() != compartment.GetCompartmentSection())
			return false;

		auto commandHandler = character.GetAnimationComponent().GetCommandHandler();
		if (commandHandler && commandHandler.IsVehicleSwitchingSeats())
			return false;

		// Prevents switching seats within different vehicles,
		if (characterCompartment.GetOwner().GetRootParent() != GetOwner().GetRootParent())
			return false;

		//! Check if some other action or animation is preventing the seat switch
		auto vehicleController = VehicleControllerComponent.Cast(characterCompartment.GetController());
		if (vehicleController && !vehicleController.CanSwitchSeat())
			return false;
		
		if (m_bPilotOnly)
		{
			// Check if the Compartment the player is in, is part of the Vehicle and not a sub compartment
			if (characterCompartment.GetOwner() != characterCompartment.GetOwner().GetRootParent())
				return false;
		}
		
		if (m_bCargoOnly)
		{
			// Check if the Compartment the player is in, is not the main Compartment of the Vehicle but a sub compartment
			if (characterCompartment.GetOwner() == characterCompartment.GetOwner().GetRootParent())
				return false;
		}

		return true;
	}
}
