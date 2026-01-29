[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_CommandPrefab", true)]
class SCR_SpawnPointBaseCommandAction : SCR_BaseCommandAction
{
	override void FilterEntities(notnull set<SCR_EditableEntityComponent> inEntities, out notnull set<SCR_EditableEntityComponent> outEntities)
	{
		foreach (SCR_EditableEntityComponent entity: inEntities)
		{
			if (entity.GetFaction())
				outEntities.Insert(entity);
		}
	}
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return flags & EEditorCommandActionFlags.OBJECTIVE;
	}
};