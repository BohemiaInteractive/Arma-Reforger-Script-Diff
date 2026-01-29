[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_MotorExhaustEffectGeneralComponentClass : MotorExhaustEffectComponentClass
{
}

class SCR_MotorExhaustEffectGeneralComponent : MotorExhaustEffectComponent
{
//	protected Particles								m_ExhaustParticles;
	protected ParticleEffectEntity					m_ExhaustEmitter;
	protected ParticleEffectEntity					m_DamagedEmitter;
	protected SCR_VehicleDamageManagerComponent		m_DamageManager;
	protected SignalsManagerComponent				m_SignalsManager;
	protected float									m_fRPMScaled; // 0 to 1 scale of RPM between current and max RPM
	protected float									m_fRPMScaledOld; // RPM scale in previous frame to calculate acceleration
	protected float									m_fThrust; // 0 to 1. Represents how much pedal to the metal is the driver pushing.
	protected float									m_fEngineLoad; // How much load is the engine being pushed through
	protected float									m_fPreviousLoad = -1; // Previous load state to detect necessary change
	protected float									m_fLifetimeScale; // Lifetime of the exhaust particles
	protected int									m_iIsExhaustUnderWaterSignalIdx; // IsExhaustUnderWater signal index
	protected int									m_iEngineLoadIdx; // engine load singal index
	protected bool									m_bIsUnderwater;
	protected bool									m_bIsDefective;

	// There are some heavy calculations with particles going on per frame. The following variables solve the performance impact by calling them less often when they happen far away from the camera.
	protected float								m_fUpdateDelay; // Desired delay between each particle calculations in seconds. 0 means per frame.
	protected float								m_fCurrentUpdateDelay; // The time of current delay
	protected const float						TICK_TIME_DELAY_MAX = 2; // Max acceptable delay in seconds
	protected const float						TICK_DELAY_RANGE_START = 15; // Starting range at which the delay begins to increase (from 0 to TICK_TIME_DELAY_MAX)
	protected const float						TICK_DELAY_RANGE_END = 100; // End range at which delay reaches its maximum (TICK_TIME_DELAY_MAX)

	// Arrays of arrays of emitter indices (one array of indices per stage).
	// E.g. m_aExhaustStageEmitters[1][0] gets emitter index of the emitter #0 of stage #1
	protected ref array<ref array<int>>		m_aExhaustStagesEmitters;
	protected ref array<ref array<int>>		m_aDamageStagesEmitters;

	protected float								m_fStartupTimeLeft; // Remaining time to keep playing the startup particle effect after startup

	[Attribute("{AEA751F0BE7FE821}Particles/Vehicle/Vehicle_smoke_car_exhaust_damage.ptc", UIWidgets.ResourceNamePicker, desc: "Particle effect for damaged engine", params: "ptc")]
	protected ResourceName						m_sDamagedParticle;

	[Attribute("1", UIWidgets.Auto, desc: "Time to play damaged exhaust particle after startup")]
	protected float								m_fStartupTime;

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		if (System.IsConsoleApp())
		{
			Deactivate(owner);
			return;
		}

		m_DamageManager = SCR_VehicleDamageManagerComponent.Cast(owner.FindComponent(SCR_VehicleDamageManagerComponent));
		m_SignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));

		if (m_SignalsManager)
		{
			m_iIsExhaustUnderWaterSignalIdx = m_SignalsManager.AddOrFindSignal("IsExhaustUnderWater");
			m_iEngineLoadIdx = m_SignalsManager.AddOrFindSignal("engineLoad");
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromMotorExhaustSystem();

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	// Parses an emitter name and returns index of a stage the emitter belongs to.
	// Returns -1 if the emitter's name does not meet the stage naming WeatherWindPattern
	// (s<X>_<name> where <X> is a number of a stage and <name> is any string, e.g. s0_smoke, s1_smoke, s10_debris etc.)
	protected static int GetStageIndex(string emitterName)
	{
		static const string EMITTER_STAGE_PREFIX = "s";
		static const string EMITTER_STAGE_SUFIX = "_";

		int offset = EMITTER_STAGE_PREFIX.Length();
		if (!emitterName.StartsWith(EMITTER_STAGE_PREFIX) || !emitterName.IsDigitAt(offset))
			return -1;

		int numberLen;
		int stageNo = emitterName.ToInt(offset: offset, parsed: numberLen);
		offset += numberLen;
		if (!emitterName.ContainsAt(EMITTER_STAGE_SUFIX, offset))
			return -1;

		return stageNo;
	}

	//------------------------------------------------------------------------------------------------
	// Creates an index of emitters in stages of Particles of ParticleEffectEntity,
	// i.e. array of arrays of emitter indices - one array of emitter indices for each stage.
	// E.g. for stageEmitters = CreateStageIndex(effectEntity),
	// at stageEmitters[2] there is a an array of indices of emitters belonging to the the stage #2.
	protected static array<ref array<int>> CreateStageIndexes(notnull ParticleEffectEntity effectEntity)
	{
		Particles particles = effectEntity.GetParticles();
		if (!particles)
			return {};

		array<ref array<int>> ret = {{}};

		array<string> emitterNames = {};
		int emitterNamesCount = particles.GetEmitterNames(emitterNames);

		for (int i; i < emitterNamesCount; i++)
		{
			int stage = GetStageIndex(emitterNames[i]);
			if (stage < 0)
				continue;

			if (stage >= ret.Count())
				ret.Resize(stage + 1);

			if (!ret[stage])
				ret[stage] = {};

			ret[stage].Insert(i);
		}

		return ret;
	}

	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		// Check if delay treshold was crossed. This prevents calling intense calculations per frame for vehicles which are far away from camera as they have low priority
		m_fCurrentUpdateDelay += timeSlice;
		if (m_fCurrentUpdateDelay < m_fUpdateDelay)
			return;

		m_fCurrentUpdateDelay = 0;

		// Check if effect exists. If not then try to get it.
		if (!m_ExhaustEmitter)
			m_ExhaustEmitter = ParticleEffectEntity.Cast(GetParticleEntity());

		if (!m_ExhaustEmitter)
			return;

		// Check if exhaust is underwater
		bool isUnderwater = SCR_WorldTools.IsObjectUnderwater(m_ExhaustEmitter);
		if (m_bIsUnderwater != isUnderwater)
		{
			m_bIsUnderwater = isUnderwater;
			m_SignalsManager.SetSignalValue(m_iIsExhaustUnderWaterSignalIdx, isUnderwater);
		}

		// Check if engine is damaged
		m_bIsDefective = !isUnderwater && m_DamageManager && m_DamageManager.GetEngineEfficiency() <= m_DamageManager.GetEngineMalfunctionThreshold();

		// Update emitters
		UpdateExhaustEmitter(timeSlice);

		if (m_bIsDefective || m_DamagedEmitter)
			UpdateDamagedEmitter(timeSlice);

		// Calculate distance between owner and camera
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		if (!camera)
			return;

		float distance = vector.Distance(camera.GetOrigin(), m_ExhaustEmitter.GetOrigin());

		// Prolong update interval based on the distance between camera and effect
		if (distance > TICK_DELAY_RANGE_START)
			m_fUpdateDelay = TICK_TIME_DELAY_MAX * (Math.Clamp((distance - TICK_DELAY_RANGE_START) / TICK_DELAY_RANGE_END, 0, 1));
		else
			m_fUpdateDelay = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateExhaustParticles(float timeSlice)
	{
		// We are about to call relatively performance heavy update of particle in OnUpdateEffect(...). This will be done per frame but only if the effect is very close to camera. Otherwise the update will be separated by time intervals to save performance.
		m_fRPMScaled = GetRpmScaled();
		if (m_fRPMScaled > 0 || m_fPreviousLoad < 0)
		{
			// Prepare values which will be used for particle calculations later in OnUpdateEffect(...)
			m_fThrust = GetSignalThrust();
			float RPM_acceleration = (m_fRPMScaled - m_fRPMScaledOld) * 1000 * timeSlice;
			m_fRPMScaledOld = m_fRPMScaled;
		}

		// Startup force full engine load
		if (m_fStartupTimeLeft > 0)
			m_fEngineLoad = 1;
		else if (m_SignalsManager)
			m_fEngineLoad = Math.Max(m_SignalsManager.GetSignalValue(m_iEngineLoadIdx), 0);

		if (float.AlmostEqual(m_fEngineLoad, m_fPreviousLoad))
			return;

		m_fPreviousLoad = m_fEngineLoad;

		// Update exhaust stages
		if (!m_aExhaustStagesEmitters)
			m_aExhaustStagesEmitters = CreateStageIndexes(m_ExhaustEmitter);

		AdjustEngineEffects(m_ExhaustEmitter, m_aExhaustStagesEmitters);
	}

	//------------------------------------------------------------------------------------------------
	//! Ignition engine exhaust
	protected void UpdateExhaustEmitter(float timeSlice)
	{
		bool shouldBePlaying = !m_bIsUnderwater && !(m_bIsDefective && m_DamagedEmitter);
		bool isPlaying = m_ExhaustEmitter && m_ExhaustEmitter.GetState() == EParticleEffectState.PLAYING;

		// Enable only if no damage particles are being played
		if (shouldBePlaying)
		{
			if (!isPlaying)
				m_ExhaustEmitter.Play();

			UpdateExhaustParticles(timeSlice);
		}
		else
		{
			// Pause so that emitter does not get deleted until engine is stopped
			if (isPlaying)
				m_ExhaustEmitter.Pause();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Damaged engine exhaust
	protected void UpdateDamagedEmitter(float timeSlice)
	{
		bool isEmitting = m_DamagedEmitter && m_DamagedEmitter.GetState() == EParticleEffectState.PLAYING;
		if (m_bIsDefective == isEmitting)
			return;

		if (m_bIsDefective)
		{
			if (m_DamagedEmitter)
				m_DamagedEmitter.Play();
			else if (!m_sDamagedParticle.IsEmpty())
				CreateDamageEffect();
		}
		else
		{
			if (m_DamagedEmitter)
				m_DamagedEmitter.StopEmission();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create damaged exhaust particles
	protected void CreateDamageEffect()
	{
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();

		EntitySlotInfo effectPosition = GetEffectPosition();
		if (effectPosition)
		{
			spawnParams.PivotID = effectPosition.GetNodeId();
			effectPosition.GetLocalTransform(spawnParams.Transform);
		}

		spawnParams.UseFrameEvent = true;
		spawnParams.Parent = GetOwner();

		m_DamagedEmitter = ParticleEffectEntity.SpawnParticleEffect(m_sDamagedParticle, spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	protected void AdjustEngineEffects(notnull ParticleEffectEntity effectEntity, array<ref array<int>> stageIndexes)
	{
		Particles particles = effectEntity.GetParticles();
		if (!particles)
			return;

		if (stageIndexes && !stageIndexes.IsEmpty()) // Check if the assigned particle effect supports staging (Staging divides effect's emittors into groups which are selectively enabled/disabled according to our needs)
		{
			// Staging is supported, so disable all emittors...
			particles.SetParam(-1, EmitterParam.BIRTH_RATE, 0.0);
			particles.SetParam(-1, EmitterParam.BIRTH_RATE_RND, 0.0);

			// ... now enable only the relevant emittors and work with them
			int iMaxStage = stageIndexes.Count();
			int stage = Math.ClampInt(Math.Ceil(iMaxStage * m_fEngineLoad), 1, iMaxStage) - 1;
			array<int> stageEmitterIDs = stageIndexes[stage];

			// Now we work only with the relevant stage (group) of emitters
			for (int i; i < stageEmitterIDs.Count(); i++)
			{
				particles.MultParam(stageEmitterIDs[i], EmitterParam.BIRTH_RATE, m_fRPMScaled* 0.5 + 0.5);
				particles.MultParam(stageEmitterIDs[i], EmitterParam.BIRTH_RATE_RND, m_fRPMScaled* 0.5 + 0.5);
				particles.MultParam(stageEmitterIDs[i], EmitterParam.VELOCITY, (m_fRPMScaled* 3));
				particles.MultParam(stageEmitterIDs[i], EmitterParam.VELOCITY_RND, (m_fRPMScaled* 3));
				particles.MultParam(stageEmitterIDs[i], EmitterParam.AIR_RESISTANCE, m_fRPMScaled);
				particles.MultParam(stageEmitterIDs[i], EmitterParam.AIR_RESISTANCE_RND, m_fRPMScaled);
			}
		}
		else
		{
			// Staging is not supported so apply changes to all emittors.
			particles.MultParam(-1, EmitterParam.BIRTH_RATE, m_fRPMScaled* 1);
			particles.MultParam(-1, EmitterParam.BIRTH_RATE_RND, m_fRPMScaled* 1);
			particles.MultParam(-1, EmitterParam.LIFETIME, m_fLifetimeScale);
			particles.MultParam(-1, EmitterParam.LIFETIME_RND, m_fLifetimeScale);
			particles.MultParam(-1, EmitterParam.VELOCITY, (m_fRPMScaled* 7));
			particles.MultParam(-1, EmitterParam.VELOCITY_RND, (m_fRPMScaled* 7));
			particles.MultParam(-1, EmitterParam.AIR_RESISTANCE, m_fRPMScaled);
			particles.MultParam(-1, EmitterParam.AIR_RESISTANCE_RND, m_fRPMScaled);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Turn on the effect for engine start and reset flow variables
	void OnEngineStart(bool startup)
	{
		m_fCurrentUpdateDelay = 0;
		m_fPreviousLoad = -1;
		m_bIsUnderwater = false;

		if (startup)
			m_fStartupTimeLeft = m_fStartupTime;
		else
			m_fStartupTimeLeft = 0;

		TurnOn(GetOwner());
		ConnectToMotorExhaustSystem();
	}

	//------------------------------------------------------------------------------------------------
	//! Turn off the effect and pause the damaged exhaust effect
	void OnEngineStop()
	{
		TurnOff();
		DisconnectFromMotorExhaustSystem();

		if (m_DamagedEmitter)
			m_DamagedEmitter.StopEmission();
	}

	//------------------------------------------------------------------------------------------------
	protected void ConnectToMotorExhaustSystem()
	{
		World world = GetOwner().GetWorld();
		MotorExhaustSystem updateSystem = MotorExhaustSystem.Cast(world.FindSystem(MotorExhaustSystem));
		if (!updateSystem)
			return;

		updateSystem.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromMotorExhaustSystem()
	{
		World world = GetOwner().GetWorld();
		MotorExhaustSystem updateSystem = MotorExhaustSystem.Cast(world.FindSystem(MotorExhaustSystem));
		if (!updateSystem)
			return;

		updateSystem.Unregister(this);
	}
}
