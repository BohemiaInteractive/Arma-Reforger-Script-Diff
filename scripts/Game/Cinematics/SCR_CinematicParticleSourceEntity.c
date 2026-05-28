[EntityEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_CinematicParticleSourceEntityClass : GenericEntityClass
{
}

class SCR_CinematicParticleSourceEntity : GenericEntity
{
	[Attribute(defvalue: "", desc: "Particle to spawn", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "ptc")]
	protected ResourceName m_sParticle;

	[Attribute(defvalue: "1", desc: "Time after which the particle effect freezes", params: "0 inf")]
	protected float m_fPauseTime;

#ifdef WORKBENCH

	protected bool m_bInitialised;
	protected float m_fDuration;

	protected ParticleEffectEntity m_ParticleEffectEntity;

	protected static ref set<ParticleEffectEntity> s_PlayingParticles = new set<ParticleEffectEntity>(); // int counting is not enough

	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		m_bInitialised = true;
		if (!m_sParticle)
			return;

		if (m_ParticleEffectEntity)
		{
			s_PlayingParticles.RemoveItem(m_ParticleEffectEntity);
			delete m_ParticleEffectEntity;
		}

		EntitySpawnParams entitySpawnParams = new EntitySpawnParams();
		entitySpawnParams.Parent = this;

		m_ParticleEffectEntity = ParticleEffectEntity.Cast(GetGame().SpawnEntity(ParticleEffectEntity, this.GetWorld(), entitySpawnParams));
		if (!m_ParticleEffectEntity)
			return;

		m_ParticleEffectEntity.SetEffectPath(m_sParticle);
		m_ParticleEffectEntity.Play();

		m_fDuration = m_fPauseTime;
		s_PlayingParticles.Insert(m_ParticleEffectEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void PauseParticle()
	{
		if (!m_ParticleEffectEntity)
			return;

		m_ParticleEffectEntity.Pause();
		s_PlayingParticles.RemoveItem(m_ParticleEffectEntity);
		s_PlayingParticles.RemoveItem(null); // this is the magic

		if (s_PlayingParticles.IsEmpty())
			Print("[SCR_CinematicParticleSourceEntity] Particle scene is ready (all particles are paused)", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetParticle()
	{
		if (!m_ParticleEffectEntity)
			return;

		m_ParticleEffectEntity.Stop();
		m_ParticleEffectEntity.Play();

		m_fDuration = m_fPauseTime;
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bInitialised)
			Init();

		m_fDuration -= timeSlice;
		if (m_fDuration <= 0 || !m_ParticleEffectEntity)
		{
			PauseParticle();
			ClearEventMask(EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	override event void _WB_SetTransform(inout vector mat[4], IEntitySource src)
	{
		if (!m_ParticleEffectEntity)
			return;

		m_ParticleEffectEntity.SetOrigin(GetOrigin());
		m_ParticleEffectEntity.SetAngles(GetAngles());
		ResetParticle(); // unfortunately, particles do not follow the emitter
	}

	//------------------------------------------------------------------------------------------------
	override event void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		if (!m_bInitialised)
			Init();
	}

	//------------------------------------------------------------------------------------------------
	override event void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (!m_bInitialised)
			Init();

		if (m_fDuration <= 0 || !m_ParticleEffectEntity) // required as _WB_GetAfterWorldUpdateSpecs is not called every time before this method
			return;

		m_fDuration -= timeSlice;
		if (m_fDuration <= 0)
			PauseParticle();
	}

	//------------------------------------------------------------------------------------------------
	override event int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_ALWAYS;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CinematicParticleSourceEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CinematicParticleSourceEntity()
	{
		if (m_ParticleEffectEntity)
		{
			s_PlayingParticles.RemoveItem(m_ParticleEffectEntity);
			delete m_ParticleEffectEntity;
		}
	}

	#endif // WORKBENCH
}
