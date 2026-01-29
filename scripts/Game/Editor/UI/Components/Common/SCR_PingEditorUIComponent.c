//! @ingroup Editor_UI Editor_UI_Components

class SCR_PingEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected SCR_PingEditorComponent m_PingManager;
	protected SCR_HoverEditableEntityFilter m_HoverFilter;
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorPing(float value, EActionTrigger reason)
	{
		SendPing(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorPingUnlimitedOnly(float value, EActionTrigger reason)
	{
		SendPing(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendPing(bool unlimitedOnly)
	{
		vector position;
		SCR_CursorEditorUIComponent cursorComponent = SCR_CursorEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_CursorEditorUIComponent, true));
		if (cursorComponent)
		{
			if (!cursorComponent.GetCursorWorldPos(position))
				return;
		}
		
		SCR_EditableEntityComponent target;
		if (m_HoverFilter)
			target = m_HoverFilter.GetEntityUnderCursor();

		m_PingManager.SendPing(unlimitedOnly, position, target);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_PingManager = SCR_PingEditorComponent.Cast(SCR_PingEditorComponent.GetInstance(SCR_PingEditorComponent, true));
		if (!m_PingManager)
			return;
		
		m_HoverFilter = SCR_HoverEditableEntityFilter.Cast(SCR_HoverEditableEntityFilter.GetInstance(EEditableEntityState.HOVER));
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("EditorPing", EActionTrigger.DOWN, OnEditorPing);
			inputManager.AddActionListener("EditorPingUnlimitedOnly", EActionTrigger.DOWN, OnEditorPingUnlimitedOnly);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("EditorPing", EActionTrigger.DOWN, OnEditorPing);
			inputManager.RemoveActionListener("EditorPingUnlimitedOnly", EActionTrigger.DOWN, OnEditorPingUnlimitedOnly);
		}
	}
}
