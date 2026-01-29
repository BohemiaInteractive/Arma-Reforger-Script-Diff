class SCR_CommandActionsEditorUIComponent : SCR_BaseContextMenuEditorUIComponent
{
	[Attribute("EditorSetCommand")]
	protected string m_sSetCommandAction;
	
	[Attribute("EditorAddCommand")]
	protected string m_sAddCommandAction;
	
	protected SCR_CommandActionsEditorComponent m_CommandActionsComponent;
	protected SCR_BaseEditableEntityFilter m_Filter;
	protected EEditorCommandActionFlags m_State;
	
	protected ref ScriptInvoker Event_OnCommandStateChange = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! Get state of commanding.
	//! \return State based on selected entities
	EEditorCommandActionFlags GetCommandState()
	{
		return m_State;
	}

	//------------------------------------------------------------------------------------------------
	//! Get even tcalled every time the commanding state changes.
	//! \return Script invoker
	ScriptInvoker GetOnCommandStateChange()
	{
		return Event_OnCommandStateChange;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PerformDefaultCommand(bool isQueue)
	{
		OnOpenActionsMenuDown();
		if (!OnCancelUp() || IsContextMenuOpen())
			return;
		
		m_CommandActionsComponent.PerformDefaultAction(isQueue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorSetCommand()
	{
		PerformDefaultCommand(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorAddCommand()
	{
		PerformDefaultCommand(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorCommandModifier(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.DOWN)
		{
			UpdateState();
			m_Filter.GetOnChanged().Insert(OnSelectedChanged);
		}
		else
		{
			UpdateState(true);
			m_Filter.GetOnChanged().Remove(OnSelectedChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelectedChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		UpdateState();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateState(bool clear = false)
	{
		
		if (clear)
			m_State = 0;
		else
			m_State = m_CommandActionsComponent.ValidateSelection(false);
		
		Event_OnCommandStateChange.Invoke(m_State);
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnMenuUpdate(float tDelta)
	{
		m_InputManager.ActivateContext("EditorCommandContext");
		//m_InputManager.ActivateContext("EditorCommandActionsContext");
		//m_InputManager.SetContextDebug("EditorCommandActionsContext", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{		
		super.HandlerAttachedScripted(w);
		
		m_CommandActionsComponent = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent, true));
		m_EditorActionsComponent = m_CommandActionsComponent;
		if (!m_CommandActionsComponent)
			return;
		
		if (m_InputManager)
		{
			m_InputManager.AddActionListener(m_sSetCommandAction, EActionTrigger.DOWN, OnEditorSetCommand);
			m_InputManager.AddActionListener(m_sAddCommandAction, EActionTrigger.DOWN, OnEditorAddCommand);
			
			m_InputManager.AddActionListener("EditorCommandModifier", EActionTrigger.DOWN, OnEditorCommandModifier);
			m_InputManager.AddActionListener("EditorCommandModifier", EActionTrigger.UP, OnEditorCommandModifier);
		}
		
		m_Filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.COMMANDED, true); //--- ToDo: Don't hardcode
				
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_InputManager)
		{			
			m_InputManager.RemoveActionListener(m_sSetCommandAction, EActionTrigger.DOWN, OnEditorSetCommand);
			m_InputManager.RemoveActionListener(m_sAddCommandAction, EActionTrigger.DOWN, OnEditorAddCommand);
			
			m_InputManager.RemoveActionListener("EditorCommandModifier", EActionTrigger.DOWN, OnEditorCommandModifier);
			m_InputManager.RemoveActionListener("EditorCommandModifier", EActionTrigger.UP, OnEditorCommandModifier);
		}
		
		if (m_Filter)
			m_Filter.GetOnChanged().Remove(OnSelectedChanged);
		
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
	}
}
