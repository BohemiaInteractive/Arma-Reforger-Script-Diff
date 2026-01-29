class SCR_RotorDamageManagerComponentClass : SCR_DamageManagerComponentClass
{
}

class SCR_RotorDamageManagerComponent : SCR_DamageManagerComponent
{
	protected VehicleHelicopterSimulation m_HelicopterSimulation;
	protected SCR_VehicleDamageManagerComponent m_RootDamageManager;
	protected SoundComponent m_SoundComponent;
	protected SCR_RotorHitZone m_RotorHitZone;
	
	protected ParticleEffectEntity m_ParticleEffectEntity;
	protected ref ParticleEffectEntitySpawnParams m_ParticleSpawnParams;
	// Collider index for static rotor
	protected int m_iShapeIndex1 = -1;
	
	protected const float MIN_ROTOR_DAMAGE = 15.0; //everything below 15.0 gets discarded when calling HandleDamage	
	
	[Attribute(defvalue: "0.5")]
	protected float m_fImpulseDamageMultiplier;
	
	[Attribute(defvalue: "1.0")]
	protected float m_fRotorDamageMultiplier;

	[Attribute(defvalue: "1.0")]
	protected float m_fCharacterDamageMultiplier;

	[Attribute(defvalue: "10.0")]
	protected float m_fMinCharacterDamage;

	[Attribute(defvalue: "10000.0")]
	protected float m_fMaxCharacterDamage;

	[Attribute("{4AAF69CCD1AEA70E}Particles/Vehicle/Helicopter/Rotor Collision Effects/Helicopter_Rotor_Collision_Main_Solid.ptc")]
	protected ResourceName m_sDefaultParticlePath;
	
	[Attribute(defvalue: "2")]
	protected int m_iEffectIndex;
	
	[Attribute(desc: "Blade collision sound event")]
	protected string m_sImpactSoundEvent;

	//------------------------------------------------------------------------------------------------
	//! Calculate whether bullets actually hit the rotor with RotorHitChance
	override bool ShouldCountAsHit(notnull BaseDamageContext damageContext)
	{
		SCR_RotorHitZone hitZone = SCR_RotorHitZone.Cast(damageContext.struckHitZone);
		if (!hitZone)
			return true;
		
		if (damageContext.damageType == EDamageType.KINETIC && Math.RandomFloat01() > hitZone.RotorHitChance(damageContext.hitPosition))
			return false;
	
		return true;
	}
	//------------------------------------------------------------------------------------------------
	protected void DamageRotors(float damage)
	{		
		damage *= m_fRotorDamageMultiplier;
		if (damage < MIN_ROTOR_DAMAGE)
			damage = MIN_ROTOR_DAMAGE;
		
		m_RotorHitZone.HandleDamage(damage, EDamageType.COLLISION, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void DamageOther(notnull IEntity other, float damage)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(other);
		SCR_DamageManagerComponent characterDamageManager;
		if (character)
			characterDamageManager = character.GetDamageManager();

		if (!characterDamageManager || characterDamageManager.IsDestroyed())
			return;
		
		damage = Math.Clamp(damage * m_fCharacterDamageMultiplier, m_fMinCharacterDamage, m_fMaxCharacterDamage);

		Physics otherPhysics = other.GetPhysics();
		array<HitZone> characterHitZones = {};
		characterDamageManager.GetAllHitZones(characterHitZones);

		SCR_DamageContext context;
		SCR_CollisionDamageEffect collisionEffect = new SCR_CollisionDamageEffect();
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return;

		Instigator instigator = Instigator.CreateInstigator(vehicle.GetPilot());
		GameMaterial gameMaterial;
		array<int> colliderIDs = {};
		array<SurfaceProperties> surfaces = {};
		vector transformDir[4], rotorMat[4];//dir z poz rotora do charactera
		m_RotorHitZone.GetPointInfo().GetWorldTransform(rotorMat);
		vector direction;
		int nodeID;
		MeshObject meshObject = character.GetVObject().ToMeshObject();

		foreach (HitZone characterHitZone : characterHitZones)
		{
			if (!characterHitZone.HasColliderNodes() || characterHitZone.GetDamageState() == EDamageState.DESTROYED)
				continue;

			colliderIDs.Clear();
			characterHitZone.GetColliderIDs(colliderIDs);

			foreach (int colliderID : colliderIDs)
			{
				if (!m_RotorHitZone.HasCollision(otherPhysics.GetGeomWorldPosition(colliderID)))
					continue;

				surfaces.Clear();
				otherPhysics.GetGeomSurfaces(colliderID, surfaces);
				if (surfaces.IsEmpty())
					continue;

				direction = vector.Direction(rotorMat[3], otherPhysics.GetGeomWorldPosition(colliderID));
				Math3D.DirectionAndUpMatrix(direction, rotorMat[0], transformDir);
				nodeID = PhysicsUtils.GetNodeIndex(meshObject, colliderID);
				context = new SCR_DamageContext(
					EDamageType.COLLISION, damage, transformDir,
					character, characterHitZone, instigator,
					surfaces[0], colliderID, nodeID
				);

				context.damageEffect = collisionEffect;

				characterDamageManager.HandleDamage(context);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EmitRotorParticles(vector contactPos, GameMaterial contactMat)
	{
		//set particle emitter transform

		vector ownerTransform[4];
		m_RootDamageManager.GetOwner().GetWorldTransform(ownerTransform);

		vector rotorTransformLocal[4];
		m_HelicopterSimulation.RotorGetTransform(m_RotorHitZone.GetRotorIndex(), rotorTransformLocal);
		
		float rotorRadius = m_RotorHitZone.GetRotorRadius();
		float rotorRadiusSq = rotorRadius * rotorRadius;

		vector rotorEdge = vector.Right * rotorRadius;
		vector contactPositionLocal = contactPos.InvMultiply4(ownerTransform).InvMultiply4(rotorTransformLocal).Normalized() * rotorRadius;

		float distanceEdgeToContactSq = vector.DistanceSq(rotorEdge, contactPositionLocal);
		float rotorContactAngle = Math.RAD2DEG * Math.Acos((2 * rotorRadiusSq - distanceEdgeToContactSq) / (2 * rotorRadiusSq));
		if (contactPositionLocal[1] < 0) // if contact is below half-circle, add 180 degrees to complete 360 degree circle
			rotorContactAngle = 360 - rotorContactAngle;
		
		vector rotationMatrix[3];
		if (m_HelicopterSimulation.RotorGetSpinDirection(m_RotorHitZone.GetRotorIndex()) == SpinDirection.CLOCKWISE)
			Math3D.AnglesToMatrix({180, 0, rotorContactAngle}, rotationMatrix);
		else
			Math3D.AnglesToMatrix({180, 0, rotorContactAngle + 180}, rotationMatrix);

		vector resultRotation[3];
		Math3D.MatrixMultiply3(rotorTransformLocal, rotationMatrix, resultRotation);

		vector translatedTransform[4];
		Math3D.MatrixMultiply3(ownerTransform, resultRotation, translatedTransform);
		translatedTransform[3] = contactPos;

		//set particle emitter ptc
		GameMaterial material = contactMat;
		ParticleEffectInfo effectInfo = material.GetParticleEffectInfo();
		
		ResourceName resource = effectInfo.GetVehicleDustResource(m_iEffectIndex);
		if (!resource || resource.IsEmpty())
			resource = m_sDefaultParticlePath;		

		//play effect				
		m_ParticleSpawnParams.Transform = translatedTransform;
		m_ParticleEffectEntity = ParticleEffectEntity.SpawnParticleEffect(resource, m_ParticleSpawnParams);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] contactPos
	//! \param[in] contactShapeIndex1
	void PlayRotorImpactSound(vector contactPos, int contactShapeIndex1)
	{
		if (!m_SoundComponent)
			return;
		
		if (m_sImpactSoundEvent.IsEmpty())
			return; 
		
		RotorState rotorState = m_HelicopterSimulation.RotorGetState(m_RotorHitZone.GetRotorIndex());
		if (rotorState == RotorState.DESTROYED)
			return;
		
		if (rotorState == RotorState.STATIC)
		{
			int shapeIndex1 = contactShapeIndex1;
			if (m_iShapeIndex1 == shapeIndex1)
				return;
			
			m_iShapeIndex1 = shapeIndex1;
		}
		
		// Get sound position
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = contactPos;
		
		m_SoundComponent.SoundEventTransform(m_sImpactSoundEvent, mat);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)
	{
		if (!super.ShouldOverrideInstigator(currentInstigator, newInstigator))
			return false;

		DamageManagerComponent parentDamageManager = DamageManagerComponent.Cast(GetParentHitZoneContainer());
		if (parentDamageManager && newInstigator.GetInstigatorType() != InstigatorType.INSTIGATOR_NONE)
			parentDamageManager.SetInstigator(newInstigator);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		//! Disable collision damage calculations for wrecks
		if (newState != EDamageState.DESTROYED)
			SetEventMask(GetOwner(), EntityEvent.CONTACT);
		else
			ClearEventMask(GetOwner(), EntityEvent.CONTACT);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TraceFilter(notnull IEntity e)
	{
		return e != GetOwner() && e != m_RootDamageManager.GetOwner(); // ignore if traced entity is rotor or hull
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clients don't simulate physics so this rpc replicates collision information for particles, sound etc.
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_OnContactBroadcast(vector contactPos, vector contactNormal, int contactShapeIndex1)
	{					
		TraceParam trace = new TraceParam();
		trace.Start = contactPos + contactNormal;
		trace.End = contactPos - contactNormal;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		
		GetGame().GetWorld().TraceMove(trace, TraceFilter);
		
		GameMaterial contactMat = trace.SurfaceProps;
		
		EmitRotorParticles(contactPos, contactMat);
		PlayRotorImpactSound(contactPos, contactShapeIndex1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Contacts aren't handled without hitzones on rotorDamageManager
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		if (!m_RotorHitZone)
			return false;
		
		return super.FilterContact(owner, other, contact);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFilteredContact(IEntity owner, IEntity other, Contact contact)
	{			
		//compute damage
		vector rotorTransform[4];
		m_RotorHitZone.GetPointInfo().GetTransform(rotorTransform);

		// GetComponentsDelayed may not have enough time to initialise members. To avoid a VME, initialise m_HelicopterSimulation
		if(!m_HelicopterSimulation)
		{
			m_HelicopterSimulation = VehicleHelicopterSimulation.Cast(GetOwner().GetRootParent().FindComponent(VehicleHelicopterSimulation));
			if(!m_HelicopterSimulation)
				return;
		}
		
		float rotorRPM = m_HelicopterSimulation.RotorGetRPM(m_RotorHitZone.GetRotorIndex()) * 0.1;
		float rotorRadius = m_RotorHitZone.GetRotorRadius();				
		float rotorCenterToContact = vector.Distance(rotorTransform[3], contact.Position);
		float rotorRPMMultiplier = Math.Clamp(rotorCenterToContact / rotorRadius, 0, 1);
		int rotorBladeCount = m_RotorHitZone.GetBladeCount();
		
		if (rotorRadius <= 0)
			rotorRadius = 1;
		if (rotorBladeCount <= 0)
			rotorBladeCount = 1;
		
		float damage = rotorRPM * rotorRPMMultiplier / m_RotorHitZone.GetBladeCount() + contact.Impulse * m_fImpulseDamageMultiplier;
		if (damage <= 0)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(other);
		if (!character && m_RootDamageManager && contact.Impulse >= m_RootDamageManager.GetMinImpulse())
			m_RootDamageManager.ProcessCollision(new SCR_CollisionDamageContainer(GetOwner(), other, contact.Impulse, contact.VelocityBefore1, contact.VelocityAfter1, contact.VelocityBefore2, contact.VelocityAfter2, contact.Position, contact.Normal));
		
		if (m_RotorHitZone.IsSpinning())
		{
			if (character)
				DamageOther(other, damage);
			
			RPC_OnContactBroadcast(contact.Position, contact.Normal, contact.ShapeIndex1);
			Rpc(RPC_OnContactBroadcast, contact.Position, contact.Normal, contact.ShapeIndex1);
		}
		
		DamageRotors(damage);

		super.OnFilteredContact(owner, other, contact);
	}
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Call queue should not be needed there, it is because of hierarchy / slots
	//------------------------------------------------------------------------------------------------
	protected void GetComponentsDelayed()
	{
		IEntity root = GetOwner().GetRootParent();

		m_HelicopterSimulation = VehicleHelicopterSimulation.Cast(root.FindComponent(VehicleHelicopterSimulation));
		m_RootDamageManager = SCR_VehicleDamageManagerComponent.Cast(root.FindComponent(SCR_VehicleDamageManagerComponent));
		m_SoundComponent = SoundComponent.Cast(root.FindComponent(SoundComponent));
		
		m_ParticleSpawnParams = new ParticleEffectEntitySpawnParams();		
		m_ParticleSpawnParams.TransformMode = ETransformMode.WORLD;
		m_ParticleSpawnParams.UseFrameEvent = true;
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		// Do not do anything if the default hitzone is not rotor hitzone
		m_RotorHitZone = SCR_RotorHitZone.Cast(GetDefaultHitZone());
		if (!m_RotorHitZone)
			return;

		//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
		GetGame().GetCallqueue().CallLater(GetComponentsDelayed); //get components after init
		//---- REFACTOR NOTE END ----
		
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy())
			return;
		
		SetEventMask(GetOwner(), EntityEvent.CONTACT);
	}
}
