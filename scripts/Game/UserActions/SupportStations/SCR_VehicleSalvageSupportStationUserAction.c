class SCR_VehicleSalvageSupportStationUserAction : SCR_BaseUseSupportStationAction
{
	protected Vehicle m_VehicleParent;
	protected SCR_DamageManagerComponent m_DamageManager;
	protected bool m_bSupplyCostObtained;
	protected int m_iSupplyCost;
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.LOAD_WRECK;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the value of the wreck in supply amount
	//! \return m_iSupplyCost Supply amount
	int GetSupplyCost()
	{
		if (m_bSupplyCostObtained)
			return m_iSupplyCost;
		
		m_bSupplyCostObtained = true;
		
		if (!m_VehicleParent)
		{
			Print("'SCR_VehicleSalvageSupportStationUserAction': Vehicle is null!", LogLevel.WARNING);
			return 0;
		}
		
		// Get prefab data of vehicle
		EntityPrefabData prefabData = m_VehicleParent.GetPrefabData();
		if (!prefabData)
		{
			Print("'SCR_VehicleSalvageSupportStationUserAction': Vehicle does not have prefab data!", LogLevel.WARNING);
			return 0;
		}
		
		// Get prefab name of vehicle
		ResourceName prefab = prefabData.GetPrefabName();
		if (prefab.IsEmpty())
		{
			Print("'SCR_VehicleSalvageSupportStationUserAction': Prefab does not have a name!", LogLevel.WARNING);
			return 0;
		}
		
		// Get vehicle faction
		SCR_Faction faction;
		FactionAffiliationComponent factionAff = FactionAffiliationComponent.Cast(m_VehicleParent.FindComponent(FactionAffiliationComponent));
		if (factionAff)
			faction = SCR_Faction.Cast(factionAff.GetDefaultAffiliatedFaction());
		
		// Get entity catalog so we can obtain the vehicle's value
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
		{
			Print("'SCR_VehicleSalvageSupportStationUserAction': Catalog Manager is null!", LogLevel.WARNING);
			return 0;
		}
		
		SCR_EntityCatalogEntry catalogEntry = catalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.VEHICLE, prefab, faction);
		if (!catalogEntry)
		{
			Print("'SCR_VehicleSalvageSupportStationUserAction': Catalog entry is null!", LogLevel.WARNING);
			return 0;
		}
		
		SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(catalogEntry.GetEntityUiInfo());
		if (!editableUIInfo)
		{
			Print("'SCR_VehicleSalvageSupportStationUserAction': Editable UI Info of catalog entry is null!", LogLevel.WARNING);
			return 0;
		}
		
		array<ref SCR_EntityBudgetValue> budgets = {};
		editableUIInfo.GetEntityAndChildrenBudgetCost(budgets);
		
		// Loop through budgets to find the one of target vehicle
		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			if (!budget)
				continue;
			
			if (budget.GetBudgetType() != EEditableEntityBudget.CAMPAIGN)
				continue;
			
			m_iSupplyCost = budget.GetBudgetValue();
			break;
		}
		
		return m_iSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		m_VehicleParent = Vehicle.Cast(owner);
		if (!m_VehicleParent)
			m_VehicleParent = Vehicle.Cast(owner.GetParent());
		
		if (!m_VehicleParent)
		{
			Print("'SCR_VehicleSalvageSupportStationComponent': Vehicle Parent is null!", LogLevel.ERROR);
			return;
		}
		
		m_DamageManager = m_VehicleParent.GetDamageManager();
		
		if (!m_DamageManager)
		{
			Print("'SCR_VehicleSalvageSupportStationComponent': Damage Manager is null!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		m_bShowSupplyCostOnAction = true;
		
		// Check if setting is enabled
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalGameSettings)
		{
			additionalGameSettings.SetEnableVehicleSalvage_S(false);
			return false;
		}
		
		if (!additionalGameSettings.IsVehicleSalvageEnabled())
			return false;
		
		// Check if the vehicle exists and is destroyed
		if (!m_VehicleParent || !m_DamageManager)
			return false;
		
		if (!m_DamageManager.IsDestroyed())
			return false;
		
		return super.CanBeShownScript(user);
	}
}
