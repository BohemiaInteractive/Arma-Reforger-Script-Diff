//! Custom title for Workbench editor
class SCR_Spawnable_RandomParticleTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Random Particle Effect";
		return true;
	}
}

//! Random particle spawnable class that extends SCR_ParticleSpawnable
//! Enables random spawning of particle effects with sounds and deterministic synchronization in multiplayer
[BaseContainerProps(), SCR_Spawnable_RandomParticleTitle()]
class SCR_RandomParticleSpawnable : SCR_ParticleSpawnable
{
	//! Maximum duration for the spawning loop (default 60 seconds)
	[Attribute("60", UIWidgets.Slider, "Maximum duration for particle spawning loop [seconds]", "0 600 1")]
	protected float m_fMaxDuration;

	//! Minimum interval between particle spawns
	[Attribute("1", UIWidgets.Slider, "Minimum interval between particle spawns [seconds]", "0.1 30 0.1")]
	protected float m_fMinInterval;

	//! Maximum interval between particle spawns
	[Attribute("5", UIWidgets.Slider, "Maximum interval between particle spawns [seconds]", "0.1 30 0.1")]
	protected float m_fMaxInterval;

	//! Sound event to play alongside particle effects
	[Attribute("", UIWidgets.EditBox, "Sound event name to play with particles")]
	protected string m_sSoundEvent;

	//! Minimum distance from camera to trigger effects (optional)
	[Attribute("0", UIWidgets.Slider, "Minimum distance from camera to trigger effects [meters]", "0 1000 1")]
	protected float m_fTriggerDistanceMin;

	//! Maximum distance from camera to trigger effects (optional, 0 = no limit)
	[Attribute("5", UIWidgets.Slider, "Maximum distance from camera to trigger effects [meters] (0 = no limit)", "0 1000 1")]
	protected float m_fTriggerDistanceMax;

	//! Repetition count for sequences
	[Attribute("1", UIWidgets.Slider, "Number of repetitions in a sequence", "1 10 1")]
	protected int m_iRepetitionCount;

	//! Repetition count randomization
	[Attribute("0", UIWidgets.Slider, "Repetition count randomization", "0 5 1")]
	protected int m_iRepetitionCountRnd;

	//! Sequence repetition time
	[Attribute("10", UIWidgets.Slider, "Time between sequences [seconds]", "1 60 0.1")]
	protected float m_fSequenceRepetitionTime;

	//! Sequence repetition time randomization
	[Attribute("5", UIWidgets.Slider, "Sequence repetition time randomization [seconds]", "0 30 0.1")]
	protected float m_fSequenceRepetitionTimeRnd;

	// Runtime variables
	protected float m_fElapsedTime = 0;
	protected ref RandomGenerator m_prng = null;
	protected ScriptCallQueue m_CallQueue;
	protected int m_iCurrentRepCount;
	protected bool m_bInSequence = false;

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override bool CompareAttributes(SCR_BaseSpawnable other)
	{
		SCR_RandomParticleSpawnable otherRandom = SCR_RandomParticleSpawnable.Cast(other);

		if (!super.CompareAttributes(other))
			return false;

		if (!otherRandom)
			return false;

		if (otherRandom.m_fMaxDuration != m_fMaxDuration)
			return false;

		if (otherRandom.m_fMinInterval != m_fMinInterval)
			return false;

		if (otherRandom.m_fMaxInterval != m_fMaxInterval)
			return false;

		if (otherRandom.m_sSoundEvent != m_sSoundEvent)
			return false;

		if (otherRandom.m_fTriggerDistanceMin != m_fTriggerDistanceMin)
			return false;

		if (otherRandom.m_fTriggerDistanceMax != m_fTriggerDistanceMax)
			return false;

		if (otherRandom.m_iRepetitionCount != m_iRepetitionCount)
			return false;

		if (otherRandom.m_iRepetitionCountRnd != m_iRepetitionCountRnd)
			return false;

		if (otherRandom.m_fSequenceRepetitionTime != m_fSequenceRepetitionTime)
			return false;

		if (otherRandom.m_fSequenceRepetitionTimeRnd != m_fSequenceRepetitionTimeRnd)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		super.SetVariables(api, source, path, index);

		// Set all variables of the spawn object
		api.SetVariableValue(source, path, "m_fMaxDuration", m_fMaxDuration.ToString());
		api.SetVariableValue(source, path, "m_fMinInterval", m_fMinInterval.ToString());
		api.SetVariableValue(source, path, "m_fMaxInterval", m_fMaxInterval.ToString());
		api.SetVariableValue(source, path, "m_sSoundEvent", m_sSoundEvent);
		api.SetVariableValue(source, path, "m_fTriggerDistanceMin", m_fTriggerDistanceMin.ToString());
		api.SetVariableValue(source, path, "m_fTriggerDistanceMax", m_fTriggerDistanceMax.ToString());
		api.SetVariableValue(source, path, "m_iRepetitionCount", m_iRepetitionCount.ToString());
		api.SetVariableValue(source, path, "m_iRepetitionCountRnd", m_iRepetitionCountRnd.ToString());
		api.SetVariableValue(source, path, "m_fSequenceRepetitionTime", m_fSequenceRepetitionTime.ToString());
		api.SetVariableValue(source, path, "m_fSequenceRepetitionTimeRnd", m_fSequenceRepetitionTimeRnd.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_RandomParticleSpawnable", index);
			return true;
		}

		return false;
	}
#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	//! Helper method to get randomized repetition count
	protected int GetRepCount()
	{
		return Math.RandomIntInclusive(Math.Max(1, m_iRepetitionCount - m_iRepetitionCountRnd), m_iRepetitionCount + m_iRepetitionCountRnd);
	}

	//------------------------------------------------------------------------------------------------
	//! Helper method to get randomized interval time
	protected float GetRandomInterval()
	{
		if (m_fMinInterval >= m_fMaxInterval)
			return m_fMinInterval;

		return Math.RandomFloatInclusive(m_fMinInterval, m_fMaxInterval);
	}

	//------------------------------------------------------------------------------------------------
	//! Helper method to get randomized sequence repetition time
	protected float GetSequenceRepTime()
	{
		if (m_fSequenceRepetitionTimeRnd < 0.001)
			return m_fSequenceRepetitionTime;

		return Math.RandomFloatInclusive(Math.Max(0.1, m_fSequenceRepetitionTime - m_fSequenceRepetitionTimeRnd), m_fSequenceRepetitionTime + m_fSequenceRepetitionTimeRnd);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the effect should be triggered based on distance
	protected bool IsInRange(IEntity owner)
	{
		// If no distance limits are set, always trigger
		if (m_fTriggerDistanceMax <= 0 && m_fTriggerDistanceMin <= 0)
			return true;

		vector ownerTransform[4];
		vector cameraTransform[4];

		owner.GetTransform(ownerTransform);
		owner.GetWorld().GetCurrentCamera(cameraTransform);

		float distance = vector.Distance(ownerTransform[3], cameraTransform[3]);

		// Check minimum distance
		if (m_fTriggerDistanceMin > 0 && distance < m_fTriggerDistanceMin)
			return false;

		// Check maximum distance
		if (m_fTriggerDistanceMax > 0 && distance > m_fTriggerDistanceMax)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Timer callback for spawning particles and sounds
	protected void OnTimer(IEntity owner, SCR_HitInfo hitInfo)
	{
		// Check if owner still exists and hasn't been deleted
		if (!owner || owner.IsDeleted())
		{
			CleanupTimer();
			return;
		}

		// Check if we've exceeded maximum duration
		if (m_fMaxDuration > 0 && m_fElapsedTime >= m_fMaxDuration)
		{
			CleanupTimer();
			return;
		}

		// Check distance-based triggering
		if (!IsInRange(owner))
		{
			// Still schedule next timer even if out of range
			ScheduleNextTimer(owner, hitInfo);
			return;
		}

		// Spawn particle effect using parent's method
		if (hitInfo)
		{
			super.Spawn(owner, null, hitInfo, false);
		}

		// Play sound if specified
		if (!m_sSoundEvent.IsEmpty())
		{
			SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
			if (soundComponent)
			{
				soundComponent.SoundEvent(m_sSoundEvent);
			}
		}

		// Schedule next timer
		ScheduleNextTimer(owner, hitInfo);
	}

	//------------------------------------------------------------------------------------------------
	//! Schedule the next timer based on sequence logic
	protected void ScheduleNextTimer(IEntity owner, SCR_HitInfo hitInfo)
	{
		if (!m_CallQueue)
			return;

		float nextInterval;

		if (m_bInSequence && m_iCurrentRepCount > 1)
		{
			// We're in a sequence, use regular interval
			m_iCurrentRepCount--;
			nextInterval = GetRandomInterval();
		}
		else
		{
			// Start new sequence or use sequence interval
			m_iCurrentRepCount = GetRepCount();
			m_bInSequence = true;

			if (m_iCurrentRepCount > 1)
			{
				m_iCurrentRepCount--;
				nextInterval = GetRandomInterval();
			}
			else
			{
				// Single shot, use sequence time for next
				m_bInSequence = false;
				nextInterval = GetSequenceRepTime();
			}
		}

		// Update elapsed time
		m_fElapsedTime += nextInterval;

		// Schedule next call
		m_CallQueue.CallLater(OnTimer, nextInterval * 1000, false, owner, hitInfo);
	}

	//------------------------------------------------------------------------------------------------
	//! Clean up timer and reset state
	protected void CleanupTimer()
	{
		if (m_CallQueue)
		{
			m_CallQueue.Remove(OnTimer);
			m_CallQueue = null;
		}

		m_fElapsedTime = 0;
		m_iCurrentRepCount = 0;
		m_bInSequence = false;
	}


	//------------------------------------------------------------------------------------------------
	//! Override Spawn method to initialize random particle spawning
	override ParticleEffectEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (!owner || !hitInfo)
			return null;

		// Initialize pseudo random numbers generator with seed based on owner's ID for deterministic behavior
		if (!m_prng)
		{
			m_prng = new RandomGenerator;
		}
		int seed = owner.GetID();
		m_prng.SetSeed(seed);

		// Initialize timing variables
		m_fElapsedTime = 0;
		m_CallQueue = GetGame().GetCallqueue();
		m_iCurrentRepCount = GetRepCount();
		m_bInSequence = true;

		// Spawn initial particle effect
		ParticleEffectEntity initialParticle = ParticleEffectEntity.Cast(super.Spawn(owner, parentPhysics, hitInfo, snapToTerrain));

		// Play initial sound if specified
		if (!m_sSoundEvent.IsEmpty())
		{
			SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
			if (soundComponent)
			{
				vector spawnMat[4];
				GetSpawnTransform(owner, spawnMat);
				soundComponent.SoundEventTransform(m_sSoundEvent, spawnMat);
			}
		}

		// Schedule first timer for subsequent spawns
		if (m_CallQueue && m_fMaxDuration > 0)
		{
			float firstInterval;

			if (m_iCurrentRepCount > 1)
			{
				m_iCurrentRepCount--;
				firstInterval = GetRandomInterval();
			}
			else
			{
				m_bInSequence = false;
				firstInterval = GetSequenceRepTime();
			}

			m_fElapsedTime += firstInterval;
			m_CallQueue.CallLater(OnTimer, firstInterval * 1000, false, owner, hitInfo);
		}

		return initialParticle;
	}

	//------------------------------------------------------------------------------------------------
	//! Override OnDelete to clean up timers
	void OnDelete(IEntity owner)
	{
		CleanupTimer();
	}

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_RandomParticleSpawnable()
	{
		// Ensure valid interval range
		if (m_fMinInterval > m_fMaxInterval)
		{
			float temp = m_fMinInterval;
			m_fMinInterval = m_fMaxInterval;
			m_fMaxInterval = temp;
		}

		// Ensure minimum values
		if (m_fMinInterval < 0.1)
			m_fMinInterval = 0.1;

		if (m_fMaxInterval < 0.1)
			m_fMaxInterval = 0.1;
	}
}
