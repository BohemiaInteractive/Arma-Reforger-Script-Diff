//! @ingroup ManualCamera

//! Snap camera to specific target.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_EditorSnapManualCameraComponent : SCR_BaseManualCameraComponent
{	
	[Attribute("8", desc: "How agressively does the camera rotates towards the entity under cursor.")]
	private float m_fSnapStrength;
	
	[Attribute("0.25", uiwidget: UIWidgets.Slider, desc: "When hovering over geometry, what's its maximum screen ration for the snap to happen.\nIt would be impractical to snap to too large objects, especially when they're encompassing the camera (e.g., building interiors).", params: "0 1 0.01")]
	private float m_fMaxGeometryRatio;
	
	private bool m_bSnapped;
	private SCR_EditableEntityComponent m_EntityPrev;
	private vector m_vPosPrev;
	private vector m_vRotPrev;
	
	private SCR_HoverEditableEntityFilter m_HoverManager;
	private SCR_TransformingEditorComponent m_TransformingManager;
	private SCR_PreviewEntityEditorComponent m_PreviewManager;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled)
			return;
		
		if (!m_HoverManager)
		{
			Print(string.Format("%1 requires SCR_HoverEditableEntityFilter on the editor manager!", Type()), LogLevel.ERROR);
			SetEnabled(false);
			return;
		}

		if (!GetInputManager() || GetInputManager().IsUsingMouseAndKeyboard())
			return;

		bool isDelegate = false;
		SCR_EditableEntityComponent entity = m_HoverManager.GetEntityUnderCursor(isDelegate);
		if (!entity)
		{
			ResetSnap(param.transform, entity);
			return;
		}
		
		//--- Ignore currently edited entity
		if (m_TransformingManager && m_TransformingManager.IsEditing(entity))
			return;
		
		//--- Ignore when transforming an entity vertically (would mess up camera snapping there)
		if (m_PreviewManager && m_PreviewManager.IsMovingVertically())
			return;
		
		//--- Geometry (not icon) under cursor, check if it's not too close
		if (!isDelegate)
		{
			vector bboxMin, bboxMax;
			IEntity owner = entity.GetOwner();
			owner.GetWorldBounds(bboxMin, bboxMax);
			if (vector.Distance(bboxMin, bboxMax) > 0)
			{
				WorkspaceWidget workspace = GetGame().GetWorkspace();
				BaseWorld world = owner.GetWorld();
				bboxMin = workspace.ProjWorldToScreen(bboxMin, world);
				bboxMax = workspace.ProjWorldToScreen(bboxMax, world);
				int refW, refH;
				WidgetManager.GetReferenceScreenSize(refW, refH);
				if (vector.Distance(bboxMin, bboxMax) / refH > m_fMaxGeometryRatio)
				{
					//--- Too close, ignore
					ResetSnap(param.transform, entity);
					return;
				}
			}
		}
		
		//--- Don't start snap when camera is moving
		if (!m_bSnapped && (entity != m_EntityPrev || param.transform[3] != m_vPosPrev || param.transform[2] != m_vRotPrev))
		{
			ResetSnap(param.transform, entity);
			return;
		}
		
		//--- Snap to entity's composition
		//entity = m_HoverManager.GetParentBelowCurrentLayer(entity);

		vector target;
		if (!entity.GetPos(target))
			return;

		target = CoordToCamera(target);
				
		target = vector.Direction(param.transform[3], target).Normalized();
		param.transform[2] = vector.Lerp(param.transform[2], target, Math.Min(m_fSnapStrength * param.timeSlice, 1));
		
		m_bSnapped = true;
		m_EntityPrev = entity;
		m_vPosPrev = param.transform[3];
		m_vRotPrev = param.transform[2];
		
		param.isDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ResetSnap(vector transformationNew[4], SCR_EditableEntityComponent entity)
	{
		m_EntityPrev = entity;
		m_vPosPrev = transformationNew[3];
		m_vRotPrev = transformationNew[2];
		m_bSnapped = false;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_TransformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
			m_HoverManager = SCR_HoverEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.HOVER));

		return true;
	}
}
