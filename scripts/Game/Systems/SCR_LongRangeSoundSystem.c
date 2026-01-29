class SCR_LongRangeSoundSystem : LongRangeSoundSystem
{
	override protected void SoundEventTriggered(vector position, ResourceName soundProject, string soundEventName)
	{
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(GetWorld());
		if (!soundManager)
			return;
		
		SCR_AudioSourceConfiguration sourceConfig = new SCR_AudioSourceConfiguration();
		sourceConfig.m_sSoundProject = soundProject;
		sourceConfig.m_sSoundEventName = soundEventName;
		sourceConfig.m_eFlags |= EAudioSourceConfigurationFlag.FinishWhenEntityDestroyed | EAudioSourceConfigurationFlag.Static | EAudioSourceConfigurationFlag.EnvironmentSignals;
		
		if (!sourceConfig.IsValid())
			return;
		
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(sourceConfig, position);
		
		if (!audioSource)
			return;
				
		soundManager.PlayAudioSource(audioSource);
	}
}
