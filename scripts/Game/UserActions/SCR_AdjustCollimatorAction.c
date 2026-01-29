class SCR_AdjustCollimatorAction : SCR_AdjustSignalAction
{
	protected SCR_CollimatorSightsComponent m_SightsComponent;
	protected SCR_CollimatorControllerComponent m_SightControllerComponent;
	
	[Attribute(desc: "When filled, this sound effect will be used instead when the signal is adjusted back to it's starting value")]
	protected string m_sAlternativeMovementStopSoundEffect;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_SightsComponent = SCR_CollimatorSightsComponent.Cast(pOwnerEntity.FindComponent(SCR_CollimatorSightsComponent));
		m_SightControllerComponent = SCR_CollimatorControllerComponent.Cast(pOwnerEntity.FindComponent(SCR_CollimatorControllerComponent));
		
		m_SoundComponent = SoundComponent.Cast(pOwnerEntity.FindComponent(SoundComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_SightsComponent || !m_SightControllerComponent)
			return false;

		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays movement and stop movement sound events
	override void PlayMovementAndStopSound(float lerp)
	{
		if (m_sAlternativeMovementStopSoundEffect.IsEmpty() || !m_SoundComponent || m_fLerpLast == lerp || float.AlmostEqual(lerp, 1))
		{
			super.PlayMovementAndStopSound(lerp);
			return;
		}
		
		if (!float.AlmostEqual(m_fLerpLast, 0))
		{
			m_SoundComponent.Terminate(m_MovementAudioHandle);
			if (m_sMovementStopSoundEvent != string.Empty)
			{
				vector contextTransform[4];
				GetActiveContext().GetTransformationModel(contextTransform);
				m_SoundComponent.SoundEventOffset(m_sAlternativeMovementStopSoundEffect, contextTransform[3]);
			}
		}
	
		m_fLerpLast = lerp;
	}
}