class SCR_ContextMenuActionsEditorUIComponent : SCR_BaseContextMenuEditorUIComponent
{
	[Attribute(SCR_SoundEvent.FOCUS, UIWidgets.EditBox)]
	protected string m_sSoundOnOpen;
	
	[Attribute(SCR_SoundEvent.SOUND_E_TRAN_CANCEL, UIWidgets.EditBox)]
	protected string m_sSoundOnCancelClose;
	
	//~ Makes sure context action is never opened if not selection state
	protected bool m_bEditorIsSelectingState = true;
	
	protected SCR_EditableEntityComponent m_HoveredEntityReference;
	
	//------------------------------------------------------------------------------------------------
	protected void OnHoveredEntityCheck(float tDelta)
	{
		if (!m_HoveredEntityReference)
		{
			CloseContextMenu();
			GetMenu().GetOnMenuUpdate().Remove(OnHoveredEntityCheck);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PopulateContextMenu(vector cursorWorldPosition)
	{
		super.PopulateContextMenu(cursorWorldPosition);
		m_HoveredEntityReference = m_EditorActionsComponent.GetHoveredEntity();
		if (m_HoveredEntityReference)
		{
			GetMenu().GetOnMenuUpdate().Insert(OnHoveredEntityCheck);
			
			//--- Close the menu when UI of hovered entity changed, as it implies major change in its state (e.g., character died)
			if (m_HoveredEntityReference.GetOnUIRefresh())
				m_HoveredEntityReference.GetOnUIRefresh().Insert(CloseContextMenu);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnEditorModeChanged()
	{
		m_EditorActionsComponent = SCR_ContextActionsEditorComponent.Cast(SCR_ContextActionsEditorComponent.GetInstance(SCR_ContextActionsEditorComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OpenContextMenu()
	{
		super.OpenContextMenu();
		
		if (m_ContextMenu && m_ContextMenu.IsVisible())
			SCR_UISoundEntity.SoundEvent(m_sSoundOnOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	override void CloseContextMenu()
	{
		if (m_ContextMenu && m_ContextMenu.IsVisible())
			GetGame().GetCallqueue().CallLater(DelayedCloseSound, 100);
		
		if (m_HoveredEntityReference)
		{
			GetMenu().GetOnMenuUpdate().Remove(OnHoveredEntityCheck);
			m_HoveredEntityReference = null;
		}
			
		super.CloseContextMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedCloseSound()
	{
		if (!IsContextMenuOpen())
			SCR_UISoundEntity.SoundEvent(m_sSoundOnCancelClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnOpenActionsMenuUp()
	{
		//~ Check if can open
		if (!OnCancelUp() || IsContextMenuOpen() || !m_bEditorIsSelectingState) 
		{
			//~ Reset the state
			m_bEditorIsSelectingState = true;
			return;
		}
		
		//~ Open
		OpenContextMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnOpenActionsMenuDown()
	{		
		super.OnOpenActionsMenuDown();
		
		//~ Is not selecting state so make sure on up context menu is not opened
		SCR_StatesEditorComponent statesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		if (statesManager && statesManager.GetState() != EEditorState.SELECTING) 
			m_bEditorIsSelectingState = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		if (m_InputManager)
		{
			m_InputManager.AddActionListener("EditorActionsMenu", EActionTrigger.DOWN, OnOpenActionsMenuDown);
			m_InputManager.AddActionListener("EditorActionsMenu", EActionTrigger.UP, OnOpenActionsMenuUp);
		}
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU_SHOWDISABLED, string.Empty, "Show disabled actions", "Actions Menu");
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_InputManager)
		{
			m_InputManager.RemoveActionListener("EditorActionsMenu", EActionTrigger.DOWN, OnOpenActionsMenuDown);
			m_InputManager.RemoveActionListener("EditorActionsMenu", EActionTrigger.UP, OnOpenActionsMenuUp);
		}

		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU_SHOWDISABLED);
	}
}
