[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_DuplicateContextAction : SCR_BaseContextAction
{
	private SCR_PlacingEditorComponent m_PlacingManager;
	private ref set<SCR_EditableEntityComponent> m_Recipients = new set<SCR_EditableEntityComponent>;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (hoveredEntity == null || selectedEntities.Count() > 1)
		{
			return false;
		}
		if (!hoveredEntity.CanDuplicate(m_Recipients))
		{
			return false;
		}
		
		m_PlacingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, false, true));
		return m_PlacingManager != null;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if(!m_PlacingManager)
		{
			return;
		}
		
		m_Recipients.Clear();
		EntityPrefabData hoveredPrefabData = hoveredEntity.GetOwner().GetPrefabData();
		if (hoveredPrefabData && hoveredEntity.CanDuplicate(m_Recipients))
		{
			m_PlacingManager.SetSelectedPrefab(hoveredPrefabData.GetPrefabName(), false, true, m_Recipients);
		}
	}
};