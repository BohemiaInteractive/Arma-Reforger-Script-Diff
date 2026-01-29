class SCR_EngineHitZoneInfo : SCR_HitZoneInfo
{
	protected CarControllerComponent m_pCarController;

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		EVehicleInfoState state = super.GetState();

		if (!m_pCarController)
			return state;
	
		if (m_pCarController.GetEngineDrowned())
			state = EVehicleInfoState.ERROR;
	
		VehicleWheeledSimulation simulation = m_pCarController.GetSimulation();
		if (state == EVehicleInfoState.ERROR && simulation && simulation.GetThrottle() > 0.1)
			m_bIsBlinking = true;
	
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
