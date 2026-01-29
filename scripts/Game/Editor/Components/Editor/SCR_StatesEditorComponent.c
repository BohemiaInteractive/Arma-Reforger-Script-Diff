[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_StatesEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
Manager of unique editor states
*/
class SCR_StatesEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(desc: "Default state set every time other state is finished.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorState))]
	private EEditorState m_DefaultState;
	
	private EEditorState m_State;
	private bool m_bIsWaiting;
	private ref ScriptInvoker Event_OnStateChange = new ScriptInvoker;
	private ref ScriptInvoker Event_OnIsWaitingChange = new ScriptInvoker;
	protected MenuBase m_MenuUponWaiting;
	protected bool m_bIsSafeDialog;
	
	/*!
	Check if the current state is default
	\return True if default
	*/
	bool IsDefaultState()
	{
		return m_State == m_DefaultState;
	}
	/*!
	Get current editor state.
	\return Editor state
	*/
	EEditorState GetState()
	{
		return m_State;
	}
	/*!
	Set current editor state.
	\param state Editor state
	*/
	bool SetState(EEditorState state)
	{
		if (state == m_State) return true;
		
		if (CanSet(state))
		{
			EEditorState statePrev = state;
			m_State = state;
			Event_OnStateChange.Invoke(state, statePrev);
			return true;
		}
		return false;
	}
	/*!
	If given state is the current one, set the state to default
	\param state Editor state
	*/
	void UnsetState(EEditorState state)
	{
		if (m_State == state) SetState(m_DefaultState);
	}
	/*!
	Check if given state can be set.
	\param state Editor state
	*/
	bool CanSet(EEditorState state)
	{
		if (state == m_DefaultState) return true; //--- Always allow setting default
		switch (state)
		{
			case EEditorState.NONE:
				return true;
			
			case EEditorState.SELECTING:
				return true;
			
			case EEditorState.MULTI_SELECTING:
				return (m_State != EEditorState.PLACING && m_State != EEditorState.TRANSFORMING);
			
			case EEditorState.PLACING:
				return true;
			
			case EEditorState.TRANSFORMING:
				return (m_State != EEditorState.PLACING && m_State != EEditorState.MULTI_SELECTING);
		}
		return false;
	}
	/*!
	Set editor to waiting for server.
	\param isWaiting True if waiting for server
	\return When canceling waiting, true is returned if no menu or dialog was opened while waiting.
	*/
	bool SetIsWaiting(bool isWaiting)
	{
		if (isWaiting == m_bIsWaiting)
			return true;
		
		bool result = true;
		if (isWaiting)
		{
			m_MenuUponWaiting = GetGame().GetMenuManager().GetTopMenu();
		}
		else
		{
			result = (m_bIsSafeDialog || !GetGame().GetMenuManager().IsAnyDialogOpen()) && m_MenuUponWaiting == GetGame().GetMenuManager().GetTopMenu();
			m_MenuUponWaiting = null;
		}
		
		m_bIsWaiting = isWaiting;
		Event_OnIsWaitingChange.Invoke(isWaiting);
		return result;
	}
	/*!
	Check if some feature is waiting for server.
	Used for example when user click on a button to open attributes, but there's a delay before server processes the request.
	\return True when waiting
	*/
	bool IsWaiting()
	{
		return m_bIsWaiting;
	}
	/*!
	Tell the system if the currently opened dialog is safe to be ignored in 'waiting' condition.
	\param value True if the dialog will not prevent waiting status from failing (it would notmally do when any dialog is opened).
	*/
	void SetSafeDialog(bool value)
	{
		m_bIsSafeDialog = value
	}
	
	/*!
	Get event called when editor state changes.
	\return Script invoker
	*/
	ScriptInvoker GetOnStateChange()
	{
		return Event_OnStateChange;
	}
	/*!
	Get event called when waiting state changes.
	\return Script invoker
	*/
	ScriptInvoker GetOnIsWaitingChange()
	{
		return Event_OnIsWaitingChange;
	}
	
	override void EOnEditorDebug(array<string> debugTexts)
	{
		if (!IsActive()) return;
		debugTexts.Insert(string.Format("Editor State: %1", Type().EnumToString(EEditorState, m_State)));
	}
};