//------------------------------------------------------------------------------------------------
//! Returns true if count of medical items matches the condition
[BaseContainerProps()]
class SCR_MedicalItemsCountCondition : SCR_AvailableActionCondition
{
	[Attribute("3", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("1", UIWidgets.EditBox, "Desired count of items to check", "")]
	private int m_iValue;

	[Attribute("0", UIWidgets.CheckBox, "True - condition pass only if medical is equiped in quick slot | False - whole inventory")]
	protected bool m_bMustBeInQuickSlots;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity's current weapon has met the specified ammo condition
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		int count = data.GetMedicalItemCount();
		if (m_bMustBeInQuickSlots)
			count = data.GetMedicalItemCountInQuickSlots();

		bool result = false;

		result = SCR_Comparer<int>.Compare(m_eOperator, count, (int)m_iValue);
		return GetReturnResult(result);
	}
};
