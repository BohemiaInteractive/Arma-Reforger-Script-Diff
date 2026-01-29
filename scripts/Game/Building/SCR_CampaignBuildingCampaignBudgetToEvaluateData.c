[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_iBudget")]
class SCR_CampaignBuildingCampaignBudgetToEvaluateData : SCR_CampaignBuildingBudgetToEvaluateData
{
	//! Use this budgets only when the campaign (aka supply) budget is enabled.
	override bool CanBeUsed()
	{
		return SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled();
	}
}
