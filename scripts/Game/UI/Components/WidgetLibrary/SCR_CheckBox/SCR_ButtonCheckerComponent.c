//------------------------------------------------------------------------------------------------
class SCR_ButtonCheckerComponent : SCR_ButtonTextComponent 
{
	// Checker 
	const string WIDGET_CHECKER = "Checker";
	protected SCR_ModularButtonComponent m_Checker;
		
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Find checker 
		Widget checkerWidget = w.FindAnyWidget(WIDGET_CHECKER);
		if (checkerWidget)
			m_Checker = SCR_ModularButtonComponent.Cast(checkerWidget.FindHandler(SCR_ModularButtonComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetToggled(bool toggled, bool animate = true, bool invokeChange = true, bool instant = false)
	{
		super.SetToggled(toggled, animate, invokeChange, instant);	
		
		// Switch checker 
		if (m_Checker)
			m_Checker.SetToggled(toggled, true, instant);
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		
		return true;
	}
};