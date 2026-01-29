[ComponentEditorProps(category: "GameScripted/Editor", description: "Manager of ability actions in editor", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_ToolbarActionsEditorComponentClass: SCR_BaseActionsEditorComponentClass
{
};

class SCR_ToolbarActionsEditorComponent : SCR_BaseActionsEditorComponent
{
	void OnFilterChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		ValidateSelection(false);
	}
	
	override bool ActionCanBeShown(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return action.GetInfo() && action.CanBeShown(m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags);
	}
	
	override bool ActionCanBePerformed(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return action.CanBePerformed(m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags);
	}

	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		
		if (m_SelectedManager)
		{
			m_SelectedManager.GetOnChanged().Insert(OnFilterChange);
		}
		if (m_HoverManager)
		{
			m_HoverManager.GetOnChanged().Insert(OnFilterChange);
		}
	}
	
	override void EOnEditorDeactivate()
	{
		super.EOnEditorDeactivate();
		
		if (m_SelectedManager)
		{
			m_SelectedManager.GetOnChanged().Remove(OnFilterChange);
		}		
		if (m_HoverManager)
		{
			m_HoverManager.GetOnChanged().Remove(OnFilterChange);
		}
	}
};