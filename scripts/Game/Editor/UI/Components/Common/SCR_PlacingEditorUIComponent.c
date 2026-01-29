//! @ingroup Editor_UI Editor_UI_Components

class SCR_PlacingEditorUIComponent : SCR_PreviewEntityEditorUIComponent
{
	//--- When ASL vertical mode is on, how far from camera the entity appears when no ground intersection was found
	protected static const float MAX_PREVIEW_DIS = 50;
	
	private InputManager m_InputManager;
	private SCR_StatesEditorComponent m_StatesManager;
	private SCR_CursorEditorUIComponent m_CursorComponent;
	private SCR_EntitiesEditorUIComponent m_EntitiesComponent;
	private SCR_SelectionEditorUIComponent m_SelectionComponent;
	private SCR_MouseAreaEditorUIComponent m_MouseArea;
	//private SCR_SiteSlotEntity m_Slot;
	private bool m_bClicked;
	private bool m_bCanPlace;
	private ref array<Widget> m_aClickWhitelist = {};
	
	//------------------------------------------------------------------------------------------------
	override void OnHoverChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (m_StatesManager && m_StatesManager.GetState() == EEditorState.PLACING)
			super.OnHoverChange(state, entitiesInsert, entitiesRemove);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEditorTransformRotationModifierUp(float value, EActionTrigger reason)
	{
		if (m_StatesManager && m_StatesManager.GetState() == EEditorState.PLACING)
			super.OnEditorTransformRotationModifierUp(value, reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorPlace(float value, EActionTrigger reason)
	{
		Place(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorPlaceAndCancel(float value, EActionTrigger reason)
	{
		Place(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorPlacePlayer(float value, EActionTrigger reason)
	{
		Place(false, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorPlacePlayerAndCancel(float value, EActionTrigger reason)
	{
		Place(true, true);
		
		//--- Prevent multi-selection from starting right afterwards (it's mapped to the same button by default; ToDo: More sandbox?)
		if (m_SelectionComponent)
			m_SelectionComponent.DisableMultiSelection();
	}

	//------------------------------------------------------------------------------------------------
	protected void Place(bool cancelAfterwards, bool canBePlayer = false)
	{		
		//--- Placing not enabled
		if (!CanClick() || !m_bCanPlace || (m_StatesManager && m_StatesManager.GetState() != EEditorState.PLACING))
			return;
		
		if (!m_PreviewEntityManager.CanMoveInRoot() && !m_PreviewEntityManager.GetTarget())
			return;
		
		//--- Map is opened, placing not possible
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity && mapEntity.IsOpen())
			return;
		
		if (!m_PlacingManager.CreateEntity(cancelAfterwards, canBePlayer))
			return;
		
		//if (cancelAfterwards) CancelPlacing();
		m_bClicked = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorCancelPlacingUp(float value, EActionTrigger reason)
	{
		if (OnCancelUp())
			CancelPlacing();
	}

	//------------------------------------------------------------------------------------------------
	protected void CancelPlacing()
	{
		if (m_PlacingManager)
			m_PlacingManager.SetSelectedPrefab();
	}

	//------------------------------------------------------------------------------------------------
	protected void EnablePlacing()
	{
		m_bCanPlace = true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanClick()
	{
		if (m_CursorComponent && m_MouseArea && !m_MouseArea.IsMouseOn() && m_InputManager.IsUsingMouseAndKeyboard())
		{
			MenuRootComponent root = GetRootComponent();
			if (root)
			{
				Widget rootWidget = root.GetWidget();
				if (rootWidget)
				{
					//--- Get all widgets under cursor
					int mouseX, mouseY;
					m_CursorComponent.GetCursorPos(mouseX, mouseY);
					array<Widget> widgets = {};
					WidgetManager.TraceWidgets(mouseX, mouseY, rootWidget, widgets);
					
					//--- Check if the widgets are whitelisted. If not, terminate
					foreach (Widget widget: widgets)
					{
						bool canClick = false;
						foreach (Widget widgetWhitelisted: m_aClickWhitelist)
						{
							canClick |= IsChildOf(widget, widgetWhitelisted);
						}

						if (!canClick)
							return false;
					}
				}
			}
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsChildOf(Widget widget, Widget parent)
	{
		while (widget)
		{
			if (widget == parent)
				return true;

			widget = widget.GetParent();
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelectedPrefabChange(ResourceName prefab, ResourceName prefabPrev)
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		ScriptCallQueue queue = game.GetCallqueue();
		if (!queue)
			return;
		
		//--- Ignore input right after starting placing. Important for gamepads, because 'A' button is still held after selection.
		queue.CallLater(EnablePlacing, 50);
		m_bCanPlace = false;
		
		//--- Set default entity height when in ASL vertical mode
		SCR_PreviewEntityEditorComponent previewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		if (previewManager && previewManager.GetVerticalMode() == EEditorTransformVertical.SEA)
		{
			BaseWorld world = game.GetWorld();
			if (!world)
				return;
			
			WorkspaceWidget workspace = game.GetWorkspace();
			if (!workspace)
				return;
			
			//--- Get intersection in the middle of the screen (don't use cursor, it just returned form a sub-menu ans is not relevant)
			float screenW, screenH;
			workspace.GetScreenSize(screenW, screenH);
			vector cameraDir;
			vector cameraPos = workspace.ProjScreenToWorldNative(screenW * 0.5, screenH * 0.5, cameraDir, world, -1);
			
			//--- Find ground intersection, or use maximum distance when none is found
			float traceDis = GetTraceDis(cameraPos, cameraDir * TRACE_DIS, cameraPos[1]);
			if (traceDis == 1)
				traceDis = MAX_PREVIEW_DIS;
			else
				traceDis *= TRACE_DIS;
			
			//--- Set default preview height
			previewManager.SetPreviewHeight(cameraPos + cameraDir * traceDis);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		ActivatePreviewContext();
		
		if (m_StatesManager && m_StatesManager.GetState() != EEditorState.PLACING)
			return;
		
		if (m_InputManager && m_PlacingManager)
		{
			m_InputManager.ActivateContext("EditorPlacingContext");
			if (m_PlacingManager.IsPlacingFlagCompatible(EEditorPlacingFlags.CHARACTER_PLAYER))
				m_InputManager.ActivateContext("EditorPlacingPlayerContext");
		}

		ProcessInput(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusGained()
	{
		GetMenu().GetOnMenuUpdate().Insert(OnMenuUpdate);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost()
	{
		GetMenu().GetOnMenuUpdate().Remove(OnMenuUpdate);
	}

	//------------------------------------------------------------------------------------------------
	override protected SCR_EPreviewState GetPreviewStateToShow()
	{
		if (m_PlacingManager && m_PlacingManager.CanCreateEntity())
			return SCR_EPreviewState.PLACEABLE;
		else
			return SCR_EPreviewState.BLOCKED;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		MenuRootComponent root = GetRootComponent();
		if (root)
		{
			m_MouseArea = SCR_MouseAreaEditorUIComponent.Cast(root.FindComponent(SCR_MouseAreaEditorUIComponent));
			m_EntitiesComponent = SCR_EntitiesEditorUIComponent.Cast(root.FindComponent(SCR_EntitiesEditorUIComponent));
			m_CursorComponent = SCR_CursorEditorUIComponent.Cast(root.FindComponent(SCR_CursorEditorUIComponent));
			m_SelectionComponent = SCR_SelectionEditorUIComponent.Cast(root.FindComponent(SCR_SelectionEditorUIComponent));
			
			if (m_MouseArea)
				m_aClickWhitelist.Insert(m_MouseArea.GetWidget());

			if (m_EntitiesComponent)
				m_aClickWhitelist.Insert(m_EntitiesComponent.GetWidget());
		}

		if (!m_CursorComponent)
			Print("SCR_PlacingEditorUIComponent requires SCR_CursorEditorUIComponent!", LogLevel.ERROR);
				
		m_PlacingManager.GetOnSelectedPrefabChange().Insert(OnSelectedPrefabChange);
		
		m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
			menu.GetOnMenuFocusGained().Insert(OnMenuFocusGained);
			menu.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
		}
		
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			m_InputManager = game.GetInputManager();
			if (m_InputManager)
			{
				m_InputManager.AddActionListener("EditorPlace", EActionTrigger.DOWN, OnEditorPlace);
				m_InputManager.AddActionListener("EditorPlaceAndCancel", EActionTrigger.DOWN, OnEditorPlaceAndCancel);
				m_InputManager.AddActionListener("EditorPlacePlayer", EActionTrigger.DOWN, OnEditorPlacePlayer);
				m_InputManager.AddActionListener("EditorPlacePlayerAndCancel", EActionTrigger.DOWN, OnEditorPlacePlayerAndCancel);
				m_InputManager.AddActionListener("EditorCancelPlacing", EActionTrigger.DOWN, OnCancelDown);
				m_InputManager.AddActionListener("EditorCancelPlacing", EActionTrigger.UP, OnEditorCancelPlacingUp);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_PlacingManager)
			m_PlacingManager.GetOnSelectedPrefabChange().Remove(OnSelectedPrefabChange);
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
			menu.GetOnMenuFocusGained().Remove(OnMenuFocusGained);
			menu.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
		}
		
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			InputManager inputManager = game.GetInputManager();
			if (m_InputManager)
			{
				inputManager.RemoveActionListener("EditorPlace", EActionTrigger.DOWN, OnEditorPlace);
				inputManager.RemoveActionListener("EditorPlaceAndCancel", EActionTrigger.DOWN, OnEditorPlaceAndCancel);
				inputManager.RemoveActionListener("EditorPlacePlayer", EActionTrigger.DOWN, OnEditorPlacePlayer);
				inputManager.RemoveActionListener("EditorPlacePlayerAndCancel", EActionTrigger.DOWN, OnEditorPlacePlayerAndCancel);
				m_InputManager.RemoveActionListener("EditorCancelPlacing", EActionTrigger.DOWN, OnCancelDown);
				m_InputManager.RemoveActionListener("EditorCancelPlacing", EActionTrigger.UP, OnEditorCancelPlacingUp);
			}
		}
	}
}
