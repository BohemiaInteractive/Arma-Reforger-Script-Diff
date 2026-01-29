class SCR_BirdClass: AIFlockClass
{
};

class SCR_Bird : AIFlock
{	
	private const string IS_FLYING_SIGNAL_NAME = "IsFlying";
	private const string SOUND_FLOCK_TAKEOFF_EVENT_NAME = "SOUND_FLOCK_TAKEOFF";
	private const string SOUND_FLOCK_LAND_EVENT_NAME = "SOUND_FLOCK_LAND";
	
	private int 			m_iFlyingSignalId;
	SignalsManagerComponent m_signalComp;
	SoundComponent 			m_soundComp;
	
	override event protected void EOnInit(IEntity owner)
	{
	
		//--- Activate AI
		//AIControlComponent control = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		//if (control)
			//control.ActivateAI();
		
		m_signalComp = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		if (m_signalComp)
			m_iFlyingSignalId = m_signalComp.AddOrFindSignal(IS_FLYING_SIGNAL_NAME);
		
		m_soundComp = SoundComponent.Cast(FindComponent(SoundComponent));
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnReactToDanger()
	{	
		super.OnReactToDanger();
			
		SoundEvent(SOUND_FLOCK_TAKEOFF_EVENT_NAME);
		SetIsFlyingSignal(true);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnTakeOff()
	{
		super.OnTakeOff();
		
		SetIsFlyingSignal(true);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnLanding()
	{
		super.OnLanding();
		
		SoundEvent(SOUND_FLOCK_LAND_EVENT_NAME);
		SetIsFlyingSignal(false);
	}
	
	//------------------------------------------------------------------------------------------------	
	private void SetIsFlyingSignal(bool isFlying)
	{
		if (m_signalComp)
			m_signalComp.SetSignalValue(m_iFlyingSignalId, isFlying);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SoundEvent(string soundEvent)
	{		
		if (m_soundComp)
			m_soundComp.SoundEvent(soundEvent);	
	}	
}