[BaseContainerProps(), BaseContainerCustomStringTitleField("Hovered entity has flags")]
class SCR_HoveredEntityHasFlagsActionCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EEditableEntityFlag.PLACEABLE.ToString(), desc: "Check if editable entity has the selected flags.", uiwidget: UIWidgets.ComboBox, enumType: EEditableEntityFlag)]
	protected EEditableEntityFlag m_Flags;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = data.GetEditorEntitiesManagerComponent();
		if (!entitiesManager)
			return false;

		SCR_BaseEditableEntityFilter filter = entitiesManager.GetFilter(EEditableEntityState.HOVER);
		if (!filter)
			return false;

		SCR_EditableEntityComponent entity = filter.GetFirstEntity();
		return GetReturnResult(entity && entity.HasEntityFlag(m_Flags));
	}
}