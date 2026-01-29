[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorBudgetHintCondition : SCR_BaseEditorHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetMaxReached(EEditableEntityBudget budgetType, bool maxReached)
	{
		Activate();	
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_BudgetEditorComponent budgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, false, true));
		if (budgetManager)
			budgetManager.Event_OnBudgetMaxReached.Insert(OnBudgetMaxReached);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_BudgetEditorComponent budgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, false, true));
		if (budgetManager)
			budgetManager.Event_OnBudgetMaxReached.Remove(OnBudgetMaxReached);
	}
}
