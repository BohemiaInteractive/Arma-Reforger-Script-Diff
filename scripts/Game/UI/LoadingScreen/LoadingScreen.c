//#define DEBUG_LOADING_SCREENS

class ArmaReforgerLoadingAnim: BaseLoadingAnim
{
	const string LOADING_SCREEN_LAYOUT = "{4C8B29889444BDA3}UI/layouts/Menus/LoadingScreen/ScenarioLoadingScreen.layout";
	
	static protected bool s_bOpened;
	static ref ScriptInvoker s_OnEnterLoadingScreen = new ScriptInvoker();
	static ref ScriptInvoker m_onExitLoadingScreen = new ScriptInvoker();

	private ref SCR_LoadingScreenComponent m_LayoutComponent;	
	
	//------------------------------------------------------------------------------------------------
	void ArmaReforgerLoadingAnim(WorkspaceWidget workspaceWidget)
	{
		GetGame().m_OnMissionSetInvoker.Insert(OnMissionSet);
	}

	//------------------------------------------------------------------------------------------------
	override void Load()
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> Load", this);
		#endif
		
		m_wRoot = m_WorkspaceWidget.CreateWidgets(LOADING_SCREEN_LAYOUT, m_WorkspaceWidget);
			
		m_LayoutComponent = new SCR_LoadingScreenComponent();
		m_wRoot.AddHandler(m_LayoutComponent);
		
		SplashScreen.s_iSplashShown++;	
	}	

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice, float progress, float minDurationRatio)
	{
		#ifdef DEBUG_SPLASH_SCREENS
		//PrintFormat(">> %1 >> Update | timeSlice: %2 | progress: %3 | minDurationRatio: %4", this, timeSlice, progress, minDurationRatio);
		#endif
		
		if (!m_wRoot)		
			return;
		
		m_LayoutComponent.Update(timeSlice, progress, minDurationRatio);
	}	
			
	//------------------------------------------------------------------------------------------------
	override void Show()
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> Show", this);
		#endif
		
		super.Show();
		
		s_bOpened = true;
		
		EnableSounds(false);		
		
		WidgetManager.SetCursor(12);		// Hide cursor

		s_OnEnterLoadingScreen.Invoke();		
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Hide()
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> Hide", this);
		#endif
		
		s_bOpened = false;
		
		EnableSounds(true);
		
		m_onExitLoadingScreen.Invoke();

		if (m_LayoutComponent)
			m_LayoutComponent.OnHideInternal();		
		
		WidgetManager.SetCursor(0);		// Show cursor
				
		super.Hide();
	}	

	//------------------------------------------------------------------------------------------------
	void EnableSounds(bool enable)
	{
		AudioSystem.SetMasterVolume(AudioSystem.SFX, enable);
		AudioSystem.SetMasterVolume(AudioSystem.VoiceChat, enable);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, enable);
		AudioSystem.SetMasterVolume(AudioSystem.Music, enable);
		AudioSystem.SetMasterVolume(AudioSystem.UI, enable);
	}	
			
	//------------------------------------------------------------------------------------------------
	static bool IsOpen()
	{
		return s_bOpened;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMissionSet(SCR_MissionHeader header)
	{
		if (m_LayoutComponent)
			m_LayoutComponent.OnMissionDataRetrieved(header);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetJoiningCrossPlay(bool isCrossPlay)
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> SetJoiningCrossPlay | isCrossPlay: %1", isCrossPlay);
		#endif		
		
		if (m_LayoutComponent)
			m_LayoutComponent.SetJoiningCrossPlay(isCrossPlay);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLoadingModded(bool isModded)
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> SetLoadingModded | isModded: %1", isModded);
		#endif		
		
		if (m_LayoutComponent)
			m_LayoutComponent.SetLoadingModded(isModded);
	}			
}