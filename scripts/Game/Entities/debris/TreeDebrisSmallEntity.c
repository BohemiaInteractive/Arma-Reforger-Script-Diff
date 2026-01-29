[EntityEditorProps(category: "GameScripted/Debris", description: "Entity used to represent small chunks of tree debris.", dynamicBox: true)]
class SCR_TreeDebrisSmallEntityClass: SCR_BaseDebrisSmallEntityClass
{
}

class SCR_TreeDebrisSmallEntity : SCR_BaseDebrisSmallEntity
{
	[Attribute(defvalue:"30", params:"0 inf", desc:"How long until the debris despawns (seconds)")]
	float m_fTimeToDespawn;
	
	[Attribute(defvalue:"10", params:"0 inf", desc:"Randomly increases despawn time by 0 to x (seconds)")]
	int m_iDespawnRandomizationTime;
	
	[Attribute(defvalue:"5000", params:"0 inf", desc:"Distance from camera when entity will be deleted")]
	float m_fMaxDistance;
	
	[Attribute()]
	ref SCR_AudioSourceConfiguration m_audioSourceConfigurationImpact;
	
	[Attribute()]
	ref SCR_AudioSourceConfiguration m_audioSourceConfigurationBreak;
	
	//! Position of last played sound
	protected vector m_vSoundPositionLast;
	
	//! "Scaling factor" based on entity size
	protected float m_fEntityDimensionMax;
	
	//! Minimum distance to last sound based on entitySize
	protected float m_fMinimalDistance;
	
	//! Cooldown time based on entitySize
	protected float m_fSoundCoolDownEnd;
	
	//! Sound threshold
	protected float m_fSoundThreshold;
		
	//! Stores debris impact sound
	protected AudioHandle m_AudioHandleDebrisImpact = AudioHandle.Invalid;
	
	//! Stores debris break sound
	protected AudioHandle m_AudioHandleDebrisBreak = AudioHandle.Invalid;
	
	//! Sound triggered is kinetic energy is above the threshold
	protected static const float TREE_KINETIC_ENERGY_THRESHOLD = 20;
		
	//! Debris break deletion minimum time
	protected static const float DEBRIS_BREAK_TIME_MIN = 0.7;
	
	//! Entity size factor
	protected static const float ENTITY_SIZE_FACTOR = 0.06;
			
	//------------------------------------------------------------------------------------------------
	void SCR_TreeDebrisSmallEntity(IEntitySource src, IEntity parent)
	{	
		m_fLifeTime = m_fTimeToDespawn;
		
		m_fLifeTime = m_fLifeTime + Math.RandomInt(0, m_iDespawnRandomizationTime);
		
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);		
	}
	
	//------------------------------------------------------------------------------------------------				
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_bDelete)
			return;
		
		float spdDiff = contact.GetRelativeNormalVelocityAfter() - contact.GetRelativeNormalVelocityBefore();
		
		// Play sound
		if (spdDiff > m_fSoundThreshold && vector.DistanceSq(m_vSoundPositionLast, contact.Position) >= m_fMinimalDistance && m_fAgeTime > MINIMAL_AGE && m_fAgeTime > m_fSoundCoolDownEnd)
			PlaySound(contact.Position, spdDiff);
		
		// Sound debug
#ifdef ENABLE_DIAG
		m_fdVelocity = spdDiff;
#endif // ENABLE_DIAG
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySound(vector pos, float dVelocity)
	{	
		// Get SoundManager
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(GetWorld());
		if (!soundManager)
			return;
									
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(this, m_audioSourceConfigurationImpact, pos);
		if (!audioSource)
			return;
		
		// Stop previous sound and/or debris break sound
		AudioSystem.TerminateSound(m_AudioHandleDebrisImpact);
		
		if (m_AudioHandleDebrisBreak != AudioHandle.Invalid && m_fAgeTime >= DEBRIS_BREAK_TIME_MIN)
		{
			AudioSystem.TerminateSound(m_AudioHandleDebrisBreak);
			m_AudioHandleDebrisBreak = AudioHandle.Invalid;
		}
				
		// Set signals
		audioSource.SetSignalValue(SCR_AudioSource.COLLISION_D_V_SIGNAL_NAME, dVelocity - m_fSoundThreshold);
		audioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, GetPhysics().GetMass());

		// Play sound
		soundManager.PlayAudioSource(audioSource);		
		m_AudioHandleDebrisImpact = audioSource.m_AudioHandle;
		
		// Store position of the last played sound
		m_vSoundPositionLast = pos;
		
		// Set CoolDown based on boundingbox size
		m_fSoundCoolDownEnd = m_fAgeTime + m_fEntityDimensionMax * ENTITY_SIZE_FACTOR;
		
#ifdef ENABLE_DIAG
		SoundDebugPlaySound(m_fSoundThreshold, dVelocity, pos);
#endif // ENABLE_DIAG
	}
	
	//------------------------------------------------------------------------------------------------	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		// Check if within camera range, set to delete if not.
		if (!m_bDelete)
		{
			const float distance = GetDistanceToCamera(owner.GetWorld(), owner.GetOrigin());
			if (distance >= m_fMaxDistance)
				DeleteDebris();
		}
		
#ifdef ENABLE_DIAG
		SoundDebugOnFrame(m_fSoundThreshold);
#endif // ENABLE_DIAG
	}
	
	//------------------------------------------------------------------------------------------------
	//! On initialization the debris is registered into the list.
	override void EOnInit(IEntity owner)
	{		
		// Play "debris break" sound if defined
		if (m_audioSourceConfigurationBreak && m_audioSourceConfigurationBreak.IsValid())
		{
			// Get SoundManager
			SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
			if (!soundManager)
				return;			
			
			SCR_AudioSource audioSource = soundManager.CreateAudioSource(this, m_audioSourceConfigurationBreak);
			if (!audioSource)
				return;
			
			audioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, GetPhysics().GetMass());
			soundManager.PlayAudioSource(audioSource);		
			m_AudioHandleDebrisBreak = audioSource.m_AudioHandle;	
		}		
		
		// Only activate OnContact event and proceed if the audioSourceConfiguration is valid
		if (m_audioSourceConfigurationImpact && m_audioSourceConfigurationImpact.IsValid())
		{
			SetEventMask(EntityEvent.CONTACT);
			
			// Get bounding volume, set member variables
			vector mins, maxs;
			GetBounds(mins, maxs);
			m_fEntityDimensionMax = Math.Max(maxs[0] - mins[0], Math.Max(maxs[1] - mins[1], maxs[2] - mins[2]));
			
			m_fMinimalDistance = MINIMAL_DISTANCE_SQ * m_fEntityDimensionMax;
			
			m_fSoundThreshold =  Math.Sqrt(2 * TREE_KINETIC_ENERGY_THRESHOLD / GetPhysics().GetMass());
			
			m_vSoundPositionLast = vector.Lerp(mins, maxs, 0.5);
		}
	}
	
	//------------------------------------------------------------------------------------------------				
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (!activeState)
		{
			owner.GetPhysics().ChangeSimulationState(SimulationState.COLLISION);
			ClearEventMask(EntityEvent.SIMULATE | EntityEvent.PHYSICSACTIVE);
		}
	}
}