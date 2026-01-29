class SCR_OpenVehicleStorageAction : SCR_InventoryAction
{
#ifndef DISABLE_INVENTORY
	[Attribute("1")]
	protected bool m_bShowFromOutside;

	[Attribute("1")]
	protected bool m_bShowInside;
	
	[Attribute("0", desc: "If true will get Inventory from self even if parent (eg vehicle) also has an inventory.")]
	protected bool m_bForceInventoryFromSelf;

	protected IEntity m_Vehicle;
	protected IEntity m_InventoryOwner;
	protected DamageManagerComponent m_DamageManager;
	protected FactionAffiliationComponent m_VehicleAffiliation;

	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.SetStorageToOpen(m_InventoryOwner);
		manager.OpenInventory();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!user || !m_Vehicle || !m_InventoryOwner)
			return false;

		if (m_DamageManager && m_DamageManager.GetState() == EDamageState.DESTROYED)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (character)
		{
			CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
			if (compAccess && (compAccess.IsGettingIn() || compAccess.IsGettingOut()))
				return false;

			if (!m_bShowFromOutside && !character.IsInVehicle())
				return false;

			if (!m_bShowInside && character.IsInVehicle())
				return false;
		}
		
		if (!m_VehicleAffiliation)
			return true;
		
		Faction vehicleFaction = m_VehicleAffiliation.GetAffiliatedFaction();
		if (!vehicleFaction)
			return true;

		FactionAffiliationComponent userAffiliation = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
		if (!userAffiliation)
			return true;

		Faction userFaction = userAffiliation.GetAffiliatedFaction();
		if (!userFaction)
			return true;

		return !vehicleFaction.IsFactionEnemy(userFaction);
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		//~ Call one frame later as otherwise client slotted entity does not know yet who the vehicle is on init
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: pOwnerEntity, param2: pManagerComponent); 
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!pOwnerEntity)
			return;

		if (!Vehicle.Cast(pOwnerEntity) && pOwnerEntity.GetParent())
			m_Vehicle = pOwnerEntity.GetParent();
		else
			m_Vehicle = pOwnerEntity;
		
		//~ Set inventory owner the same as vehicle
		if (!m_bForceInventoryFromSelf)
			m_InventoryOwner = m_Vehicle;
		//~ Set inventory owner as self
		else 
			m_InventoryOwner = pOwnerEntity;

		if (!m_Vehicle)
			return;

		m_DamageManager = DamageManagerComponent.Cast(m_Vehicle.FindComponent(DamageManagerComponent));
		m_VehicleAffiliation = FactionAffiliationComponent.Cast(m_Vehicle.FindComponent(FactionAffiliationComponent));
	}
#endif
};
