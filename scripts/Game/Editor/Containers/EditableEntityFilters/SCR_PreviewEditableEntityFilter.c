[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_PreviewEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	private SCR_PreviewEntityEditorComponent m_PreviewManager;
	
	protected void GetPreviewEntities(IEntity previewEntity, out notnull set<SCR_EditableEntityComponent> outEntities)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(previewEntity);
		if (entity && entity.IsRegistered()) outEntities.Insert(entity);
		
		IEntity child = previewEntity.GetChildren();
		while (child)
		{
			GetPreviewEntities(child, outEntities);
			child = child.GetSibling();
		}
	}
	protected void OnPreviewCreate(SCR_BasePreviewEntity previewEntity)
	{
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
		GetPreviewEntities(previewEntity, entities);
		Add(entities, true);
	}
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return SCR_BasePreviewEntity.Cast(entity.GetOwner()) != null;
	}
	override void EOnEditorActivate()
	{
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, true));
		if (!m_PreviewManager) return;
		
		m_PreviewManager.GetOnPreviewCreate().Insert(OnPreviewCreate);
		m_PreviewManager.GetOnWaitingPreviewCreate().Insert(OnPreviewCreate);
	}
	override void EOnEditorDeactivate()
	{
		if (!m_PreviewManager) return;
		
		m_PreviewManager.GetOnPreviewCreate().Remove(OnPreviewCreate);
		m_PreviewManager.GetOnWaitingPreviewCreate().Remove(OnPreviewCreate);
	}
};