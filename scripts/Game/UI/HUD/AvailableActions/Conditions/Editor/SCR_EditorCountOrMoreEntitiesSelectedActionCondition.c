//! Returns true if count or more is selected
[BaseContainerProps(), BaseContainerCustomStringTitleField("Count or more selected")]
class SCR_EditorCountOrMoreEntitiesSelectedActionCondition: SCR_AvailableActionCondition
{		
	[Attribute("1", desc: "if this amount or higher entities selected. Will return true")]
	protected int m_iCount;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = data.GetEditorEntitiesManagerComponent();
		if (!entitiesManager)
			return false;

		SCR_BaseEditableEntityFilter filter = entitiesManager.GetFilter(EEditableEntityState.SELECTED);
		if (!filter)
			return false;

		set <SCR_EditableEntityComponent> entities = new set <SCR_EditableEntityComponent>;
		return GetReturnResult(filter.GetEntities(entities) >= m_iCount);
	}
}
