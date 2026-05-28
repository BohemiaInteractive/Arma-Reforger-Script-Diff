//! Returns true if any selected entities is a group
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_EnityType", "Selected Entities Contains Type: %1")]
class SCR_SelectedEntitiesContainsTypeActionComponent : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EEditableEntityType.GENERIC.ToString(), desc: "Enity Type", uiwidget: UIWidgets.ComboBox, enumType: EEditableEntityType)]
	protected EEditableEntityType m_EnityType;

	//~Todo: Once Command filter works correctly this is no longer needed to check if group and waypoints are selected
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = data.GetEditorEntitiesManagerComponent();
		if (!entitiesManager)
			return GetReturnResult(false);

		SCR_BaseEditableEntityFilter filter = entitiesManager.GetFilter(EEditableEntityState.SELECTED);
		if (!filter)
			return GetReturnResult(false);

		set <SCR_EditableEntityComponent> entities = new set <SCR_EditableEntityComponent>;
		filter.GetEntities(entities);

		foreach (SCR_EditableEntityComponent entity : entities)
		{
			if (entity.GetEntityType() == m_EnityType)
				return GetReturnResult(true);
		}

		return GetReturnResult(false);
	}
}
