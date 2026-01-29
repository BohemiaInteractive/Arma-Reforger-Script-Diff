//#define DEBUG_WARNING_SCREEN

//------------------------------------------------------------------------------------------------
//! Main menu entity
[EntityEditorProps(category: "GameScripted/Menu", description:"When put into a level will make sure that the main menu is open.")]
class SCR_MainMenuEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MainMenuEntity : GenericEntity
{
	[Attribute("ChimeraMenuPreset.MainMenu", UIWidgets.SearchComboBox, "Menu to launch at start of the world", "", ParamEnumArray.FromEnum(ChimeraMenuPreset) )]
	ChimeraMenuPreset m_eMenu;
	
	[Attribute("{1C71B463B3B66BAB}UI/layouts/Menus/MainMenu/IntroSplashScreen.layout")]
	private ResourceName m_sSplashScreenLayout;
	
	protected static SCR_MainMenuEntity m_Instamce;
	
	//------------------------------------------------------------------------------------------------
	void SCR_MainMenuEntity(IEntitySource src, IEntity parent)
	{
		m_Instamce = this;
		
		// Enable
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MainMenuEntity()
	{
		if (m_eMenu == ChimeraMenuPreset.MainMenu)
			GetGame().m_bIsMainMenuOpen = false;
	}

	// Wait until first update - all systems, like MenuManager should be initialized
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		bool isDevVersion = Game.IsDev();
		bool showWarningScreen = SplashScreen.GetGameReloads() == 0 && SplashScreen.s_iSplashShown == 1;
		
		#ifdef DEBUG_WARNING_SCREEN
		isDevVersion = false;
		#endif
				
		if (showWarningScreen && !isDevVersion)
			ShowWarningScreen();
		else if (!SplashScreen.IsOpen())
			ShowMainMenu();
		
		// Reset scenario loading time
		SCR_BaseLoadingScreenComponent.ResetLoadingTime();
		
		// Disable all events
		ClearEventMask(EntityEvent.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	private void ShowWarningScreen()
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sSplashScreenLayout, GetGame().GetWorkspace());
		
		SCR_IntroSplashScreen2Component comp = new SCR_IntroSplashScreen2Component();
		w.AddHandler(comp);
		
		if (!comp)
			return;
	
		// If splash screen was skipped (due to CLI for example) make sure, we have it flagged and do not run ShowWarningScreen again
		if (SplashScreen.s_iSplashShown == 0)
			SplashScreen.s_iSplashShown++;
		
		comp.m_OnFinished.Insert(ShowMainMenu);		
	}
	
	//------------------------------------------------------------------------------------------------
	private void ShowMainMenu()
	{
		if (m_eMenu == ChimeraMenuPreset.MainMenu)
			GetGame().m_bIsMainMenuOpen = true;
		
		GetGame().GetMenuManager().OpenMenu(m_eMenu);
			
		// Check hosting fail 
		SCR_KickDialogs.CheckLastServerHost();
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_MainMenuEntity GetInstance()
	{
		return m_Instamce;
	}
};
