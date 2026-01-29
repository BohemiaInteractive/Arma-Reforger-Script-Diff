
//! Returns true if a entity is hovered and that entity is selected
[BaseContainerProps(), BaseContainerCustomStringTitleField("Hovered entity selected")]
class SCR_EditorHoveredEntityIsSelectedActionCondition: SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		SCR_EditableEntityComponent entity = SCR_BaseEditableEntityFilter.GetFirstEntity(EEditableEntityState.HOVER);
		
		if (!entity)
			return GetReturnResult(false);
		
		set <SCR_EditableEntityComponent> entities = new set <SCR_EditableEntityComponent>;
		SCR_BaseEditableEntityFilter.GetEnititiesStatic(entities, EEditableEntityState.SELECTED);
		
		if (entities.Contains(entity))
			return GetReturnResult(true);

		SCR_EditableEntityComponent parent = entity.GetParentEntity();
		if (!parent)
			return GetReturnResult(false);
		
		return GetReturnResult(entities.Contains(parent));
	}
};