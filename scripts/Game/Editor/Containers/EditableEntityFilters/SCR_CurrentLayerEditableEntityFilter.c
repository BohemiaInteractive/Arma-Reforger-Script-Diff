[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_CurrentLayerEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	private SCR_LayersEditorComponent m_LayersManager;
	
	protected void OnCurrentLayerChange(SCR_EditableEntityComponent currentLayer, SCR_EditableEntityComponent currentLayerPrev)
	{
		Replace(currentLayer, true);
	}
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return entity ==  m_LayersManager.GetCurrentLayer();
	}
	override void EOnEditorActivate()
	{
		m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (m_LayersManager) m_LayersManager.Event_OnCurrentLayerChange.Insert(OnCurrentLayerChange);
	}
	override void EOnEditorDeactivate()
	{
		if (m_LayersManager) m_LayersManager.Event_OnCurrentLayerChange.Remove(OnCurrentLayerChange);
	}
};