class SCR_HeadgearInventoryItemComponentClass : InventoryItemComponentClass
{
	[Attribute(defvalue: "0.44", desc: "Kinetic Impulse Multiplier\nKinetic damage * Kinetic Impulse Multiplier = force", params: "0 inf 0.01")]
	protected float m_fKineticImpulseMultiplier;

	[Attribute(defvalue: "2.48", desc: "Explosive Impulse Multiplier\nExplosive damage * Explosive Impulse Multiplier = force", params: "0 inf 0.01")]
	protected float m_fExplosiveImpulseMultiplier;

	[Attribute(defvalue: "0.017", desc: "Melee Impulse Multiplier\nMelee damage * Melee Impulse Multiplier = force", params: "0 inf 0.01")]
	protected float m_fMeleeImpulseMultiplier;

	[Attribute(defvalue: "1", desc: "Amount of force that is required to have a chance on detaching the headgear.\nValue below 0 means that this item cannot be detached", params: "-1 inf 0.01")]
	protected float m_fForceThreshold;

	[Attribute(defvalue: "37", desc: "Chance (%) to detach the headgear when threashold is crossed,\n 0 means that there is no chance on that happening", params: "0 100")]
	protected int m_fChanceToDetach;

	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	//! \return
	float GetDamageTypeImpulseMultiplier(EDamageType type)
	{
		switch (type)
		{
			case EDamageType.KINETIC:
				return m_fKineticImpulseMultiplier;

			case EDamageType.EXPLOSIVE:
				return m_fExplosiveImpulseMultiplier;

			case EDamageType.MELEE:
				return m_fMeleeImpulseMultiplier;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetForceThreshold()
	{
		return m_fForceThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetChanceToDetach()
	{
		return m_fChanceToDetach;
	}
}

class SCR_HeadgearInventoryItemComponent : InventoryItemComponent
{
	//! Lock that is used to ensure that detaching procedure will not start twice
	protected bool m_bLock;

	//! Limit force to 10m/s or else headgear may become an interstellar body
	protected static const float FORCE_IMPULSE_LIMIT = 10;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		NwkMovementComponent networkedMovementComp = NwkMovementComponent.Cast(owner.FindComponent(NwkMovementComponent));
		if (networkedMovementComp)
			networkedMovementComp.EnableSimulation(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluates if provided damage should be capable of detaching the helmet from the character, and in such case schedules force application based on the damage amount
	//! This method should only be called by the authority
	//! \param[in] damageValue
	//! \param[in] damageType
	//! \param[in] impactDirection
	//! \param[in] impactPosition
	//! \param[in] character who is wearing the headgear
	void DetachHelmet(float damageValue, EDamageType damageType, vector impactDirection, vector impactPosition, notnull ChimeraCharacter character)
	{
		if (m_bLock)
			return;

		IEntity hat = GetOwner();
		if (!hat)
			return;

		SCR_HeadgearInventoryItemComponentClass data = SCR_HeadgearInventoryItemComponentClass.Cast(GetComponentData(hat));
		if (!data)
			return;

		CompartmentAccessComponent compartmentAccessComp = character.GetCompartmentAccessComponent();
		if (!compartmentAccessComp)
			return;

		BaseCompartmentSlot compartment = compartmentAccessComp.GetCompartment();
		if (compartment)
			return;//dont do that in the vehicle

		float threshold = data.GetForceThreshold();
		if (threshold < 0)
			return;

		int probability = data.GetChanceToDetach();
		if (probability <= 0)
			return;

		Physics hatPhysics = hat.GetPhysics();
		if (!hatPhysics || !hatPhysics.IsDynamic())
			return;

		float force = damageValue * data.GetDamageTypeImpulseMultiplier(damageType);
		if (force < threshold)
			return;

		int probablilityTest = Math.RandomIntInclusive(0, 100);
		if (probablilityTest == 0 || probablilityTest > probability)
			return;

		m_bLock = true;
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent storageMgr = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!storageMgr)
			return;

		SCR_CharacterInventoryStorageComponent characterStorage = storageMgr.GetCharacterStorage();
		if (!characterStorage)
			return;

		force = Math.Min(force, FORCE_IMPULSE_LIMIT);

		hat.ClearFlags(EntityFlags.TRACEABLE);
		SCR_ForceImpulseCallback callback = new SCR_ForceImpulseCallback();
		callback.m_vPosition = character.EyePosition();
		callback.m_vForceOffset = callback.m_vPosition - impactPosition;
		callback.m_vForceImpulse = impactDirection * force + vector.Up * 3; // up*3 to add a bit of upward force to make it seem like its jumping of the head
		callback.m_ItemComponent = this;
		storageMgr.TryRemoveItemFromStorage(hat, characterStorage, callback);
	}

	//------------------------------------------------------------------------------------------------
	//! Applies the force to the entity and enables synchronization of the physics simulation.
	//! Should be only executed by the authority.
	//! \param[in] positionFrom
	//! \param[in] offset
	//! \param[in] force
	void ApplyForce(vector positionFrom, vector offset, vector force)
	{
		m_bLock = false;
		IEntity hat = GetOwner();
		NwkMovementComponent networkedMovementComp = NwkMovementComponent.Cast(hat.FindComponent(NwkMovementComponent));
		if (!networkedMovementComp)
			return;

		Physics phys = hat.GetPhysics();
		if (!phys)
			return;

		// start networked synchronization of the moving helmet
		networkedMovementComp.EnableSimulation(true);
		hat.SetOrigin(positionFrom);
		hat.Update();

		phys.ChangeSimulationState(SimulationState.SIMULATION);
		phys.ApplyImpulse(force);
		phys.ApplyImpulseAt(offset, force * 0.0002); // tiny amount of offset force to add a bit of rotation
		SCR_PhysicsObserverSystem system = SCR_PhysicsObserverSystem.GetInstance();
		if (system)
			system.RegisterEntity(hat, this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] isActive
	void OnPhysicsStateChanged(bool isActive)
	{
		if (isActive)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		Physics phys = owner.GetPhysics();
		if (!phys)
			return;

		SCR_PhysicsObserverSystem system = SCR_PhysicsObserverSystem.GetInstance();
		if (system)
			system.UnregisterEntity(owner);

		phys.ChangeSimulationState(SimulationState.NONE);
		NwkMovementComponent networkedMovementComp = NwkMovementComponent.Cast(owner.FindComponent(NwkMovementComponent));
		if (networkedMovementComp)
			networkedMovementComp.EnableSimulation(false);

		owner.SetFlags(EntityFlags.TRACEABLE);
		// force snap to ground to ensure that everyone will see this item in the same place
		PlaceOnGround();
	}
}