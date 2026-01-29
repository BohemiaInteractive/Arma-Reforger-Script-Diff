[BaseContainerProps(), BaseContainerCustomStringTitleField("Hovered entity has flags")]
class SCR_HoveredEntityHasFlagsActionComponent: SCR_AvailableActionCondition
{	
	[Attribute("", UIWidgets.Flags, desc: "Check if editable entity has the selected flags.", enums: ParamEnumArray.FromEnum(EEditableEntityFlag))]
	protected EEditableEntityFlag m_Flags;
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		SCR_EditableEntityComponent entity = SCR_BaseEditableEntityFilter.GetFirstEntity(EEditableEntityState.HOVER);
		
		if (!entity)
			return false;
		
		return GetReturnResult(entity.HasEntityFlag(m_Flags));
	}
};