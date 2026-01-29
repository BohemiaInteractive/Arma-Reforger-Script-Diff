class SCR_VInfoAutoHover : SCR_BaseVehicleInfo
{
	protected SCR_HelicopterControllerComponent m_HelicoperController;
	protected SignalsManagerComponent m_SignalsManager;

	protected int m_iSignalIndexSpeed;
	protected const string SIGNAL_SPEED = "airspeed";
	
	protected int m_iSignalIndexAlt;
	protected const string SIGNAL_ALTITUDE = "altitudeAGL";	
	
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		if (!m_HelicoperController || !m_SignalsManager)
			return EVehicleInfoState.DISABLED;

		m_bIsBlinking = false;
		
		float speed = m_SignalsManager.GetSignalValue(m_iSignalIndexSpeed);
		float alt = m_SignalsManager.GetSignalValue(m_iSignalIndexAlt);
		
		bool bIsAutoHoverOn = m_HelicoperController.GetAutohoverEnabled();		

		if (!bIsAutoHoverOn)
			return EVehicleInfoState.DISABLED;
						
		EVehicleInfoState state;
		
	 	if (speed > 20 && alt > 1)
		{
			state = EVehicleInfoState.WARNING;
		}
		else
		{
			state = EVehicleInfoState.ENABLED;		
		}
		
		return state;
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		m_HelicoperController = SCR_HelicopterControllerComponent.Cast(owner.FindComponent(SCR_HelicopterControllerComponent));
		
		if (!m_HelicoperController)
			return;
		
		m_SignalsManager = m_HelicoperController.GetSignalsManager();
		
		if (!m_SignalsManager)
			return;		
		
		m_iSignalIndexSpeed = m_SignalsManager.FindSignal(SIGNAL_SPEED);
		m_iSignalIndexAlt = m_SignalsManager.FindSignal(SIGNAL_ALTITUDE);
	}
};
