//#define DEBUG_SPLASH_SCREENS

//------------------------------------------------------------------------------------------------
class SplashScreen: BaseLoadingAnim
{
	// Game intro splash screen, shown when game starts-up + used for EA extra screen
	const string INTRO_SPLASH_LAYOUT = "{1C71B463B3B66BAB}UI/layouts/Menus/MainMenu/IntroSplashScreen.layout";
	
	// Scenario pre-load screen
	const string SCENARIO_PRELOAD_LAYOUT = "{4C8B29889444BDA3}UI/layouts/Menus/LoadingScreen/ScenarioLoadingScreen.layout";
	
	static int s_iSplashShown;
	static protected bool s_bOpened;
	
	private ref SCR_IntroSplashScreenComponent m_IntroSplashScreenComponent;
	private ref SCR_PreloadScreenComponent m_PreloadScreenComponent;

	//------------------------------------------------------------------------------------------------
	override void Load()
	{
		#ifdef DEBUG_SPLASH_SCREENS
		PrintFormat(">> %1 >> Load", this);
		#endif
		
		// Create intro splash screen; used when game is starting and loading the MainMenu world
		if (BaseLoadingAnim.s_NumReloads == 0 && s_iSplashShown == 0)
		{
			m_wRoot = m_WorkspaceWidget.CreateWidgets(INTRO_SPLASH_LAYOUT, m_WorkspaceWidget);
			
			if (m_wRoot)
			{
				m_IntroSplashScreenComponent = new SCR_IntroSplashScreenComponent();
				m_wRoot.AddHandler(m_IntroSplashScreenComponent);				
			}			
		}
		// Create scenario preload screen; used when game is being restarted due to addons
		else
		{
			m_wRoot = m_WorkspaceWidget.CreateWidgets(SCENARIO_PRELOAD_LAYOUT, m_WorkspaceWidget);
			
			if (m_wRoot)
			{
				m_PreloadScreenComponent = new SCR_PreloadScreenComponent();
				m_wRoot.AddHandler(m_PreloadScreenComponent);				
			}
		}
		
		s_iSplashShown++;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsOpen()
	{
		return s_bOpened;
	}

	//------------------------------------------------------------------------------------------------
	override void Show()
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> Show", this);
		#endif
		
		s_bOpened = true;
		
		WidgetManager.SetCursor(12);		// Hide cursor
		super.Show();
	}	

	//------------------------------------------------------------------------------------------------
	override void Hide()
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> Hide", this);
		#endif		
		
		s_bOpened = false;
		
		if (m_PreloadScreenComponent)
			m_PreloadScreenComponent.OnHideInternal();		

		//WidgetManager.SetCursor(0);		// Show cursor
		super.Hide();
	}	
			
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice, float progress, float minDurationRatio)
	{
		if (!m_wRoot)		
			return;
		
		if (m_IntroSplashScreenComponent)
			m_IntroSplashScreenComponent.Update(timeSlice);
		else
			m_PreloadScreenComponent.Update(timeSlice, progress, minDurationRatio);
	}

	//------------------------------------------------------------------------------------------------
	static int GetGameReloads()
	{
		return s_NumReloads;
	}	
};