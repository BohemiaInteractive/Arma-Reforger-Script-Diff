[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_CreateLayerWithSelectionContextAction : SCR_BaseContextAction
{
	protected ref set<SCR_EditableEntityComponent> m_EntitiesToMove = new set<SCR_EditableEntityComponent>();
	protected SCR_EditableEntityCore m_Core;
	protected SCR_BaseEditableEntityFilter m_SelectionFilter;
	protected bool m_bSelectionCanceled;
	
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		if (!m_Core)
		{	
			m_Core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			if (!m_Core)
				return false;
		}
		
		if (!m_SelectionFilter)
		{
			m_SelectionFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
			if (!m_SelectionFilter)
				return false;
		}
		
		SCR_LayersEditorComponent layerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (!layerManager || !layerManager.IsEditingLayersEnabled())
			return false;
		
		bool hasValidEntities = false;
		
		SCR_EditableEntityComponent parent;
		
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			if (entity)
			{
				parent = entity.GetParentEntity();
				break;
			}
		}
		
		EEditableEntityFlag newLayerFlags;
		newLayerFlags |= EEditableEntityFlag.LAYER;
		int count = selectedEntities.Count();
		
		SCR_EditableEntityInteraction interaction;
		for (int i = 0; i < count; i++)
		{
			interaction = selectedEntities[i].GetEntityInteraction();
			if (interaction && interaction.CanCreateParentFor(EEditableEntityType.GENERIC, newLayerFlags, parent, i == 0))
			{
				hasValidEntities = true;
				break;
			}
		}
		
		
		return hasValidEntities;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{		
		SCR_LayersEditorComponent layerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (!layerManager) return;
		
		
		//If selected the current layer the user is in
		if (selectedEntities.Contains(layerManager.GetCurrentLayer()))
		{
			selectedEntities.Clear();
			selectedEntities.Insert(layerManager.GetCurrentLayer());
			layerManager.SetCurrentLayer(layerManager.GetCurrentLayer().GetParentEntity());
		}
		
		EEditableEntityFlag newLayerFlags;
		newLayerFlags |= EEditableEntityFlag.LAYER;
		SCR_EditableEntityComponent parent = selectedEntities[0].GetParentEntity();
		m_EntitiesToMove.Clear();
		int count = selectedEntities.Count();
		
		SCR_EditableEntityInteraction interaction;
		for (int i = 0; i < count; i++)
		{		
			interaction = selectedEntities[i].GetEntityInteraction();	
			if (interaction && interaction.CanCreateParentFor(EEditableEntityType.GENERIC, newLayerFlags, parent, i == 0))
			{
				m_EntitiesToMove.Insert(selectedEntities[i]);
			}
		}
		
		m_Core.Event_OnParentEntityChanged.Insert(OnEditableParentChanged);
		m_SelectionFilter.GetOnChanged().Insert(OnSelectionChanged);
		
		layerManager.CreateNewLayerWithSelected(m_EntitiesToMove, cursorWorldPosition);
	}
	
	//Parent was changed so set these as selected
	protected void OnEditableParentChanged(SCR_EditableEntityComponent entity, SCR_EditableEntityComponent newParent,  SCR_EditableEntityComponent prevParent)
	{
		int index = m_EntitiesToMove.Find(entity);
		if (index > -1)
		{
			m_EntitiesToMove.Remove(index);
			
			if (m_EntitiesToMove.IsEmpty())
			{
				if (m_Core)
					m_Core.Event_OnParentEntityChanged.Remove(OnEditableParentChanged);
				if (m_SelectionFilter)
					m_SelectionFilter.GetOnChanged().Remove(OnSelectionChanged);
				
				m_SelectionFilter.Add(newParent, true);
			}	
		}
	}
	
	//If any selection is changed Remove the events
	protected void OnSelectionChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesSelected, set<SCR_EditableEntityComponent> entitiesDeselected)
	{			
		if (m_Core)
			m_Core.Event_OnParentEntityChanged.Remove(OnEditableParentChanged);
		if (m_SelectionFilter)
			m_SelectionFilter.GetOnChanged().Remove(OnSelectionChanged);
	}

};