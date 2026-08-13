class SCR_WatermarkComponent: ScriptedWidgetComponent
{
	[Attribute("WatermarkPlatformText")]
	private string m_sWatermarkWidgetName;

	[Attribute("WatermarkVersionText")]
	private string m_sVersionTextWidgetName;
		
	private Widget m_wRoot;
	private Widget m_wLayoutRoot;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wLayoutRoot = m_wRoot.GetParent();
		
		m_wLayoutRoot.SetVisible(false);

		TextWidget wWatermark = TextWidget.Cast(w.FindAnyWidget(m_sWatermarkWidgetName));
		TextWidget wVersion = TextWidget.Cast(w.FindAnyWidget(m_sVersionTextWidgetName));
		
		if (!wWatermark || !wVersion)
			return;	
				
		PlatformService platformService = GetGame().GetPlatformService();
		
		if (!platformService)
			return;
		
		PlatformKind platformID = platformService.GetLocalPlatformKind();
		
		if (platformID < 0)
			return;

		// Set current build name		
		string buildName = "";
		
		if (GetGame().IsExperimentalBuild())
		{
			buildName = "#AR-Experimental_WelcomeLabel";
		}
		else if (platformID == 0)
		{
			buildName = "#AR-Watermark_Development";
		}

		wWatermark.SetText(buildName);	
		
		// Get game version & platform suffix
		string version = GetGame().GetBuildVersion();
		
		array <string> platformStrings = 
		{
			"", /* Workbench - not to be displayed */
			"#AR-UI_Xbox",
			"#AR-UI_PSN",
			"#AR-UI_Steam"
		};		
		
		if (platformID < platformStrings.Count())
		{
			// FIXME: Not the greatest idea to loosely map enum value to this local array. Slight change may throw this off
			string platformSuffix = platformStrings[platformID];
		
			if (platformSuffix != "")
			{
				// TODO: Implement me better including localization!
				string isHandheld = "";
				if (System.GetPlatform() == EPlatform.WINDOWS_HANDHELD)
					isHandheld = " #AR-Settings_Title_Handheld";
				version = string.Format("%1 (%2)%3", version, platformSuffix, isHandheld);
			}
		}
		
		// Update the game version widget (number + platform suffix)
		wVersion.SetText(version);
		m_wLayoutRoot.SetVisible(true);
	}
};
