//! Returns true if any selected entities is a group
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_EnityType", "Selected Entities Contains Type: %1")]
class SCR_SelectedEntitiesContainsTypeActionComponent: SCR_AvailableActionCondition
{	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	protected EEditableEntityType m_EnityType;
	
	//~Todo: Once Command filter works correctly this is no longer needed to check if group and waypoints are selected
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		set <SCR_EditableEntityComponent> entities = new set <SCR_EditableEntityComponent>;
		SCR_BaseEditableEntityFilter.GetEnititiesStatic(entities, EEditableEntityState.SELECTED);
		
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			if (entity.GetEntityType() == m_EnityType)
				return GetReturnResult(true);
		}
		
		return GetReturnResult(false);
	}
};