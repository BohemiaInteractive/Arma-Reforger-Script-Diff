class SCR_IntroSplashScreenComponent : ScriptedWidgetComponent
{
	protected Widget m_wRoot;

	protected ref SCR_IntroSplashScreenWidgets m_Widgets;	
	protected SCR_LoadingSpinner m_SpinnerComp;
	
	protected static const float FADEIN_TIME = 2;
	
	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_Widgets = new SCR_IntroSplashScreenWidgets();
		m_Widgets.Init(m_wRoot);		
		
		m_wRoot.SetZOrder(1001);
		
		// Get spinner component
		if (m_Widgets.m_wSpinner)
		{
			Widget compWidget = m_Widgets.m_wSpinner.FindAnyWidget("Spinner");
			
			if (compWidget)
				m_SpinnerComp = SCR_LoadingSpinner.Cast(compWidget.FindHandler(SCR_LoadingSpinner));		
		}
		
		InitWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		// Disable widgets
		m_Widgets.m_wDust1.SetVisible(false);
		m_Widgets.m_wDust2.SetVisible(false);
		m_Widgets.m_wWarning.SetVisible(false);
		m_Widgets.m_wExperimentalBuild.SetVisible(false);
		m_Widgets.m_wArtExperimental.SetVisible(false);

		// Prepare widgets for fade-in
		m_Widgets.m_wBILogo.SetOpacity(0);
		m_Widgets.m_wEnfusionLogo.SetOpacity(0);
		m_Widgets.m_wDisclaimer.SetOpacity(0);
		m_Widgets.m_wSpinner.SetOpacity(0);
		
		// Show widgets
		m_Widgets.m_wArt.SetOpacity(1);	
	}	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		Fade(m_Widgets.m_wBILogo, true, FADEIN_TIME, timeSlice);
		Fade(m_Widgets.m_wEnfusionLogo, true, FADEIN_TIME, timeSlice);
		Fade(m_Widgets.m_wDisclaimer, true, FADEIN_TIME, timeSlice);
		Fade(m_Widgets.m_wSpinner, true, FADEIN_TIME, timeSlice);
		
		// Rotate spinner
		if (m_SpinnerComp)
			m_SpinnerComp.Update(timeSlice);
	}	
		
	//------------------------------------------------------------------------------------------------
	protected void Fade(Widget w, bool show, float length, float timeslice)
	{
		if (length <= 0)
		{
			w.SetOpacity(show);
			return;		
		}
		
		float opacityCurrent = w.GetOpacity();
		
		if (opacityCurrent == show)
			return;
		
		float progressDelta = timeslice / length;
		
		if (!show)
			progressDelta = -progressDelta;
		
		float progress = opacityCurrent + progressDelta;
		
		progress = Math.Clamp(progress, 0, 1);
		
		float opacity = Math.Lerp(0, 1, progress);
		w.SetOpacity(opacity);
		w.SetVisible(true);
	}
}
