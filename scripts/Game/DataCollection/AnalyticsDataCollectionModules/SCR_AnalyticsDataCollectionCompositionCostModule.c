[BaseContainerProps()]
class SCR_AnalyticsDataCollectionCompositionCostModule : SCR_AnalyticsDataCollectionSupplyCostBaseModule
{
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			campaign.GetOnEntityRequested().Insert(OnCompositionSpawned);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			campaign.GetOnEntityRequested().Remove(OnCompositionSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//! Player spawned a new composition, add composition name and cost to data
	//! \param[in] userEntity
	//! \param[in] spawnedEntity
	protected void OnCompositionSpawned(IEntity userEntity, IEntity spawnedEntity)
	{
		// Spawned entity is a vehicle, not a composition
		if (spawnedEntity.IsInherited(Vehicle))
			return;

		// Spawned entity is an AI group, not a composition
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(spawnedEntity);
		if (aiGroup)
			return;

		// User is not a player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity);
		if (playerId == 0)
			return;

		string compositionName = SCR_AnalyticsDataCollectionHelper.GetEntityPrefabName(spawnedEntity);
		int compositionCost = SCR_AnalyticsDataCollectionHelper.GetEntitySupplyCost(spawnedEntity);

		AddSupplySpendingData(playerId, compositionName, compositionCost);
	}

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionCompositionCostModule()
	{
		Disable();
	}
}
