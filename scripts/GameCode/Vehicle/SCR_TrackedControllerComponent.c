class SCR_TrackedControllerComponentClass : TrackedControllerComponentClass
{
}

/*!
	Class responsible for game tracked vehicles.
	It connects all tracked components together and handles all comunication between them.
*/
class SCR_TrackedControllerComponent : TrackedControllerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentEntered(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
		{
			GetGame().GetInputManager().AddActionListener("VehicleLightsToggle", EActionTrigger.DOWN, ActionVehicleLightsToggle);
			GetGame().GetInputManager().AddActionListener("TrackedLightsHiBeamToggle", EActionTrigger.DOWN, ActionTrackedLightsHiBeamToggle);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentLeft(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
		{
			GetGame().GetInputManager().RemoveActionListener("VehicleLightsToggle",  EActionTrigger.DOWN, ActionVehicleLightsToggle);
			GetGame().GetInputManager().RemoveActionListener("TrackedLightsHiBeamToggle", EActionTrigger.DOWN, ActionTrackedLightsHiBeamToggle);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionTrackedLightsHiBeamToggle(float value, EActionTrigger trigger)
	{
		PilotCompartmentSlot pilotSlot = GetPilotCompartmentSlot();
		if (!pilotSlot || pilotSlot.GetOccupant() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (lightManager)
			lightManager.SetLightsState(ELightType.HiBeam, !lightManager.GetLightsState(ELightType.HiBeam));
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionVehicleLightsToggle(float value, EActionTrigger trigger)
	{
		PilotCompartmentSlot pilotSlot = GetPilotCompartmentSlot();
		if (!pilotSlot || pilotSlot.GetOccupant() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (lightManager)
			lightManager.SetLightsState(ELightType.Presence, !lightManager.GetLightsState(ELightType.Presence));
	}
}