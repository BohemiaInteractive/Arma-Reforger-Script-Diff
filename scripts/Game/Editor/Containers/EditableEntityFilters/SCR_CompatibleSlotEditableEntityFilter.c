[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_CompatibleSlotEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	private ResourceName m_PreviewSlotPrefab;
	private bool m_bIsPreview;
	private SCR_PrefabsCacheEditorComponent m_PrefabsCache;
	private SCR_PreviewEntityEditorComponent m_PreviewManager;
	
	protected void OnPreviewCreate()
	{
		m_PreviewSlotPrefab = m_PreviewManager.GetSlotPrefab();// previewEntity.GetSlotPrefab();
		if (m_PreviewSlotPrefab.IsEmpty()) return;
		
		m_bIsPreview = true;
		SetFromPredecessor();
	}
	protected void OnPreviewDelete()
	{
		m_bIsPreview = false;
		m_PreviewSlotPrefab = "";
		Clear();
	}
	
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		if (m_PrefabsCache && entity.HasEntityFlag(EEditableEntityFlag.SLOT))
		{
			return m_PrefabsCache.IsPrefabInherited(m_PreviewSlotPrefab, entity.GetPrefab());
		}
		else
		{
			return m_bIsPreview && m_PreviewSlotPrefab == entity.GetPrefab();
		}
		
		ResourceName prefab = entity.GetPrefab();
				
	}
	override void EOnEditorActivate()
	{
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, true));
		if (!m_PreviewManager)
			return;
		
		m_PrefabsCache = SCR_PrefabsCacheEditorComponent.Cast(SCR_PrefabsCacheEditorComponent.GetInstance(SCR_PrefabsCacheEditorComponent));
		
		m_PreviewManager.GetOnPreviewCreate().Insert(OnPreviewCreate);
		m_PreviewManager.GetOnPreviewDelete().Insert(OnPreviewDelete);
	}
	override void EOnEditorDeactivate()
	{
		if (!m_PreviewManager)
			return;
		
		m_PreviewManager.GetOnPreviewCreate().Remove(OnPreviewCreate);
		m_PreviewManager.GetOnPreviewDelete().Remove(OnPreviewDelete);
	}
};