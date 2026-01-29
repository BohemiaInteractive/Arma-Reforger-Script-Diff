[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_BudgetType")]
class SCR_EditableEntityCoreBudgetSetting
{
	[Attribute()]
	protected ref SCR_UIInfo m_Info;

	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_BudgetType;

	[Attribute("1", UIWidgets.Auto, "")]
	protected int m_iMinBudgetCost;

	protected int m_iCurrentBudget;

	protected SCR_BudgetEditorComponent m_BudgetEditor;
	/*! Reserved budget is used for budget that is not yet added to the current budget */
	protected int m_iReservedBudget;

	EEditableEntityBudget GetBudgetType()
	{
		return m_BudgetType;
	}
	int GetMinBudgetCost()
	{
		return m_iMinBudgetCost;
	}
	int GetCurrentBudget()
	{
		return m_iCurrentBudget;
	}
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}

	//------------------------------------------------------------------------------------------------
	/*! Returns reserved budget */
	int GetReservedBudget()
	{
		return m_iReservedBudget;
	}

	//------------------------------------------------------------------------------------------------
	/*! Adds reserved budget */
	void ReserveBudget(int reserveValue)
	{
		m_iReservedBudget += reserveValue;
	}

	//------------------------------------------------------------------------------------------------
	/*! Removes reserved budget */
	void UnreserveBudget(int reserveValue)
	{
		m_iReservedBudget -= reserveValue;
		if (m_iReservedBudget < 0)
			m_iReservedBudget = 0;
	}

	int SetCurrentBudget(int budgetValue)
	{
		int oldBudget = m_iCurrentBudget;
		m_iCurrentBudget = budgetValue;

		return m_iCurrentBudget - oldBudget;
	}

	int AddToBudget(SCR_EntityBudgetValue budgetCost = null)
	{
		int budgetCostValue = GetMinBudgetCost();
		if (budgetCost) budgetCostValue = budgetCost.GetBudgetValue();

		int budgetChange = Math.Max(budgetCostValue, GetMinBudgetCost());
		m_iCurrentBudget += budgetChange;

		return budgetChange;
	}

	int AddToBudget(int budgetValue)
	{
		int budgetChange = Math.Max(budgetValue, GetMinBudgetCost());
		m_iCurrentBudget += budgetChange;

		return budgetChange;
	}

	int SubtractFromBudget(SCR_EntityBudgetValue budgetCost = null)
	{
		int budgetCostValue = GetMinBudgetCost();
		if (budgetCost) budgetCostValue = Math.Max(budgetCost.GetBudgetValue(), budgetCostValue);
		
		const int previousBudget = m_iCurrentBudget;
		m_iCurrentBudget = Math.Max(m_iCurrentBudget - budgetCostValue, 0);

		return previousBudget - m_iCurrentBudget;
	}

	int SubtractFromBudget(int budgetValue)
	{
		const int prevBudget = m_iCurrentBudget;
		m_iCurrentBudget = Math.Max(m_iCurrentBudget - budgetValue, 0);

		const int budgetChange = prevBudget - m_iCurrentBudget;
		
		#ifdef BUDGET_OPTIMIZATION_CHECKS
		if(budgetChange != budgetValue)
		{
			Print("GM Budget got clamped at some point!", LogLevel.WARNING);
		}

		return -budgetChange;
	}

	void SetBudgetComponent(notnull SCR_BudgetEditorComponent budgetEditor)
	{
		m_BudgetEditor = budgetEditor;
	}

	void SCR_EditableEntityCoreBudgetSetting(int budgetValue = -1, EEditableEntityBudget budgetType = EEditableEntityBudget.PROPS, int minBudgetCost = 0, SCR_UIInfo info = null)
	{
		if (budgetValue == -1)
			return;

		m_BudgetType = budgetType;
		m_iMinBudgetCost = minBudgetCost;
		m_Info = info;
	}
}
