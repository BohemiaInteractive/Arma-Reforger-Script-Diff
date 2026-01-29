//! @ingroup Editor_UI Editor_UI_Components

class SCR_SelectionEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected static const float FRAME_SIZE_MIN = 10;		//!< How many ref pixels must cursor move for frame selection to begin (mouse & keyboard)
	protected static const float FRAME_DURATION_MIN = 0.15;	//!< How long must the button  bepressed for frame selection to behin (controller)
	
	//! State in which entities are unselected.
	[Attribute(desc: "State in which entities are unselected.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_UnselectedState;
	
	[Attribute(desc: "Only entities of this state can be multi-selected, e.g., using selection frame or by pressing Ctrl+A.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_MultiSelectStates;
	
	[Attribute(desc: "When true, multi-selection operations are enabled (e.g., selection frame or Ctrl+A).", defvalue: "1")]
	protected bool m_bEnableMultiSelection;
	
	[Attribute(desc: "Name of a widget representing setting selection with mouse & keyboard.")]
	protected string m_sSetSelectionWidgetName;
	
	[Attribute(desc: "Name of a widget representing toggling selection with mouse & keyboard.")]
	protected string m_sToggleSelectionWidgetName;
	
	[Attribute(desc: "Name of a widget representing setting selection with gamepad.")]
	protected string m_sGamepadSetSelectionWidgetName;
	
	[Attribute(desc: "Name of a widget representing toggling selection with gamepad.")]
	protected string m_sGamepadToggleSelectionWidgetName;
	
	[Attribute("1", desc: "Initial size of multi-selection widget, measured as a coefficient of default cursor widget size (1 = same size).")]
	protected float m_fGamepadSelectionWidgetStartSize;
	
	[Attribute(defvalue: "0.5")]
	protected float m_fGamepadSelectionDuration;
	
	[Attribute(desc: "Curve defining how will gamepad selection widget expand.", defvalue: "0 0 1 1", uiwidget: UIWidgets.GraphDialog, params: "1 1 0 0")]
	protected ref Curve m_GamepadSelectionCurve;
	
	[Attribute(SCR_SoundEvent.SOUND_E_MULTI_SELECT_START_KEYBOARD, UIWidgets.EditBox)]
	protected string m_sSfxStartDrawingSelectionFrame_Keyboard;
	
	[Attribute(SCR_SoundEvent.SOUND_E_MULTI_SELECT_START_GAMEPAD, UIWidgets.EditBox)]
	protected string m_sSfxStartDrawingSelectionFrame_Gamepad;
	
	[Attribute(SCR_SoundEvent.SOUND_E_TRAN_CANCEL, UIWidgets.EditBox)]
	protected string m_sSfxSelectionFrameClose_NothingSelected;
	
	[Attribute(SCR_SoundEvent.SOUND_E_TRAN_CANCEL, UIWidgets.EditBox)]
	protected string m_sSfxSelectionFrameClose_Cancel;
	
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_SELECT, UIWidgets.EditBox)]
	protected string m_sSfxSelectEntities;
	
	[Attribute(SCR_SoundEvent.TAB_SWITCH, UIWidgets.EditBox)]
	protected string m_sSfxToggleEntities;
	
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_SELECT, UIWidgets.EditBox)]
	protected string m_sSfxToggleSingleEntity_On;
	
	[Attribute(SCR_SoundEvent.SOUND_E_TRAN_CANCEL, UIWidgets.EditBox)]
	protected string m_sSfxToggleSingleEntity_Off;
	
	[Attribute(SCR_SoundEvent.SOUND_E_TRAN_CANCEL, UIWidgets.EditBox)]
	protected string m_sSfxClearSelection;
	
	protected Widget m_SetSelectionWidget;
	protected Widget m_ToggleSelectionWidget;
	protected Widget m_GamepadSetSelectionWidget;
	protected Widget m_GamepadToggleSelectionWidget;
	protected InputManager m_InputManager;
	protected WorkspaceWidget m_Workspace;
	
	protected SCR_MenuEditorComponent m_EditorMenuManager;
	protected SCR_StatesEditorComponent m_StatesManager;
	protected SCR_EntitiesManagerEditorComponent m_EntitiesManager;
	protected SCR_LayersEditorComponent m_LayersManager;
	protected SCR_BaseEditableEntityFilter m_UnselectedManager;
	protected SCR_BaseEditableEntityFilter m_FocusedManager;
	protected SCR_HoverEditableEntityFilter m_HoverManager;
	protected SCR_SelectedEditableEntityFilter m_SelectedManager;
	
	protected SCR_CursorEditorUIComponent m_CursorComponent;
	protected SCR_MouseAreaEditorUIComponent m_MouseArea;
	
	protected vector m_vCursorPosClick;
	protected float m_fSelectionFrameDuration;
	protected bool m_bEnableSelection = true;
	protected bool m_bIsDrawingFrame;
	protected bool m_bIsDrawingFrameIsToggle;
	protected bool m_bIsDrawingFrameConfirmed;
	protected bool m_bIsDrawingFrameCancelled;
	protected bool m_bIsDrawingFrameDisabled;
	protected bool m_bIsAnimatingFrame;
	protected bool m_bHideCursor;
	protected int m_iGamepadSelectionHeight;
	protected BaseWorld m_World;
	
	//------------------------------------------------------------------------------------------------
	//! Disable gamepad multi-selection.
	//! On gamepad, multi-selection is mapped to holding a button.
	//! Use when you have the same input on an action, but don't want multi-selection to start right after the action is activated.
	void DisableMultiSelection()
	{
		if (!m_InputManager.IsUsingMouseAndKeyboard())
			m_bIsDrawingFrameDisabled = true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Input listeners
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void EditorToggleSelection(float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		if (IsInputDisabled())
			return;
		
		set<SCR_EditableEntityComponent> focused = new set<SCR_EditableEntityComponent>();
		m_FocusedManager.GetEntities(focused);
		
		//~ Multiple entities selected so play general selection sfx
		if (focused.Count() > 1)
		{
			SCR_UISoundEntity.SoundEvent(m_sSfxToggleEntities, true);
		}
		//~ Only one entity so check if toggle on or off sfx should be played
		else if (!focused.IsEmpty())
		{
			set<SCR_EditableEntityComponent> selected = new set<SCR_EditableEntityComponent>();
			m_SelectedManager.GetEntities(selected, true);
			
			if (selected.Contains(focused[0]))
				SCR_UISoundEntity.SoundEvent(m_sSfxToggleSingleEntity_Off);
			else 
				SCR_UISoundEntity.SoundEvent(m_sSfxToggleSingleEntity_On);
		}
		
		m_SelectedManager.Toggle(focused);
		ShowGUI();
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorSetSelection(float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		if (IsInputDisabled())
			return;
		
		set<SCR_EditableEntityComponent> focused = new set<SCR_EditableEntityComponent>();
		m_FocusedManager.GetEntities(focused);
		
		//~ Set selected
		if (!focused.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sSfxSelectEntities, true);
		//~ Check if deselecting any selected entities
		else if (m_SelectedManager.GetEntitiesCount() > 0)
			SCR_UISoundEntity.SoundEvent(m_sSfxClearSelection);
		
		m_SelectedManager.Replace(focused, true);
		
		ShowGUI();
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorAddSelection(float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		if (IsInputDisabled())
			return;
		
		set<SCR_EditableEntityComponent> focused = new set<SCR_EditableEntityComponent>();
		m_FocusedManager.GetEntities(focused);
		m_SelectedManager.Add(focused, true);
		
		if (!focused.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sSfxSelectEntities);

		ShowGUI();
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorClearSelection(float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		if (!m_SelectedManager)
			return;
		
		if (m_SelectedManager.GetEntitiesCount() > 0)
			SCR_UISoundEntity.SoundEvent(m_sSfxClearSelection);
		
		m_SelectedManager.Clear();
		ShowGUI();
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorSelectAll(float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		if (!m_UnselectedManager || !m_SelectedManager || IsMapOpened())
			return;
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		m_UnselectedManager.GetEntities(entities);
		if (entities.IsEmpty())
			return;
		
		int screenW = m_Workspace.GetWidth();
		int screenH = m_Workspace.GetHeight();
		
		set<SCR_EditableEntityComponent> selectEntities = new set<SCR_EditableEntityComponent>();
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			if (!SCR_Enum.HasPartialFlag(entity.GetEntityStates(), m_MultiSelectStates))
				continue;
			
			vector posWorld;
			if (!entity.GetPos(posWorld))
				continue;
			
			vector posScreen = m_Workspace.ProjWorldToScreenNative(posWorld, m_World); //--- ToDo: Also check for distance visibility
			if (posScreen[2] > 0 && posScreen[0] > 0 && posScreen[0] < screenW && posScreen[1] > 0 && posScreen[1] < screenH)
				selectEntities.Insert(entity);
		}

		m_SelectedManager.Replace(selectEntities);
		
		if (!selectEntities.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sSfxSelectEntities);

		ShowGUI();
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorDrawSetSelectionDown(float value, EActionTrigger reason)
	{
		DrawFrameDown(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorDrawSetSelectionPressed(float value, EActionTrigger reason)
	{
		DrawFramePressed(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorDrawSetSelectionUp(float value, EActionTrigger reason)
	{
		DrawFrameUp(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EditorDrawToggleSelectionDown(float value, EActionTrigger reason)
	{
		DrawFrameDown(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorDrawToggleSelectionPressed(float value, EActionTrigger reason)
	{
		DrawFramePressed(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void EditorDrawToggleSelectionUp(float value, EActionTrigger reason)
	{
		DrawFrameUp(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EditorDrawSelectionCancel(float value, EActionTrigger reason)
	{
		CancelFrame();
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Multi-selection
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void DrawFrameDown(bool isToggle)
	{
		//--- Exit when clicked not on mouse area (e.g., on a button)
		if (m_MouseArea && !m_MouseArea.IsMouseOn() || m_bIsDrawingFrameDisabled || IsMapOpened())
		{
			m_bIsDrawingFrameCancelled = true;
			return;
		}
		
		m_bIsAnimatingFrame = true;
		m_bIsDrawingFrameIsToggle = isToggle;
		m_bIsDrawingFrameConfirmed = false;
		m_bIsDrawingFrameCancelled = false;
		m_fSelectionFrameDuration = 0;
		if (m_vCursorPosClick == vector.Zero)
			m_vCursorPosClick = m_CursorComponent.GetCursorPos();
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawFrameUp(bool isToggle)
	{
		if (!m_bIsDrawingFrame || IsMapOpened())
		{
			ResetFrame();
			return;
		}

		if (isToggle != m_bIsDrawingFrameIsToggle)
			return;

		m_bIsDrawingFrameConfirmed = true;
		GetGame().GetCallqueue().Call(ConfirmFrame, isToggle); //--- Needs a delay to faciliate switching between Set (LMB) and add (Ctrl+LMB) variants
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawFramePressed(bool isToggle)
	{
		if (!m_CursorComponent || !m_UnselectedManager)
			return;
		
		//--- Some other state is preventing multi-selecting - instantly cancel, so it won't become activated after the other state is unset
		if ((m_StatesManager && !m_StatesManager.CanSet(EEditorState.MULTI_SELECTING)) || IsMapOpened())
		{
			m_bIsDrawingFrameCancelled = true;
			if (m_bIsDrawingFrame)
				ResetFrame();
			
			return;
		}
		
		if (m_bIsDrawingFrameCancelled)
			return;

		//--- Get visible entities
		set<SCR_EditableEntityComponent> entitiesInside = new set<SCR_EditableEntityComponent>();
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		m_UnselectedManager.GetEntities(entities);
		
		//--- Resize frame
		if (m_InputManager.IsUsingMouseAndKeyboard())
		{
			if (!DrawFrameMouseAndKeyboard(entities, entitiesInside)) 
				return;
			
			//~ Start drawing frame audio
			if (!m_bIsDrawingFrame)
				SCR_UISoundEntity.SoundEvent(m_sSfxStartDrawingSelectionFrame_Keyboard);
		}
		else
		{
			if (!DrawFrameController(entities, entitiesInside)) 
				return;
			
			if (!m_bIsDrawingFrame)
				SCR_UISoundEntity.SoundEvent(m_sSfxStartDrawingSelectionFrame_Gamepad);
		}
		
		if (m_StatesManager && !m_StatesManager.SetState(EEditorState.MULTI_SELECTING))
		{
			DrawFrameUp(isToggle);
			return;
		}
		
		if (!m_bIsDrawingFrame && m_EditorMenuManager)
			m_EditorMenuManager.SetVisible(true);

		m_bIsDrawingFrame = true;
		m_HoverManager.SetEntityUnderCursorEnabled(false);
				
		if (entitiesInside.Count() == m_FocusedManager.GetEntitiesCount())
			return; //--- Ignore when nothing changed

		m_FocusedManager.Replace(entitiesInside);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool DrawFrameMouseAndKeyboard(set<SCR_EditableEntityComponent> entities, out set<SCR_EditableEntityComponent> entitiesInside)
	{		
		//--- Too small frame, ignore (e.g., when just clicking)
		vector cursorPos = m_CursorComponent.GetCursorPos();
		if (!m_bIsDrawingFrame && vector.Distance(m_vCursorPosClick, cursorPos) < FRAME_SIZE_MIN)
			return false;

		//Print("DrawFrameMouseAndKeyboard");
		
		float xMin = m_vCursorPosClick[0];
		float yMin = m_vCursorPosClick[1];
		float xMax = cursorPos[0];
		float yMax = cursorPos[1];
		if (xMin > xMax)
		{
			xMin = xMax;
			xMax = m_vCursorPosClick[0];
		} 

		if (yMin > yMax)
		{
			yMin = yMax;
			yMax = m_vCursorPosClick[1];
		}

		if (m_bIsDrawingFrameIsToggle)
		{
			FrameSlot.SetPos(m_ToggleSelectionWidget, xMin, yMin);
			FrameSlot.SetSize(m_ToggleSelectionWidget, xMax - xMin, yMax - yMin);
			if (m_SetSelectionWidget)
				m_SetSelectionWidget.SetVisible(false);

			if (m_ToggleSelectionWidget)
				m_ToggleSelectionWidget.SetVisible(true);
		}
		else
		{
			FrameSlot.SetPos(m_SetSelectionWidget, xMin, yMin);
			FrameSlot.SetSize(m_SetSelectionWidget, xMax - xMin, yMax - yMin);
			if (m_ToggleSelectionWidget)
				m_ToggleSelectionWidget.SetVisible(false);

			if (m_SetSelectionWidget)
				m_SetSelectionWidget.SetVisible(true);
		}
		
		//--- Find entities in the frame
		vector worldPos, screenPos;
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			if (!SCR_Enum.HasPartialFlag(entity.GetEntityStates(), m_MultiSelectStates))
				continue;
			
			if (!entity.GetPos(worldPos))
				continue;
			
			screenPos = m_Workspace.ProjWorldToScreen(worldPos, m_World);
			if (screenPos[2] > 0 && screenPos[0] > xMin && screenPos[0] < xMax && screenPos[1] > yMin && screenPos[1] < yMax)
			{
				if (m_LayersManager)
					entity = m_LayersManager.GetParentBelowCurrentLayer(entity);

				if (entity)
					entitiesInside.Insert(entity);
			}
		}
		
		return true;
	}
	
	//---- REFACTOR NOTE START: Use of unclear hardcoded values

	//------------------------------------------------------------------------------------------------
	protected bool DrawFrameController(set<SCR_EditableEntityComponent> entities, out set<SCR_EditableEntityComponent> entitiesInside)
	{
		//--- Too fast, ignore (e.g., when just clicking)
		if (!m_bIsDrawingFrame && m_fSelectionFrameDuration < FRAME_DURATION_MIN)
			return false;
		
		float size;
		vector cursorPos = m_CursorComponent.GetCursorPos();
		float curveCoef = LegacyCurve.Curve(ECurveType.CurveProperty2D, (m_fSelectionFrameDuration - FRAME_DURATION_MIN) / m_fGamepadSelectionDuration, m_GamepadSelectionCurve)[1];

		Widget widgetShow = m_GamepadSetSelectionWidget;
		Widget widgetHide = m_GamepadToggleSelectionWidget;
		if (m_bIsDrawingFrameIsToggle)
		{
			widgetShow = m_GamepadToggleSelectionWidget;
			widgetHide = m_GamepadSetSelectionWidget;
		}

		if (widgetHide)
			widgetHide.SetVisible(false);

		if (widgetShow)
		{
			float cursorRadius = 0;
			if (m_CursorComponent)
				cursorRadius = 2 * m_CursorComponent.GetCursorRadius() * m_fGamepadSelectionWidgetStartSize;

			size = cursorRadius + (m_iGamepadSelectionHeight - cursorRadius) * curveCoef;
			FrameSlot.SetPos(widgetShow, cursorPos[0], cursorPos[1]);
			FrameSlot.SetSize(widgetShow, size, size);
			widgetShow.SetVisible(true);
			widgetShow.SetOpacity(1);
			
			//--- Convert to squared radius
			size *= 0.5;
			size *= size;
		}
		
		//--- Hide gamepad cursor
		if (m_CursorComponent && !m_bHideCursor && curveCoef > 0.01)
		{
			m_bHideCursor = true;
			m_CursorComponent.SetCursorAlpha(0, 32);
		}
		
		//--- Find entities in the frame
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			if (!SCR_Enum.HasPartialFlag(entity.GetEntityStates(), m_MultiSelectStates))
				continue;
			
			vector worldPos;
			if (!entity.GetPos(worldPos))
				continue;
			
			vector screenPos = m_Workspace.ProjWorldToScreen(worldPos, m_World);
			if (screenPos[2] > 0 && vector.DistanceSq(cursorPos, screenPos) < size)
			{
				if (m_LayersManager)
					entity = m_LayersManager.GetParentBelowCurrentLayer(entity);

				if (entity)
					entitiesInside.Insert(entity);
			}
		}
		
		return true;
	}
	
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected void ConfirmFrame(bool isToggle)
	{
		if (!m_bIsDrawingFrameConfirmed)
			return;

		m_bIsDrawingFrame = false;
		if (m_bIsDrawingFrameIsToggle)
		{
			EditorToggleSelection();
		}
		else
		{
			EditorSetSelection();
			
			set<SCR_EditableEntityComponent> selected = new set<SCR_EditableEntityComponent>();
			m_SelectedManager.GetEntities(selected, true);
			
			//Nothing selected so play SFX
			if (m_SelectedManager.GetEntitiesCount() <= 0)
				SCR_UISoundEntity.SoundEvent(m_sSfxSelectionFrameClose_NothingSelected, true);
			
//			if (m_InputManager.IsUsingMouseAndKeyboard())
//				EditorSetSelection();
//			else
//				EditorAddSelection(); //--- With gamepad, add to selection, don't replace it
		}

		ResetFrame();
		m_bIsDrawingFrameCancelled = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void CancelFrame()
	{
		if (m_bIsDrawingFrame)
		{				
			//~ Cancel Sfx
			SCR_UISoundEntity.SoundEvent(m_sSfxSelectionFrameClose_Cancel, true);
			
			m_bIsDrawingFrameCancelled = true;
			ResetFrame();
		}
	}
	
	//---- REFACTOR NOTE START: Use of unclear hardcoded values
	// Double click handled by delay using call later
	
	//------------------------------------------------------------------------------------------------
	protected void ResetFrame()
	{
		m_bIsDrawingFrameDisabled = false;
		m_fSelectionFrameDuration = 0;
		m_vCursorPosClick = vector.Zero;
		m_bIsDrawingFrame = false;
		m_bHideCursor = false;
		
		if (m_StatesManager)
			m_StatesManager.UnsetState(EEditorState.MULTI_SELECTING);

		//if (m_FocusedManager) m_FocusedManager.Clear(); //--- Disabled, handled in m_HoverManager.SetEntityUnderCursorEnabled
		if (m_HoverManager)
			m_HoverManager.SetEntityUnderCursorEnabled(true);
		
		if (m_SetSelectionWidget)
			m_SetSelectionWidget.SetVisible(false);

		if (m_ToggleSelectionWidget)
			m_ToggleSelectionWidget.SetVisible(false);
		//if (m_GamepadSetSelectionWidget)
		//	m_GamepadSetSelectionWidget.SetVisible(false);
		//if (m_GamepadToggleSelectionWidget)
		//	m_GamepadToggleSelectionWidget.SetVisible(false);
		if (m_CursorComponent)
			m_CursorComponent.SetCursorAlpha(1, 8);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCurrentLayerChange(SCR_EditableEntityComponent currentLayer, SCR_EditableEntityComponent prevLayer)
	{
		//--- Clear selection when layers change
		EnableSelection(false);
		
		//--- Block input for a few frames (double-click also counts as selection click)
		GetGame().GetCallqueue().CallLater(EnableSelection, 100, false, true);
		m_SelectedManager.Clear();
	}
	
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected void EnableSelection(bool enable)
	{
		m_bEnableSelection = enable;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMapOpened()
	{
		return SCR_MapEntity.GetMapInstance() && SCR_MapEntity.GetMapInstance().IsOpen();
	}

	//---- REFACTOR NOTE START: Use of unclear hardcoded values
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		if (!m_Workspace || !m_InputManager/* || !GetWidget().IsEnabledInHierarchy()*/)
			return;

		if (m_StatesManager && (m_StatesManager.GetState() != EEditorState.SELECTING && m_StatesManager.GetState() != EEditorState.MULTI_SELECTING))
			return; //--- ToDo: Move to OnStateChange event?

		//--- Activate context for input listeners
		m_InputManager.ActivateContext("EditorSelectingContext");
		
		if (m_bIsAnimatingFrame)
		{
			//--- Fade in gamepad selection frame
			m_fSelectionFrameDuration += tDelta;
			
			//--- Fade out gamepad selection frame
			if (!m_bIsDrawingFrame && (m_bIsDrawingFrameConfirmed || m_bIsDrawingFrameCancelled))
			{
				Widget frameWidget;
				if (m_bIsDrawingFrameIsToggle)
					frameWidget = m_GamepadToggleSelectionWidget;
				else
					frameWidget = m_GamepadSetSelectionWidget;
				
				if (frameWidget)
				{
					float opacity = Math.Lerp(frameWidget.GetOpacity(), 0, 32 * tDelta);
					if (opacity < 0.001)
					{
						opacity = 0;
						m_bIsAnimatingFrame = false;
					}
					frameWidget.SetOpacity(opacity);
				}
			}
		}
	}

	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost()
	{
		CancelFrame();
	}

	//------------------------------------------------------------------------------------------------
	protected Widget GetFrameWidget(Widget root, string name)
	{
		Widget widget = root.FindWidget(name);
		if (widget && !name.IsEmpty())
		{
			widget.SetVisible(false);
			return widget;
		}
		else
		{
			Print(string.Format("Selection frame widget '%1' not found!", name), LogLevel.ERROR);
			return null;
		}
	}
	
	//---- REFACTOR NOTE START: Worth rethinking?
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Saved selection (cannot be done dynamically, action listeners cannot pass index as param)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void EditorSaveSelection(int index)
	{
		m_SelectedManager.SaveSelection(index);
	}
	protected void EditorSaveSelection1() { EditorSaveSelection(1); }
	protected void EditorSaveSelection2() { EditorSaveSelection(2); }
	protected void EditorSaveSelection3() { EditorSaveSelection(3); }
	protected void EditorSaveSelection4() { EditorSaveSelection(4); }
	protected void EditorSaveSelection5() { EditorSaveSelection(5); }
	protected void EditorSaveSelection6() { EditorSaveSelection(6); }
	protected void EditorSaveSelection7() { EditorSaveSelection(7); }
	protected void EditorSaveSelection8() { EditorSaveSelection(8); }
	protected void EditorSaveSelection9() { EditorSaveSelection(9); }
	protected void EditorSaveSelection0() { EditorSaveSelection(0); }

	//------------------------------------------------------------------------------------------------
	protected void EditorLoadSelection(int index)
	{
		m_SelectedManager.LoadSelection(index);
	}
	protected void EditorLoadSelection1() { EditorLoadSelection(1); }
	protected void EditorLoadSelection2() { EditorLoadSelection(2); }
	protected void EditorLoadSelection3() { EditorLoadSelection(3); }
	protected void EditorLoadSelection4() { EditorLoadSelection(4); }
	protected void EditorLoadSelection5() { EditorLoadSelection(5); }
	protected void EditorLoadSelection6() { EditorLoadSelection(6); }
	protected void EditorLoadSelection7() { EditorLoadSelection(7); }
	protected void EditorLoadSelection8() { EditorLoadSelection(8); }
	protected void EditorLoadSelection9() { EditorLoadSelection(9); }
	protected void EditorLoadSelection0() { EditorLoadSelection(0); }

	//------------------------------------------------------------------------------------------------
	protected void EditorTeleportSelection(int index)
	{
		m_SelectedManager.TeleportSelection(index);
	}
	protected void EditorTeleportSelection1() { EditorTeleportSelection(1); }
	protected void EditorTeleportSelection2() { EditorTeleportSelection(2); }
	protected void EditorTeleportSelection3() { EditorTeleportSelection(3); }
	protected void EditorTeleportSelection4() { EditorTeleportSelection(4); }
	protected void EditorTeleportSelection5() { EditorTeleportSelection(5); }
	protected void EditorTeleportSelection6() { EditorTeleportSelection(6); }
	protected void EditorTeleportSelection7() { EditorTeleportSelection(7); }
	protected void EditorTeleportSelection8() { EditorTeleportSelection(8); }
	protected void EditorTeleportSelection9() { EditorTeleportSelection(9); }
	protected void EditorTeleportSelection0() { EditorTeleportSelection(0); }
	
	//---- REFACTOR NOTE END ----
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Custom methods
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected bool IsInputDisabled()
	{
		return m_bIsDrawingFrame //--- Multi-selection is active
			//--- Required classes unavailable
			|| !m_FocusedManager || !m_SelectedManager || !m_MouseArea 
			//--- Disabled internally
			|| !m_bEnableSelection
			//--- Clicked with mouse on unrelated widget with no entity under cursor
			|| (m_InputManager.IsUsingMouseAndKeyboard() && m_FocusedManager.GetEntitiesCount() == 0 && !m_MouseArea.IsMouseOn());
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowGUI()
	{
		if (m_EditorMenuManager)
			m_EditorMenuManager.SetVisible(true);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default methods
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//---- REFACTOR NOTE START: Use of hardcoded values and specific methods limits modularity
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		if (SCR_Global.IsEditMode())
			return; //--- Run-time only

		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		m_InputManager = game.GetInputManager();
		m_Workspace = game.GetWorkspace();
		m_World = game.GetWorld();
		if (!m_Workspace)
			return;

		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		m_SetSelectionWidget = GetFrameWidget(w, m_sSetSelectionWidgetName);
		m_ToggleSelectionWidget = GetFrameWidget(w, m_sToggleSelectionWidgetName);
		m_GamepadSetSelectionWidget = GetFrameWidget(w, m_sGamepadSetSelectionWidgetName);
		m_GamepadToggleSelectionWidget = GetFrameWidget(w, m_sGamepadToggleSelectionWidgetName);
		
		if (m_GamepadSetSelectionWidget)
			m_iGamepadSelectionHeight = FrameSlot.GetSizeY(m_GamepadSetSelectionWidget);

		MenuRootComponent root = GetRootComponent();
		if (root)
		{
			m_CursorComponent = SCR_CursorEditorUIComponent.Cast(root.FindComponent(SCR_CursorEditorUIComponent, true));
			if (!m_CursorComponent)
				return;
			
			m_MouseArea = SCR_MouseAreaEditorUIComponent.Cast(root.FindComponent(SCR_MouseAreaEditorUIComponent));
		}
		
		m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		if (m_StatesManager)
			m_StatesManager.SetState(EEditorState.SELECTING);
		
		m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (m_LayersManager)
			m_LayersManager.GetOnCurrentLayerChange().Insert(OnCurrentLayerChange);
		
		m_EditorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		m_EntitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
		if (!m_EntitiesManager)
			return;
		
		m_UnselectedManager = m_EntitiesManager.GetFilter(m_UnselectedState);
		m_FocusedManager = m_EntitiesManager.GetFilter(EEditableEntityState.FOCUSED);
		m_SelectedManager = SCR_SelectedEditableEntityFilter.Cast(m_EntitiesManager.GetFilter(EEditableEntityState.SELECTED));
		m_HoverManager = SCR_HoverEditableEntityFilter.Cast(m_EntitiesManager.GetFilter(EEditableEntityState.HOVER));
		
		if (m_InputManager)
		{
			m_InputManager.AddActionListener("EditorSetSelection", EActionTrigger.DOWN, EditorSetSelection);
			m_InputManager.AddActionListener("EditorClearSelection", EActionTrigger.DOWN, EditorClearSelection);
			if (m_bEnableMultiSelection)
			{
				m_InputManager.AddActionListener("EditorToggleSelection", EActionTrigger.DOWN, EditorToggleSelection);
				m_InputManager.AddActionListener("EditorSelectAll", EActionTrigger.DOWN, EditorSelectAll);
				m_InputManager.AddActionListener("EditorDrawSetSelection", EActionTrigger.DOWN, EditorDrawSetSelectionDown);
				m_InputManager.AddActionListener("EditorDrawSetSelection", EActionTrigger.PRESSED, EditorDrawSetSelectionPressed);
				m_InputManager.AddActionListener("EditorDrawSetSelection", EActionTrigger.UP, EditorDrawSetSelectionUp);
				
				m_InputManager.AddActionListener("EditorDrawToggleSelection", EActionTrigger.DOWN, EditorDrawToggleSelectionDown);
				m_InputManager.AddActionListener("EditorDrawToggleSelection", EActionTrigger.PRESSED, EditorDrawToggleSelectionPressed);
				m_InputManager.AddActionListener("EditorDrawToggleSelection", EActionTrigger.UP, EditorDrawToggleSelectionUp);
				
				m_InputManager.AddActionListener("EditorDrawSelectionCancel", EActionTrigger.DOWN, EditorDrawSelectionCancel);
			}
			
			m_InputManager.AddActionListener("EditorSaveSelection1", EActionTrigger.DOWN, EditorSaveSelection1);
			m_InputManager.AddActionListener("EditorSaveSelection2", EActionTrigger.DOWN, EditorSaveSelection2);
			m_InputManager.AddActionListener("EditorSaveSelection3", EActionTrigger.DOWN, EditorSaveSelection3);
			m_InputManager.AddActionListener("EditorSaveSelection4", EActionTrigger.DOWN, EditorSaveSelection4);
			m_InputManager.AddActionListener("EditorSaveSelection5", EActionTrigger.DOWN, EditorSaveSelection5);
			m_InputManager.AddActionListener("EditorSaveSelection6", EActionTrigger.DOWN, EditorSaveSelection6);
			m_InputManager.AddActionListener("EditorSaveSelection7", EActionTrigger.DOWN, EditorSaveSelection7);
			m_InputManager.AddActionListener("EditorSaveSelection8", EActionTrigger.DOWN, EditorSaveSelection8);
			m_InputManager.AddActionListener("EditorSaveSelection9", EActionTrigger.DOWN, EditorSaveSelection9);
			m_InputManager.AddActionListener("EditorSaveSelection0", EActionTrigger.DOWN, EditorSaveSelection0);
			
			m_InputManager.AddActionListener("EditorLoadSelection1", EActionTrigger.DOWN, EditorLoadSelection1);
			m_InputManager.AddActionListener("EditorLoadSelection2", EActionTrigger.DOWN, EditorLoadSelection2);
			m_InputManager.AddActionListener("EditorLoadSelection3", EActionTrigger.DOWN, EditorLoadSelection3);
			m_InputManager.AddActionListener("EditorLoadSelection4", EActionTrigger.DOWN, EditorLoadSelection4);
			m_InputManager.AddActionListener("EditorLoadSelection5", EActionTrigger.DOWN, EditorLoadSelection5);
			m_InputManager.AddActionListener("EditorLoadSelection6", EActionTrigger.DOWN, EditorLoadSelection6);
			m_InputManager.AddActionListener("EditorLoadSelection7", EActionTrigger.DOWN, EditorLoadSelection7);
			m_InputManager.AddActionListener("EditorLoadSelection8", EActionTrigger.DOWN, EditorLoadSelection8);
			m_InputManager.AddActionListener("EditorLoadSelection9", EActionTrigger.DOWN, EditorLoadSelection9);
			m_InputManager.AddActionListener("EditorLoadSelection0", EActionTrigger.DOWN, EditorLoadSelection0);
			
			m_InputManager.AddActionListener("EditorTeleportSelection1", EActionTrigger.DOWN, EditorTeleportSelection1);
			m_InputManager.AddActionListener("EditorTeleportSelection2", EActionTrigger.DOWN, EditorTeleportSelection2);
			m_InputManager.AddActionListener("EditorTeleportSelection3", EActionTrigger.DOWN, EditorTeleportSelection3);
			m_InputManager.AddActionListener("EditorTeleportSelection4", EActionTrigger.DOWN, EditorTeleportSelection4);
			m_InputManager.AddActionListener("EditorTeleportSelection5", EActionTrigger.DOWN, EditorTeleportSelection5);
			m_InputManager.AddActionListener("EditorTeleportSelection6", EActionTrigger.DOWN, EditorTeleportSelection6);
			m_InputManager.AddActionListener("EditorTeleportSelection7", EActionTrigger.DOWN, EditorTeleportSelection7);
			m_InputManager.AddActionListener("EditorTeleportSelection8", EActionTrigger.DOWN, EditorTeleportSelection8);
			m_InputManager.AddActionListener("EditorTeleportSelection9", EActionTrigger.DOWN, EditorTeleportSelection9);
			m_InputManager.AddActionListener("EditorTeleportSelection0", EActionTrigger.DOWN, EditorTeleportSelection0);
		}

		menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
		menu.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
			menu.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
		}
		
		if (m_LayersManager)
			m_LayersManager.GetOnCurrentLayerChange().Remove(OnCurrentLayerChange);

		if (m_InputManager)
		{
			m_InputManager.RemoveActionListener("EditorSetSelection", EActionTrigger.DOWN, EditorSetSelection);
			m_InputManager.RemoveActionListener("EditorClearSelection", EActionTrigger.DOWN, EditorClearSelection);
			if (m_bEnableMultiSelection)
			{
				m_InputManager.RemoveActionListener("EditorToggleSelection", EActionTrigger.DOWN, EditorToggleSelection);
				m_InputManager.RemoveActionListener("EditorSelectAll", EActionTrigger.DOWN, EditorSelectAll);

				m_InputManager.RemoveActionListener("EditorDrawSetSelection", EActionTrigger.DOWN, EditorDrawSetSelectionDown);
				m_InputManager.RemoveActionListener("EditorDrawSetSelection", EActionTrigger.PRESSED, EditorDrawSetSelectionPressed);
				m_InputManager.RemoveActionListener("EditorDrawSetSelection", EActionTrigger.UP, EditorDrawSetSelectionUp);
				
				m_InputManager.RemoveActionListener("EditorDrawToggleSelection", EActionTrigger.DOWN, EditorDrawToggleSelectionDown);
				m_InputManager.RemoveActionListener("EditorDrawToggleSelection", EActionTrigger.PRESSED, EditorDrawToggleSelectionPressed);
				m_InputManager.RemoveActionListener("EditorDrawToggleSelection", EActionTrigger.UP, EditorDrawToggleSelectionUp);
				
				m_InputManager.RemoveActionListener("EditorDrawSelectionCancel", EActionTrigger.DOWN, EditorDrawSelectionCancel);
			}
		}
	}
	
	//---- REFACTOR NOTE END ----
}
