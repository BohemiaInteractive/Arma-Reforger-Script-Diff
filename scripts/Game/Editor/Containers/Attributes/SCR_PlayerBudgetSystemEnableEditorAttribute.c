[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PlayerBudgetSystemEnableEditorAttribute: SCR_BasePlayerBudgetEnableEditorAttribute
{	
	protected override EEditableEntityBudget GetBudgetType()
	{
		return EEditableEntityBudget.SYSTEMS;
	}
	
	protected override ENotification GetEnableNotification()
	{
		return ENotification.EDITOR_ENABLE_SYSTEM_BUDGET;
	}
	
	protected override ENotification GetDisableNotification()
	{
		return ENotification.EDITOR_DISABLE_SYSTEM_BUDGET;
	}	
};