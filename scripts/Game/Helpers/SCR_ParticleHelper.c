class SCR_ParticleHelper
{
	protected static const float MINIMAL_LV = -20;

	//------------------------------------------------------------------------------------------------
	//! Stop emitter from producing new particles and turn off its light
	//! \param[in] particleEntity
	//! \param[in] lightEmitterID ID of the emitter whose light will be dimmed where order is based on the order in which they are in particle editor (starting from 0) and if -1 then script will dim all emitters
	static void StopParticleEmissionAndLights(notnull ParticleEffectEntity particleEntity, int lightEmitterID = 0)
	{
		particleEntity.StopEmission();
		Particles particles = particleEntity.GetParticles();
		if (!particles)
			return;

		if (lightEmitterID >= 0)
		{
			particles.SetParam(lightEmitterID, EmitterParam.MAX_LV, MINIMAL_LV);
			return;
		}

		for (int i, count = particles.GetNumEmitters(); i < count; i++)
		{
			particles.SetParam(i, EmitterParam.MAX_LV, MINIMAL_LV);
		}
	}
}
