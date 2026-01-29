[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_BudgetType")]
class SCR_EntityBudgetValue
{
	[Attribute("0", UIWidgets.SearchComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	private EEditableEntityBudget m_BudgetType;
	
	[Attribute("0", UIWidgets.Auto, "")]
	private int m_Value;
	
	EEditableEntityBudget GetBudgetType()
	{
		return m_BudgetType;
	}
	
	int GetBudgetValue()
	{
		return m_Value;
	}
	
	void SetBudgetValue(int newValue)
	{
		m_Value = newValue;
	}
	
	bool AddBudgetValue(SCR_EntityBudgetValue entityBudgetValue)
	{
		if (entityBudgetValue.GetBudgetType() == GetBudgetType())
		{
			m_Value += entityBudgetValue.GetBudgetValue();
			return true;
		}
		return false;
	}
	
	static void MergeBudgetCosts(out notnull array<ref SCR_EntityBudgetValue> outExistingBudgets, array<ref SCR_EntityBudgetValue> newBudgetCosts)
	{
		foreach (SCR_EntityBudgetValue newBudgetCost : newBudgetCosts)
		{
			bool budgetExists = false;
			foreach (SCR_EntityBudgetValue existingBudget : outExistingBudgets)
			{
				if (existingBudget.AddBudgetValue(newBudgetCost))
				{
					budgetExists = true;
					break;
				}
			}
			if (!budgetExists)
			{
				outExistingBudgets.Insert(newBudgetCost);
			}
		}
	}

	static void AddBudgetValueToBudgetArray(out notnull array<ref SCR_EntityBudgetValue> budgets, EEditableEntityBudget budgetType, int value)
	{
		foreach (SCR_EntityBudgetValue budgetValue : budgets)
		{
			if (budgetValue.GetBudgetType() == budgetType)
			{
				budgetValue.SetBudgetValue(budgetValue.GetBudgetValue() + value);
				return;
			}
		}
		
		budgets.Insert(new SCR_EntityBudgetValue(budgetType, value));
	}

	void SCR_EntityBudgetValue(EEditableEntityBudget budgetType, int budgetValue = -1)
	{
		if (budgetValue == -1) return;
		m_BudgetType = budgetType;
		m_Value = budgetValue;
	}
};