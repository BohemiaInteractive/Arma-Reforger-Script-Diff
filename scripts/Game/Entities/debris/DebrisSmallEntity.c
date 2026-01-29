[EntityEditorProps(category: "GameScripted/Debris", description: "Entity used to represent small chunks of debris. Automatically managed.", dynamicBox: true)]
class SCR_DebrisSmallEntityClass: SCR_BaseDebrisSmallEntityClass
{
}

//! Entity used to represent small debris in the world. Is managed automatically and will never
//! allow the user to exceed the specified maximum amount. If new spawn is requested, priority
//! is evaluated. If the new debris has higher priority than any of the debris spawned before,
//! it will be replaced by the new one.
class SCR_DebrisSmallEntity : SCR_BaseDebrisSmallEntity
{
#ifdef ENABLE_BASE_DESTRUCTION
	//! The maximum limit of small debris in the scene at any given time. This limit is never exceeded.
	private static int s_iDebrisMaximumCount = 1000;

	//! List of all the small debris in the world.
	private static ref array<SCR_DebrisSmallEntity> s_aDebrisSmallList = null;

	//! The priority of this debris. Higher priority debris will replace lower priority if limit is reached.
	private int m_iPriority;

	//! The maximum distance from the camera in metres. If camera is beyond this distance, debris will despawn.
	private float m_fMaxDistance;

	//! Position of last played sound
	protected vector m_vSoundPositionLast;
	
	//! Sound threshold
	protected float m_fSoundThreshold;
	
	//! Stores last sound
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	//! Material type of item for sound
	protected SCR_EMaterialSoundTypeDebris m_eMaterialSoundType;

	//! Entity spawned only outside
	protected bool m_bExteriorSource;
	
	//! Count of debris spawned this frame
	static private int s_iSpawnedThisFrame;

	//! Max debris per frame s_iSpawnedThisFrame
	static private int s_iDebrisPerFrameLimit = 128;
			
	//------------------------------------------------------------------------------------------------
	//! Register debris into the list.
	protected void RegisterDebris()
	{
		if (!IsGamePlaying())
			return;
		
		// If array is not created, create it
		if (!s_aDebrisSmallList)
			s_aDebrisSmallList = {};

		// Insert element into the list
		if (s_aDebrisSmallList)
		{
			s_aDebrisSmallList.Insert(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregister debris from the list.
	private void UnregisterDebris()
	{
		if (!IsGamePlaying())
			return;
		
		// If the array exists only
		if (s_aDebrisSmallList)
		{
			// Remove the element, resize the array
			// If no debris is left in the list, delete it
			int idx = s_aDebrisSmallList.Find(this);
			int count = s_aDebrisSmallList.Count();
			if (idx >= 0)
			{
				s_aDebrisSmallList.Remove(idx);
				s_aDebrisSmallList.Resize(count-1);
				
				if (s_aDebrisSmallList.Count() <= 0)
				{
					s_aDebrisSmallList = null;
				}				
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------				
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_bDelete)
			return;
									
		float spdDiff = contact.GetRelativeNormalVelocityAfter() - contact.GetRelativeNormalVelocityBefore();
		
		// Play sound
		if (spdDiff > m_fSoundThreshold && vector.DistanceSq(m_vSoundPositionLast, contact.Position) >= MINIMAL_DISTANCE_SQ && m_eMaterialSoundType != 0 && m_fAgeTime > MINIMAL_AGE)
			PlaySound(contact.Position, spdDiff);
		
		// Sound debug
#ifdef ENABLE_DIAG
		m_fdVelocity = spdDiff;
#endif // ENABLE_DIAG
		
		if (spdDiff < 20)
			return;
		
		DeleteDebris();
	}
		
	//------------------------------------------------------------------------------------------------
	void PlaySound(vector pos, float dVelocity)
	{		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(GetWorld());
		if (!soundManager)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = destructionManager.GetAudioSourceConfiguration();
		if (!audioSourceConfiguration)
			return;
		
		// Override InteriorSignal flag
		if (m_bExteriorSource)
			audioSourceConfiguration.m_eFlags = SCR_Enum.SetFlag(audioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.ExteriorSource);
		else
			audioSourceConfiguration.m_eFlags = SCR_Enum.RemoveFlag(audioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.ExteriorSource);
		
		
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_MPD_ + typename.EnumToString(SCR_EMaterialSoundTypeDebris, m_eMaterialSoundType);					
		
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(this, audioSourceConfiguration, pos);
		if (!audioSource)
			return;
		
		// Stop previous sound
		AudioSystem.TerminateSound(m_AudioHandle);
		
		// Set signals
		audioSource.SetSignalValue(SCR_AudioSource.COLLISION_D_V_SIGNAL_NAME, dVelocity - m_fSoundThreshold);
		audioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, GetPhysics().GetMass());
							
		// Play sound
		soundManager.PlayAudioSource(audioSource);		
		m_AudioHandle = audioSource.m_AudioHandle;	
		
		// Store position of the last played sound
		m_vSoundPositionLast = pos;
		
		// Sound Debug
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
		
		// Sound debug
#ifdef ENABLE_DIAG
		SoundDebugOnFrame(m_fSoundThreshold);
#endif // ENABLE_DIAG
	}
		
	//------------------------------------------------------------------------------------------------
	//! Method for spawning debris. All is handled internally.
	//! \param mat Transformation matrix for the debris.
	//! \param model Model that will be used by the newly spawned debris.
	//! \param mass The mass in kilos the debris' physics will have.
	//! \param lifeTime The maximum lifetime of this debris in seconds.
	//! \param priority The priority of this debris. Higher priority overrides lower priority if at or over debris limit.
	//! \param linearVelocity Linear velocity of the debris (in m/s)
	//! \param angularVelocity Angular velocity of the debris (in deg/s)
	//! \param remap The materials to be remapped to, see SetObject() for more info
	static SCR_DebrisSmallEntity SpawnDebris(BaseWorld world, vector mat[4], ResourceName model, float mass = 10, float lifeTime = 10.0, float maxDistance = 256.0, int priority = 1, vector linearVelocity = "0 0 0", vector angularVelocity = "0 0 0", string remap = "", bool isStatic = false, SCR_EMaterialSoundTypeDebris materialSoundType = 0, bool exteriorSource = false)
	{
		if (s_iSpawnedThisFrame >= s_iDebrisPerFrameLimit)
			return null;
		
		// Check if model is valid
		if (model == string.Empty)
			return null;
		
		// Check if model is not being spawned out of its range
		if (GetDistanceToCamera(world, mat[3]) > maxDistance)
			return null;
		
		SCR_DebrisSmallEntity entity = null;		
		
		// See if this is first entity or not, if so, create the list
		if (!s_aDebrisSmallList)
			s_aDebrisSmallList = {};
		
		// If the list exists, check count. If over limit, replate debris with lower priority by this one.
		if (s_aDebrisSmallList)
		{
			int count = s_aDebrisSmallList.Count();
			// Over the limit
			if (count >= s_iDebrisMaximumCount)
			{
				foreach(SCR_DebrisSmallEntity debrisSmallEntity : s_aDebrisSmallList)
				{	
					if (debrisSmallEntity)
					{
						if (debrisSmallEntity.m_iPriority < priority)
						{
							entity = debrisSmallEntity;
							break;
						}
					}
				}
				// TODO: Try out if this impacts performance or not (or how heavily)
				if (entity)
				{
					Physics entityPhysics = entity.GetPhysics();
					if (entityPhysics)
						entityPhysics.Destroy();
				}
			}
			// Below the limit, spawn new one.
			else
			{
				entity = SCR_DebrisSmallEntity.Cast(GetGame().SpawnEntity(SCR_DebrisSmallEntity));
			}
		}
	
		// Spawning has failed? There was no possible replacement? 
		if (!entity)
			return null; 
		
		s_iSpawnedThisFrame++;
		// Set newly spawned entity (or the one being reused)'s data to the new
		entity.SetTransform(mat);
		Resource resource = Resource.Load(model);
		if (!resource)
			return null;
		
		BaseResourceObject baseRes = resource.GetResource();
		if (!baseRes)
			return null;
		
		VObject obj = baseRes.ToVObject();
		entity.SetObject(obj, remap);
		
		entity.m_fLifeTime = lifeTime;
		entity.m_iPriority = priority;
		entity.m_fMaxDistance = maxDistance;
		entity.m_eMaterialSoundType = materialSoundType;
		
		// Store sound parameters	
		entity.m_fSoundThreshold =  Math.Sqrt(2 * KINETIC_ENERGY_THRESHOLD / mass);
		
		vector mins, maxs;
		entity.GetWorldBounds(mins, maxs);			
		entity.m_vSoundPositionLast = vector.Lerp(mins, maxs, 0.5);	
					
		entity.m_bExteriorSource = exteriorSource;
		
		// Set physics
		Physics entityPhysics = entity.GetPhysics();
		if (!entityPhysics)
		{
			if (isStatic)
			{
				entityPhysics = Physics.CreateStatic(entity, -1);
			}
			else
			{
				entityPhysics = Physics.CreateDynamic(entity, mass, -1);
				if (entityPhysics)
				{
					//hotfix for debris getting stuck in terrain causing low fps
					vector entityOrigin = entity.GetOrigin();
					float terrainYMins = GetGame().GetWorld().GetSurfaceY(mins[0], mins[2]);	
					float terrainYMaxs = GetGame().GetWorld().GetSurfaceY(maxs[0], maxs[2]);
				
					if ((mins[1] < terrainYMins || mins[1] < terrainYMaxs) && (maxs[1] > terrainYMins || maxs[1] > terrainYMaxs))
					{
						float highestTerrainY;
						if (terrainYMaxs > terrainYMins)
							highestTerrainY = terrainYMaxs;
						else
							highestTerrainY = terrainYMins;
				
						float newHeight = highestTerrainY - mins[1] + entityOrigin[1] + 0.1;
						entity.SetOrigin({entityOrigin[0], newHeight, entityOrigin[2]});
					}
					
					entityPhysics.SetVelocity(linearVelocity);
					entityPhysics.SetAngularVelocity(angularVelocity * Math.DEG2RAD);
				}
			}

			if (entityPhysics)
				entityPhysics.SetInteractionLayer(EPhysicsLayerDefs.Debris);
		}
		
		return entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delete random debris from all existing debris.
	//1 \param count The amount of debris to delete.
	static void DeleteRandomDebris(int count = 1)
	{
		for (int i = 0; i < count; i++)
		{
			if (s_aDebrisSmallList)
			{
				if (s_aDebrisSmallList.Count() > 0)
				{
					SCR_DebrisSmallEntity ent = s_aDebrisSmallList.GetRandomElement();
					if (ent)
					{
						ent.DeleteDebris();
					}
				}
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Delete debris - unregisters it from the list and makes it scale down and delete.
	override void DeleteDebris()
	{
		UnregisterDebris();
		super.DeleteDebris();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		s_iSpawnedThisFrame = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! On initialization the debris is registered into the list.
	override void EOnInit(IEntity owner)
	{
		RegisterDebris();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DebrisSmallEntity(IEntitySource src, IEntity parent)
	{	
		SetEventMask(EntityEvent.INIT | EntityEvent.POSTFRAME | EntityEvent.CONTACT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DebrisSmallEntity()
	{
		UnregisterDebris();
	}
#endif // ENABLE_BASE_DESTRUCTION
}
