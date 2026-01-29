class SCR_SettingsManagerVideoModule : SCR_SettingsManagerModuleBase
{
	protected const int COMBO_OPTION_QUALITY = 0;
	protected const int COMBO_OPTION_PERFORMANCE = 1;
	
	//------------------------------------------------------------------------------------------------
	void SetConsolePreset(int presetIndex)
	{
		// TODO: marian: split into methods and use switch
		ArmaReforgerScripted game = GetGame();
		UserSettings engineUserSettings = game.GetEngineUserSettings();
		BaseContainer displayUserSettings = engineUserSettings.GetModule("DisplayUserSettings");
		UserSettings videoUserSettings = engineUserSettings.GetModule("VideoUserSettings");
		BaseContainer videoSettings = game.GetGameUserSettings().GetModule("SCR_VideoSettings");

		if (!displayUserSettings || !videoUserSettings || !videoSettings)
			return;

		if (presetIndex == EVideoQualityPreset.SERIES_S_PRESET_QUALITY)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
			videoUserSettings.Set("ResolutionScale", 0.7);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 30);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
		}		
		else if (presetIndex == EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
			videoUserSettings.Set("ResolutionScale", 0.60);
			videoUserSettings.Set("Fsaa", 0);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 60);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
		}
		else if (presetIndex == EVideoQualityPreset.SERIES_X_PRESET_QUALITY)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
			videoUserSettings.Set("ResolutionScale", 0.75);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE );
			videoUserSettings.Set("MaxFps", 30);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
		}
		else if (presetIndex == EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
			videoUserSettings.Set("ResolutionScale", 0.50);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 60);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
		}
		else if (presetIndex == EVideoQualityPreset.PS5_QUALITY)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.PS5_QUALITY);
			SetConsoleResolution(EVideoQualityPreset.PS5_QUALITY);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 30);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.PS5_QUALITY);
		}
		else if (presetIndex == EVideoQualityPreset.PS5_PERFORMANCE)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.PS5_PERFORMANCE);
			SetConsoleResolution(EVideoQualityPreset.PS5_PERFORMANCE);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 60);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.PS5_PERFORMANCE);
		}
		else if (presetIndex == EVideoQualityPreset.PS5_PRO_QUALITY)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.PS5_PRO_QUALITY);
			SetConsoleResolution(EVideoQualityPreset.PS5_PRO_QUALITY);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 30);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.PS5_PRO_QUALITY);
		}
		else if (presetIndex == EVideoQualityPreset.PS5_PRO_PERFORMANCE)
		{
			displayUserSettings.Set("OverallQuality", EVideoQualityPreset.PS5_PRO_PERFORMANCE);
			SetConsoleResolution(EVideoQualityPreset.PS5_PRO_PERFORMANCE);
			videoUserSettings.Set("Fsaa", 2);
			videoSettings.Set("m_bNearDofEffect", false);
			videoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			videoUserSettings.Set("MaxFps", 60);
			videoUserSettings.Set("Vsync", true);
			videoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.PS5_PRO_PERFORMANCE);
		}
		
		GetGame().ApplySettingsPreset();
		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SettingsManagerVideoModule()
	{
		SetModuleType(ESettingManagerModuleType.SETTINGS_MANAGER_VIDEO);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetConsoleResolution(EVideoQualityPreset preset)
	{
		ArmaReforgerScripted game = GetGame();
		UserSettings engineUserSettings = game.GetEngineUserSettings();
		if (!engineUserSettings)
			return;
		
		UserSettings videoUserSettings = engineUserSettings.GetModule("VideoUserSettings");
		if (!videoUserSettings)
			return;
		
		float resolutionScale;
		int width, height;
		
		System.GetNativeResolution(width, height);
		SCR_EResolutionName resolution = SCR_SystemHelper.GetResolutionName(width, height);
		
		if (resolution == SCR_EResolutionName.E1080p || (resolution == SCR_EResolutionName.E1440p && preset != EVideoQualityPreset.PS5_PERFORMANCE))
		{
			videoUserSettings.Set("ResolutionScale", 1);
			return;
		} 
		else if (resolution == SCR_EResolutionName.E1440p && preset == EVideoQualityPreset.PS5_PERFORMANCE)
		{
			videoUserSettings.Set("ResolutionScale", 0.75);
			return;
		}
		
		switch (preset)
		{
			case EVideoQualityPreset.PS5_PERFORMANCE:
				resolutionScale = 0.5;
				break;
			case EVideoQualityPreset.PS5_QUALITY:
				resolutionScale = 0.75;
				break;
			case EVideoQualityPreset.PS5_PRO_PERFORMANCE:
				resolutionScale = 0.6666;
				break;
			case EVideoQualityPreset.PS5_PRO_QUALITY:
				resolutionScale = 0.9;
				break;
		}
		
		videoUserSettings.Set("ResolutionScale",resolutionScale);		
	}
}

enum EVideoQualityPreset
{
	SERIES_X_PRESET_QUALITY = 4,
	SERIES_S_PRESET_QUALITY = 5,
	SERIES_X_PRESET_PERFORMANCE = 6,
	SERIES_S_PRESET_PERFORMANCE = 7,
	PS5_QUALITY = 8,
	PS5_PERFORMANCE = 9,
	PS5_PRO_QUALITY = 10,
	PS5_PRO_PERFORMANCE = 11
}
