enum ETurretState
{
	NONE,
	DESTROYED,
	DISASSEMBLED,
	ASSEMBLED,
};

class SCR_TurretAssemblyAction : ScriptedUserAction
{
	private IEntity m_Owner;
	private ETurretState m_CurrentState;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	//! INV_TODO: Refactor
	override bool CanBePerformedScript(IEntity user)
	{
		m_CurrentState = ETurretState.NONE;
		return false;
		/*
		// Check if user has inventory
		SCR_InventoryManagerComponent userInventory = SCR_InventoryManagerComponent.Cast(user.FindComponent(SCR_InventoryManagerComponent));
		if (!userInventory)
			return false;

		GenericEntity genOwner = GenericEntity.Cast(m_Owner);

		// Check if turret has an occupant
		auto controller = TurretControllerComponent.Cast(genOwner.FindComponent(TurretControllerComponent));
		if (!controller)
			return false;
		auto compartment = controller.GetCompartmentSlot();
		if (compartment && compartment.GetOccupant())
			return false;

		// Check if destroyed
		auto damageManagerComponent = DamageManagerComponent.Cast(genOwner.FindComponent(DamageManagerComponent));
		if (damageManagerComponent && damageManagerComponent.GetState() != EDamageState.ALIVE)
		{
			m_CurrentState = ETurretState.DESTROYED;
			return false;
		}

		// Disassembly
		bool turretHasWeapon = false;
		bool turretHasSights = false;

		// Check Turret Weapon
		auto genericWeaponSlotComponent = genOwner.FindComponent(WeaponSlotComponent);
		if (!genericWeaponSlotComponent)
			return false;
		auto weaponSlotComponent = WeaponSlotComponent.Cast(genericWeaponSlotComponent);
		IEntity weaponEntityInSlot = weaponSlotComponent.GetWeaponEntity();
		IEntity sightsEntityInSlot;
		if (weaponEntityInSlot)
		{
			turretHasWeapon = true;

			// Check Turret Sights
			GenericComponent genericWeaponComponent = weaponEntityInSlot.FindComponent(WeaponComponent);
			auto attachmentSlot = AttachmentSlotComponent.Cast(genericWeaponComponent.FindComponent(AttachmentSlotComponent));
			if (attachmentSlot)
			{
				sightsEntityInSlot = attachmentSlot.GetAttachedEntity();
				if (sightsEntityInSlot)
				turretHasSights = true;
			}
		}

		// Check if user can carry items
		if (turretHasWeapon)
		{
			bool canCarryBase = false;
			bool canCarryCannon = false;
			bool canCarrySights = !turretHasSights;

			// Check Turret Base
			canCarryBase = userInventory.CanInsert(m_Owner);

			// Check Turret Cannon
			canCarryCannon = userInventory.CanInsert(weaponEntityInSlot);

			// Check Turret Sights
			if(turretHasSights)
				canCarrySights = userInventory.CanInsert(sightsEntityInSlot);

			if (canCarryBase && canCarryCannon && canCarrySights)
			{
				m_CurrentState = ETurretState.ASSEMBLED;
				return true;
			}

			return false;
		}

		// Assembly
		bool userHasWeapon = false;
		bool userHasSights = false;

		// Check for suitable Turret Weapon
		IEntity weaponInInventory = userInventory.OnGetWeapon(weaponSlotComponent.GetWeaponSlotType());
		if (weaponInInventory)
		{
			userHasWeapon = true;

			// Check for suitable Turret Sights
			GenericComponent genericWeaponComponent = weaponInInventory.FindComponent(WeaponComponent);
			auto attachmentSlot = AttachmentSlotComponent.Cast(genericWeaponComponent.FindComponent(AttachmentSlotComponent));
			IEntity sightsInInventory = userInventory.OnGetAttachment(attachmentSlot.GetAttachmentSlotType());
			if (sightsInInventory)
				userHasSights = true;
		}

		if (userHasWeapon && userHasSights)
		{
			m_CurrentState = ETurretState.DISASSEMBLED;
			return true;
		}

		return false;
		*/
	}

	//---------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		/*
		GenericEntity genOwner = GenericEntity.Cast(m_Owner);
		auto genericWeaponSlotComponent = genOwner.FindComponent(WeaponSlotComponent);
		auto weaponSlotComponent = WeaponSlotComponent.Cast(genericWeaponSlotComponent);
		auto controller = TurretControllerComponent.Cast(genOwner.FindComponent(TurretControllerComponent));

		if (m_CurrentState == ETurretState.ASSEMBLED)
		{
			// Get Turret Weapon entity
			IEntity weaponEntityInSlot = weaponSlotComponent.GetWeaponEntity();
			GenericEntity genericWeapon = GenericEntity.Cast(weaponEntityInSlot);
			IEntity sightsEntityInSlot;

			if(weaponEntityInSlot)
			{
				// Get Turret Sights entity
				GenericComponent genericWeaponComponent = genericWeapon.FindComponent(WeaponComponent);
				auto attachmentSlot = AttachmentSlotComponent.Cast(genericWeaponComponent.FindComponent(AttachmentSlotComponent));
				sightsEntityInSlot = attachmentSlot.GetAttachedEntity();
			}

			// Request disassembly action
			if(controller.DisassembleTurret(pUserEntity) && weaponEntityInSlot)
			{
				GenericEntity genericUserEntity = GenericEntity.Cast(pUserEntity);
				BaseInventoryManagerComponent baseInventoryManager = BaseInventoryManagerComponent.Cast(genericUserEntity.FindComponent(BaseInventoryManagerComponent));
				SCR_InventoryManagerComponent inventoryManager = SCR_InventoryManagerComponent.Cast(baseInventoryManager);

				// Turret Weapon
				vector itemTransform[4]; // transform of item before it was picked up
				weaponEntityInSlot.GetWorldTransform(itemTransform);
				BaseSoundComponent soundComponent = BaseSoundComponent.Cast(genericWeapon.FindComponent(BaseSoundComponent));
				if (soundComponent)
				{
					soundComponent.SetTransformation(itemTransform);
					soundComponent.PlayStr(SCR_SoundEvent.SOUND_PICK_UP);
				}

				// Turret Sights
				sightsEntityInSlot.GetWorldTransform(itemTransform);
				GenericEntity genericSights = GenericEntity.Cast(sightsEntityInSlot);
				soundComponent = BaseSoundComponent.Cast(genericSights.FindComponent(BaseSoundComponent));
				if (soundComponent)
				{
					soundComponent.SetTransformation(itemTransform);
					soundComponent.PlayStr(SCR_SoundEvent.SOUND_PICK_UP);
				}

				// Turret Base
				GenericComponent genericItemComponent = m_Owner.FindComponent(SCR_InventoryItemComponent);
				pOwnerEntity.GetWorldTransform(itemTransform);
				GenericEntity genericEntity = GenericEntity.Cast(pOwnerEntity);
				soundComponent = BaseSoundComponent.Cast(genericEntity.FindComponent(BaseSoundComponent));
				if (soundComponent)
				{
					soundComponent.SetTransformation(itemTransform);
					soundComponent.PlayStr(SCR_SoundEvent.SOUND_PICK_UP);
				}
			}
		}
		else if (m_CurrentState == ETurretState.DISASSEMBLED)
		{
			SCR_InventoryManagerComponent userInventory = SCR_InventoryManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryManagerComponent));
			IEntity newWeapon = userInventory.OnGetWeapon(weaponSlotComponent.GetWeaponSlotType());
			IEntity newSights = userInventory.OnGetAttachment(weaponSlotComponent.GetWeaponSlotType());

			// Request assembly action
			controller.AssembleTurret(pUserEntity, newWeapon, newSights);

		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		switch(m_CurrentState)
		{
			case ETurretState.NONE:
			case ETurretState.DESTROYED:
				return false;
			case ETurretState.DISASSEMBLED:
				outName = "#AR-UserAction_Assemble-UC";
				return true;
			case ETurretState.ASSEMBLED:
				outName = "#AR-UserAction_Disassemble-UC";
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_Owner = pOwnerEntity;
		m_CurrentState = ETurretState.NONE;
	}
};