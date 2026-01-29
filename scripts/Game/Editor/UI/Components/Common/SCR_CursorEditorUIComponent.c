//! @ingroup Editor_UI Editor_UI_Components

class SCR_CursorEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected const float TRACE_DIS_SINGLE = 10000;
	protected const float TRACE_DIS_REPEATED = 250;
	
	//[Attribute()]
	//private string m_sGamepadCursorWidgetName;
	
	[Attribute()]
	private ref array<ref SCR_CursorEditor> m_Cursors;
	
	private ref map<EEditorCursor, ref SCR_CursorEditor> m_CursorsMap = new map<EEditorCursor, ref SCR_CursorEditor>();
	private SCR_CursorEditor m_CurrentCursor;
	private vector m_vGamepadCursorSize;
	private float m_fGamepadCursorRadius;
	private float m_fGamepadCursorRadiusSq;
	private InputManager m_InputManager;
	private WorkspaceWidget m_wWorkspace;
	private SCR_EditorManagerEntity m_EditorManager;
	private SCR_StatesEditorComponent m_StatesManager;
	private SCR_EditableEntityComponent m_EntityUnderCursor;
	private SCR_HoverEditableEntityFilter m_HoverManager;
	private SCR_PreviewEntityEditorComponent m_PreviewManager;
	private SCR_CommandActionsEditorUIComponent m_CommandManager;
	private SCR_MapEntity m_MapEntity;
	private IEntity m_TraceEntity;
	//private vector m_vCursorPosRef;
	private vector m_vCursorPos;
	private vector m_vCursorPosWorld;
	private vector m_vCursorPosWorldNormalized;
	private int m_iCursorPosX;
	private int m_iCursorPosY;
	private bool m_bCanFocus = true;
	private int m_iFrameIndex;
	private int m_iFrameUpdate;
	private int m_iFrameUpdateWorld;
	private float m_fTargetAlpha = 1;
	private float m_fTargetAlphaStrength;
	private SCR_MouseAreaEditorUIComponent m_MouseArea;
	protected TraceFlags m_DefaultWorldTraceFlags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;
	
	//------------------------------------------------------------------------------------------------
	//! Set cursor position.
	//! \param[in] pos Cursor position
	//! \param[in] DPIScale True if the position is in reference resolution, false if in native (current)
	void SetCursorPos(vector pos, bool DPIScale = false)
	{
		if (!m_InputManager || !m_InputManager.IsUsingMouseAndKeyboard())
			return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace)
			return;
		
		float mouseX = pos[0];
		float mouseY = pos[1];
		if (DPIScale)
		{
			mouseX = workspace.DPIScale(mouseX);
			mouseY = workspace.DPIScale(mouseY);
		}

		m_InputManager.SetCursorPosition(mouseX, mouseY);
		
		m_iCursorPosX = mouseX;
		m_iCursorPosY = mouseY;
		m_vCursorPos = Vector(m_wWorkspace.DPIUnscale(m_iCursorPosX), m_wWorkspace.DPIUnscale(m_iCursorPosY), 0);
		m_iFrameUpdate = m_iFrameIndex + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Get cached cursor position in reference resolution.
	//! \return Unscaled screen coordinates
	vector GetCursorPos()
	{
		//--- Get cached position
		if (m_iFrameUpdate == m_iFrameIndex)
			return m_vCursorPos;

		m_iFrameUpdate = m_iFrameIndex;
		
		//--- Calculate new position
		if (m_InputManager && m_InputManager.IsUsingMouseAndKeyboard())
		{
			WidgetManager.GetMousePos(m_iCursorPosX, m_iCursorPosY);
			m_vCursorPos = Vector(m_wWorkspace.DPIUnscale(m_iCursorPosX), m_wWorkspace.DPIUnscale(m_iCursorPosY), 0);
		}
		else
		{
			float cursorX, cursorY;
			//if (m_GamepadCursorWidget) m_GamepadCursorWidget.GetScreenPos(cursorX, cursorY);
			if (m_CurrentCursor && m_CurrentCursor.GetWidget())
				m_CurrentCursor.GetWidget().GetScreenPos(cursorX, cursorY);

			m_vCursorPos = Vector(m_wWorkspace.DPIUnscale(cursorX), m_wWorkspace.DPIUnscale(cursorY), 0) + m_vGamepadCursorSize;
			m_iCursorPosX = cursorX;
			m_iCursorPosY = cursorY;
		}

		return m_vCursorPos;
	}

	//------------------------------------------------------------------------------------------------
	//! Get cached cursor position as integers in native (current) resolution.
	//! \param[out] posX
	//! \param[out] posY
	void GetCursorPos(out int posX, out int posY)
	{
		posX = m_iCursorPosX;
		posY = m_iCursorPosY;
	}

	//------------------------------------------------------------------------------------------------
	//! Get world position below cursor.
	//! \param[out] worldPos Vector to be filled with world position
	//! \param[in] isNormalized
	//! \param[in] flags Trace flags to be used (send -1 for default flags)
	//! \return True if the cursor is above world position (e.g., not pointing at sky)
	bool GetCursorWorldPos(out vector worldPos, bool isNormalized = false, TraceFlags flags = -1)
	{
		//--- Get cached position
		if (m_iFrameUpdateWorld == m_iFrameIndex && flags == m_DefaultWorldTraceFlags)
		{
			if (isNormalized)
				worldPos = m_vCursorPosWorldNormalized;
			else
				worldPos = m_vCursorPosWorld;

			return true;
		}
		m_iFrameUpdateWorld = m_iFrameIndex;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return false;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace)
			return false;
		
		BaseWorld world = game.GetWorld();
		if (!world)
			return false;
		
		// If map is open return map cursor world position
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity && mapEntity.IsOpen())
		{
			float worldX, worldY;
			mapEntity.GetMapCursorWorldPosition(worldX, worldY);
			worldPos[0] = worldX;
			worldPos[2] = worldY;
			worldPos[1] = world.GetSurfaceY(worldPos[0], worldPos[2]);
			
			m_vCursorPosWorld = worldPos;
			m_vCursorPosWorldNormalized = worldPos;
			return true;
		}
		
		vector cursorPos = GetCursorPos();
		vector outDir;
		vector startPos = workspace.ProjScreenToWorld(cursorPos[0], cursorPos[1], outDir, world, -1);
		outDir *= TRACE_DIS_SINGLE;
		
		//--- Use default flags
		if (flags < 0)
			flags = m_DefaultWorldTraceFlags;
		
		//--- Camera below ocean surface, don't trace ocean
		if (startPos[1] < world.GetOceanBaseHeight())
			flags = flags & ~TraceFlags.OCEAN;
	
		TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = flags;
		
		float traceDis = world.TraceMove(trace, null);
		if (traceDis == 1)
		{
			m_vCursorPosWorldNormalized = startPos + outDir * traceDis;
			if (isNormalized)
			{
				worldPos = m_vCursorPosWorldNormalized;
				return true;
			}
			return false;
		}

		worldPos = startPos + outDir * traceDis;
		if (flags == m_DefaultWorldTraceFlags)
		{
			//--- Cache only when default flags were used
			m_vCursorPosWorld = worldPos;
			m_vCursorPosWorldNormalized = worldPos;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetTraceEntity()
	{
		return m_TraceEntity;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCursorType(EEditorCursor type)
	{
		SCR_CursorEditor cursor;
		if (!m_CursorsMap.Find(type, cursor))
		{
			Print(string.Format("Configuration for cursor %1 not found!", typename.EnumToString(EEditorCursor, type)), LogLevel.WARNING);
			return;
		}

		//--- Set PC cursor
		WidgetManager.SetCursor(cursor.GetID());
		
		//--- Set gamepad cursor
		float opacity = 1;
		if (m_CurrentCursor && m_CurrentCursor.GetWidget())
		{
			opacity = m_CurrentCursor.GetWidget().GetOpacity();
			m_CurrentCursor.GetWidget().SetOpacity(0);
		}

		if (cursor.GetWidget())
			cursor.GetWidget().SetOpacity(opacity);

		m_CurrentCursor = cursor;
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetCursor()
	{
		WidgetManager.SetCursor(EEditorCursor.DEFAULT);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetCursorRadius()
	{
		return m_fGamepadCursorRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetCursorRadiusSq()
	{
		return m_fGamepadCursorRadiusSq;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] alpha
	//! \param[in] strength
	void SetCursorAlpha(float alpha, float strength = 1)
	{
		m_fTargetAlpha = alpha;
		m_fTargetAlphaStrength = strength;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCursorDebug()
	{
		Print("Debug");
		UpdateCursor();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCursor()
	{
		//--- Map is using its own custom cursors, don't overwrite them
		if (m_MapEntity && m_MapEntity.IsOpen())
			return;
		
		EEditorState editorState;
		if (m_StatesManager)
			editorState = m_StatesManager.GetState();
		
		bool isEditing = m_PreviewManager && m_PreviewManager.IsEditing();
		
		switch (true)
		{
			//case (m_InputManager && (m_InputManager.GetActionValue("ManualCameraRotateYaw") != 0 || m_InputManager.GetActionValue("ManualCameraRotatePitch") != 0)):
			//	SetCursorType(EEditorCursor.MOVE_CAMERA);
			//	break;
			
			case (m_StatesManager && m_EditorManager && (m_StatesManager.IsWaiting() || m_EditorManager.IsModeChangeRequested())):
			{
				SetCursorType(EEditorCursor.WAITING);
				break;
			}

			case (isEditing && !m_PreviewManager.IsChange()):
			{
				SetCursorType(EEditorCursor.TRANSFORM_DISABLED);
				break;
			}

			case (isEditing && m_PreviewManager.IsRotating()):
			{
				SetCursorType(EEditorCursor.ROTATE);
				break;
			}

			case (isEditing && (m_PreviewManager.GetTarget() || m_PreviewManager.IsFixedPosition())):
			{
				if (m_PreviewManager.GetTargetInteraction() == EEditableEntityInteraction.NONE)
					SetCursorType(EEditorCursor.TRANSFORM_SNAP_DISABLED);
				else
					SetCursorType(EEditorCursor.TRANSFORM_SNAP);
				break;
			}

			case (isEditing && !m_PreviewManager.CanMoveInRoot()):
			{
				SetCursorType(EEditorCursor.TRANSFORM_DISABLED);
				break;
			}

			case (editorState == EEditorState.TRANSFORMING):
			{
				if (m_PreviewManager.GetVerticalMode() == EEditorTransformVertical.GEOMETRY && m_PreviewManager.GetVerticalMode() == m_PreviewManager.GetVerticalModeReal())
					SetCursorType(EEditorCursor.TRANSFORM_GEOMETRY);
				else
					SetCursorType(EEditorCursor.TRANSFORM);	
				break;
			}

			case (editorState == EEditorState.PLACING):
			{
				if (m_PreviewManager.GetVerticalMode() == EEditorTransformVertical.GEOMETRY && m_PreviewManager.GetVerticalMode() == m_PreviewManager.GetVerticalModeReal())
					SetCursorType(EEditorCursor.PLACE_GEOMETRY);
				else
					SetCursorType(EEditorCursor.PLACE);
				break;
			}

			case (m_CommandManager && m_CommandManager.GetCommandState() & EEditorCommandActionFlags.WAYPOINT):
			{
				SetCursorType(EEditorCursor.WAYPOINT);
				break;
			}

			case (m_CommandManager && m_CommandManager.GetCommandState() & EEditorCommandActionFlags.OBJECTIVE):
			{
				SetCursorType(EEditorCursor.OBJECTIVE);
				break;
			}

			case (editorState == EEditorState.SELECTING && m_HoverManager && m_HoverManager.GetInteractiveEntityUnderCursor()):
			{
				SetCursorType(EEditorCursor.FOCUS);
				break;
			}

			default:
			{
				SetCursorType(EEditorCursor.DEFAULT);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	//! \param[in] entitiesInsert
	//! \param[in] entitiesRemove
	void OnFilterChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		UpdateCursor();
	}

	//------------------------------------------------------------------------------------------------
	protected bool TraceFilter(Class target)
	{
		GenericEntity genericTarget = GenericEntity.Cast(target);
		return genericTarget.FindComponent(SCR_EditableEntityComponent) != null;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_EditableEntityComponent TraceEntity(vector cursorPos)
	{
		vector outDir;
		vector camPos = m_wWorkspace.ProjScreenToWorld(cursorPos[0], cursorPos[1], outDir, GetGame().GetWorld(), -1);

		//--- Trace cursor position
		TraceParam trace = new TraceParam();
		trace.Start = camPos;
		trace.End = camPos + outDir * TRACE_DIS_REPEATED;
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.OCEAN;
		float coefTrace = GetGame().GetWorld().TraceMove(trace, null);//TraceFilter);
		
		m_TraceEntity = trace.TraceEnt;
		
		//--- Ignore when there is nothing or just terrain under cursor, or when cursor is not on mouse area
		if (!trace.TraceEnt || trace.TraceEnt.Type() == GenericTerrainEntity || !m_MouseArea.IsMouseOn())
			return null;
		
		//--- Find the first editable entity in default (not editor) hierarchy
		GenericEntity genericEntity = GenericEntity.Cast(trace.TraceEnt);
		while (genericEntity)
		{
			SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(genericEntity.FindComponent(SCR_EditableEntityComponent));
			if (entity)
				return entity;

			genericEntity = GenericEntity.Cast(genericEntity.GetParent());
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		// use InputManager! instead of ShowCursor
		//ShowCursor(!isGamepad);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		//if (!GetWidget().IsEnabledInHierarchy()) return;
		m_iFrameIndex++;
		
		if ((!m_MouseArea || m_MouseArea.IsMouseOn()) && (!m_MapEntity || !m_MapEntity.IsOpen()))
		{
			m_InputManager.ActivateContext("EditorMouseAreaContext");
			
			SCR_EditableEntityComponent entityUnderCursor = TraceEntity(GetCursorPos());
			if (m_HoverManager && !m_HoverManager.GetEntityUnderCursorCandidate())
				m_HoverManager.SetEntityUnderCursor(entityUnderCursor);
		}

		if (!m_CurrentCursor)
			return;

		Widget currentCursorWidget = m_CurrentCursor.GetWidget();
		if (!currentCursorWidget)
			return;

		float currentAlpha = currentCursorWidget.GetOpacity();
		if (currentAlpha != m_fTargetAlpha)
			currentCursorWidget.SetOpacity(Math.Lerp(currentAlpha, m_fTargetAlpha, m_fTargetAlphaStrength * tDelta));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapToggled(MapConfiguration config)
	{
		UpdateCursor();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{	
		super.HandlerAttachedScripted(w);
		if (SCR_Global.IsEditMode()) return; //--- Run-time only
		
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		m_wWorkspace = GetGame().GetWorkspace();
		m_InputManager = GetGame().GetInputManager();
		m_MapEntity = SCR_MapEntity.GetMapInstance();

//		if (m_InputManager)
//		{
//			m_InputManager.AddActionListener("ManualCameraRotateYaw", EActionTrigger.PRESSED, UpdateCursor);
//			m_InputManager.AddActionListener("ManualCameraRotateYaw", EActionTrigger.UP, UpdateCursor);
//			m_InputManager.AddActionListener("ManualCameraRotatePitch", EActionTrigger.PRESSED, UpdateCursor);
//			m_InputManager.AddActionListener("ManualCameraRotatePitch", EActionTrigger.UP, UpdateCursor);
//		}
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapToggled);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapToggled);
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (m_EditorManager)
			m_EditorManager.GetOnModeChangeRequest().Insert(UpdateCursor);
		
		m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		if (m_StatesManager)
		{
			m_StatesManager.GetOnStateChange().Insert(UpdateCursor);
			m_StatesManager.GetOnIsWaitingChange().Insert(UpdateCursor);
		}
		
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		if (m_PreviewManager)
		{
			m_PreviewManager.GetOnPreviewCreate().Insert(UpdateCursor);
			m_PreviewManager.GetOnPreviewChange().Insert(UpdateCursor);
			m_PreviewManager.GetOnPreviewDelete().Insert(UpdateCursor);
			m_PreviewManager.GetOnTargetChange().Insert(UpdateCursor);
		}

		//--- Track selecting
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
		{
			m_HoverManager = SCR_HoverEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.HOVER));
			if (m_HoverManager)
				m_HoverManager.GetOnChanged().Insert(OnFilterChange);
		}
		
		//--- Track commanding
		m_CommandManager = SCR_CommandActionsEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_CommandActionsEditorUIComponent));
		if (m_CommandManager)
			m_CommandManager.GetOnCommandStateChange().Insert(UpdateCursor);

		m_MouseArea = SCR_MouseAreaEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_MouseAreaEditorUIComponent));
		
		//--- Map all cursors
		foreach (SCR_CursorEditor cursor: m_Cursors)
		{
			cursor.InitWidget(w);
			m_CursorsMap.Insert(cursor.GetType(), cursor);
		}
		
		//--- Set default cursor
		SCR_CursorEditor defaultCursor;
		if (m_CursorsMap.Find(EEditorCursor.DEFAULT, defaultCursor))
		{
			SetCursorType(EEditorCursor.DEFAULT);
			if (defaultCursor.GetWidget())
			{
				vector alignment = FrameSlot.GetAlignment(defaultCursor.GetWidget());
				m_vGamepadCursorSize = FrameSlot.GetSize(defaultCursor.GetWidget());
				m_vGamepadCursorSize[0] = m_vGamepadCursorSize[0] * alignment[0];
				m_vGamepadCursorSize[1] = m_vGamepadCursorSize[1] * alignment[1];
				m_fGamepadCursorRadius = Math.Max(m_vGamepadCursorSize[0], m_vGamepadCursorSize[1]);
				m_fGamepadCursorRadiusSq = Math.Pow(m_fGamepadCursorRadius, 2);
			}
		}
		else
		{
			Print("Default cursor settings not found!", LogLevel.ERROR);
		}
		
		MenuRootComponent rootComponent = MenuRootComponent.GetRootOf(w);
		menu.GetOnMenuFocusLost().Insert(UpdateCursor);
		menu.GetOnMenuHide().Insert(UpdateCursor);
		menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		ResetCursor();

//		if (m_InputManager)
//		{
//			m_InputManager.RemoveActionListener("ManualCameraRotateYaw", EActionTrigger.PRESSED, UpdateCursor);
//			m_InputManager.RemoveActionListener("ManualCameraRotateYaw", EActionTrigger.UP, UpdateCursor);
//			m_InputManager.RemoveActionListener("ManualCameraRotatePitch", EActionTrigger.PRESSED, UpdateCursor);
//			m_InputManager.RemoveActionListener("ManualCameraRotatePitch", EActionTrigger.UP, UpdateCursor);
//		}

		if (GetGame().OnInputDeviceIsGamepadInvoker())
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapToggled);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapToggled);
		
		if (m_EditorManager)
			m_EditorManager.GetOnModeChangeRequest().Remove(UpdateCursor);

		if (m_HoverManager)
			m_HoverManager.GetOnChanged().Remove(OnFilterChange);

		if (m_PreviewManager)
		{
			m_PreviewManager.GetOnPreviewCreate().Remove(UpdateCursor);
			m_PreviewManager.GetOnPreviewChange().Remove(UpdateCursor);
			m_PreviewManager.GetOnPreviewDelete().Remove(UpdateCursor);
			m_PreviewManager.GetOnTargetChange().Remove(UpdateCursor);
		}

		if (m_StatesManager)
		{
			m_StatesManager.GetOnStateChange().Remove(UpdateCursor);
			m_StatesManager.GetOnIsWaitingChange().Remove(UpdateCursor);
		}
		
		SCR_TransformingEditorComponent transformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
		if (transformingManager)
			transformingManager.GetOnTransformationStart().Remove(UpdateCursor);
		
		if (m_CommandManager)
			m_CommandManager.GetOnCommandStateChange().Remove(UpdateCursor);
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuFocusLost().Remove(UpdateCursor);
			menu.GetOnMenuHide().Remove(UpdateCursor);
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
		}
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorCursor, "m_Type")]
class SCR_CursorEditor
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditorCursor))]
	private EEditorCursor m_Type;
	
	[Attribute()]
	private string m_sName;
	
	private Widget m_wWidget;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EEditorCursor GetType()
	{
		return m_Type;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetID()
	{
		return m_Type;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetWidget()
	{
		return m_wWidget;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] root
	void InitWidget(Widget root)
	{
		if (m_sName.IsEmpty())
			return;
		
		m_wWidget = root.FindAnyWidget(m_sName);
		if (m_wWidget)
			m_wWidget.SetOpacity(0);
		else
			Print(string.Format("Gamepad cursor '%1' for type %2 not found!", m_sName, m_Type), LogLevel.ERROR);
	}
}
