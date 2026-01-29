[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
Entities under the current layer, or recursively under on of its child layers.
*/
class SCR_LayerEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	[Attribute()]
	protected bool m_bOnlyDirect;
	
	[Attribute()]
	protected bool m_bAllowExceptions;
	
	protected SCR_LayersEditorComponent m_LayersManager;
	
	protected void OnCurrentLayerChange(SCR_EditableEntityComponent currentLayer, SCR_EditableEntityComponent currentLayerPrev)
	{
		set<SCR_EditableEntityComponent> entitiesInsert = new set<SCR_EditableEntityComponent>;
		if (currentLayer) 
		{
			entitiesInsert.Insert(currentLayer);
		}
		else
		{
			//--- When the current layer is null (i.e., root), get entities from the core
			GetCore().GetAllEntities(entitiesInsert, true);
		}
		
		Replace(entitiesInsert, false, true);
	}
	
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return !m_LayersManager || m_LayersManager.IsUnderCurrentLayer(entity, m_bOnlyDirect, m_bAllowExceptions);
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