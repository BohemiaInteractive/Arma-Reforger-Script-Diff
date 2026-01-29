class SCR_HelicopterHoverAction : SCR_VehicleActionBase
{
	//------------------------------------------------------------------------------------------------
	override void SetState(bool enable)
	{
		HelicopterControllerComponent helicopterControllerComponent = HelicopterControllerComponent.Cast(m_VehicleController);
		
		if (!helicopterControllerComponent)
			return;
				
		helicopterControllerComponent.SetAutohoverEnabled(enable);
		
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComponent)
			return;
		
		if (enable)
			soundComponent.SoundEvent(SCR_SoundEvent.SOUND_AUTOHOVER_ON);
		else
			soundComponent.SoundEvent(SCR_SoundEvent.SOUND_AUTOHOVER_OFF);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetState()
	{
		return HelicopterControllerComponent.Cast(m_VehicleController) && HelicopterControllerComponent.Cast(m_VehicleController).GetAutohoverEnabled();
	}
};
