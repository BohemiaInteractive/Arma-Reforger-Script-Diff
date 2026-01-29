//! Returns true if count or more is selected
[BaseContainerProps(), BaseContainerCustomStringTitleField("Count or more selected")]
class SCR_EditorCountOrMoreEntitiesSelectedActionCondition: SCR_AvailableActionCondition
{		
	[Attribute("1", desc: "if this amount or higher entities selected. Will return true")]
	protected int m_iCount;
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		set <SCR_EditableEntityComponent> entities = new set <SCR_EditableEntityComponent>;
		int count = SCR_BaseEditableEntityFilter.GetEnititiesStatic(entities, EEditableEntityState.SELECTED);
		
		return GetReturnResult(count >= m_iCount);
	}
};