class SCR_EditorStateConditionUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute(desc: "Show the widget only when this editor state is active.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorState))]
	protected EEditorState m_State;
	
	[Attribute(desc: "When enabled, given editor state must *not* be active.")]
	protected bool m_bInverted;
	
	//------------------------------------------------------------------------------------------------
	protected void OnStateChanged(EEditorState state, EEditorState statePrev)
	{
		bool show = state == m_State;
		if (m_bInverted)
			show = !show;
		
		GetWidget().SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsUnique()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		SCR_StatesEditorComponent stateManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent, true));
		if (!stateManager)
			return;
		
		stateManager.GetOnStateChange().Insert(OnStateChanged);
		OnStateChanged(stateManager.GetState(), -1);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_StatesEditorComponent stateManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		if (!stateManager)
			return;
		
		stateManager.GetOnStateChange().Remove(OnStateChanged);
	}
}
