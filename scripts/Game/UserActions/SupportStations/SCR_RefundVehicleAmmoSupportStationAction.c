class SCR_RefundVehicleAmmoSupportStationAction : SCR_BaseAudioSupportStationAction
{
	[Attribute("1", desc: "Hide the action if the entity has no parent")]
	protected bool m_bHideIfNoParent;
	
	[Attribute("1", desc: "When refunding how much of the supplies are refunded. (Supply taken from Catalog)", params: "0")]
	protected float m_fRefundMultiplier;
	
	protected RocketEjectorMuzzleComponent m_RocketMuzzleComp;
	protected SCR_EntityCatalogManagerComponent m_EntityCatalogManager;
	
	protected IEntity m_EntityToRefund;
	protected int m_iTotalBarrelCount;
	protected int m_iFullBarrelCount;
	protected const LocalizedString X_OUTOF_Y_FORMATTING = "#AR-SupportStation_ActionFormat_ItemAmount";
	protected const LocalizedString INVALID_EMPTY = "#AR-SupportStation_Refund_Helicopter_Weapon_Invalid_Empty";
	
	//~ Used to get supply cost. Is set to empty if new supply cost needs to be obtained and is checked with the m_EntityToRefund.Prefab to see if it is still the same item to refund
	protected ResourceName m_sRefundPrefab;
	protected SCR_ArsenalItem m_ArsenalData;
	protected SCR_NonArsenalItemCostCatalogData m_NonArsenalData;
	protected SCR_EArsenalSupplyCostType m_eArsenalSupplyCostType = SCR_EArsenalSupplyCostType.DEFAULT;
	protected SCR_BaseSupportStationComponent m_LastCheckedSupportStation;
	
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
		//~ Not valid or has no parent
		if (!m_RocketMuzzleComp)
			return false;
		
		if (m_bHideIfNoParent && (!GetOwner() || !GetOwner().GetParent()))
			return false;
			
		return super.CanBeShownScript(user);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_NOT_IN_STORAGE)
			return INVALID_EMPTY;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		m_EntityToRefund = null;
		m_iFullBarrelCount = 0;
		
		for (int i = m_iTotalBarrelCount - 1; i >= 0; i--)
		{
			if (!m_RocketMuzzleComp.CanReloadBarrel(i))
			{
				if (!m_EntityToRefund)
					m_EntityToRefund = m_RocketMuzzleComp.GetBarrelProjectile(i);
				
				m_iFullBarrelCount++;
			}
		}
		
		if (m_iFullBarrelCount <= 0)
		{
			//~ No rockets in storages
			SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_NOT_IN_STORAGE);
			return false;
		}		
		
		return super.CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void CanBePerformedUpdate(IEntity user)
	{
		super.CanBePerformedUpdate(user);
		
		//~ Check if a catalog manager exists and supplies are enabled as no need to do the supply check if they are not
		if (!m_EntityCatalogManager || !m_SupportStationComponent || !m_SupportStationComponent.AreSuppliesEnabled())
			return;
	
		//~ No item to refund
		if (!m_EntityToRefund)
		{
			m_ArsenalData = null;
			m_NonArsenalData = null;
			return;
		}
	
		//~ For some reason the ammo is not a prefab (Which is technically not possible)
		EntityPrefabData prefabData = m_EntityToRefund.GetPrefabData();
		if (!prefabData)
		{
			m_ArsenalData = null;
			m_NonArsenalData = null;
			return;
		}
		
		//~ For some reason could not get the ResourceName from prefab data
		ResourceName prefabToRefund = prefabData.GetPrefabName();
		if (prefabToRefund.IsEmpty())
		{
			m_ArsenalData = null;
			m_NonArsenalData = null;
			m_sRefundPrefab = string.Empty;
			return;
		}
			
		//~ The prefab to refund has changed so update supply amount
		if (prefabToRefund != m_sRefundPrefab)
		{
			m_sRefundPrefab = prefabToRefund;
			
			//~ Check if can find the entry in the catalog
			SCR_EntityCatalogEntry catalogEntry = m_EntityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, m_sRefundPrefab, m_SupportStationComponent.GetFaction());
			if (!catalogEntry)
			{
				Print("'SCR_RefundVehicleAmmoSupportStationAction' could not find SCR_EntityCatalogEntry for '" + m_sRefundPrefab + "' so the action will not have any supply refund amount!", LogLevel.WARNING);
				m_ArsenalData = null;
				m_NonArsenalData = null;
				return;
			}
			
			//~ Check if can find the arsenal data
			m_ArsenalData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
			if (!m_ArsenalData)
				m_NonArsenalData = SCR_NonArsenalItemCostCatalogData.Cast(catalogEntry.GetEntityDataOfType(SCR_NonArsenalItemCostCatalogData));
			
			if (!m_ArsenalData && !m_NonArsenalData)
				Print("'SCR_RefundVehicleAmmoSupportStationAction' could not find SCR_ArsenalItem nor m_NonArsenalData for '" + m_sRefundPrefab + "' so the action will not have any supply refund amount!", LogLevel.WARNING);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	RocketEjectorMuzzleComponent GetRocketMuzzle()
	{
		return m_RocketMuzzleComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.VEHICLE_WEAPON;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		m_RocketMuzzleComp = RocketEjectorMuzzleComponent.Cast(GetOwner().FindComponent(RocketEjectorMuzzleComponent));
		if (!m_RocketMuzzleComp)
			return;
		
		m_iTotalBarrelCount = m_RocketMuzzleComp.GetBarrelsCount();
		
		m_EntityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{		
		if (!m_bCanPerform || m_iTotalBarrelCount <= 1)
			return string.Empty;

		return WidgetManager.Translate(X_OUTOF_Y_FORMATTING, m_iFullBarrelCount, m_iTotalBarrelCount);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo uiInfo = GetUIInfo();
		if (!uiInfo)
			return super.GetActionNameScript(outName);
		
		//~ No item to refund
		if (!m_EntityToRefund)
		{
			outName = uiInfo.GetDescription();
			return super.GetActionNameScript(outName);
		}
		
		//~ Item to refund has no inventory component
		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(m_EntityToRefund.FindComponent(InventoryItemComponent));
		if (!inventoryItem)
		{
			outName = uiInfo.GetDescription();
			return super.GetActionNameScript(outName);
		}
		
		//~ Item to refund has no UIInfo or no name assigned
		UIInfo itemUIInfo = inventoryItem.GetUIInfo();
		if (!itemUIInfo || itemUIInfo.GetName().IsEmpty())
		{
			outName = uiInfo.GetDescription();
			return super.GetActionNameScript(outName);
		}
		
		outName = WidgetManager.Translate(uiInfo.GetName(), itemUIInfo.GetName());
		
		return super.GetActionNameScript(outName);
	}
	
}