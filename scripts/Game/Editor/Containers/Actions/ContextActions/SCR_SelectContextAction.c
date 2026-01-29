[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SelectContextAction : SCR_BaseContextAction
{
	[Attribute("1", UIWidgets.ComboBox, "Entity filter", "", ParamEnumArray.FromEnum(EEditableEntityState) )]
	EEditableEntityState m_EditableEntityFilter;
	[Attribute(desc: "Filter entities based on hovered type")]
	bool m_UseEntityTypeFilter;
	[Attribute(desc: "Filter entities only in view")]
	bool m_OnlyInView;
	
	protected EEditableEntityType m_FilterType = EEditableEntityType.GENERIC;
	
	protected bool IsEntityIsInView(WorkspaceWidget workspace, SCR_EditableEntityComponent entity, int screenW, int screenH)
	{
		vector posWorld;
		if (!entity.GetPos(posWorld)) return false;
		
		vector posScreen = workspace.ProjWorldToScreenNative(posWorld, entity.GetOwner().GetWorld()); //--- ToDo: Also check for distance visibility
		return (posScreen[2] > 0 && posScreen[0] > 0 && posScreen[0] < screenW && posScreen[1] > 0 && posScreen[1] < screenH);
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (m_UseEntityTypeFilter && hoveredEntity == null)
		{
			return false;
		}
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		SCR_SelectedEditableEntityFilter selectedEntitiesManager = SCR_SelectedEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.SELECTED));
		
		if (!entitiesManager || !selectedEntitiesManager)
		{
			return;
		}
		
		if (m_UseEntityTypeFilter)
		{
			m_FilterType = hoveredEntity.GetEntityType();
		}
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		int screenW, screenH;
		screenW = workspace.GetWidth();
		screenH = workspace.GetHeight();			
		
		set<SCR_EditableEntityComponent> filteredEntities = new set<SCR_EditableEntityComponent>;
		entitiesManager.GetEntities(m_EditableEntityFilter, filteredEntities);
		
		for (int i = 0; i < filteredEntities.Count(); i++)
		{
			if (m_UseEntityTypeFilter && filteredEntities[i].GetEntityType() != m_FilterType)
			{
				continue;
			}
			if (m_OnlyInView && !IsEntityIsInView(workspace, filteredEntities[i], screenW, screenH))
			{
				continue;
			}
			selectedEntitiesManager.Add(filteredEntities[i]);
		}
	}
};