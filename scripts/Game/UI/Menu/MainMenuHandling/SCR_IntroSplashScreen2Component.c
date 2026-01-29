//#define DEBUG_WARNING_SCREEN

class SCR_IntroSplashScreen2Component : SCR_IntroSplashScreenComponent
{
	ref ScriptInvoker m_OnFinished = new ScriptInvoker();
	
	const float TRANSITION_TIME = 0.5;
	
	const float FADEIN_TIME_DUST = 10;
	const float FADEIN_TIME_ART = 4;
	const float FADEIN_TIME_INFO = 0.5;
	
	//---------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		WidgetManager.SetCursor(12);			// Hide cursor
		
		if (m_Widgets.m_wPressKeyMsg)
			m_Widgets.m_wPressKeyMsg.SetTextFormat("#AR-SplashScreen_Continue", "<color rgba='226, 167, 79, 255'><action name='CloseSplashScreen' index=0/></color>");
		
		// Animate: Dust
		if (m_Widgets.m_wDust1 && m_Widgets.m_wDust2)
		{
			float dust1pos[2] = {15000, 0};
			float dust2pos[2] = {35000, 0};			
			
			AnimateWidget.Opacity(m_Widgets.m_wDust1, 0.35, 1 / FADEIN_TIME_DUST);
			AnimateWidget.Opacity(m_Widgets.m_wDust2, 0.35, 1 / FADEIN_TIME_DUST);
			AnimateWidget.Position(m_Widgets.m_wDust1, dust1pos, 0.0005);
			AnimateWidget.Position(m_Widgets.m_wDust2, dust2pos, 0.0005);		
		}

		// Animate: Warning widgets
		if (m_Widgets.m_wWarning)
		{
			AnimateWidget.Opacity(m_Widgets.m_wWarning, 1, 1 / FADEIN_TIME_INFO);
		}
		
		// Create and animate-in build version layout
		Widget buildVersion = GetGame().GetWorkspace().CreateWidgets("{B7A765172F0BD4D9}UI/layouts/Common/GameVersionWatermark.layout", w);
		
		if (buildVersion)
		{
			buildVersion.SetOpacity(0);
			AnimateWidget.Opacity(buildVersion, 0.2, 1 / FADEIN_TIME_ART);
		}
		
		
		// Show experimental build desclaimer 
		bool isExperimentalBuild = GetGame().IsExperimentalBuild();
		
		#ifdef DEBUG_WARNING_SCREEN
		isExperimentalBuild = true;
		#endif

		m_Widgets.m_wExperimentalBuild.SetOpacity(isExperimentalBuild);
				
		if (isExperimentalBuild && m_Widgets.m_wArtExperimental)
		{
			//m_Widgets.m_wArtExperimental.SetOpacity(0);
			AnimateWidget.Opacity(m_Widgets.m_wArtExperimental, 1, 1 / FADEIN_TIME_ART);
		}
		
		ActivateContext();
		GetGame().GetInputManager().AddActionListener("CloseSplashScreen", EActionTrigger.DOWN, OnInput);		
	}
	
	//---------------------------------------------------------------------------------------------
	override protected void InitWidgets()
	{
		// Disable widgets
		m_Widgets.m_wSpinner.SetVisible(false);

		// Prepare widgets for fade-in
		m_Widgets.m_wDust1.SetOpacity(0);
		m_Widgets.m_wDust2.SetOpacity(0);
		m_Widgets.m_wWarning.SetOpacity(0);
		m_Widgets.m_wArtExperimental.SetOpacity(0);			
		
		// Show widgets
		m_Widgets.m_wArt.SetOpacity(1);	
		m_Widgets.m_wBILogo.SetOpacity(1);			
		m_Widgets.m_wEnfusionLogo.SetOpacity(1);	
		m_Widgets.m_wDisclaimer.SetOpacity(1);			
	}	
	
	//---------------------------------------------------------------------------------------------
	void ActivateContext()
	{
		GetGame().GetInputManager().ActivateContext("BetaWarningContext", 1);
		GetGame().GetCallqueue().CallLater(ActivateContext, 0);
	}
	
	//---------------------------------------------------------------------------------------------
	void OnInput()
	{
		WidgetManager.SetCursor(0);		// Show cursor
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.CLICK);
		m_Widgets.m_wPressKeyMsg.SetVisible(false);
		
		m_Widgets.m_wBlackOverlay.SetOpacity(0);
		m_Widgets.m_wBlackOverlay.SetVisible(true);
		AnimateWidget.Opacity(m_Widgets.m_wBlackOverlay, 1, 1 / TRANSITION_TIME);

		GetGame().GetCallqueue().CallLater(OnFadedOut, TRANSITION_TIME * 1000 * 1.1);
		GetGame().GetInputManager().RemoveActionListener("CloseSplashScreen", EActionTrigger.DOWN, OnInput);
	}
	
	//---------------------------------------------------------------------------------------------
	void OnFadedOut()
	{
		m_Widgets.m_wContent.SetVisible(false);
		m_Widgets.m_wBackground.SetVisible(false);
		
		AnimateWidget.Opacity(m_Widgets.m_wBlackOverlay, 0, 1 / TRANSITION_TIME);
		
		m_OnFinished.Invoke();
		
		GetGame().GetCallqueue().CallLater(Close, TRANSITION_TIME * 1000 * 1.1);
	}
	
	//---------------------------------------------------------------------------------------------
	void Close()
	{
		GetGame().GetCallqueue().Remove(ActivateContext);
		m_wRoot.RemoveFromHierarchy();
	}
};