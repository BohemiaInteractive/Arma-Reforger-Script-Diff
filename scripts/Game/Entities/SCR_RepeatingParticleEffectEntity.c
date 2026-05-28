class SCR_RepeatingParticleEffectEntityClass: ParticleEffectEntityClass
{
	[Attribute(defvalue: "1000", desc: "Delay in ms until particles will play again", params: "0 inf", category: "Loop")]
	protected int m_iLoopDelay;

	[Attribute(defvalue: "0", desc: "Max value of additional time in ms, which is going to be randomized when scheduling next particle playback", params: "0 inf", category: "Loop")]
	protected int m_iRandomDelayVariation;

	[Attribute(desc: "Configuration of the audio, which will be played when particles are triggered", category: "Sound")]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	//------------------------------------------------------------------------------------------------
	int GetLoopDelay()
	{
		return m_iLoopDelay;
	}

	//------------------------------------------------------------------------------------------------
	int GetRandomDelayVariation()
	{
		return m_iRandomDelayVariation;
	}

	//------------------------------------------------------------------------------------------------
	SCR_AudioSourceConfiguration GetAudioSourceConfiguration()
	{
		return m_AudioSourceConfiguration;
	}
}

class SCR_RepeatingParticleEffectEntity: ParticleEffectEntity
{
	//------------------------------------------------------------------------------------------------
	protected void LoopPlayback()
	{
		if (GetState() != EParticleEffectState.STOPPED)
			Stop();

		Play();

		SCR_RepeatingParticleEffectEntityClass data = SCR_RepeatingParticleEffectEntityClass.Cast(GetPrefabData());

		SCR_AudioSourceConfiguration audioConfig = data.GetAudioSourceConfiguration();
		if (audioConfig)
			SCR_SoundManagerModule.CreateAndPlayAudioSource(this, audioConfig);

		int delay = data.GetLoopDelay();
		if (delay <= 0)
			return;

		int additionalDelay = data.GetRandomDelayVariation();
		if (additionalDelay > 0)
			additionalDelay = Math.RandomInt(0, additionalDelay);

		GetGame().GetCallqueue().CallLater(LoopPlayback, delay + additionalDelay);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		LoopPlayback();
	}

	//------------------------------------------------------------------------------------------------
	protected void ParticleEffectEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
}