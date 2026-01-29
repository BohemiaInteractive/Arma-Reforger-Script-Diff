//! Returns true if all selected entities have less or equal to waypoint count
[BaseContainerProps(), BaseContainerCustomStringTitleField("Have less or equal to waypoint count")]
class SCR_EditorLessThenCountWaypointsActionCondition: SCR_AvailableActionCondition
{		
	[Attribute("1", desc: "Returns true if all selected entities have less or equal to waypoint count")]
	protected int m_iCount;
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		set <SCR_EditableEntityComponent> entities = new set <SCR_EditableEntityComponent>;
		SCR_BaseEditableEntityFilter.GetEnititiesStatic(entities, EEditableEntityState.SELECTED);
		
		int count = 0;
		int maxCount = 0;
		
		SCR_EditableGroupComponent group;
		set <SCR_EditableEntityComponent> groupChildren = new set <SCR_EditableEntityComponent>;
		
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			group = SCR_EditableGroupComponent.Cast(entity);
			
			if (group)
			{
				count = group.GetWaypointCount();
				
				if (count > maxCount)
					maxCount = count; 
			}			
		}
		
		return GetReturnResult(maxCount <= m_iCount);
	}
};