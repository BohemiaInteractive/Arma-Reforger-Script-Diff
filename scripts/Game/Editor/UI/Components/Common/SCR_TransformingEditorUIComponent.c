//! @ingroup Editor_UI Editor_UI_Components

class SCR_TransformingEditorUIComponent : SCR_PreviewEntityEditorUIComponent
{
	protected InputManager m_InputManager;
	protected SCR_TransformingEditorComponent m_TransformingManager;
	protected SCR_StatesEditorComponent m_StatesManager;
	protected SCR_CursorEditorUIComponent m_CursorComponent;
	//protected SCR_HoverEditableEntityFilter m_HoverFilter;
	protected SCR_BaseEditableEntityFilter m_SelectedFilter;
	protected SCR_BaseEditableEntityFilter m_FocusedFilter;
	protected SCR_EditableEntityComponent m_HoverEntity;
	protected SCR_EditableEntityComponent m_ActiveHoverEntity;
	protected SCR_LayersEditorComponent m_LayersManager;
	protected SCR_EntitiesToolbarEditorUIComponent m_EntityToolbar;
	protected vector m_vClickTransform[4];
	protected bool m_bClicked;
	
	//------------------------------------------------------------------------------------------------
	override void OnHoverChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (m_StatesManager && m_StatesManager.GetState() == EEditorState.TRANSFORMING)
			super.OnHoverChange(state, entitiesInsert, entitiesRemove);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEditorTransformRotationModifierUp(float value, EActionTrigger reason)
	{
		if (m_StatesManager && m_StatesManager.GetState() == EEditorState.TRANSFORMING)
			super.OnEditorTransformRotationModifierUp(value, reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorTransformToggleDown(float value, EActionTrigger reason)
	{
		if (m_TransformingManager.IsEditing())
			ConfirmEditing();
		else
			OnEditorTransformDown(value, reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorTransformDown(float value, EActionTrigger reason)
	{
		if (!m_InputManager || !m_SelectedFilter || !m_TransformingManager || !m_CursorComponent)
			return;
		
		//--- Can't rotate from the entity toolbar
		if (m_EntityToolbar && m_EntityToolbar.IsUnderCursor() && m_InputManager.GetActionTriggered("EditorTransformRotateYawModifier"))
			return;
		
		//--- Ignore when not hovering over an entity
		bool isDelegate;
		SCR_EditableEntityComponent hoverEntity = m_HoverFilter.GetEntityUnderCursor(isDelegate);
		if (!hoverEntity)
			return;
		
		m_HoverEntity = hoverEntity;
		m_HoverEntity.GetOwner().GetWorldTransform(m_vClickTransform);
		
		//--- Hovered entity not selected - replace the selection
		if (m_LayersManager)
			m_ActiveHoverEntity = m_LayersManager.GetParentBelowCurrentLayer(m_HoverEntity);
		else
			m_ActiveHoverEntity = m_HoverEntity;
		
		vector worldPos;
		if (isDelegate)
		{
			//--- Clicked on icon - use entity's position as a pivot
			if (!m_HoverEntity.GetPos(worldPos))
				return;
		}
		else
		{
			//--- Use cursor world position as a pivot
			if (!m_CursorComponent.GetCursorWorldPos(worldPos))
				return;
		}
		
		m_vClickTransform[3] = worldPos;
		m_bClicked = true;
		
		if (m_InputManager.IsUsingMouseAndKeyboard())
			SetClickPos(m_CursorComponent.GetCursorPos()); //--- Don't start editing just yet, do so only after cursor moves a bit
		else
			StartEditing();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorCancelTransformingUp(float value, EActionTrigger reason)
	{
		if (OnCancelUp())
			CancelEditing();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StartEditing()
	{
		if (m_StatesManager && !m_StatesManager.CanSet(EEditorState.TRANSFORMING) || m_StatesManager.GetState() == EEditorState.TRANSFORMING)
			return;
		
		//--- Transforming unselected entity - replace the selection upon click
		if (!m_SelectedFilter.Contains(m_ActiveHoverEntity))
			m_SelectedFilter.Replace(m_ActiveHoverEntity);
		
		//--- Get selected entities
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		m_SelectedFilter.GetEntities(entities);
		
		//--- Start editing process
		m_TransformingManager.StartEditing(m_HoverEntity, entities, m_vClickTransform);
		
		m_TransformingManager.GetOnCleanStart().Insert(OnTransformCleanStart);
	}

	//------------------------------------------------------------------------------------------------
	protected void ConfirmEditing()
	{
		if (!m_TransformingManager.IsEditing() && !m_bClicked)
			return;
		
		//--- Cannot move attachable entity when target is not set
		if (!m_PreviewEntityManager.CanMoveInRoot() && !m_PreviewEntityManager.GetTarget())
		{
			if (m_InputManager.IsUsingMouseAndKeyboard())
				CancelEditing();
			
			return;
		}
		
		Clean();
		
		if (!m_StatesManager || m_StatesManager.GetState() != EEditorState.TRANSFORMING)
			return;
		
		if (!m_TransformingManager.ConfirmEditing())
		{
			if (m_InputManager.IsUsingMouseAndKeyboard()) //--- ToDo: Audio feedback
				m_TransformingManager.CancelEditing();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CancelEditing()
	{
		if (!m_bClicked && !m_TransformingManager.IsEditing())
			return;
		
		if (m_TransformingManager)
			m_TransformingManager.CancelEditing();

		Clean();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTransformCleanStart()
	{
		CancelEditing();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Clean()
	{
		m_bClicked = false;
		m_InputManager.ResetAction("EditorTransform");
		m_HoverEntity = null;
		m_ActiveHoverEntity = null;
		
		m_TransformingManager.GetOnCleanStart().Remove(OnTransformCleanStart);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTransformationEnd()
	{
		Clean();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		CancelEditing();
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Update
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		ActivatePreviewContext();
		
		if (!m_InputManager || !m_StatesManager || !m_CursorComponent)
			return;

		if (SCR_MapEntity.GetMapInstance() && SCR_MapEntity.GetMapInstance().IsOpen()) // map shown
			return;
		
		m_InputManager.ActivateContext("EditorTransformingContext");
		
		//--- Allow interaction only when cursor moves far enough from clicked position
		if (m_InputManager.IsUsingMouseAndKeyboard())
		{
			if (m_bClicked && HasMouseMoved())
				StartEditing();
			else
				return;
		}
		
		//--- Exit when not transforming
		if (m_StatesManager.GetState() != EEditorState.TRANSFORMING)
			return;
		
		ProcessInput(tDelta);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default methods
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{	
		super.HandlerAttachedScripted(w);
		
		MenuRootComponent root = GetRootComponent();
		if (root)
			m_CursorComponent = SCR_CursorEditorUIComponent.Cast(root.FindComponent(SCR_CursorEditorUIComponent, true));

		if (!m_CursorComponent)
			return;
		
		m_TransformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent, true));
		if (!m_TransformingManager)
			return;

		m_TransformingManager.GetOnTransformationConfirm().Insert(OnTransformationEnd);
		m_TransformingManager.GetOnTransformationCancel().Insert(OnTransformationEnd);
		
		m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent, true));
		if (!m_StatesManager)
			return;
		
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent, true));
		if (entitiesManager)
		{
			//m_HoverFilter = SCR_HoverEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.HOVER));
			m_SelectedFilter = entitiesManager.GetFilter(EEditableEntityState.SELECTED);
			m_FocusedFilter = entitiesManager.GetFilter(EEditableEntityState.FOCUSED);
		}
		
		m_EntityToolbar = SCR_EntitiesToolbarEditorUIComponent.Cast(root.FindComponent(SCR_EntitiesToolbarEditorUIComponent));
		
		m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
			menu.GetOnMenuFocusGained().Insert(Clean); //--- Reset dragging if a dialog was opened and closed during that
		}
		
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			m_InputManager = game.GetInputManager();
			if (m_InputManager)
			{
				m_InputManager.ResetAction("EditorTransform");
				m_InputManager.AddActionListener("EditorTransform", EActionTrigger.DOWN, OnEditorTransformDown);
				m_InputManager.AddActionListener("EditorTransform", EActionTrigger.UP, ConfirmEditing);
				
				m_InputManager.AddActionListener("EditorTransformToggle", EActionTrigger.DOWN, OnEditorTransformToggleDown);
				
				//m_InputManager.AddActionListener("EditorTransformRotationModifier", EActionTrigger.DOWN, OnEditorTransformRotationModifierDown);
				
				m_InputManager.AddActionListener("EditorConfirmTransforming", EActionTrigger.DOWN, ConfirmEditing);
				m_InputManager.AddActionListener("EditorCancelTransforming", EActionTrigger.DOWN, OnCancelDown);
				m_InputManager.AddActionListener("EditorCancelTransforming", EActionTrigger.UP, OnEditorCancelTransformingUp);
			}
			
			game.OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		}
		
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapOpen(MapConfiguration config)
	{
		CancelEditing();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_TransformingManager)
		{
			m_TransformingManager.GetOnTransformationConfirm().Remove(OnTransformationEnd);
			m_TransformingManager.GetOnTransformationCancel().Remove(OnTransformationEnd);
		}
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
			menu.GetOnMenuFocusGained().Remove(Clean);
		}
		
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			if (m_InputManager)
			{
				m_InputManager.RemoveActionListener("EditorTransform", EActionTrigger.DOWN, OnEditorTransformDown);
				m_InputManager.RemoveActionListener("EditorTransform", EActionTrigger.UP, ConfirmEditing);
				
				m_InputManager.RemoveActionListener("EditorTransformToggle", EActionTrigger.DOWN, OnEditorTransformToggleDown);
				
				//m_InputManager.RemoveActionListener("EditorTransformRotationModifier", EActionTrigger.DOWN, OnEditorTransformRotationModifierDown);
				
				m_InputManager.RemoveActionListener("EditorConfirmTransforming", EActionTrigger.DOWN, ConfirmEditing);
				m_InputManager.RemoveActionListener("EditorCancelTransforming", EActionTrigger.DOWN, OnCancelDown);
				m_InputManager.RemoveActionListener("EditorCancelTransforming", EActionTrigger.UP, OnEditorCancelTransformingUp);
			}
			
			game.OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		}
		
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	//! If the composition can't be transformed to a new position, turn the colour of the preview to red.
	override protected SCR_EPreviewState GetPreviewStateToShow()
	{
		if (m_TransformingManager.CanBeTransformed())
			return SCR_EPreviewState.PLACEABLE;
		else
			return SCR_EPreviewState.BLOCKED;
	}
}
