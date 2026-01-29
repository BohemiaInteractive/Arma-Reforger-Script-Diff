
class SCR_SettingsSuperMenu : SCR_SuperMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		SCR_InputButtonComponent comp = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK,GetRootWidget());
		if (comp)
			comp.m_OnActivated.Insert(OnBack);
		
		bool showBlur = !GetGame().m_bIsMainMenuOpen;

		Widget img = GetRootWidget().FindAnyWidget("MenuBackground");
		if (img)
			img.SetVisible(!showBlur);

		BlurWidget blur = BlurWidget.Cast(GetRootWidget().FindAnyWidget("Blur0"));
		if (blur)
		{
			blur.SetVisible(showBlur);
			if (showBlur)
			{
				float w, h;
				GetGame().GetWorkspace().GetScreenSize(w, h);
				blur.SetSmoothBorder(0, 0, w * 0.8, 0);
			}
		}
		
		bool isDebug = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, false);

		if (!isDebug && System.GetPlatform() == EPlatform.WINDOWS)
		{
			m_SuperMenuComponent.GetTabView().RemoveTabByIdentifier("SettingsVideoConsole");
			m_SuperMenuComponent.GetTabView().ShowTabByIdentifier(identifier: "SettingsVideoPC", playSound: false);
		}

// Remove video settings for consoles, if not in settings debug mode
#ifdef PLATFORM_CONSOLE
		if (isDebug)
			return;
		
		m_SuperMenuComponent.GetTabView().RemoveTabByIdentifier("SettingsVideoPC");
		m_SuperMenuComponent.GetTabView().ShowTabByIdentifier(identifier: "SettingsVideoConsole", playSound: false);	
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
				
		SCR_AnalyticsApplication.GetInstance().OpenSettings();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
	
		SCR_AnalyticsApplication.GetInstance().CloseSettings();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		Close();
	}
};
