
//! Returns true if any hovered entity has children (returns false if not hovered entity)
[BaseContainerProps(), BaseContainerCustomStringTitleField("Can Enter Layer")]
class SCR_EditorCanEnterLayerActionCondition: SCR_AvailableActionCondition
{	
	
	SCR_LayersEditorComponent m_LayersManager;
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		SCR_EditableEntityComponent entity = SCR_BaseEditableEntityFilter.GetFirstEntity(EEditableEntityState.HOVER);
		
		if (!entity)
			return GetReturnResult(false);
		
		if (!m_LayersManager)
		{
			m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
			if (!m_LayersManager)
				return false;
		}
		
		return m_LayersManager.IsEditingLayersEnabled() && GetReturnResult(entity.CanEnterLayer(m_LayersManager));
	}
};