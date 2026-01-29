class SCR_VehicleLockControlsAction : SCR_VehicleActionBase
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		if (controller)
		{	
			bool bWantedState = controller.ArePilotControlsLocked();
			if (RplSession.Mode() != RplMode.Client)
				bWantedState = !bWantedState;
			controller.LockPilotControls(bWantedState);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Current state of the feature
	override bool GetState()
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		if (controller)
		{	
			bool bCurrentState = controller.ArePilotControlsLocked();
			return bCurrentState;
		}
		
		return false;
	}
};
