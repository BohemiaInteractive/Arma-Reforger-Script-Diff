enum SCR_EImpactSoundType
{
	DEFAULT,
	CHARACTER,
	SMALL_OBJECT
}

enum SCR_EImpactSoundEvent
{
	SOUND_VEHICLE_IMPACT_FRONT,
	SOUND_VEHICLE_IMPACT_SIDE,
	SOUND_VEHICLE_IMPACT_TOP,
	SOUND_VEHICLE_IMPACT_BOTTOM
}

class SCR_ImpactEffectComponentClass : ScriptComponentClass
{	
	[Attribute()]
	ref array<ResourceName> m_aDefaultParticles;
	
	[Attribute()]
	ref array<ResourceName> m_aWaterParticles;
	
	[Attribute(defvalue: "-1", desc: "If set to -1 component will use impulse to determine effect intensity")]
	int m_iEffectMagnitude;
}

class SCR_ImpactEffectComponent : ScriptComponent
{	
	protected ref ParticleEffectEntitySpawnParams m_ParticleSpawnParams;
	protected SoundComponent m_SoundComponent;
	protected SignalsManagerComponent m_SignalsManagerComponent;

	//Shared
	// Distance (squared) between current impact and last contact, below which impact is ignored
	protected const float IMPACT_DIST_SQ_THRESHOLD = 2;	
	// Change in velocity (m/s) above which an impact is registered
	protected const float VELOCITY_IMPACT_THRESHOLD = 0.8;
		
	protected bool m_bIsImpactEffectScheduled;
	
	protected vector m_vPosition;
	protected vector m_vPositionLast;
	protected vector m_vNormal;
	
	// Particles
	protected const float MIN_TINY_IMPULSE = 2500;
	protected const float MIN_SMALL_IMPULSE = 5000;
	protected const float MIN_MEDIUM_IMPULSE = 10000;
	protected const float MIN_BIG_IMPULSE = 20000;
	protected const float MIN_HUGE_IMPULSE = 40000;	
	protected int m_iMagnitude = -1;
	protected GameMaterial m_GameMaterial;
	
	// Sound
	protected const string IMPACT_SURFACE_SIGNAL_NAME = "ImpactSurface";
	protected const string COLLISION_D_M_SIGNAL_NAME = "CollisionDM";
	protected const float DEFAULT_DENSITY = 100;
	protected const float VEHICLE_VELOCITY_MINIMUM = 3;
	protected const float COLLISION_LAST_POSITION_REST_TIME = 500;
	protected const float CHARACTER_IMPULSE_MINIMUM = 20;
	protected const float SMALL_OBJECT_IMPULSE_MINIMUM = 10000;
	protected const float SMALL_OBJECT_VOLUME_MAXIMUM = 90;
	
	protected static const ref array<string> WATER_SOUNDS = { SCR_SoundEvent.SOUND_VEHICLE_WATER_SMALL, SCR_SoundEvent.SOUND_VEHICLE_WATER_MEDIUM, SCR_SoundEvent.SOUND_VEHICLE_WATER_BIG, SCR_SoundEvent.SOUND_VEHICLE_WATER_BIG };
	protected static const int m_aAproximatedMasses[5] = { 250, 400, 1500, 3500, 10000 };
	protected SCR_EImpactSoundType m_eImpactType;
	protected int m_iSurface;
	protected float m_fdM;
	protected float m_fMass = 2000;

	
	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> GetDefaultParticles()
	{
		return SCR_ImpactEffectComponentClass.Cast(GetComponentData(GetOwner())).m_aDefaultParticles;
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> GetWaterParticles()
	{
		return SCR_ImpactEffectComponentClass.Cast(GetComponentData(GetOwner())).m_aWaterParticles;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetEffectMagnitude()
	{
		return SCR_ImpactEffectComponentClass.Cast(GetComponentData(GetOwner())).m_iEffectMagnitude;
	}
	
	//------------------------------------------------------------------------------------------------	
	void OnImpact(notnull IEntity other, float impulse, vector impactPosition, vector impactNormal, GameMaterial mat, vector velocityBefore = vector.Zero, vector velocityAfter = vector.Zero)
	{				
		// Change in velocity due to contact
		float dV = Math.AbsFloat(vector.Dot(velocityBefore - velocityAfter, impactNormal));	

		SCR_EImpactSoundType impactSoundType = SCR_EImpactSoundType.DEFAULT;
				
		// Some types of entities may have a negligible effect on the vehicle's speed but should play an impact sound anyway - this is handled here
		if (dV < VELOCITY_IMPACT_THRESHOLD)
		{
			float velocity = velocityBefore.Length();
			
			if (velocity < VEHICLE_VELOCITY_MINIMUM)
				return;
			
			if (ChimeraCharacter.Cast(other) && impulse > CHARACTER_IMPULSE_MINIMUM)
			{
				impactSoundType = SCR_EImpactSoundType.CHARACTER;
			}
			// high impulse but negligible change in velocity = small destructible object
			else if (impulse > SMALL_OBJECT_IMPULSE_MINIMUM)
			{				
				impactSoundType = SCR_EImpactSoundType.SMALL_OBJECT;
															
				float otherMass = GetAproximatedMass(other);
				float finalMass = m_fMass + otherMass;
				if (finalMass != 0.0)
				{								
					float velocityAfterImpact = m_fMass * velocity / finalMass;
					dV = velocity - velocityAfterImpact;						
				}
				else
				{
					dV = velocity;
				}
			}
		}
		
		dV *= m_fMass;
		
		if (dV > m_fdM)
		{
			m_fdM = dV;
			m_eImpactType = impactSoundType;
			
			m_vPosition = impactPosition;
			m_vNormal = impactNormal;
			m_iSurface = mat.GetSoundInfo().GetSignalValue();
			
			m_GameMaterial = mat;
		
			UpdateParticlesMagnitude(other, impulse);
			ScheduleImpactEffect();
		}		
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnWaterEnter()
	{
		if (GetWaterParticles().IsEmpty())
			return;
		
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;

		// Only vertical axis is measured since water particles only go up
		float impulse = Math.AbsFloat(physics.GetVelocity()[1]) * m_fMass;
		
		if (impulse < MIN_TINY_IMPULSE)
			return;
		
		int magnitude = -1;	
		if (impulse < MIN_SMALL_IMPULSE)
			magnitude = 0;
		else if (impulse < MIN_MEDIUM_IMPULSE)
			magnitude = 1;
		else if (impulse < MIN_BIG_IMPULSE)
			magnitude = 2;
		else
			magnitude = 3;

		vector ownerTransform[4];
		GetOwner().GetTransform(ownerTransform);

		EWaterSurfaceType surfaceType;
		float lakeArea;
		float waterHeight = SCR_WorldTools.GetWaterSurfaceY(GetGame().GetWorld(), ownerTransform[3], surfaceType, lakeArea);

		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		vector particleOrigin = {ownerTransform[3][0], waterHeight, ownerTransform[3][2]};
		transform[3] = particleOrigin;
		
		ResourceName resourceName = GetWaterParticles()[magnitude];
		string soundEvent = WATER_SOUNDS[magnitude];	

		if (resourceName.IsEmpty())
			resourceName = GetWaterParticles()[0];

		EmitParticles(transform, resourceName);
		PlaySound(soundEvent);
		
		if (magnitude > -1)
			Rpc(RPC_OnWaterEnterBroadcast, transform, magnitude);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateParticlesMagnitude(notnull IEntity other, float impulse)
	{
		int magnitude = GetEffectMagnitude();		
		if (magnitude <= -1)
		{
			Physics physics = GetOwner().GetPhysics();
			if (physics == null)
				return;

			int responseIndex = physics.GetResponseIndex();
		
			// Exclude collisions with physics objects with tiny response index e.g. bushes, fences etc.
			if (responseIndex == SCR_EPhysicsResponseIndex.TINY_DESTRUCTIBLE || responseIndex == SCR_EPhysicsResponseIndex.SMALL_DESTRUCTIBLE)
				return;
			
			// Exclude collisions with other vehicle parts
			if (SCR_VehicleDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(other.GetRootParent())))
				return;
					
			if (impulse < MIN_TINY_IMPULSE)
				return;
		
			if (impulse < MIN_SMALL_IMPULSE)
				magnitude = 0;
			else if (impulse < MIN_MEDIUM_IMPULSE)
				magnitude = 1;
			else
				magnitude = 2;
		}
		
		if (magnitude > m_iMagnitude)
			m_iMagnitude = magnitude;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EmitParticles(vector transform[4], ResourceName particleResource)
	{
		if (!m_ParticleSpawnParams || particleResource.IsEmpty())
			return;
		
		m_ParticleSpawnParams.Transform = transform;
		ParticleEffectEntity.SpawnParticleEffect(particleResource, m_ParticleSpawnParams);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string soundEvent)
	{
		if (!m_SoundComponent || soundEvent.IsEmpty())
			return;
		
		m_SoundComponent.SoundEvent(soundEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ScheduleImpactEffect()
	{
		if (m_bIsImpactEffectScheduled)
			return;
		
		// Largest contact was cashed and will be processed only "once per frame"
		GetGame().GetCallqueue().CallLater(HandleImpactEffect, 0);
		m_bIsImpactEffectScheduled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleImpactEffect()
	{
		m_bIsImpactEffectScheduled = false;
		
		// Ignore impacts that occur within close range of the last one
		if (vector.DistanceSq(m_vPosition, m_vPositionLast) > IMPACT_DIST_SQ_THRESHOLD) 
		{	
			float collisionDM = m_fdM;			
			
			// Ignore velocity change when colliding with characters
			if (m_eImpactType == SCR_EImpactSoundType.CHARACTER)
			{
				collisionDM = -1 * m_eImpactType;
			}
							
			const SCR_EImpactSoundEvent eventIndex = GetImpactSoundEventIndex(m_vNormal);	

			PlayImpactSound(eventIndex, m_vPosition, m_iSurface, collisionDM);
			
			if (m_iMagnitude != -1)
			{
				PlayImpactParticle(m_vPosition, m_iMagnitude, m_GameMaterial);
				Rpc(RPC_OnImpactSoundAndParticlesBroadcast, m_vPosition, collisionDM, m_vNormal, m_iMagnitude);
			}
			else
			{
				Rpc(RPC_OnImpactSoundBroadcast, eventIndex, m_vPosition, m_iSurface, collisionDM);
			}
							
			// Set cashed values
			m_iMagnitude = -1;
			m_vPositionLast = m_vPosition;
			
			// Reset position last after COLLISION_LAST_POSITION_REST_TIME
			GetGame().GetCallqueue().CallLater(ResetContactLastPosition, COLLISION_LAST_POSITION_REST_TIME);
		}
		
		m_fdM = 0;
		m_bIsImpactEffectScheduled = false;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void PlayImpactParticle(vector position, int magnitude, GameMaterial material)
	{
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = position;

		ParticleEffectInfo effectInfo = material.GetParticleEffectInfo();		
		ResourceName resourceName = effectInfo.GetBlastResource(magnitude);

		if (resourceName.IsEmpty())
			resourceName = GetDefaultParticles()[magnitude];
		
		EmitParticles(transform, resourceName);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlayImpactSound(SCR_EImpactSoundEvent eventIndex, vector position, int surface, float collisionDM)
	{			
		if (!m_SignalsManagerComponent || !m_SoundComponent)
			return;

		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(IMPACT_SURFACE_SIGNAL_NAME), surface);				
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(COLLISION_D_M_SIGNAL_NAME), collisionDM);			
		m_SoundComponent.SoundEventOffset(typename.EnumToString(SCR_EImpactSoundEvent, eventIndex), GetOwner().CoordToLocal(position));
	}
		
	//------------------------------------------------------------------------------------------------
	protected float GetAproximatedMass(IEntity entity)
	{
		Physics physics = GetOwner().GetPhysics();
		if (physics == null)
			return 0;
		
		int responseIndex = physics.GetResponseIndex();
				
		if (responseIndex < SCR_EPhysicsResponseIndex.TINY_DESTRUCTIBLE || responseIndex >= SCR_EPhysicsResponseIndex.NO_COLLISION)
			return 1700;
		
		return m_aAproximatedMasses[responseIndex - SCR_EPhysicsResponseIndex.TINY_DESTRUCTIBLE];				
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetContactLastPosition()
	{
		m_vPositionLast = vector.Zero;
	}
		
	//------------------------------------------------------------------------------------------------
	SCR_EImpactSoundEvent GetImpactSoundEventIndex(vector normal)
	{
		vector normalLocal = GetOwner().VectorToLocal(normal);
		normalLocal =  normalLocal.VectorToAngles();
		
		float pitch = normalLocal[1];
	
		if (pitch > 225 && pitch <= 315)
		{
			return SCR_EImpactSoundEvent.SOUND_VEHICLE_IMPACT_TOP;
		}
		else if (pitch > 45 && pitch <= 135)
		{
			return SCR_EImpactSoundEvent.SOUND_VEHICLE_IMPACT_BOTTOM;
		}
		else
		{
			float yaw = normalLocal[0];
	
			if ((yaw > 45 && yaw <= 135) || (yaw > 225 && yaw <= 315))
			{
				return SCR_EImpactSoundEvent.SOUND_VEHICLE_IMPACT_SIDE;
			}
			
			return SCR_EImpactSoundEvent.SOUND_VEHICLE_IMPACT_FRONT;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_OnImpactParticlesBroadcast(vector contactPos, vector contactNormal, int magnitude)
	{
		if(IsPhysicActive())
			return;
		
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = contactPos;
		
		TraceParam trace = new TraceParam();
		trace.Start = contactPos + contactNormal;
		trace.End = contactPos - contactNormal;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		
		GetOwner().GetWorld().TraceMove(trace, TraceFilter);
		
		GameMaterial contactMat = trace.SurfaceProps;
		ParticleEffectInfo effectInfo = contactMat.GetParticleEffectInfo();		
		ResourceName resourceName = effectInfo.GetBlastResource(magnitude);
		
		if (resourceName.IsEmpty())
			resourceName = GetDefaultParticles()[magnitude];
		
		EmitParticles(transform, resourceName);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_OnImpactSoundBroadcast(SCR_EImpactSoundEvent eventIndex, vector impactPosition, int impactSurface, float collisionDM)
	{
		if(IsPhysicActive())
			return;
		
		PlayImpactSound(eventIndex, impactPosition, impactSurface, collisionDM);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_OnImpactSoundAndParticlesBroadcast(vector impactPosition, float collisionDM, vector contactNormal, int magnitude)
	{
		
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = impactPosition;
		
		TraceParam trace = new TraceParam();
		trace.Start = impactPosition + contactNormal;
		trace.End = impactPosition - contactNormal;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		
		GetOwner().GetWorld().TraceMove(trace, TraceFilter);
		
		GameMaterial contactMat = trace.SurfaceProps;
		ParticleEffectInfo effectInfo = contactMat.GetParticleEffectInfo();		
		ResourceName resourceName = effectInfo.GetBlastResource(magnitude);
		
		if (resourceName.IsEmpty())
			resourceName = GetDefaultParticles()[magnitude];
		
		EmitParticles(transform, resourceName);
		
		PlayImpactSound(GetImpactSoundEventIndex(contactNormal), impactPosition, contactMat.GetSoundInfo().GetSignalValue(), collisionDM);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_OnWaterEnterBroadcast(vector transform[4], int magnitude)
	{
		ResourceName resourceName = GetWaterParticles()[magnitude];
		string soundEvent = WATER_SOUNDS[magnitude];
		
		if (resourceName.IsEmpty())
			resourceName = GetWaterParticles()[0];
		
		EmitParticles(transform, resourceName);
		PlaySound(soundEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsPhysicActive()
	{
		Physics physics = GetOwner().GetPhysics();
		if (physics && physics.IsActive())
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TraceFilter(notnull IEntity e)
	{
		return e != GetOwner() && e.GetRootParent() != GetOwner(); // ignore if traced entity is vehicle or vehicle part
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_ParticleSpawnParams = new ParticleEffectEntitySpawnParams();
		m_ParticleSpawnParams.TransformMode = ETransformMode.WORLD;
		m_ParticleSpawnParams.UseFrameEvent = true;
		
		m_SoundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		
		Physics physics = owner.GetPhysics();
		if (physics)
			m_fMass = physics.GetMass();
		
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rpl || rpl.IsProxy())
			return;
		
		SCR_VehicleBuoyancyComponent buoyancyComp = SCR_VehicleBuoyancyComponent.Cast(GetOwner().FindComponent(SCR_VehicleBuoyancyComponent));
		if (!buoyancyComp)
			return;
		
		buoyancyComp.GetOnWaterEnter().Insert(OnWaterEnter);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(GetOwner(), EntityEvent.INIT);
	}
}