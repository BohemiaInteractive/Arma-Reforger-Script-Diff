//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_eBudget")]
class SCR_CampaignBuildingMaxValueBudgetToEvaluateData : SCR_CampaignBuildingBudgetToEvaluateData
{	
	[Attribute("50", UIWidgets.EditBox, "Maximum value of budget for this provider.")]
	protected int m_iMaxValue;
		
	//------------------------------------------------------------------------------------------------
	int GetMaxValue()
	{
		return m_iMaxValue;
	}
}
