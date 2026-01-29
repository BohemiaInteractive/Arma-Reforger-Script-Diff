//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_eBudget")]
class SCR_CampaignBuildingBudgetToEvaluateData
{
	[Attribute(desc: "Fill in the budgets to be used with this provider", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_eBudget;
	
	[Attribute("0", UIWidgets.CheckBox, "If this provider has set: Use master provider and the same budget is set on master provider too, master provider budget will be used to evaluate placement.")]
	protected bool m_bUseMasterProviderBudget;
	
	[Attribute("0", UIWidgets.CheckBox, "Set if this budget should be used in UI bar. Two budgets can be set per provider.")]
	protected bool m_bShowBudgetInUI;

	//------------------------------------------------------------------------------------------------
	//! To be overridden in inherited classes, allow to set an additional custom condition to usage of the budget. For an example only with specific game mode etc.
	bool CanBeUsed()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	EEditableEntityBudget GetBudget()
	{
		return m_eBudget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should be used the budget of master provider.
	bool UseMasterProviderBudget()
	{
		return m_bUseMasterProviderBudget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should be this budget shown in UI.
	bool CanShowBudgetInUI()
	{
		return m_bShowBudgetInUI;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	void SetShowBudgetInUI(bool value)
	{
		m_bShowBudgetInUI = value;
	}
}
