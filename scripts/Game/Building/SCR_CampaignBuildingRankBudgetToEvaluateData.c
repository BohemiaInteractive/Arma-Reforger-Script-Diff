//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_iBudget")]
class SCR_CampaignBuildingRankBudgetToEvaluateData : SCR_CampaignBuildingBudgetToEvaluateData
{	
	//! Use this budgets only when the ranks are handled by GM and player can rank up.
	override bool CanBeUsed()
	{
		return SCR_XPHandlerComponent.IsXpSystemEnabled();
	}
}
