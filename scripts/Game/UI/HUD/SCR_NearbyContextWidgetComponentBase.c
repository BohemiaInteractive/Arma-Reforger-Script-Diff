class SCR_NearbyContextWidgetComponentBase : ScriptedWidgetComponent
{
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	protected ref Color m_VisibleWidgetColor;

	[Attribute("0.1 0.1 0.1 1", UIWidgets.ColorPicker)]
	protected ref Color m_NotVisibleWidgetColor;

	[Attribute()]
	protected string m_sBackgroundWidgetName;

	[Attribute()]
	protected string m_sGlowWidgetName;

	protected Widget m_wRoot;

	protected ImageWidget m_wBackground;
	protected ImageWidget m_wGlow;

	protected UserActionContext m_AssignedContext;
	protected BaseUserAction m_FirstAction;

	// Invokes when a new Context is assigned to this Widget
	// Can be used to detect if this Widget is not assigned to the needed context anymore
	protected ref ScriptInvokerVoid m_OnContextAssigned;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wBackground = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundWidgetName));
		m_wGlow = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sGlowWidgetName));
	}

	//------------------------------------------------------------------------------------------------
	//! Will be called everytime the Widget gets assigned to a new context
	//! This happens everytime the context is in line of sight again
	//! \param[in] UIInfo containing all the information for the widget Icon, etc. Can be null
	//! \param[in] UserActionContext Context the Widget gets assigned to
	void OnAssigned(SCR_ActionContextUIInfo info, UserActionContext context)
	{
		m_AssignedContext = context;
		
		array<BaseUserAction> allActions = {};
		context.GetActionsList(allActions);
		
		if (!allActions.IsEmpty())
			m_FirstAction = allActions[0];
		
		if (m_OnContextAssigned)
			m_OnContextAssigned.Invoke();

		if (info)
			info.OnWidgetAssigned(m_wRoot);
		
#ifdef NEARBY_INTERACTIONS_CONTEXT_DEBUG
		RichTextWidget debugTextWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget("DebugText"));
		if (debugTextWidget)
		{
			debugTextWidget.SetText("  Context: " + context.GetContextName());
			debugTextWidget.SetVisible(true);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Change the color of the Background Widget
	//! \param[in] Color The color, the backgroung should have
	void ChangeBackgroundColor(Color newColor)
	{
		if (m_wBackground)
			m_wBackground.SetColor(newColor);
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the color of the root to make the whole widget darker / lighter to indicate if it's on line of sight or not
	//! \param[in] bool Is in line of sight
	void ChangeVisibility(bool isInLineOfSight)
	{
		if (isInLineOfSight)
			m_wRoot.SetColor(m_VisibleWidgetColor);
		else
			m_wRoot.SetColor(m_NotVisibleWidgetColor);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the context that currently uses this Widget
	UserActionContext GetAssignedContext()
	{
		return m_AssignedContext;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnContextAssigned()
	{
		if (!m_OnContextAssigned)
			m_OnContextAssigned = new ScriptInvokerVoid();

		return m_OnContextAssigned;
	}
}
