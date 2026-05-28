[BaseContainerProps()]
class SCR_BudgetUIInfo : SCR_UIInfo
{
	[Attribute(desc: "Value sets an order when more budget should be shown at the asset card. Higher number has higher priority to be shown.")]
	protected int m_iPriorityOrderUI;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPriorityOrder()
	{
		return m_iPriorityOrderUI;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_BudgetUIInfo CreateInfo(int priority, LocalizedString name, LocalizedString description = LocalizedString.Empty, ResourceName icon = ResourceName.Empty, string iconSetName = string.Empty)
	{
		SCR_BudgetUIInfo info = new SCR_BudgetUIInfo();
		info.Name = name;
		info.Description = description;
		info.Icon = icon;
		info.IconSetName = iconSetName;
		info.m_iPriorityOrderUI = priority;
		return info;
	}
}
