[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PlayerBudgetVehicleEnableEditorAttribute: SCR_BasePlayerBudgetEnableEditorAttribute
{	
	protected override EEditableEntityBudget GetBudgetType()
	{
		return EEditableEntityBudget.VEHICLES;
	}	
	
	protected override ENotification GetEnableNotification()
	{
		return ENotification.EDITOR_ENABLE_VEHICLE_BUDGET;
	}
	
	protected override ENotification GetDisableNotification()
	{
		return ENotification.EDITOR_DISABLE_VEHICLE_BUDGET;
	}	
};