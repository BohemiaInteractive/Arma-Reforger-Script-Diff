//------------------------------------------------------------------------------------------------
class SCR_BaseContextMenuEditorUIComponent : SCR_BaseEditorUIComponent
{	
	[Attribute("", UIWidgets.ResourceNamePicker, "Button", "layout")]
	protected ResourceName m_ButtonPath;
	
	[Attribute("VLayout", UIWidgets.EditBox)]
	protected string m_VLayoutWidgetName;
	
	[Attribute("BottomBar")]
	protected string m_sBottomBarName;
	
	[Attribute("250", desc: "Screen width of the button prefab")]
	protected int m_ButtonPrefabSizeX;
	[Attribute("34", desc: "Screen height of the button prefab")]
	protected int m_ButtonPrefabSizeY;
	
	[Attribute("4")]
	protected float m_fMenuMouseOffsetX;
	
	[Attribute("0.025", "Fade delay in seconds. For every new button that is added the delay increases slightly")]
	protected float m_fFadeDelayNextButton;
	
	[Attribute("4", "How fast each button appears")]
	protected float m_fButtonFadeSpeed;
	
	protected InputManager m_InputManager;
	protected WorkspaceWidget m_WorkSpace;
	
	protected Widget m_ContextMenu;
	protected Widget m_VLayout;
	protected SCR_FadeUIComponent m_BottomBarFadeUI;
	
	protected vector m_MouseDownScreenPos;
	protected vector m_MouseDownWorldPos;
	
	protected ref map<Widget, SCR_BaseEditorAction> m_ButtonActions = new map<Widget, SCR_BaseEditorAction>;
	
	protected SCR_BaseActionsEditorComponent m_EditorActionsComponent;
	protected SCR_EditorManagerEntity m_EditorManager;
	protected SCR_CursorEditorUIComponent m_CursorComponent;
	protected SCR_MouseAreaEditorUIComponent m_MouseArea;
	
	protected int m_ActionFlagsOnOpen;
	
	protected ref ScriptInvoker m_OnContextMenuToggle = new ScriptInvoker();
	
	void CloseContextMenu()
	{
		if (m_ContextMenu)
			m_ContextMenu.SetVisible(false);
		
		if (m_WorkSpace)
			m_WorkSpace.SetFocusedWidget(null);
		
		m_OnContextMenuToggle.Invoke(false);
	}
	ScriptInvoker GetOnContextMenuToggle()
	{
		return m_OnContextMenuToggle;
	}
	
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		CloseContextMenu();
	}
	
	//---- REFACTOR NOTE START: UI behavior might be unifiend 
	// Using hard coded widget name
	
	protected void PopulateContextMenu(vector cursorWorldPosition)
	{
		array<ref SCR_EditorActionData> actions = {};
		m_EditorActionsComponent.GetAndEvaluateActions(cursorWorldPosition, actions, m_ActionFlagsOnOpen);
		m_BottomBarFadeUI.GetFadeWidget().SetVisible(false);
		
		float fadeDelay = 0;	
		
		Widget lastButtonLayout;
		
		// Add Context Action buttons
		for (int i = 0; i < actions.Count(); i++)
		{
			SCR_BaseEditorAction action = actions[i].GetAction();
			
			// Don't show actions in context menu that can't be performed
			bool canBePerformed = actions[i].GetCanBePerformed();
			if (!canBePerformed)
				continue;
			
			SCR_UIInfo info = action.GetInfo();
			// Create button layout
			Widget buttonLayout = m_WorkSpace.CreateWidgets(m_ButtonPath, m_VLayout);
			lastButtonLayout = buttonLayout;
			
			SCR_ContextMenuButtonEditorUIComponent contextMenuButtonComponent = SCR_ContextMenuButtonEditorUIComponent.Cast(buttonLayout.FindHandler(SCR_ContextMenuButtonEditorUIComponent));
			
			ButtonActionComponent.GetOnAction(buttonLayout).Insert(OnContextMenuOptionClicked);
			
			contextMenuButtonComponent.SetData(info, action.GetShortcut(), canBePerformed);
			
			SCR_LinkTooltipTargetEditorUIComponent.SetInfo(buttonLayout, info, action);
			
			m_ButtonActions.Set(buttonLayout, action);
			
			SCR_FadeUIComponent fadeComponent = SCR_FadeUIComponent.Cast(buttonLayout.FindHandler(SCR_FadeUIComponent));
			if (fadeComponent)
			{
				fadeComponent.SetFadeInSpeed(m_fButtonFadeSpeed);
				fadeComponent.DelayedFadeIn(fadeDelay * 1000, true);
				fadeDelay += m_fFadeDelayNextButton;
			}
		}
		
		// Hide the divider for lastButtonLayout
		if (lastButtonLayout)
		{
			Widget divider = lastButtonLayout.FindAnyWidget("Stripe");
		
			if (divider)
				divider.SetVisible(false);
		}
		
		
		if (m_BottomBarFadeUI)
			m_BottomBarFadeUI.DelayedFadeIn(fadeDelay * 1000, true);
	}
	
	//---- REFACTOR NOTE END ----
	
	bool IsContextMenuOpen()
	{
		return m_ContextMenu != null && m_ContextMenu.IsVisibleInHierarchy();
	}
	
	protected void OpenContextMenu()
	{
		if (!m_InputManager.IsUsingMouseAndKeyboard() || !m_EditorActionsComponent)
		{
			return;
		}
		
		// Remove existing button widgets
		RemoveButtonWidgets();
		
		m_MouseDownWorldPos = GetCursorWorldPos();
		
		PopulateContextMenu(m_MouseDownWorldPos);
		
		// Get context menu size and screen size
		float contextMenuWidth, contextMenuHeight, screenWidth, screenHeight;
		m_WorkSpace.GetScreenSize(screenWidth, screenHeight);
		
		// Convert to reference resolution
		screenWidth = m_WorkSpace.DPIUnscale(screenWidth);
		screenHeight = m_WorkSpace.DPIUnscale(screenHeight);
		
		contextMenuHeight = m_ButtonActions.Count() * m_ButtonPrefabSizeY;
		contextMenuWidth = m_ButtonPrefabSizeX;
		
		
		
		// If context menu goes offscreen, move to other side of cursor for both x and y
		if (m_MouseDownScreenPos[0] + contextMenuWidth > screenWidth)
		{
			m_MouseDownScreenPos[0] = (m_MouseDownScreenPos[0]) - (contextMenuWidth + m_fMenuMouseOffsetX);
		}
		else 
		{
			m_MouseDownScreenPos[0] = m_MouseDownScreenPos[0] + m_fMenuMouseOffsetX;
		}
		if (m_MouseDownScreenPos[1] + contextMenuHeight > screenHeight)
		{
			m_MouseDownScreenPos[1] = (m_MouseDownScreenPos[1]) - contextMenuHeight;
		}
		
		// Set Context menu position
		FrameSlot.SetPos(m_ContextMenu, m_MouseDownScreenPos[0], m_MouseDownScreenPos[1]);
		// Show context menu
		if (m_ButtonActions.Count() > 0)
		{
			m_ContextMenu.SetVisible(true);	
			m_OnContextMenuToggle.Invoke(true);
		}
	}
	
	protected vector GetCursorWorldPos()
	{
		vector cursorWorldPosition;
		
		if (!m_CursorComponent)
			m_CursorComponent = SCR_CursorEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_CursorEditorUIComponent));
		
		if (m_CursorComponent && m_CursorComponent.GetCursorWorldPos(cursorWorldPosition))
			return cursorWorldPosition;
		
		return vector.Zero;
	}
	
	protected void RemoveButtonWidgets()
	{		
		Widget childWidget = m_VLayout.GetChildren();
		
		while (childWidget)
		{
			childWidget.RemoveFromHierarchy();
			childWidget = m_VLayout.GetChildren();
		}
		
		m_ButtonActions.Clear();
	}
	
	protected void OnMouseLeftDown()
	{
		if (!IsContextMenuOpen()) return;
		
		float mouseX, mouseY, layoutX1, layoutY1, layoutX2, layoutY2, layoutWidth, layoutHeight;
		mouseX = m_InputManager.GetActionValue("MouseX");
		mouseY = m_InputManager.GetActionValue("MouseY");
		
		m_ContextMenu.GetScreenPos(layoutX1, layoutY1);
		m_ContextMenu.GetScreenSize(layoutWidth, layoutHeight);
		layoutX2 = layoutX1 + layoutWidth;
		layoutY2 = layoutY1 + layoutHeight;
		
		// Close context menu when clicking outside of it
		if (mouseX < layoutX1 || mouseY < layoutY1 || mouseX > layoutX2 || mouseY > layoutY2)
		{
			CloseContextMenu();
		}
	}
	
	protected void OnOpenActionsMenuDown()
	{
		CloseContextMenu();
		SCR_BaseEditableEntityFilter hoverFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER);
		if ((!m_MouseArea || m_MouseArea.IsMouseOn()) || (!hoverFilter || !hoverFilter.IsEmpty()))
			m_MouseDownScreenPos = OnCancelDown();
	}
	
	protected void OnContextMenuOptionClicked(Widget widget, float value, EActionTrigger reason)
	{
		SCR_BaseEditorAction action = m_ButtonActions.Get(widget);
		if (action && m_EditorActionsComponent)
		{
			m_EditorActionsComponent.ActionPerform(action, m_MouseDownWorldPos, m_ActionFlagsOnOpen);
		}
		CloseContextMenu();
	}
	
	protected void OnEditorModeChanged()
	{
		// Override to assign m_EditorActionsComponent	
	}
	
	override void HandlerAttachedScripted(Widget w)
	{		
		super.HandlerAttachedScripted(w);
		
		ArmaReforgerScripted game = GetGame();
		m_WorkSpace = game.GetWorkspace();
		m_InputManager = game.GetInputManager();
		
		if (m_InputManager)
		{
			m_InputManager.AddActionListener("MouseLeft", EActionTrigger.DOWN, OnMouseLeftDown);
			m_InputManager.AddActionListener("EditorContextMenuClose", EActionTrigger.DOWN, CloseContextMenu);
		}
		
		if (m_ButtonPath.IsEmpty())
		{
			Print("Button prefab not set on (actions/waypoint) ContextMenuEditorUIComponent", LogLevel.ERROR);
		}
		
		game.OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		
		m_EditorManager.GetOnModeChange().Insert(OnEditorModeChanged);
		
		m_CursorComponent = SCR_CursorEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_CursorEditorUIComponent));
		m_MouseArea = SCR_MouseAreaEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_MouseAreaEditorUIComponent));
		
		OnEditorModeChanged();
		
		m_ContextMenu = GetWidget();
		m_ContextMenu.SetVisible(false);	
		m_VLayout = m_ContextMenu.FindAnyWidget(m_VLayoutWidgetName);
		
		if (!m_VLayout)
		{
			Print("Vertical layout not found on (actions/waypoint) ContextMenuEditorUIComponent, see m_VLayoutWidgetName", LogLevel.ERROR);
		}
		
		Widget bottomBar = w.FindAnyWidget(m_sBottomBarName);
		if (bottomBar)
		{
			m_BottomBarFadeUI = SCR_FadeUIComponent.Cast(bottomBar.FindHandler(SCR_FadeUIComponent));
			if (m_BottomBarFadeUI)
			{
				m_BottomBarFadeUI.SetFadeInSpeed(m_fButtonFadeSpeed);
				bottomBar.SetVisible(false);
			}
		}
	}
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_InputManager)
		{
			m_InputManager.RemoveActionListener("MouseLeft", EActionTrigger.DOWN, OnMouseLeftDown);
			m_InputManager.RemoveActionListener("EditorContextMenuClose", EActionTrigger.DOWN, CloseContextMenu);
		}
		
		if (m_EditorManager)
		{
			m_EditorManager.GetOnModeChange().Remove(OnEditorModeChanged);
		}
		
		if (GetGame().OnInputDeviceIsGamepadInvoker())
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
};
