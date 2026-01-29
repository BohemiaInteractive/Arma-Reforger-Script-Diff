class SCR_CarControllerComponentClass : CarControllerComponentClass
{
}

/*!
	Class responsible for game car.
	It connects all car components together and handles all comunication between them.
*/
class SCR_CarControllerComponent : CarControllerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentEntered(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
			GetGame().GetInputManager().AddActionListener("CarLightsHiBeamToggle", EActionTrigger.DOWN, ActionCarLightsHiBeamToggle);
	}

	//------------------------------------------------------------------------------------------------
	override void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentLeft(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
			GetGame().GetInputManager().RemoveActionListener("CarLightsHiBeamToggle", EActionTrigger.DOWN, ActionCarLightsHiBeamToggle);
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionCarLightsHiBeamToggle(float value, EActionTrigger trigger)
	{
		PilotCompartmentSlot pilotSlot = GetPilotCompartmentSlot();
		if (!pilotSlot || pilotSlot.GetOccupant() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (lightManager)
			lightManager.SetLightsState(ELightType.HiBeam, !lightManager.GetLightsState(ELightType.HiBeam));
	}
}
