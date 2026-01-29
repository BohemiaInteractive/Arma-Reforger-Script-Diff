//! Returns true if given editor entity filter is not empty
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State", "SCR_EditorFilterActionCondition: %1")]
class SCR_EditorFilterActionCondition: SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_State;

	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		return GetReturnResult(SCR_BaseEditableEntityFilter.GetFirstEntity(m_State) != null);
	}
};