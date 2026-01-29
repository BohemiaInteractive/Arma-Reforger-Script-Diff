[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_CommandPrefab", true)]
class SCR_TaskBaseCommandAction : SCR_BaseCommandAction
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
		if (!(flags & EEditorCommandActionFlags.OBJECTIVE))
			return false;
		
		Resource resource = Resource.Load(m_CommandPrefab);
		return resource.IsValid();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		super.Perform(hoveredEntity, selectedEntities, cursorWorldPosition, EEditorCommandActionFlags.ATTACH, param);
	}
};