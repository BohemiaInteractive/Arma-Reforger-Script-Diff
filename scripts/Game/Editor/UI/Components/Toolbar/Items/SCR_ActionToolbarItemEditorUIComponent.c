class SCR_ActionToolbarItemEditorUIComponent : SCR_BaseToolbarItemEditorUIComponent
{
	[Attribute()]
	protected string m_sIconWidgetName;
	
	[Attribute()]
	protected string m_sToggleWidgetName;
	
	protected SCR_BaseToggleToolbarAction m_ToggleAction;
	protected ImageWidget m_IconWidget;
	protected ImageWidget m_ToggleWidget;
	protected SCR_LinkTooltipTargetEditorUIComponent m_TooltipTarget;
	
	//------------------------------------------------------------------------------------------------
	protected void OnToggleChange(int value, bool highlight = false)
	{
		m_IconWidget.SetVisible(!highlight);
		m_ToggleWidget.SetVisible(highlight);
		
		if (m_TooltipTarget)
		{
			if (highlight)
				m_TooltipTarget.SetInfo(m_ToggleAction.GetInfoToggled(), m_ToggleAction);
			else
				m_TooltipTarget.SetInfo(m_ToggleAction.GetInfo(), m_ToggleAction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] action
	//! \param[in] widget
	void SetAction(SCR_BaseEditorAction action, Widget widget)
	{
		SCR_UIInfo info = action.GetInfo();
		if (!info)
			return;
		
		//--- Assign tooltip
		m_TooltipTarget = SCR_LinkTooltipTargetEditorUIComponent.Cast(widget.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
		if (m_TooltipTarget)
			m_TooltipTarget.SetInfo(info, action);

		//---Assign icon (or exit if it's not available)
		m_IconWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sIconWidgetName));
		if (!m_IconWidget)
			return;
		
		info.SetIconTo(m_IconWidget);
		
		//--- Recolor the icon according to action's UI info
		SCR_ColorUIInfo colorInfo = SCR_ColorUIInfo.Cast(info);
		if (colorInfo)
			m_IconWidget.SetColor(colorInfo.GetColor());

		//--- Initialize toggling if available
		m_ToggleWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sToggleWidgetName));
		if (m_ToggleWidget)
		{
			m_ToggleWidget.SetVisible(false);
			
			m_ToggleAction = SCR_BaseToggleToolbarAction.Cast(action);
			if (m_ToggleAction)
			{
				if (m_ToggleAction.GetInfoToggled())
					m_ToggleAction.GetInfoToggled().SetIconTo(m_ToggleWidget);
				else
					info.SetIconTo(m_ToggleWidget);

				m_ToggleAction.GetOnToggleChange().Insert(OnToggleChange);
				m_ToggleAction.Track();
				OnToggleChange(m_ToggleAction.GetCurrentValue(), m_ToggleAction.GetCurrentHighlight());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_ToggleAction)
		{
			m_ToggleAction.GetOnToggleChange().Remove(OnToggleChange);
			m_ToggleAction.Untrack();
		}
	}
}
