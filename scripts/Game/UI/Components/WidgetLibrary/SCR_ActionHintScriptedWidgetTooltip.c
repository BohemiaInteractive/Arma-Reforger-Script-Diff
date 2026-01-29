[BaseContainerProps(configRoot : true)]
class SCR_ActionHintScriptedWidgetTooltip : SCR_ScriptedWidgetTooltipContentBase
{
	[Attribute("", desc: "action to display")]
	protected string m_sAction;

	[Attribute(UIColors.GetColorAttribute(UIColors.IDLE_ACTIVE))]
	ref Color m_ActionColor;
	
	protected SCR_InputButtonComponent m_ActionDisplay;
	
	protected const string WIDGET_ACTION = "Action";

	//------------------------------------------------------------------------------------------------
	override bool Init(WorkspaceWidget workspace, Widget wrapper)
	{
		if (!super.Init(workspace, wrapper))
			return false;
		
		Widget action = m_wContentRoot.FindAnyWidget(WIDGET_ACTION);
		if (action)
			m_ActionDisplay = SCR_InputButtonComponent.FindComponent(action);

		if (!m_ActionDisplay)
			return false;

		ResetAction();
		ResetActionColor();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetAction(string action)
	{
		if (!m_ActionDisplay || action.IsEmpty())
			return false;

		m_ActionDisplay.SetAction(action);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SetActionColor(Color color)
	{
		if (!m_ActionDisplay)
			return false;

		m_ActionDisplay.SetColorActionDisabled(color);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool ResetAction()
	{
		return SetAction(GetDefaultAction());
	}

	//------------------------------------------------------------------------------------------------
	bool ResetActionColor()
	{
		return SetActionColor(GetDefaultActionColor());
	}

	//------------------------------------------------------------------------------------------------
	string GetDefaultAction()
	{
		return m_sAction;
	}

	//------------------------------------------------------------------------------------------------
	Color GetDefaultActionColor()
	{
		return Color.FromInt(m_ActionColor.PackToInt());
	}
}