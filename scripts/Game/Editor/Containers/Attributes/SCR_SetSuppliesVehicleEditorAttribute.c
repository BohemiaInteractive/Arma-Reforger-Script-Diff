[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SetSuppliesVehicleEditorAttribute : SCR_SetSuppliesBaseEditorAttribute
{	
	protected override bool CanDisplay(SCR_EditableEntityComponent editableEntity)
	{
		//~ Ignore if not vehicle
		return editableEntity.GetEntityType() == EEditableEntityType.VEHICLE;
	}
}