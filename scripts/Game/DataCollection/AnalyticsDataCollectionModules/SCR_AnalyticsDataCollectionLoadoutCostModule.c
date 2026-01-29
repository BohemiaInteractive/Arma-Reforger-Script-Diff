[BaseContainerProps()]
class SCR_AnalyticsDataCollectionLoadoutCostModule : SCR_AnalyticsDataCollectionSupplyCostBaseModule
{
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_ResourcePlayerControllerInventoryComponent.GetOnArsenalItemRequested().Insert(OnPlayerItemRequested);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Insert(OnSupportStationExecuted);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_ResourcePlayerControllerInventoryComponent.GetOnArsenalItemRequested().Remove(OnPlayerItemRequested);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Remove(OnSupportStationExecuted);
	}

	//------------------------------------------------------------------------------------------------
	//! Player spawned/respawned, add respawn cost and loadout cost to data
	//! \param[in] requestComponent
	//! \param[in] playerId
	//! \param[in] data
	//! \param[in] controlledEntity
	protected override void OnPlayerSpawned(SCR_SpawnRequestComponent requestComponent, int playerId, SCR_SpawnData data, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(requestComponent, playerId, data, controlledEntity);

		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		if (!spawnPointData)
			return;

		SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(requestComponent.GetPlayerController().FindComponent(SCR_PlayerLoadoutComponent));
		if (!loadoutComp)
			return;

		// Get the supply cost of player loadout
		SCR_CampaignMilitaryBaseComponent base;
		SCR_ResourceComponent resourceComp;
		int spawnSupplyCost = SCR_ArsenalManagerComponent.GetLoadoutCalculatedSupplyCost(loadoutComp.GetLoadout(), false, playerId, null, spawnPointData.GetSpawnPoint(), base, resourceComp);

		// Player spawned on Main Base or the loadout has a cost of 0, no need to add to any data
		if (spawnSupplyCost == 0)
			return;

		SCR_BasePlayerLoadout loadout = loadoutComp.GetLoadout();
		if (!loadout)
			return;

		string loadoutName = SCR_AnalyticsDataCollectionHelper.GetShortEntityPrefabName(loadout.GetLoadoutResource());

		AddSupplySpendingData(playerId, loadoutName, spawnSupplyCost);
	}

	//------------------------------------------------------------------------------------------------
	//! Player requests an item from Armory, add item name and cost to data
	//! \param[in] resourceName
	//! \param[in] inventoryStorageComponent
	//! \param[in] resourceValue
	protected void OnPlayerItemRequested(SCR_ResourceComponent resourceComponent, ResourceName resourceName, IEntity requesterEntity, BaseInventoryStorageComponent inventoryStorageComponent, EResourceType resourceType, int resourceValue)
	{
		// Item is free, no need to add to collected data
		if (resourceValue == 0)
			return;

		// In case the item is added to storage inside player inventory, we need to get player entity
		IEntity ownerEntity = inventoryStorageComponent.GetOwner();
		ownerEntity = ownerEntity.GetRootParent();

		// User is not a player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ownerEntity);
		if (playerId == 0)
			return;

		string itemName = SCR_AnalyticsDataCollectionHelper.GetShortEntityPrefabName(resourceName);

		AddSupplySpendingData(playerId, itemName, resourceValue);
	}

	//------------------------------------------------------------------------------------------------
	//! Support station used, if it resupply ammo or resupply medical support station we add the supply cost to supply spending data
	//! \param[in] supportStation
	//! \param[in] supportStationType
	//! \param[in] actionTarget
	//! \param[in] actionUser
	//! \param[in] action
	protected void OnSupportStationExecuted(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionTarget, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!action)
			return;

		// We only care about resupply ammo and resupply medical support stations
		if (supportStationType != ESupportStationType.RESUPPLY_AMMO && supportStationType != ESupportStationType.RESUPPLY_MEDICAL)
			return;

		// User is not a player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionUser);
		if (playerId == 0)
			return;

		// The cost is 0, no need to add
		int actionCost = action.GetSupportStationSuppliesOnUse();
		if (actionCost == 0)
			return;

		SCR_BaseItemHolderSupportStationAction itemHolderAction = SCR_BaseItemHolderSupportStationAction.Cast(action);
		if (!itemHolderAction)
			return;

		string itemName = SCR_AnalyticsDataCollectionHelper.GetShortEntityPrefabName(itemHolderAction.GetItemPrefab());

		AddSupplySpendingData(playerId, itemName, actionCost);
	}

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionLoadoutCostModule()
	{
		Disable();
	}
}
