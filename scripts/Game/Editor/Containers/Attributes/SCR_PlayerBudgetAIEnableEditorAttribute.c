[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PlayerBudgetAIEnableEditorAttribute: SCR_BasePlayerBudgetEnableEditorAttribute
{	
	protected override EEditableEntityBudget GetBudgetType()
	{
		return EEditableEntityBudget.AI;
	}
	
	protected override ENotification GetEnableNotification()
	{
		return ENotification.EDITOR_ENABLE_AI_BUDGET;
	}
	
	protected override ENotification GetDisableNotification()
	{
		return ENotification.EDITOR_DISABLE_AI_BUDGET;
	}		
};