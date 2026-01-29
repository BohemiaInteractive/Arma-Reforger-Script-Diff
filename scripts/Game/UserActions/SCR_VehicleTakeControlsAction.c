class SCR_VehicleTakeControlsAction : SCR_VehicleActionBase
{
	protected static const string CONTROLS_LOCKED = "#AR-UserAction_Locked";
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (character)
		{
			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			compartmentAccess.SetActivePilotCompartment(compartment);
		}
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		// See if character is in vehicle
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		// We cannot be pilot nor interior, if we are not seated in vehicle at all.
		if (!character.IsInVehicle())
			return !(m_bInteriorOnly || m_bPilotOnly);

		// See if character is in "this" (owner) vehicle
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		// Character is in compartment
		// that belongs to owner of this action
		BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
		if (!slot)
			return false;

		// Check if we are a pilot compartmenr
		if (slot.GetType() != ECompartmentType.PILOT)
			return false;
				
		// Check if we are controlling the vehicle
		if (slot.IsPiloting())
			return false;

		// Check interior only condition
		if (m_bInteriorOnly && slot.GetOwner().GetRootParent() != GetOwner().GetRootParent())
			return false;

		// Todo: make sure we're not switching any other compartment's user action
		
		return true;
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		if (controller)
		{	
			if (controller.ArePilotControlsLocked()) 
			{
				SetCannotPerformReason(CONTROLS_LOCKED);
				return false;
			}
		}
		
		return true;
	}
}