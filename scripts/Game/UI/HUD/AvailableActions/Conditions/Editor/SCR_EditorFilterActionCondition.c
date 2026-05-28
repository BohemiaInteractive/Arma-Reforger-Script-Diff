//! Returns true if given editor entity filter is not empty
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State", "SCR_EditorFilterActionCondition: %1")]
class SCR_EditorFilterActionCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EEditableEntityState.UNLOCKED.ToString(), desc: "State", uiwidget: UIWidgets.ComboBox, enumType: EEditableEntityState)]
	protected EEditableEntityState m_State;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = data.GetEditorEntitiesManagerComponent();
		if (!entitiesManager)
			return GetReturnResult(false);

		SCR_BaseEditableEntityFilter filter = entitiesManager.GetFilter(m_State);
		return GetReturnResult(filter && filter.GetFirstEntity() != null);
	}
}
