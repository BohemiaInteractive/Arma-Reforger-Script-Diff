class SCR_WeaponLoadAction : ScriptedUserAction
{
	private IEntity m_Owner;
	private IEntity m_LastObserver;

	private BaseWeaponComponent m_Weapon;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		m_LastObserver = user;
		return CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	//! INV_TODO: Refactor
	override bool CanBePerformedScript(IEntity user)
	{
		return false;
		/*
		if(!m_Weapon)
			return false;

		IEntity parent = m_Owner.GetParent();
		if (!parent)
			return false;

		auto genericUser = GenericEntity.Cast(user);

		// Check if user is in a compartment
		ChimeraCharacter char = ChimeraCharacter.Cast(genericUser);
		if (char)
		{
			CompartmentAccessComponent comp = char.GetCompartmentAccessComponent();
			if (comp && comp.IsInCompartment())
				return false;
		}

		// Check for compatible magazines
		SCR_InventoryManagerComponent inventory = SCR_InventoryManagerComponent.Cast(genericUser.FindComponent(SCR_InventoryManagerComponent));
		if (inventory && inventory.GetMagazineCountByWeapon(m_Weapon) <= 0)
			return false;

		/// Check ammo count
		MortarMuzzleComponent mortarMuzzleComponent = MortarMuzzleComponent.Cast(m_Weapon.GetCurrentMuzzle());
		if (mortarMuzzleComponent && mortarMuzzleComponent.GetAmmoCount() >= mortarMuzzleComponent.GetMaxAmmoCount())
			return false;

		return true;
		*/
	}

	//---------------------------------------------------------
	//! INV_TODO: Refactor
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		/*
		GenericEntity genericUserEntity = GenericEntity.Cast(pUserEntity);
		BaseInventoryManagerComponent baseInventoryManager = BaseInventoryManagerComponent.Cast(genericUserEntity.FindComponent(BaseInventoryManagerComponent));
		SCR_InventoryManagerComponent inventoryManager = SCR_InventoryManagerComponent.Cast(baseInventoryManager);

		MortarMuzzleComponent mortarMuzzleComponent = MortarMuzzleComponent.Cast(m_Weapon.GetCurrentMuzzle());
		if (mortarMuzzleComponent && mortarMuzzleComponent.GetAmmoCount() < mortarMuzzleComponent.GetMaxAmmoCount())
		{
			IEntity shell = inventoryManager.OnGetMagazine(mortarMuzzleComponent.GetMagazineWell());
			if (shell)
			{
				inventoryManager.Remove(shell, null, true);
				mortarMuzzleComponent.LoadMortar(shell);
			}
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_LastObserver)
		{
			GenericEntity watcher = GenericEntity.Cast(m_LastObserver);
			outName = "Load";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_Owner = pOwnerEntity;
		m_Weapon = BaseWeaponComponent.Cast(m_Owner.FindComponent(BaseWeaponComponent));
	}

	//------------------------------------------------------------------------------------------------
	/*override bool HasLocalEffectOnlyScript()
	{
		return true;
	}*/
};