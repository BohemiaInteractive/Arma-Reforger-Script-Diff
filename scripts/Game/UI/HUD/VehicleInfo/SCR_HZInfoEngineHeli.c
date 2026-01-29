class SCR_HZInfoEngineHeli : SCR_HZInfoHeli
{
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		EVehicleInfoState state = super.GetState();

		if (m_HelicoperController && m_HelicoperController.GetEngineDrowned())
			return EVehicleInfoState.ERROR;

		return state;
	}	
}
