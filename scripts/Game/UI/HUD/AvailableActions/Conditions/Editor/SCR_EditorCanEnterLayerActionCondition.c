
//! Returns true if any hovered entity has children (returns false if not hovered entity)
[BaseContainerProps(), BaseContainerCustomStringTitleField("Can Enter Layer")]
class SCR_EditorCanEnterLayerActionCondition: SCR_AvailableActionCondition
{
	SCR_LayersEditorComponent m_LayersManager;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = data.GetEditorEntitiesManagerComponent();
		if (!entitiesManager)
			return false;

		SCR_BaseEditableEntityFilter filter = entitiesManager.GetFilter(EEditableEntityState.HOVER);
		if (!filter)
			return false;

		SCR_EditableEntityComponent entity = filter.GetFirstEntity();
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
}