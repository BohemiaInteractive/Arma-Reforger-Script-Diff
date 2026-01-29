[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PlayerBudgetPropEnableEditorAttribute: SCR_BasePlayerBudgetEnableEditorAttribute
{	
	protected override EEditableEntityBudget GetBudgetType()
	{
		return EEditableEntityBudget.PROPS;
	}	
	
	protected override ENotification GetEnableNotification()
	{
		return ENotification.EDITOR_ENABLE_PROP_BUDGET;
	}
	
	protected override ENotification GetDisableNotification()
	{
		return ENotification.EDITOR_DISABLE_PROP_BUDGET;
	}	
};