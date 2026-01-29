[BaseContainerProps()]
class SCR_AnalyticsDataCollectionAiGroupCostModule : SCR_AnalyticsDataCollectionSupplyCostBaseModule
{
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			campaign.GetOnEntityRequested().Insert(OnAIGroupSpawned);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			campaign.GetOnEntityRequested().Remove(OnAIGroupSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//! Player spawned an AI group, add group name and cost to data
	//! \param[in] userEntity
	//! \param[in] spawnedEntity
	protected void OnAIGroupSpawned(IEntity userEntity, IEntity spawnedEntity)
	{
		// Spawned entity is not an AI group
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(spawnedEntity);
		if (!aiGroup)
			return;

		// User is not a player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity);
		if (playerId == 0)
			return;

		string aiGroupName = SCR_AnalyticsDataCollectionHelper.GetEntityPrefabName(spawnedEntity);
		int aiGroupCost = GetAiGroupCost(spawnedEntity);

		AddSupplySpendingData(playerId, aiGroupName, aiGroupCost);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate supply cost of AI group
	//! \param[in] groupEntity
	//! return int supply cost
	protected int GetAiGroupCost(IEntity groupEntity)
	{
		SCR_EditableGroupComponent editableGroupComponent = SCR_EditableGroupComponent.Cast(SCR_EditableEntityComponent.GetEditableEntity(groupEntity));
		if (!editableGroupComponent)
			return 0;

		array<ref SCR_EntityBudgetValue> budgets = {};
		editableGroupComponent.GetPrefabBudgetCost(budgets);

		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			// We only care about Conflict budget
			if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
				return budget.GetBudgetValue();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionAiGroupCostModule()
	{
		Disable();
	}
}
