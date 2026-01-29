[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_CommandPrefab", true)]
class SCR_WaypointBaseCommandAction : SCR_BaseCommandAction
{
	//------------------------------------------------------------------------------------------------
	override void FilterEntities(notnull set<SCR_EditableEntityComponent> inEntities, out notnull set<SCR_EditableEntityComponent> outEntities)
	{
		SCR_EditableEntityComponent group;
		foreach (SCR_EditableEntityComponent entity: inEntities)
		{
			group = entity.GetAIGroup();
			if (group)
				outEntities.Insert(group);
		}
	}
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return flags & EEditorCommandActionFlags.WAYPOINT;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		flags |= EEditorCommandActionFlags.ATTACH;
		super.Perform(hoveredEntity, selectedEntities, cursorWorldPosition, flags, param);
	}
};