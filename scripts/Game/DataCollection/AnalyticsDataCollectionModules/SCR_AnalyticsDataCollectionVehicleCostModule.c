[BaseContainerProps()]
class SCR_AnalyticsDataCollectionVehicleCostModule : SCR_AnalyticsDataCollectionSupplyCostBaseModule
{
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			campaign.GetOnEntityRequested().Insert(OnVehicleSpawned);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Insert(OnSupportStationExecuted);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			campaign.GetOnEntityRequested().Remove(OnVehicleSpawned);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Remove(OnSupportStationExecuted);
	}

	//------------------------------------------------------------------------------------------------
	//! Player spawned a new vehicle, add vehicle name and cost to data
	//! \param[in] userEntity
	//! \param[in] spawnedEntity
	protected void OnVehicleSpawned(IEntity userEntity, IEntity spawnedEntity)
	{
		// Spawned entity is not a vehicle
		if (!spawnedEntity.IsInherited(Vehicle))
			return;

		// User is not a player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity);
		if (playerId == 0)
			return;

		string vehicleName = SCR_AnalyticsDataCollectionHelper.GetEntityPrefabName(spawnedEntity);
		int vehicleCost = SCR_AnalyticsDataCollectionHelper.GetEntitySupplyCost(spawnedEntity);

		AddSupplySpendingData(playerId, vehicleName, vehicleCost);
	}

	//------------------------------------------------------------------------------------------------
	//! Support station used, if it is vehicle repair or vehicle weapon resupply add supply cost to spending data
	//! \param[in] supportStation
	//! \param[in] supportStationType
	//! \param[in] actionTarget
	//! \param[in] actionUser
	//! \param[in] action
	protected void OnSupportStationExecuted(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionTarget, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!action)
			return;

		// We only care about repair and vehicle weapon support station types
		if (supportStationType != ESupportStationType.REPAIR && supportStationType != ESupportStationType.VEHICLE_WEAPON)
			return;

		// User is not a player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionUser);
		if (playerId == 0)
			return;

		// The cost is 0, no need to add
		int actionCost = action.GetSupportStationSuppliesOnUse();
		if (actionCost == 0)
			return;

		// Refund action does not cost supplies
		if (SCR_RefundVehicleAmmoSupportStationAction.Cast(action))
			return;

		IEntity vehiclePrefab = actionTarget.GetRootParent();
		string vehicleName = SCR_AnalyticsDataCollectionHelper.GetEntityPrefabName(vehiclePrefab);

		AddSupplySpendingData(playerId, vehicleName, actionCost);
	}

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionVehicleCostModule()
	{
		Disable();
	}
}
