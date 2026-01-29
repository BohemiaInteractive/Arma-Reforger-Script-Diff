//! @ingroup ManualCamera

//! Detect editable entity under cursor
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_EditorHoveredManualCameraComponent : SCR_BaseManualCameraComponent
{
	private SCR_HoverEditableEntityFilter m_HoverManager;
	private SCR_PreviewEntityEditorComponent m_PreviewManager;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{		
		if (m_PreviewManager)
		{
			IEntity previewEntity = m_PreviewManager.GetPreviewEntity();
			if (previewEntity)
			{
				param.SetCursorWorldPos(previewEntity.GetTransformAxis(3));
				param.target = previewEntity;
				return;
			}
		}

		if (m_HoverManager)
		{
			bool isDelegate;
			SCR_EditableEntityComponent entity = m_HoverManager.GetEntityUnderCursor(isDelegate);
			if (!entity) return;
			
			param.target = entity.GetOwner();
			if (!isDelegate) return;
			
			vector pos;
			if (entity.GetPos(pos)) param.SetCursorWorldPos(pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
		if (entitiesManager)
			m_HoverManager = SCR_HoverEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.HOVER));

		return true;
	}
}
