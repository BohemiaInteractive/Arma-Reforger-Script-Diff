class SCR_PreloadScreenComponent : SCR_BaseLoadingScreenComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wRoot.SetZOrder(10001);
	}

	//------------------------------------------------------------------------------------------------
	override protected void InitWidgets()
	{
		super.InitWidgets();
			
		m_Widgets.m_wContent.SetVisible(false);
		m_Widgets.m_wContentOverlay.SetVisible(false);
	}	
}
