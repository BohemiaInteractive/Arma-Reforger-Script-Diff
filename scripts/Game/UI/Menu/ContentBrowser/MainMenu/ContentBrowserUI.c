enum EWorkshopTabId
{
	MANW = 0,
	ONLINE,
	GAME_SAVES,
	OFFLINE,
	MOD_MANAGER
}

//! The super menu class for WORKSHOP content browser
class ContentBrowserUI : SCR_SuperMenuBase
{
	protected SCR_InputButtonComponent m_NavBack;
	static const float SMALL_DOWNLOAD_THRESHOLD = 50 * 1024 * 1024; // 50 Megabytes
	
	float m_fTimerLowFreqUpdate;
	
	//------------------------------------------------------------------------------------------------
	static ContentBrowserUI Create()
	{
		MenuBase menuBase = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowser);
		ContentBrowserUI browser = ContentBrowserUI.Cast(menuBase);

		return browser;
	}
	
	// --- Overrides ---
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// Setup the 'back' nav button
		m_NavBack = m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK);
		if (m_NavBack)
			m_NavBack.m_OnActivated.Insert(Close);
		
		// Hide news menu button (top right corner) on PS
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
		{
			Widget newsButton = GetRootWidget().FindAnyWidget("NewsButton");
			if (newsButton)
			{
				newsButton.SetVisible(false);
				newsButton.SetEnabled(false);
			}
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();

		SCR_AnalyticsApplication.GetInstance().OpenWorkshop();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		// Restore server hosting 
		if (!GetGame().IsPlatformGameConsole())
		{		
			if (ServerHostingUI.GetTemporaryConfig())
			{
				GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu);
				SCR_CommonDialogs.CreateServerHostingDialog();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();

		SCR_AnalyticsApplication.GetInstance().CloseWorkshop();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		m_fTimerLowFreqUpdate += tDelta;
		if (m_fTimerLowFreqUpdate > 1)
		{
			LowFreqUpdate();
			m_fTimerLowFreqUpdate -= 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update non important UI elements
	protected void LowFreqUpdate()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		if(!mgr)
			return;
		
		// Warning sign
		// Show warning icon if there is any offline mod which has any issue
		array<ref SCR_WorkshopItem> allAddons = mgr.GetAllAddons();
		array<ref SCR_WorkshopItem> offlineAddons = SCR_AddonManager.SelectItemsBasic(allAddons, EWorkshopItemQuery.OFFLINE);
		
		bool anyIssue;
		foreach (SCR_WorkshopItem item : offlineAddons)
		{
			anyIssue = 
				item.GetAnyDependencyMissing() ||
				item.GetAnyDependencyUpdateAvailable() ||
				item.GetEnabledAndAnyDependencyDisabled() ||
				item.GetUpdateAvailable();
			
			if (anyIssue)
				break;
		}
		
		m_SuperMenuComponent.GetTabView().ShowIcon(EWorkshopTabId.OFFLINE, anyIssue);		
	}
	
	// --- Public ---
	//-----------------------------------------------------------------------------------------------
	SCR_SubMenuBase GetOpenedSubMenu()
	{
		return m_SuperMenuComponent.GetOpenedSubMenu();
	}
	
	//-----------------------------------------------------------------------------------------------
	void OpenModManager()
	{
		m_SuperMenuComponent.GetTabView().ShowTab(EWorkshopTabId.MOD_MANAGER);
	}
	
	// --- Debug ---
	//------------------------------------------------------------------------------------------------
	#ifdef WORKSHOP_DEBUG
 	static void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[Content Browser] %1", str), logLevel);
	}
	#endif
}