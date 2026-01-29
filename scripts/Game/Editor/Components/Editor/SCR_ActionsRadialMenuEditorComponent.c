//! \addtogroup EditorRadialMenu
//! \{

[ComponentEditorProps(category: "GameScripted/Editor", description: "Responsible for controlling radial menus used within GameMaster", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_ActionsRadialMenuEditorComponentClass : SCR_BaseEditorComponentClass
{
}

//! Responsible for controlling and managing radial menus used within GameMaster.
class SCR_ActionsRadialMenuEditorComponent : SCR_BaseEditorComponent
{
	[Attribute()]
	protected ref SCR_RadialMenuController m_ActionsMenuController;

	[Attribute()]
	protected ref SCR_RadialMenuController m_CommandsMenuController;

	[Attribute()]
	protected ref SCR_RadialMenuController m_AddCommandsMenuController;
	
	// Action names 

	[Attribute()]
	protected string m_sActionName;

	[Attribute()]
	protected string m_sCommandActionName;

	[Attribute()]
	protected string m_sAddCommandActionName;
	
	//---- REFACTOR NOTE START: Implementation of multiple radial menus is not very modular ----
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] oldDevice
	//! \param[in] newDevice
	void OnInputDeviceChanged(bool isGamepad)
	{	
		// Clearup and close radial menu if not using gamepad
		if (!isGamepad)
		{
			CleanMenu();

			if (m_ActionsMenuController)
			{
				m_ActionsMenuController.SetEnableControl(false);
				m_ActionsMenuController.StopControl(true);
			}
			
			if (m_CommandsMenuController)
			{
				m_CommandsMenuController.SetEnableControl(false);
				m_CommandsMenuController.StopControl(true);
			}

			if (m_AddCommandsMenuController)
			{
				m_AddCommandsMenuController.SetEnableControl(false);
				m_AddCommandsMenuController.StopControl(true);
			}
			
			return;
		}
		
		// Gamepad
		if (m_ActionsMenuController)
			m_ActionsMenuController.SetEnableControl(true);
		
		if (m_CommandsMenuController)
			m_CommandsMenuController.SetEnableControl(true);

		if (m_AddCommandsMenuController)
			m_AddCommandsMenuController.SetEnableControl(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBeforeMenuOpen(notnull SCR_RadialMenuController menuController, notnull SCR_BaseActionsEditorComponent actionsComponent, int flags = 0)
	{
		CleanMenu();

		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD)
			return;

		SCR_MenuLayoutEditorComponent editorMenuLayout = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent, true));
		if (!editorMenuLayout)
			return;

		SCR_RadialMenu radialMenu = menuController.GetRadialMenu();
		if (!radialMenu)
			return;

		vector cursorWorldPosition;
		if (!editorMenuLayout.GetCursorWorldPos(cursorWorldPosition))
			return;

		array<ref SCR_EditorActionData> filteredActions = {};
		actionsComponent.GetAndEvaluateActions(cursorWorldPosition, filteredActions, flags);

		// Close menu if there is nothing to display.
		if (filteredActions.IsEmpty())
		{
			CleanMenu();
			return;
		}

		array<ref SCR_SelectionMenuEntry> radialMenuEntries = {};
		foreach (SCR_EditorActionData actionData : filteredActions)
		{
			SCR_BaseEditorAction action = actionData.GetAction();
			if (!action)
				continue;

			SCR_UIInfo info = action.GetInfo();
			if (!info)
				continue;

			SCR_EditorActionSelectionMenuEntry menuEntry = new SCR_EditorActionSelectionMenuEntry(action, actionsComponent, cursorWorldPosition, flags);

			menuEntry.Enable(true);
			menuEntry.SetName(info.GetName());
			menuEntry.SetIcon(info.GetIconPath());
			menuEntry.SetDescription(info.GetDescription());
			menuEntry.SetInputAction(action.GetShortcut());

			radialMenuEntries.Insert(menuEntry);
		}

		radialMenu.GetOnClose().Insert(CleanMenu);
		radialMenu.AddEntries(radialMenuEntries, true);
		
		menuController.OnInputOpen();
	}

	//------------------------------------------------------------------------------------------------
	protected void CleanMenu()
	{
		SCR_RadialMenu radialMenu = null;

		if (m_ActionsMenuController)
			radialMenu = m_ActionsMenuController.GetRadialMenu();

		if (radialMenu)
		{
			radialMenu.ClearEntries();
			radialMenu = null;
		}

		if (m_CommandsMenuController)
			radialMenu = m_CommandsMenuController.GetRadialMenu();

		if (radialMenu)
		{
			radialMenu.ClearEntries();
			radialMenu = null;
		}

		if (m_AddCommandsMenuController)
			radialMenu = m_AddCommandsMenuController.GetRadialMenu();

		if (radialMenu)
		{
			radialMenu.ClearEntries();
			radialMenu = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionRadialMenuOpen()
	{
		if (!m_ActionsMenuController)
			return;
		
		m_ActionsMenuController.Control(GetOwner());

		SCR_ContextActionsEditorComponent editorActionsComponent = SCR_ContextActionsEditorComponent.Cast(SCR_ContextActionsEditorComponent.GetInstance(SCR_ContextActionsEditorComponent, true));
		if (!editorActionsComponent)
			return;

		OnBeforeMenuOpen(m_ActionsMenuController, editorActionsComponent);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommandsRadialMenu()
	{
		if (!m_CommandsMenuController)
			return;
		
		m_CommandsMenuController.Control(GetOwner());

		SCR_CommandActionsEditorComponent editorCommandsComponent = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent, true));
		if (!editorCommandsComponent)
			return;
		
		if (m_CommandsMenuController)
		{
			SCR_RadialMenu radialMenu = m_CommandsMenuController.GetRadialMenu();
			if (radialMenu)
				radialMenu.m_eRadialType = SCR_ERadialMenuType.ADD_COMMANDS_MENU;
		}

		OnBeforeMenuOpen(m_CommandsMenuController, editorCommandsComponent, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddCommandsRadialMenu()
	{
		if (!m_AddCommandsMenuController)
			return;
		
		m_AddCommandsMenuController.Control(GetOwner());
		
		SCR_CommandActionsEditorComponent editorCommandsComponent = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent, true));
		if (!editorCommandsComponent)
			return;

		OnBeforeMenuOpen(m_AddCommandsMenuController, editorCommandsComponent, EEditorCommandActionFlags.IS_QUEUE);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivate()
	{
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
			return;
#endif
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceChanged);
		
		InputManager inputManager =	GetGame().GetInputManager();
		
		// Clearup
		inputManager.RemoveActionListener(m_sActionName, EActionTrigger.DOWN, OnActionRadialMenuOpen);
		inputManager.RemoveActionListener(m_sCommandActionName, EActionTrigger.DOWN, OnCommandsRadialMenu);
		inputManager.RemoveActionListener(m_sAddCommandActionName, EActionTrigger.DOWN, OnAddCommandsRadialMenu);
		
		// Setup
		inputManager.AddActionListener(m_sActionName, EActionTrigger.DOWN, OnActionRadialMenuOpen);
		inputManager.AddActionListener(m_sCommandActionName, EActionTrigger.DOWN, OnCommandsRadialMenu);
		inputManager.AddActionListener(m_sAddCommandActionName, EActionTrigger.DOWN, OnAddCommandsRadialMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorDelete()
	{
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
			return;
#endif
	}
	//---- REFACTOR NOTE END ----
}
//! \}
