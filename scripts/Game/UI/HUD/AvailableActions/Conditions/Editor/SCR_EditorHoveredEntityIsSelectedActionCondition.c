//! Returns true if a entity is hovered and that entity is selected
[BaseContainerProps(), BaseContainerCustomStringTitleField("Hovered entity selected")]
class SCR_EditorHoveredEntityIsSelectedActionCondition: SCR_AvailableActionCondition
{
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = data.GetEditorEntitiesManagerComponent();
		if (!entitiesManager)
			return false;

		SCR_BaseEditableEntityFilter filter = entitiesManager.GetFilter(EEditableEntityState.HOVER);
		if (!filter)
			return false;

		SCR_EditableEntityComponent entity = filter.GetFirstEntity();
		if (!entity)
			return GetReturnResult(false);
		
		filter = entitiesManager.GetFilter(EEditableEntityState.SELECTED);
		if (!filter)
			return false;
		
		set <SCR_EditableEntityComponent> entities = new set <SCR_EditableEntityComponent>;
		filter.GetEntities(entities);
		if (entities.Contains(entity))
			return GetReturnResult(true);

		SCR_EditableEntityComponent parent = entity.GetParentEntity();
		return GetReturnResult(parent && entities.Contains(parent));
	}
}