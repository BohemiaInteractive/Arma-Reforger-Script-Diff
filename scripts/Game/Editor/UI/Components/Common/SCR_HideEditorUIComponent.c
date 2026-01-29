//! @ingroup Editor_UI Editor_UI_Components

class SCR_HideEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute("10", UIWidgets.Auto, "Speed of smooth transition between visible and hidden state.")]
	protected float m_fTransitionSpeed;
	
	[Attribute(desc: "Show interface again when any of these actions is triggered")]
	protected ref array<string> m_aShowActions;
	
	protected SCR_MenuEditorComponent m_EditorMenuManager;
	protected float m_fTargetOpacity = 1;
	protected bool m_bInTransition;
	protected ref ScriptInvoker Event_OnOpacityChange;
	
	//------------------------------------------------------------------------------------------------
	//! Set widget visibility.
	//! \param[in] visible True when visible
	//! \param[in] instant True to set visibility instantly, false to animate it
	void SetVisible(bool visible, bool instant = false)
	{
		Widget widget = GetWidget();
		if (!widget)
			return;
		
		widget.SetEnabled(visible);
		
		if (visible)
			m_fTargetOpacity = 1;
		else
			m_fTargetOpacity = 0;
		
		if (instant)
			SetWidgetOpacity(widget, m_fTargetOpacity);
		else
			m_bInTransition = true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return event invoker every time opacity changes.
	ScriptInvoker GetOnOpacityChange()
	{
		if (!Event_OnOpacityChange)
			Event_OnOpacityChange = new ScriptInvoker();

		return Event_OnOpacityChange;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Show()
	{
		//--- Use delay, so clicking on hidden button won't activate it instantly
		GetGame().GetCallqueue().CallLater(SetMenuVisible, 1, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetMenuVisible(bool visible)
	{
		if (m_EditorMenuManager)
			m_EditorMenuManager.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWidgetOpacity(Widget widget, float opacity)
	{
		if (widget)
			widget.SetOpacity(opacity);

		if (Event_OnOpacityChange)
			Event_OnOpacityChange.Invoke(opacity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		Widget widget = GetWidget();
		if (!widget)
			return;
		
		//--- Disabled, handled by SCR_ToggleInterfaceToolbarAction now
//		if (m_InputManager.GetActionTriggered("EditorToggleUI"))
//		{
//			if (m_EditorMenuManager)
//				m_EditorMenuManager.ToggleVisible();
//			else
//				SetVisible(!widget.IsEnabled());
//		}
		
		if (m_bInTransition)
		{
			float opacity = widget.GetOpacity();
			if (Math.AbsFloat(opacity - m_fTargetOpacity) < 0.01)
			{
				SetWidgetOpacity(widget, m_fTargetOpacity);
				m_bInTransition = false;
				return;
			}
			SetWidgetOpacity(widget, Math.Lerp(opacity, m_fTargetOpacity, m_fTransitionSpeed * tDelta));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuInit()
	{
		if (m_EditorMenuManager)
			SetVisible(m_EditorMenuManager.IsVisible(), true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		if (SCR_Global.IsEditMode())
			return; //--- Run-time only
		
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;

		OnMenuInit();
		menu.GetOnMenuInit().Insert(OnMenuInit);
		menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
		
		m_EditorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			foreach (string showAction : m_aShowActions)
			{
				inputManager.AddActionListener(showAction, EActionTrigger.DOWN, Show);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			foreach (string showAction : m_aShowActions)
			{
				inputManager.RemoveActionListener(showAction, EActionTrigger.DOWN, Show);
			}
		}
	}
}
