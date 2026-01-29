class SCR_HZInfoHeli : SCR_HitZoneInfo
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
		EVehicleInfoState state = super.GetState();

		m_bIsBlinking = false;
		
		if (!m_HelicoperController || !m_SignalsManager)
			return state;		
				
		//4ZGUBA: We can make the indicators for damaged hitzones blink, if heli is flying; or not :-)
		float speed = m_SignalsManager.GetSignalValue(m_iSignalIndexSpeed);
		float alt = m_SignalsManager.GetSignalValue(m_iSignalIndexAlt);
		m_bIsBlinking = state == EVehicleInfoState.ERROR && (alt > 5 || speed > 5);
		
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
		
		m_iSignalIndexSpeed = m_SignalsManager.AddOrFindSignal(SIGNAL_SPEED);
		m_iSignalIndexAlt = m_SignalsManager.AddOrFindSignal(SIGNAL_ALTITUDE);
	}
};
