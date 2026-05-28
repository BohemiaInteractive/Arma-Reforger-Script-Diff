//! Returns true if all selected entities have less or equal to waypoint count
[BaseContainerProps(), BaseContainerCustomStringTitleField("Have less or equal to waypoint count")]
class SCR_EditorLessThenCountWaypointsActionCondition : SCR_AvailableActionCondition
{
	[Attribute("1", desc: "Returns true if all selected entities have less or equal to waypoint count")]
	protected int m_iCount;

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

		int count;
		int maxCount;
		SCR_EditableGroupComponent group;
		foreach (SCR_EditableEntityComponent entity : entities)
		{
			group = SCR_EditableGroupComponent.Cast(entity);
			if (!group)
				continue;

			count = group.GetWaypointCount();
			if (count > maxCount)
				maxCount = count;
		}

		return GetReturnResult(maxCount <= m_iCount);
	}
}
