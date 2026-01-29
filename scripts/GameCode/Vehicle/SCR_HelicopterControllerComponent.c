class SCR_HelicopterControllerComponentClass : HelicopterControllerComponentClass
{
}

/*!
	Class responsible for game helicopter.
	It connects all helicopter components together and handles all comunication between them.
*/
class SCR_HelicopterControllerComponent : HelicopterControllerComponent
{
	BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
	
	//------------------------------------------------------------------------------------------------
	override void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentEntered(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
		{
			GetGame().GetInputManager().AddActionListener("HelicopterLightsTaxiToggle", EActionTrigger.DOWN, ActionHelicopterLightsTaxiToggle);
			GetGame().GetInputManager().AddActionListener("HelicopterLightsLandingToggle", EActionTrigger.DOWN, ActionHelicopterLightsLandingToggle);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentLeft(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
		{
			GetGame().GetInputManager().RemoveActionListener("HelicopterLightsTaxiToggle", EActionTrigger.DOWN, ActionHelicopterLightsTaxiToggle);
			GetGame().GetInputManager().RemoveActionListener("HelicopterLightsLandingToggle", EActionTrigger.DOWN, ActionHelicopterLightsLandingToggle);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionHelicopterLightsTaxiToggle(float value, EActionTrigger trigger)
	{
		PilotCompartmentSlot pilotSlot = GetPilotCompartmentSlot();
		if (!pilotSlot || pilotSlot.GetOccupant() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (lightManager)
			lightManager.SetLightsState(ELightType.SearchLight, !lightManager.GetLightsState(ELightType.SearchLight));
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionHelicopterLightsLandingToggle(float value, EActionTrigger trigger)
	{
		PilotCompartmentSlot pilotSlot = GetPilotCompartmentSlot();
		if (!pilotSlot || pilotSlot.GetOccupant() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (lightManager)
			lightManager.SetLightsState(ELightType.HiBeam, !lightManager.GetLightsState(ELightType.HiBeam));
	}
	
	//------------------------------------------------------------------------------------------------
	// This is required because GameCode CollectiveMode has more options that are not available in the settings (ECollectiveModeForSettings).
	// The usage of this function can not be avoided by explicitly specifying enumerator values for ECollectiveModeForSettings because that will break UI.
	CollectiveMode GetCollectiveModeFromSettings(ECollectiveModeForSettings settingCollectiveMode)
	{
		if (settingCollectiveMode == ECollectiveModeForSettings.Default)
			return CollectiveMode.LocalYOffsetAuto;
		return settingCollectiveMode;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnPrepareControls()
	{
		// Set the collective based on the last used input device.
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (deviceType == EInputDeviceType.GAMEPAD)
		{
			ECollectiveModeForSettings collectiveMode;
			if (gameplaySettings.Get("m_eGamepadCollective", collectiveMode))
				SetCollectiveMode(GetCollectiveModeFromSettings(collectiveMode));
		}
		else if (deviceType == EInputDeviceType.JOYSTICK)
		{
			ECollectiveModeForSettings collectiveMode;
			if (gameplaySettings.Get("m_eHotasCollective", collectiveMode))
				SetCollectiveMode(GetCollectiveModeFromSettings(collectiveMode));
		}
		else
		{
			ECollectiveModeForSettings collectiveMode;
			if (gameplaySettings.Get("m_eKeyboardCollective", collectiveMode))
				SetCollectiveMode(GetCollectiveModeFromSettings(collectiveMode));
		}
	}
}
