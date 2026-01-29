[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_MoveToRootContextAction : SCR_BaseContextAction
{	
	bool ParentInSelection(SCR_EditableEntityComponent entity, notnull set<SCR_EditableEntityComponent> selectedEntities)
	{				
		SCR_EditableEntityComponent parent = entity.GetParentEntity();
		while (parent)
		{
			if (selectedEntities.Find(parent) != -1)
			{
				return true;
			}
			parent = parent.GetParentEntity();
		}
		return false;
	}

	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{					
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if	(!layersManager || !layersManager.IsEditingLayersEnabled())
			return false;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return false;
		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (entity.CanSetParent(null) && entity.GetEntityType() != EEditableEntityType.CHARACTER)
				return true;
		}
		
		return false;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if	(!layersManager || !layersManager.IsEditingLayersEnabled())
			return false;
		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (ParentInSelection(entity, selectedEntities))
				continue;

			if (entity.GetParentEntity())
			{
				return true;
			}
		}
		
		return false;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{	
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		
		set<SCR_EditableEntityComponent> entitiesToMove = new set<SCR_EditableEntityComponent>();
		
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			if (!entity.CanSetParent(null) && entity.GetEntityType() != EEditableEntityType.CHARACTER)
				continue;
			
			entitiesToMove.Insert(entity);
		}
		
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (layersManager)
		{			
			layersManager.MoveToLayerAndSelect(entitiesToMove, null);
		}
	}
	
};
