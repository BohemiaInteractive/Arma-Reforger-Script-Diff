//--- For gamepad only
enum EPreviewEntityEditorOperation
{
	MOVE_HORIZONTAL,
	MOVE_VERTICAL,
	ROTATE,
}

//! @ingroup Editor_UI Editor_UI_Components

class SCR_PreviewEntityEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected static const int MIN_CURSOR_DIS_TO_TRANSFORM = 10; //--- Upon clicking, how far must cursor move to initiate
	protected static const float TRACE_DIS = 2000;
	
	[Attribute(defvalue: "30")]
	protected float m_fMoveVerticalCoef;
	
	[Attribute(defvalue: "-500")]
	protected float m_fRotationCoef;
	
	[Attribute(defvalue: "0.05")]
	protected float m_fRotationInertia;
	
	[Attribute(defvalue: "0.25")]
	protected float m_fUnsnapDuration;
	
	[Attribute(defvalue: "45", desc: "When vertical mode is GEOMETRY, hovering over surface with at least this slope will place preview entity below the surface, not on it.\n\nCurrently disabled, entity editing will be disable beyond this angle.")]
	protected float m_fMinAngleWall;
	
	[Attribute(defvalue: "101", desc: "When vertical mode is GEOMETRY, hovering over surface with at least this slope will prevent showing preview entity and confirming the change.")]
	protected float m_fMinAngleCeiling;
	
	[Attribute(defvalue: "1", desc: "When adjusting entity height, camera will move together with the entity when above this vertical offset.\nWhen below, the camera will rotate towards the entity instead.")]
	protected float m_fMinCameraVerticalOffset;
	
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_DirIndicatorPrefab;
	
	protected InputManager m_InputManagerBase;
	protected SCR_PreviewEntityEditorComponent m_PreviewEntityManager;
	protected SCR_CameraEditorComponent m_CameraManagerBase;
	protected SCR_PlacingEditorComponent m_PlacingManager;
	protected SCR_CursorEditorUIComponent m_CursorComponentBase;
	protected SCR_HoverEditableEntityFilter m_HoverFilter;
	protected BaseWorld m_World;
	protected vector m_vClickTransformBase[4];
	protected vector m_vClickPosBase;
	protected vector m_vClickPosWorldBase;
	protected bool m_bMouseMoved;
	protected bool m_bIsRotatingTowardsCursor;
	protected SCR_EditableEntityComponent m_Target;
	protected float m_fTargetYaw;
	protected float m_fUnsnapProgress;
	protected vector m_vTransformOrigin[4];
	protected vector m_vAnglesOrigin;
	protected vector m_vAnimatedTransform[4];
	protected vector m_vRotationPivot;
	protected bool m_bIsAnimated;
	protected vector m_vTerrainNormal;
	protected EPreviewEntityEditorOperation m_Operation;
	protected IEntity m_DirIndicator;
	protected float m_fDirIndicatorScale;
	protected ref TraceParam m_RotationTrace;
	
	//------------------------------------------------------------------------------------------------
	protected void OnHoverChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (!m_PreviewEntityManager || !m_PreviewEntityManager.IsEditing() || m_PreviewEntityManager.IsFixedPosition() || m_bIsRotatingTowardsCursor)
			return;
		
		if (entitiesRemove && entitiesRemove.Count() == 1)
		{
			if (m_Target && !m_bIsRotatingTowardsCursor)
				m_bIsAnimated = m_PreviewEntityManager.SetTarget(null);

			m_Target = null;
			m_PreviewEntityManager.GetPreviewTransform(m_vAnimatedTransform);
		}

		if (entitiesInsert && entitiesInsert.Count() == 1)
		{
			SCR_EditableEntityComponent entity = entitiesInsert[0];
			bool isDelegate = true;
			if (m_HoverFilter)
				isDelegate = m_HoverFilter.IsDelegate();

			if (m_PreviewEntityManager.SetTarget(entity, isDelegate))
			{
				m_Target = entity;
				m_PreviewEntityManager.GetPreviewTransform(m_vAnimatedTransform);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPreviewCreate()
	{
		m_PreviewEntityManager.GetPreviewTransformOrigin(m_vTransformOrigin);
		m_vAnglesOrigin = m_vTransformOrigin[2].VectorToAngles();
		m_bIsAnimated = false;
		SetRotationPivot();
		
		//--- Hotfix: OnEditorTransformRotationModifierUp is not called when not editing, this will reset it. ToDo: Remove
		if (!m_InputManagerBase.GetActionTriggered("EditorTransformRotateYawModifier"))
			OnEditorTransformRotationModifierUp(0, 0);
		
		//--- Activate currently hovered entity.
		//--- For example, on client, dragging a soldier on a vehicle before server callback about transformation arrives would not recognize the hover.
		set<SCR_EditableEntityComponent> hoverEntities = new set<SCR_EditableEntityComponent>();
		m_HoverFilter.GetEntities(hoverEntities);
		OnHoverChange(EEditableEntityState.HOVER, hoverEntities, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPreviewDelete()
	{
		m_bIsRotatingTowardsCursor = false;
		delete m_DirIndicator;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorTransformSnapToSurface()
	{
		m_PreviewEntityManager.ResetPreviewHeight();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorTransformRotationModifierDown(float value, EActionTrigger reason)
	{
		if (!m_CursorComponentBase)
			return;
		
		SetRotationPivot();
		SetClickPos(m_CursorComponentBase.GetCursorPos());		
		m_bIsRotatingTowardsCursor = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorTransformRotationModifierUp(float value, EActionTrigger reason)
	{
		if (!m_bIsRotatingTowardsCursor || !m_PreviewEntityManager.IsEditing())
			return;
		
		//m_PreviewEntityManager.SetTarget(null); //--- Don't reset, messes up restoring pre-snap transformation
		m_bIsRotatingTowardsCursor = false;
		m_PreviewEntityManager.SetIsRotating(false);
		
		//--- Return cursor back
		if (m_CursorComponentBase)
		{
			m_CursorComponentBase.SetCursorPos(m_vClickPosBase, true);
			m_vClickPosCancel = m_vClickPosBase;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorTransformMoveVerticalModifierDown(float value, EActionTrigger reason)
	{
		if (!m_PreviewEntityManager.IsRotating())
			SetClickPos(m_CursorComponentBase.GetCursorPos());	
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorTransformMoveVerticalModifierUp(float value, EActionTrigger reason)
	{
		if (!m_PreviewEntityManager.IsEditing())// || !m_bIsRotatingTowardsCursor)
			return;
		
		//--- Return cursor back
		if (m_CursorComponentBase && !m_PreviewEntityManager.IsRotating() && m_PreviewEntityManager.GetVerticalMode() == EEditorTransformVertical.GEOMETRY)
		{
			m_CursorComponentBase.SetCursorPos(m_vClickPosBase, true);
			m_vClickPosCancel = m_vClickPosBase;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetRotationPivot()
	{
		vector previewTransform[4];
		if (m_PreviewEntityManager.GetPreviewTransform(previewTransform))
		{
			m_vClickTransformBase = previewTransform;
		}
		else
		{
			vector worldPos;
			if (!m_CursorComponentBase.GetCursorWorldPos(worldPos))
				return;

			m_vClickTransformBase[3] = worldPos;
		}
		
		m_RotationTrace = null;
		if (m_PreviewEntityManager.GetVerticalMode() == EEditorTransformVertical.GEOMETRY)
		{
			m_RotationTrace = new TraceParam();
			m_RotationTrace.ExcludeArray = m_PreviewEntityManager.GetExcludeArray();
		}
		
		vector pos = m_vClickTransformBase[3];
		m_vTerrainNormal = SCR_TerrainHelper.GetTerrainNormal(pos, m_World, !m_PreviewEntityManager.IsUnderwater(), m_RotationTrace);
	}

	//---- REFACTOR NOTE START: Unclear hardcoded multiplier 
	
	//------------------------------------------------------------------------------------------------
	protected void SetClickPos(vector clickPos)
	{
		m_vClickPosBase = clickPos;
		m_bMouseMoved = false;
		
		ArmaReforgerScripted game = GetGame();
		if (!game || !m_World)
			return;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace)
			return;
		
		vector clickDir;
		vector cameraPos = workspace.ProjScreenToWorld(clickPos[0], clickPos[1], clickDir, m_World, -1);
		m_vClickPosWorldBase = cameraPos + clickDir * 10;
	}
	
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected bool HasMouseMoved()
	{
		if (m_bMouseMoved || !m_InputManagerBase.IsUsingMouseAndKeyboard())
			return true;
		
		ArmaReforgerScripted game = GetGame();
		if (!game || !m_World)
			return false;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace)
			return false;
		
		//--- Check not only if cursor moved, but also if camera moved, resulting in cursor movement in the world
		vector clickPos = workspace.ProjWorldToScreen(m_vClickPosWorldBase, m_World);
		
		m_bMouseMoved = vector.Distance(m_CursorComponentBase.GetCursorPos(), clickPos/*m_vClickPosBase*/) > MIN_CURSOR_DIS_TO_TRANSFORM;
		return m_bMouseMoved;
	}

	protected void GetCursorPos(out vector cameraPos, out vector cursorDir)
	{
		ArmaReforgerScripted game = GetGame();
		if (!game || !m_World)
			return;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace)
			return;
		
		vector cursorPos = m_CursorComponentBase.GetCursorPos();
		cameraPos = workspace.ProjScreenToWorld(cursorPos[0], cursorPos[1], cursorDir, m_World, -1);
	}

	//------------------------------------------------------------------------------------------------
	protected float GetTraceDis(vector pos, vector dir, float cameraHeight)
	{
		TraceParam trace = new TraceParam();
		trace.Start = pos;
		trace.End = trace.Start + dir;
		if (cameraHeight >= m_World.GetOceanBaseHeight())
			trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN;
		else
			trace.Flags = TraceFlags.WORLD; //--- Don't check for water intersection when under water

		trace.LayerMask = TRACE_LAYER_CAMERA;
		
		return m_World.TraceMove(trace, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDirIndicator(vector previewTransform[4])
	{
		if (m_Operation == EPreviewEntityEditorOperation.ROTATE)
		{
			//--- Show dir indicator
			if (!m_DirIndicator && m_DirIndicatorPrefab && m_PreviewEntityManager.GetPreviewEntity())
			{
				//--- Create object
				EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.Parent = m_PreviewEntityManager.GetPreviewEntity();
				m_DirIndicator = GetGame().SpawnEntityPrefab(Resource.Load(m_DirIndicatorPrefab), spawnParams.Parent.GetWorld(), spawnParams);
				
				//--- Get object size
				vector previewBoundMin, previewBoundMax;
				m_PreviewEntityManager.GetPreviewEntity().GetPreviewBounds(previewBoundMin, previewBoundMax);
				float previewSize = vector.DistanceXZ(previewBoundMin, previewBoundMax);
				float offsetSize = vector.DistanceXZ(vector.Zero, previewBoundMin + previewBoundMax);
				float localOffsetSize = vector.DistanceXZ(vector.Zero, m_PreviewEntityManager.GetLocalOffset());
				m_fDirIndicatorScale = (previewSize + offsetSize + localOffsetSize) * 0.5;
			}

			if (m_DirIndicator)
			{
				//--- Align the indicator to surface
				vector angles = Math3D.MatrixToAngles(previewTransform);
				angles[1] = 0; //--- Reset pitch
				angles[2] = 0; //--- Reset roll
				
				vector orientedTransform[4];
				Math3D.AnglesToMatrix(angles, orientedTransform);
				
				orientedTransform[3] = m_vRotationPivot;
				SCR_TerrainHelper.OrientToTerrain(orientedTransform, trace: m_RotationTrace);
				
				m_DirIndicator.SetWorldTransform(orientedTransform);
				m_DirIndicator.SetScale(m_fDirIndicatorScale);
			}
		}
		else if (m_DirIndicator)
		{
			m_RotationTrace = null;
			delete m_DirIndicator;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return SCR_EPreviewState of the preview according to which the preview color is set. To be overridden by inherited classes.
	protected SCR_EPreviewState GetPreviewStateToShow();
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Transformation methods
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void MoveHorizontalTowardsCursor(float tDelta, out vector transform[4], out bool canTransform, out EEditorTransformVertical verticalMode)
	{
		vector cameraPos, cursorDir;
		GetCursorPos(cameraPos, cursorDir);
		cursorDir *= TRACE_DIS;
		vector pos = transform[3];
		
		switch (verticalMode)
		{
			case EEditorTransformVertical.GEOMETRY:
			{
				canTransform = GetPreviewPosAboveGeometry(cameraPos, cursorDir, pos, verticalMode);
				break;
			}
			case EEditorTransformVertical.TERRAIN:
			{
				canTransform = GetPreviewPosAboveTerrain(cameraPos, cursorDir, pos, verticalMode);
				break;
			}
			case EEditorTransformVertical.SEA:
			{
				canTransform = GetPreviewPosAboveSea(cameraPos, cursorDir, pos, verticalMode);
				break;
			}
		}

		transform[3] = pos;
		m_fUnsnapProgress = 0;
		m_PreviewEntityManager.SetIsMovingVertically(false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool MoveVertical(float tDelta, out vector transform[4], float moveVertical, EEditorTransformVertical verticalMode)
	{		
		//--- Gamepad move vertical
		if (m_PreviewEntityManager.CanUnsnap(moveVertical))
		{
			m_fUnsnapProgress += tDelta;
			if (m_fUnsnapProgress < m_fUnsnapDuration)
				return false;
		}
		else
		{
			m_fUnsnapProgress = 0;
		}
		
		//--- Apply changes
		vector pos = transform[3];
		transform[3][1] = pos[1] + moveVertical;// * tDelta;
		
		//--- Snap vertically (not when unsnapped recently)
		if (m_fUnsnapProgress == 0)
			transform[3] = m_PreviewEntityManager.SnapVertically(transform[3]);
		
		//-- Update height
		m_PreviewEntityManager.SetPreviewHeight(transform[3]);
		
		//--- Adjust camera to keep looking at preview
		SCR_ManualCamera camera;
		if (verticalMode != EEditorTransformVertical.GEOMETRY && m_CameraManagerBase && m_CameraManagerBase.GetCamera(camera))
		{
			vector cameraPos = camera.GetWorldTransformAxis(3) + transform[3] - pos;
			pos = transform[3];
			
			if (cameraPos[1] > pos[1] + m_fMinCameraVerticalOffset && m_PreviewEntityManager.GetPreviewHeightAboveTerrain() > 0)
			{
				//--- Move camera with the entity when it's above the entity and the entity is above ground
				camera.SetOrigin(cameraPos);
			}
			else
			{
				//--- Rotate camera towards the entity in all other cases
				vector cameraTransform[4] = {vector.Right, vector.Up, vector.Forward, camera.GetWorldTransformAxis(3)};
				Math3D.DirectionAndUpMatrix(transform[3] - cameraTransform[3], vector.Up, cameraTransform);
				camera.SetTransform(cameraTransform);
			}

			camera.SetDirty(true);
		}

		m_PreviewEntityManager.SetIsMovingVertically(true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveVerticalTowardsCursor(float tDelta, out vector transform[4])
	{
		vector cameraPos, cursorDir;
		GetCursorPos(cameraPos, cursorDir);
		
		float dis = vector.DistanceXZ(cameraPos, transform[3]);
		float angle = cursorDir.VectorToAngles()[1];
		vector tracePos = cameraPos + cursorDir * (dis / Math.Cos(angle * Math.DEG2RAD));
		transform[3][1] = tracePos[1];
		transform[3] = m_PreviewEntityManager.SnapVertically(transform[3]);
		m_PreviewEntityManager.SetPreviewHeight(transform[3]);
		m_PreviewEntityManager.SetIsMovingVertically(true);
		
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SHOW_DEBUG))
		{
			string text = string.Format("    %1 m", m_PreviewEntityManager.GetPreviewHeightAboveTerrain().ToString(lenDec: 3));
			DebugTextWorldSpace.Create(m_World, text, DebugTextFlags.ONCE | DebugTextFlags.FACE_CAMERA, transform[3][0], transform[3][1], transform[3][2], color: Color.PINK);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void Rotate(float tDelta, out vector transform[4], float rotationValue)
	{		
		vector angles = transform[2].VectorToAngles();
		bool freeRotation = true;
		SCR_EditableEntityComponent target = m_PreviewEntityManager.GetTarget();
		if (target)
		{
			if (!RotateInSlot(target, rotationValue, angles, freeRotation))
				return;
		}

		if (freeRotation)
		{
			angles -= m_vAnglesOrigin;
			angles[0] = angles[0] + rotationValue;// * tDelta;
		}
		
		vector basis[4];
		Math3D.AnglesToMatrix(angles, basis);
		Math3D.MatrixMultiply3(basis, m_vTransformOrigin, transform);
		m_vRotationPivot = transform[3];
	}

	//------------------------------------------------------------------------------------------------
	protected bool RotateInSlot(SCR_EditableEntityComponent slot, float rotationValue, out vector angles, out bool freeRotation)
	{
		if (m_fTargetYaw)
		{
			angles[0] = m_fTargetYaw;
			return true;
		}
		
		SCR_SiteSlotEntity slotEntity = SCR_SiteSlotEntity.Cast(slot.GetOwner());
		if (!slotEntity)
			return false;
		
		float step = slotEntity.GetRotationStep();
		if (step == -1)
			return false;

		if (step == 0)
			return true;
		
		angles[0] = angles[0] + step * rotationValue.Sign();
		m_fTargetYaw = angles[0];
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RotateTowardsCursor(float tDelta, out vector transform[4])
	{
		//--- Reset when camera starts moving
		if (m_CameraManagerBase && m_CameraManagerBase.GetCamera() && m_CameraManagerBase.GetCamera().IsManualInput())
		{
			OnEditorTransformRotationModifierUp(0, 0);
			return;
		}
		
		vector pivotPos;
		SCR_EditableEntityComponent target = m_PreviewEntityManager.GetTarget();
		if (target)
		{
			//--- Attached to a target - use its center instead of relative click position
			vector targetTransform[4];
			target.GetTransform(targetTransform);
			pivotPos = targetTransform[3];
		}
		else
		{
			pivotPos = m_vClickTransformBase[3];
			pivotPos[1] = pivotPos[1] - m_PreviewEntityManager.GetLocalOffset()[1];
		}
		
		//--- Get intersection with terrain plane
		vector cameraPos, cursorDir;
		GetCursorPos(cameraPos, cursorDir);
		vector worldPos = SCR_Math3D.IntersectPlane(cameraPos, cursorDir * TRACE_DIS, pivotPos, m_vTerrainNormal);
		float dir = (worldPos - pivotPos).VectorToAngles()[0] - m_vAnglesOrigin[0];
		
		vector basis[4];
		Math3D.AnglesToMatrix(Vector(dir, 0, 0), basis);
		Math3D.MatrixMultiply3(basis, m_vTransformOrigin, transform);
		
		m_vRotationPivot = pivotPos;
		m_PreviewEntityManager.SetIsRotating(true);
		m_Operation = EPreviewEntityEditorOperation.ROTATE;
		
		//--- Debug
		//Shape.CreateSphere(Color.White.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, m_vClickTransformBase[3], 1);
		//Shape.CreateSphere(Color.Red.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, worldPos, 1);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Intersections
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected bool GetPreviewPosAboveGeometry(vector cameraPos, vector cursorDir, out vector worldPos, out EEditorTransformVertical verticalMode)
	{
		//--- Trace entity under cursor
		TraceParam trace = new TraceParam();
		trace.Start = cameraPos;
		trace.End = trace.Start + cursorDir;
		trace.LayerMask = EPhysicsLayerPresets.Projectile; //--- Use more detailed mask which can also detect e.g., rack shelves
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		if (cameraPos[1] >= m_World.GetOceanBaseHeight())
			trace.Flags |= TraceFlags.OCEAN;

		trace.ExcludeArray = m_PreviewEntityManager.GetExcludeArray();
		float traceCursor = m_World.TraceMove(trace, null);
		
		//--- No entity intersection, calculate by terrain
		if (traceCursor == 1)// || (trace.TraceEnt && trace.TraceEnt.IsInherited(GenericTerrainEntity)))
			return GetPreviewPosAboveTerrain(cameraPos, cursorDir, worldPos, verticalMode);
		
		worldPos = cameraPos + cursorDir * traceCursor;
		
		//--- Terrain or sea is under cursor, change real vertical mode type
		if (!trace.TraceEnt || trace.TraceEnt.Type() == GenericTerrainEntity || trace.TraceEnt.Type() == RoadEntity)
			verticalMode = EEditorTransformVertical.TERRAIN;
		
		//--- Check slope of intersected surface
		//vector basePos = worldPos;
		float dotProduct = vector.Dot(trace.TraceNorm, vector.Up);
		float slope = 90 - Math.Tan(dotProduct) * Math.RAD2DEG;
		
		//--- When intersecting ceiling, prevent editing altogether
		if (slope > m_fMinAngleCeiling)
			return false;
		
		//--- When intersecting wall, find surface at its base.
		//--- Proved to be quite unreliable when the entity was moved vertically before (Alt+ LMB drag).
		//--- Let's disable walls for now, but keep the code in case we need in in the future.
		if (slope > m_fMinAngleWall)
		{
			return false;

//			//--- Rotate trace normal vector to point down
//			vector matrixNorm[3];
//			Math3D.MatrixFromForwardVec(trace.TraceNorm, matrixNorm);
//
//			vector matrixConvert[3];
//			matrixConvert[0] = vector.Up;
//			matrixConvert[1] = vector.Right;
//			matrixConvert[2] = vector.Forward;
//
//			Math3D.MatrixMultiply3(matrixNorm, matrixConvert, matrixNorm);
//			vector vectorNorm = matrixNorm[0] * 100;
//			if (vectorNorm[1] > 0)
//				vectorNorm = -vectorNorm;
//
//			//--- Trace down along the wall
//			vector offsetNorm = trace.TraceNorm * 0.01;
//			TraceParam traceBase = new TraceParam();
//			traceBase.Start = worldPos + offsetNorm;
//			traceBase.End = trace.Start + vectorNorm;
//			traceBase.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
//			traceBase.ExcludeArray = m_PreviewEntityManager.GetSourceEntities();
//			float traceBaseDis = m_World.TraceMove(traceBase, null);
//
//			basePos = traceBase.Start + vectorNorm * traceBaseDis - offsetNorm;
		}

//		worldPos = basePos;
		
		//--- Add entity height
		worldPos[1] = worldPos[1] + m_PreviewEntityManager.GetPreviewHeightAboveTerrain();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetPreviewPosAboveTerrain(vector cameraPos, vector cursorDir, out vector worldPos, out EEditorTransformVertical verticalMode)
	{
		//float heightASL = m_PreviewEntityManager.GetPreviewHeightAboveSea();
		float heightATL = m_PreviewEntityManager.GetPreviewHeightAboveTerrain();
		
		//--- Cursor below horizon
		vector offsetPos = cameraPos;
		if (verticalMode != EEditorTransformVertical.GEOMETRY && cameraPos[1] > worldPos[1]) //--- When camera is above ground, offset tracing pos by entity height to keep preview under cursor
			offsetPos -= vector.Up * heightATL;
		
		float traceDis = GetTraceDis(offsetPos, cursorDir, cameraPos[1]);
		if (traceDis != 1)
		{
			//--- Cursor on the ground: Use intersection positon
			worldPos = offsetPos + cursorDir * traceDis;
			worldPos[1] = worldPos[1] + heightATL;
			
			verticalMode = EEditorTransformVertical.TERRAIN;
			return true;
		}
		else
		{
			if (cursorDir[1] > 0)
				return false;
		
			//--- Cursor above horizon: Use ASL
			return GetPreviewPosAboveSea(cameraPos, cursorDir, worldPos, verticalMode);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetPreviewPosAboveSea(vector cameraPos, vector cursorDir, out vector worldPos, out EEditorTransformVertical verticalMode)
	{
		float heightASL = m_PreviewEntityManager.GetPreviewHeightAboveSea();
		
		float traceDis = 1;
		if (verticalMode != EEditorTransformVertical.SEA && cursorDir[1] < 0)
		{
			//--- Cursor points below the camera: Use ground intersection
			traceDis = GetTraceDis(cameraPos, cursorDir, cameraPos[1]);
			if (traceDis == 1)
				return false;
		}
		
		//--- Force ASL mode to prevent snapping to terrain
		verticalMode = EEditorTransformVertical.SEA;

		//--- Check for intersection with horizontal plane in entity's ASL height
		if (traceDis == 1)
			traceDis = Math3D.IntersectionRayBox(cameraPos, cameraPos + cursorDir, Vector(-float.MAX, heightASL, -float.MAX), Vector(float.MAX, heightASL, float.MAX));
		
		//--- No plane intersection: Ignore (e.g., when camera is above the entity and cursor points at the sky)
		if (traceDis == -1)
			return false;
		
		worldPos = cameraPos + cursorDir * traceDis;
		worldPos[1] = heightASL; //--- Make sure ASL height is maintained
		
		return true;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Main Loop
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void ProcessInput(float tDelta)
	{
		if (!m_InputManagerBase || !m_PlacingManager || !m_PreviewEntityManager || m_PreviewEntityManager.IsFixedPosition() || !m_PreviewEntityManager.GetPreviewEntity())
			return;
		
		//--- Transforming
		bool canTransform = true;
		bool instant = false;
		vector previewTransform[4];
		m_PreviewEntityManager.GetPreviewTransform(previewTransform);
		EEditorTransformVertical verticalMode = m_PreviewEntityManager.GetVerticalMode();
		m_Operation = EPreviewEntityEditorOperation.MOVE_HORIZONTAL;
				
		SCR_EPreviewState previewState = SCR_EPreviewState.PLACEABLE;
		ENotification outNotification;
		m_PlacingManager.CanCreateEntity(outNotification, previewState);
		m_PreviewEntityManager.SetPreviewState(previewState);
		
		//--- There are numerous issues with editing along geometry, so it's disabled for now
		if (verticalMode == EEditorTransformVertical.GEOMETRY && m_PreviewEntityManager.GetPreviewEntity().HasMultipleEditableEntities())
			verticalMode = EEditorTransformVertical.TERRAIN;
		
		//--- Automatic animation when restoring transformation after unsnapping from a target
		if (m_bIsAnimated)
		{
			previewTransform[0] = m_vAnimatedTransform[0];
			previewTransform[1] = m_vAnimatedTransform[1];
			previewTransform[2] = m_vAnimatedTransform[2];
		}
		
		float moveVertical = m_InputManagerBase.GetActionValue("EditorTransformMoveVertical") * tDelta;
		float rotateYaw = m_InputManagerBase.GetActionValue("EditorTransformRotateYaw") * tDelta;
		
		if (rotateYaw != 0 && Math.AbsFloat(rotateYaw) > Math.AbsFloat(moveVertical) && (m_Operation == EPreviewEntityEditorOperation.MOVE_HORIZONTAL || m_Operation == EPreviewEntityEditorOperation.ROTATE))
		{
			//--- Gamepad rotation
			Rotate(tDelta, previewTransform, rotateYaw * m_fRotationCoef);
			m_bIsAnimated = false;
			moveVertical = 0;
			m_Operation = EPreviewEntityEditorOperation.ROTATE;
		}
		else
		{
			m_fTargetYaw = 0;
		}
		
		if (m_bIsRotatingTowardsCursor && HasMouseMoved())
		{
			//--- Rotation towards cursor
			RotateTowardsCursor(tDelta, previewTransform);
			m_bIsAnimated = false;
		}
		else if (m_InputManagerBase.GetActionTriggered("EditorTransformMoveVerticalModifier") && HasMouseMoved())
		{
			//--- Vertical movement towards cursor
			MoveVerticalTowardsCursor(tDelta, previewTransform);
		}
		else if (moveVertical != 0 && (m_Operation == EPreviewEntityEditorOperation.MOVE_HORIZONTAL || m_Operation == EPreviewEntityEditorOperation.MOVE_VERTICAL))
		{
			//--- Vertical movement
			instant = MoveVertical(tDelta, previewTransform, moveVertical * m_fMoveVerticalCoef, verticalMode);
			m_Operation = EPreviewEntityEditorOperation.MOVE_VERTICAL;
		}
		else
		{
			//--- Horizontal movement towards cursor
			MoveHorizontalTowardsCursor(tDelta, previewTransform, canTransform, verticalMode);
		}
		
		UpdateDirIndicator(previewTransform);
		
		//--- Apply
		if (canTransform)
			m_PreviewEntityManager.SetPreviewTransform(previewTransform, tDelta, instant, verticalMode);
		else
			m_PreviewEntityManager.ResetPreviewTransform();
	}

	//------------------------------------------------------------------------------------------------
	protected void ActivatePreviewContext()
	{
		m_InputManagerBase.ActivateContext(m_PreviewEntityManager.GetActionContext());
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default methods
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		MenuRootComponent root = GetRootComponent();
		if (!root)
			return;

		m_CursorComponentBase = SCR_CursorEditorUIComponent.Cast(root.FindComponent(SCR_CursorEditorUIComponent, true));

		if (!m_CursorComponentBase)
			return;
		
		m_PreviewEntityManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, true));
		if (!m_PreviewEntityManager)
			return;
		
		m_PlacingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		
		m_PreviewEntityManager.GetOnPreviewCreate().Insert(OnPreviewCreate);
		m_PreviewEntityManager.GetOnPreviewDelete().Insert(OnPreviewDelete);
		
		m_CameraManagerBase = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent, true));
		if (!m_CameraManagerBase)
			return;
		
		m_HoverFilter = SCR_HoverEditableEntityFilter.Cast(SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER));
		if (m_HoverFilter)
			m_HoverFilter.GetOnChanged().Insert(OnHoverChange);
		
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			m_World = game.GetWorld();
			
			m_InputManagerBase = game.GetInputManager();
			if (m_InputManagerBase)
			{
				m_InputManagerBase.AddActionListener("EditorTransformSnapToSurface", EActionTrigger.DOWN, OnEditorTransformSnapToSurface);
				
				m_InputManagerBase.AddActionListener("EditorTransformMoveVerticalModifier", EActionTrigger.DOWN, OnEditorTransformMoveVerticalModifierDown);
				m_InputManagerBase.AddActionListener("EditorTransformMoveVerticalModifier", EActionTrigger.UP, OnEditorTransformMoveVerticalModifierUp);
				
				m_InputManagerBase.AddActionListener("EditorTransformRotateYawModifier", EActionTrigger.DOWN, OnEditorTransformRotationModifierDown);
				m_InputManagerBase.AddActionListener("EditorTransformRotateYawModifier", EActionTrigger.UP, OnEditorTransformRotationModifierUp);
			}
		}
				
		m_fRotationInertia = 1 / Math.Max(m_fRotationInertia, 0.001);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_PreviewEntityManager)
		{
			m_PreviewEntityManager.GetOnPreviewCreate().Remove(OnPreviewCreate);
			m_PreviewEntityManager.GetOnPreviewDelete().Remove(OnPreviewDelete);
		}
		
		if (m_HoverFilter)
			m_HoverFilter.GetOnChanged().Remove(OnHoverChange);
		
		if (m_InputManagerBase)
		{
			m_InputManagerBase.RemoveActionListener("EditorTransformSnapToSurface", EActionTrigger.DOWN, OnEditorTransformSnapToSurface);
			
			m_InputManagerBase.RemoveActionListener("EditorTransformMoveVerticalModifier", EActionTrigger.DOWN, OnEditorTransformMoveVerticalModifierDown);
			m_InputManagerBase.RemoveActionListener("EditorTransformMoveVerticalModifier", EActionTrigger.UP, OnEditorTransformMoveVerticalModifierUp);
			
			m_InputManagerBase.RemoveActionListener("EditorTransformRotateYawModifier", EActionTrigger.DOWN, OnEditorTransformRotationModifierDown);
			m_InputManagerBase.RemoveActionListener("EditorTransformRotateYawModifier", EActionTrigger.UP, OnEditorTransformRotationModifierUp);
		}
	}
}
