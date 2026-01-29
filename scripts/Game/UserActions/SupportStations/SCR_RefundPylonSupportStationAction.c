class SCR_RefundPylonSupportStationAction : SCR_BaseAudioSupportStationAction
{
	[Attribute("1", params: "0")]
	protected float m_fRefundMultiplier;
	
	[Attribute("1", desc: "If true will disable the action if at least one of the barrels are occupied", params: "0")]
	protected bool m_bDisableIfBarrelsOccupied;

	[Attribute(defvalue: "0", desc: "Id of the weapon slot from which this action should remove the pylon.", params: "0 inf")]
	protected int m_iPylonIndex;
	
	protected const LocalizedString INVALID_BARRELS_OCCUPIED = "#AR-SupportStation_ActionInvalid_PylonBarrelsOccupied";
	
	protected SCR_ArsenalItem m_ArsenalData;
	protected SCR_NonArsenalItemCostCatalogData m_NonArsenalData;
	
	protected WeaponSlotComponent m_ManagedWeaponSlot;
	protected RocketEjectorMuzzleComponent m_RocketMuzzleComp;
	
	protected SCR_EArsenalSupplyCostType m_eArsenalSupplyCostType = SCR_EArsenalSupplyCostType.DEFAULT;
	
	protected SCR_BaseSupportStationComponent m_LastCheckedSupportStation;

	//------------------------------------------------------------------------------------------------
	int GetPylonIndex()
	{
		return m_iPylonIndex;
	}

	//------------------------------------------------------------------------------------------------
	WeaponSlotComponent GetManagedWeaponSlot()
	{
		return m_ManagedWeaponSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSupplyRefundAmount()
	{
		//~ Get supply cost type if support station changed
		if (m_SupportStationComponent && m_LastCheckedSupportStation != m_SupportStationComponent)
		{
			m_LastCheckedSupportStation = m_SupportStationComponent;
			
			SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(m_LastCheckedSupportStation.GetOwner().FindComponent(SCR_ArsenalComponent));
			if (arsenalComp)
				m_eArsenalSupplyCostType = arsenalComp.GetSupplyCostType();
		}
		
		//~ Get actual supply cost
		if (m_ArsenalData)
			return m_ArsenalData.GetSupplyCost(m_eArsenalSupplyCostType, false);
		else if (m_NonArsenalData)
			return m_NonArsenalData.GetSupplyCost(m_eArsenalSupplyCostType);
		
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_ManagedWeaponSlot)
			return false;

		const IEntity pylon = m_ManagedWeaponSlot.GetWeaponEntity();
		if (!pylon)
			return false;

		if (m_bDisableIfBarrelsOccupied && !m_RocketMuzzleComp)
		{
			m_RocketMuzzleComp = RocketEjectorMuzzleComponent.Cast(pylon.FindComponent(RocketEjectorMuzzleComponent));
			if (!m_RocketMuzzleComp)
				return false;
		}

		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!GetOwner() || GetOwner().IsDeleted())
			return false;
		
		if (m_bDisableIfBarrelsOccupied && m_RocketMuzzleComp)
		{
			const int count = m_RocketMuzzleComp.GetBarrelsCount();		
			for(int i = 0; i < count; i++)
			{
				if (!m_RocketMuzzleComp.CanReloadBarrel(i))
				{
					SetCanPerform(false, ESupportStationReasonInvalid.DISABLED);
					m_sCannotPerformReason = INVALID_BARRELS_OCCUPIED;
					return false;
				}
			}
		}
		
		return super.CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);

		array<Managed> weaponSlots = {};
		owner.FindComponents(WeaponSlotComponent, weaponSlots);

		WeaponSlotComponent weaponSlot;
		foreach (Managed slot : weaponSlots)
		{
			weaponSlot = WeaponSlotComponent.Cast(slot);
			if (!weaponSlot)
				continue;

			if (weaponSlot.GetWeaponSlotIndex() != m_iPylonIndex)
				continue;

			m_ManagedWeaponSlot = weaponSlot;
		}

		if (!m_ManagedWeaponSlot)
		{
			Print("SCR_RefundPylonSupportStationAction was unable to find weapon slot with id = " + m_iPylonIndex + " in " + FilePath.StripPath(SCR_ResourceNameUtils.GetPrefabName(owner)), LogLevel.ERROR);
			return;
		}

		
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return;
		
		EntityPrefabData prefabData = owner.GetPrefabData();
		if (!prefabData || prefabData.GetPrefabName().IsEmpty())
		{
			Print("SCR_RefundPylonSupportStationAction action attached to non-prefab!", LogLevel.ERROR);
			return;
		}
			
		SCR_EntityCatalogEntry entry = catalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, prefabData.GetPrefabName());
		if (!entry)
		{
			Print("SCR_RefundPylonSupportStationAction could not find the prefab data in catalog.", LogLevel.WARNING);
			return;
		}
		
		m_ArsenalData = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (m_ArsenalData)
			return;

		if (m_NonArsenalData)
		{
			m_NonArsenalData = SCR_NonArsenalItemCostCatalogData.Cast(entry.GetEntityDataOfType(SCR_NonArsenalItemCostCatalogData));
			return;
		}
		
		Print("SCR_RefundPylonSupportStationAction could not find the ArsenalItem nor the NonArsenalItemCost data on the catalog entry so could not get the supply cost", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.VEHICLE_WEAPON;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo uiInfo = GetUIInfo();
		if (!uiInfo)
			return super.GetActionNameScript(outName);
		
		string pylonName;
		
		//~ Item to refund has no inventory component
		if (m_RocketMuzzleComp)
		{
			pylonName = "MISSING NAME OR INVALID PREFAB";
			
			InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(m_RocketMuzzleComp.GetOwner().FindComponent(InventoryItemComponent));
			if (inventoryItem)
			{
				UIInfo itemUIInfo = inventoryItem.GetUIInfo();
				
				if (itemUIInfo)
					pylonName = itemUIInfo.GetName();
			}
		}
		
		outName = WidgetManager.Translate(uiInfo.GetName(), pylonName);
		
		return super.GetActionNameScript(outName);
	}
}