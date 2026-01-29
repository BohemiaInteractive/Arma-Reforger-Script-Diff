[BaseContainerProps(configRoot: true)]
class SCR_InsectParticle
{
	//! Attached sound audio handle
	SCR_AudioHandleLoop m_AudioHandle;

	//! Position of the insect particle
	vector m_vPosition;

	//! soundType, soundEventGroup, soundEventDefinition
	int m_iSoundGroup;

	//! Percentage ratio between sequence length and total length of played samples
	float m_fDensity;

	//! Attached particle effect
	ParticleEffectEntity m_ParticleEffect;
	
	//! Attached particle entity
	IEntity m_InsectEntity;

#ifdef ENABLE_DIAG
	ref Shape m_DebugShape;
#endif

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_InsectParticle()
	{
		delete m_ParticleEffect;
		delete m_InsectEntity;
	}
}
