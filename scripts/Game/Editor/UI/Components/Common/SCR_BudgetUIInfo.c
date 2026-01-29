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
}
