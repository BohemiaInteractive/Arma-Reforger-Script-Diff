class SCR_HandBrakeInfo : SCR_BaseVehicleInfo
{
	protected CarControllerComponent m_pCarController;

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		if (!m_pCarController)
			return EVehicleInfoState.DISABLED;

		EVehicleInfoState state = EVehicleInfoState.DISABLED;
		m_bIsBlinking = false;
		if (m_pCarController.GetHandBrake() || m_pCarController.GetPersistentHandBrake())
		{
			VehicleWheeledSimulation simulation = m_pCarController.GetSimulation();
		 	if (m_pCarController.IsEngineOn() && simulation && simulation.GetSpeedKmh() > 1)
				state = EVehicleInfoState.WARNING;
			else
				state = EVehicleInfoState.ENABLED;
		}

		return state;
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);

		m_pCarController = CarControllerComponent.Cast(owner.FindComponent(CarControllerComponent));
	}
};
