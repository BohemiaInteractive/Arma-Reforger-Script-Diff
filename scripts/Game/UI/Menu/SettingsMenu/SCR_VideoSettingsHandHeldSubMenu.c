class SCR_VideoSettingsHandheldSubMenu : SCR_SettingsSubMenuBase
{
	protected static const int COMBO_OPTION_HANDHELD 	= 0;
    protected static const int COMBO_OPTION_CUSTOM 		= 1;
	
	SCR_SettingsManagerVideoModule m_SettingsVideoModule;
	
	protected ref array<int> m_aWidths = {};
	protected ref array<int> m_aHeights = {};
	protected ref array<int> m_aAAvalues = { 0, 2, 4, 8, 16, 32 };

	protected SCR_ComboBoxComponent m_ResolutionCombo;
	protected SCR_SliderComponent m_ResolutionScale;
	// protected SCR_ComboBoxComponent m_AspectRatio;
	protected SCR_SpinBoxComponent m_WindowMode;
	protected SCR_SpinBoxComponent m_MaxFPS;

	protected string m_sResolutionScaleLocalisation = "#AR-Settings_RenderScale";

	protected UserSettings m_Video;
	protected UserSettings m_Pipeline;
	protected UserSettings m_ResourceManager;

	protected static const string RESOLUTION_FORMAT = "%1 × %2";
	
	//! list of WIDGET names
	protected static const ref array<string> PRESET_SETTINGS = {
//			the following settings are out of presets; they were not forgotten:
//			"WindowMode"
//			"Resolution"
//			"ResolutionScale"
//			"AspectRatio"
//			"VSync"
//			"MaxFPS"
//			"DOF"		// scripted
//			"NearDOF"	// scripted
		"Detail",
		"DistantShadows",
		"EnvironmentQuality",
		"FoliageSmoothing",
		"GeometricDetail",
		"GrassDistance",
		"GrassLOD",
		"HardwareAA",
		"HBAO",					// ambient occlusion
		"ObjectDrawDistance",
		"PostprocessAA",
		"RenderTargetFormat",
		"ShadowQuality",
		"SSDO",					// contact shadow
		"SSR",					// post-process quality
		// "TerrainDetail",		//disabled setting for now
		"TerrainSurfaceDetail",
		"TextureDetail",
		"TextureFiltering",
	};
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuItemChanged(SCR_SettingsBindingBase binding)
	{
		// ignore this if we are loading settings
		if (m_bLoadingSettings)
			return;

		if (binding)
			SetQualityPresetIndex(binding.GetWidgetName());

		super.OnMenuItemChanged(binding);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCustomMenuItemChanged(string widgetName)
	{
		OnMenuItemChanged(new SCR_SettingsBindingBase("", "", widgetName));
		//SCR_AnalyticsApplication.GetInstance().ChangeSetting("Video", widgetName);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetQualityPresetIndex(string changedSetting)
	{
		if (!PRESET_SETTINGS.Contains(changedSetting))
			return;

		SCR_ComboBoxComponent qualityPreset = SCR_ComboBoxComponent.GetComboBoxComponent("QualityPreset", m_wRoot);
		if (qualityPreset)
			qualityPreset.SetCurrentItem(COMBO_OPTION_CUSTOM);

		BaseContainer videoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
	    if (videoSettings)
	        videoSettings.Set("m_iLastUsedPreset", -1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_ResourceManager = GetGame().GetEngineUserSettings().GetModule("ResourceManagerUserSettings");
		m_Pipeline = GetGame().GetEngineUserSettings().GetModule("PipelineUserSettings");
		m_Video = GetGame().GetEngineUserSettings().GetModule("VideoUserSettings");

		m_ResolutionScale = SCR_SliderComponent.GetSliderComponent("ResolutionScale", m_wRoot);
		m_ResolutionCombo = SCR_ComboBoxComponent.GetComboBoxComponent("Resolution", m_wRoot);
		// m_AspectRatio = SCR_ComboBoxComponent.GetComboBoxComponent("AspectRatio", m_wRoot);
		m_WindowMode = SCR_SpinBoxComponent.GetSpinBoxComponent("WindowMode", m_wRoot);
		m_MaxFPS = SCR_SpinBoxComponent.GetSpinBoxComponent("MaxFPS", m_wRoot);
		
		SCR_SettingsManager settingsManager = GetGame().GetSettingsManager();
        if (!settingsManager)
            return;
		
		m_SettingsVideoModule = SCR_SettingsManagerVideoModule.Cast(settingsManager.GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_VIDEO));

		SCR_ComboBoxComponent aspectRatio = SCR_ComboBoxComponent.GetComboBoxComponent("AspectRatio", m_wRoot);
		if (aspectRatio)
			aspectRatio.SetVisible(false);

		// Create bindings and load all standard entries
		m_aSettingsBindings.Clear();
	
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("ResourceManagerUserSettings", "GeometricDetail", "GeometricDetail"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("TerrainGenMaterialSettings", "GeometryLevel", "TerrainDetail"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("VideoUserSettings", "Vsync", "VSync"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("GrassMaterialSettings", "Lod", "GrassLOD"));
		// We are using game settings to be able to override distance of grass
		//m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("GrassMaterialSettings", "Distance", "GrassDistance"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("TerrainGenMaterialSettings", "Detail", "TerrainSurfaceDetail"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("MaterialSystemUserSettings", "TextureFilter", "TextureFiltering"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("GraphicsQualitySettings", "ObjectDrawDistanceScale", "ObjectDrawDistance"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("VideoUserSettings", "Atoc", "FoliageSmoothing"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("VideoUserSettings", "ResolutionScale", "ResolutionScale"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("VideoUserSettings", "FsrEnabled", "FSR"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("VideoUserSettings", "RenderFormat", "RenderTargetFormat"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("VideoUserSettings", "DistantShadowsQuality", "DistantShadows"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("VideoUserSettings", "EnvironmentQuality", "EnvironmentQuality"));
		//m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("DisplayUserSettings", "OverallQuality", "QualityPreset"));

		// Post-process settings
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("DisplayUserSettings", "PPAA", "PostprocessAA", "PPQuality"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("DisplayUserSettings", "HBAO", "HBAO", "PPQuality"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("DisplayUserSettings", "SSDO", "SSDO", "PPQuality"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("DisplayUserSettings", "SSR", "SSR", "PPQuality"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_VideoSettings", "m_iDofType", "DOF"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_VideoSettings", "m_bNearDofEffect", "NearDOF"));

		// Other settings
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_b2DScopes", "ScopeMode"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("UserInterfaceSettings", "UseSoftwareCursor", "CursorMode"));

		LoadSettings();

		SetupQualityPreset();
		SetupViewDistance();
		SetupGrassDistance();
		SetupResolution();
		SetupResolutionScale();
		SetupShadowQuality();
		SetupHardwareAA();
		SetupTextureDetail();
		SetupMaxFPS();
		
		HandleDescription();

		RemoveQualityPresetFocus();

		// TODO: bind OnRenderResolutionChanged when Alt+Enter event (non-existing atm) is triggered
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupTextureDetail(bool isInit = true)
	{
		if (!m_ResourceManager)
			return;

		SCR_SpinBoxComponent textureDetail = SCR_SpinBoxComponent.GetSpinBoxComponent("TextureDetail", m_wRoot);
		if (!textureDetail)
			return;

		const float item; // TODO: check for good const usage
		float value, min, max, step;
		m_ResourceManager.Get("TextureDetail", value);
		m_ResourceManager.GetLimits(item, min, max, step);

		int current = (max - value);
		textureDetail.SetCurrentItem(current);

		if (isInit)
			textureDetail.m_OnChanged.Insert(OnTextureDetailChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when max FPS is changed
	protected void OnMaxFPSChanged(SCR_SpinBoxComponent comp, int i)
	{
		if (!comp || !m_MaxFPS || !m_Video)
			return;

		bool isEnabled = m_MaxFPS.GetCurrentIndex();

		int value = 0;
		if (isEnabled)
			value = m_MaxFPS.GetCurrentItem().ToInt();

		m_Video.Set("MaxFps", value);

		OnCustomMenuItemChanged("MaxFPS");
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when texture detail spinbox changes
	protected void OnTextureDetailChanged(SCR_SpinBoxComponent comp, int i)
	{
		if (!m_ResourceManager)
			return;

		const float item; // TODO: check for good const usage
		float min, max, step;
		m_ResourceManager.GetLimits(item, min, max, step);

		int current = max - i;
		m_ResourceManager.Set("TextureDetail", current);

		OnCustomMenuItemChanged("TextureDetail");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnQualityPresetChanged(SCR_ComboBoxComponent combobox, int itemIndex)
	{
		if (m_bLoadingSettings || !m_SettingsVideoModule)
			return;

		if (combobox.GetCurrentIndex() == COMBO_OPTION_CUSTOM)
			return;
		
		if (combobox.GetCurrentIndex() == COMBO_OPTION_HANDHELD)
			m_SettingsVideoModule.SetConsolePreset(EVideoQualityPreset.HANDHELD_PRESET);
		
		
		GetGame().ApplySettingsPreset();
		GetGame().UserSettingsChanged();
		LoadSettings(true, false);
		
		// all scripted widgets have to be manually done here
		m_bLoadingSettings = true;

		SetupHardwareAA(false);
		SetupTextureDetail(false);
		SetupShadowQuality(false);

		m_bLoadingSettings = false;
		
		//SCR_AnalyticsApplication.GetInstance().SetQualityPresetSetting(itemIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupQualityPreset()
	{
		SCR_ComboBoxComponent qualityPreset = SCR_ComboBoxComponent.GetComboBoxComponent("QualityPreset", m_wRoot);
		if (!qualityPreset)
			return;
		
		int lastPreset;
        BaseContainer videoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
        if (!videoSettings)
            return;

        videoSettings.Get("m_iLastUsedPreset", lastPreset);

        if (lastPreset == EVideoQualityPreset.HANDHELD_PRESET)
            qualityPreset.SetCurrentItem(COMBO_OPTION_HANDHELD);
        else
            qualityPreset.SetCurrentItem(COMBO_OPTION_CUSTOM);

        qualityPreset.m_OnChanged.Insert(OnQualityPresetChanged);		
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupHardwareAA(bool isInit = true)
	{
		if (!m_Video)
			return;

		SCR_SpinBoxComponent antiAliasing = SCR_SpinBoxComponent.GetSpinBoxComponent("HardwareAA", m_wRoot);
		if (!antiAliasing)
			return;

//#ifdef PLATFORM_CONSOLE
		// TODO: ifdefs are ugly and messy, make proper solution using menu configs and UI prefabs
		//antiAliasing.GetRootWidget().SetVisible(false);
		//return;
//#endif

		int value;
		m_Video.Get("Fsaa", value);

		int i = m_aAAvalues.Find(value);
		if (i > -1)
			antiAliasing.SetCurrentItem(i);

		if (isInit)
			antiAliasing.m_OnChanged.Insert(OnHardwareAAChanged);

		SCR_SpinBoxComponent foliageSmoothing = SCR_SpinBoxComponent.GetSpinBoxComponent("FoliageSmoothing", m_wRoot);
		if (foliageSmoothing && i == 0)
			foliageSmoothing.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when hardware AA spinbox changes
	protected void OnHardwareAAChanged(SCR_SpinBoxComponent comp, int i)
	{
		if (!m_Video || i < 0 || i >= m_aAAvalues.Count())
			return;

		m_Video.Set("Fsaa", m_aAAvalues[i]);

		SCR_SpinBoxComponent foliageSmoothing = SCR_SpinBoxComponent.GetSpinBoxComponent("FoliageSmoothing", m_wRoot);
		if (foliageSmoothing)
			foliageSmoothing.SetEnabled(i > 0);

		OnCustomMenuItemChanged("HardwareAA");
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when resolution scale slider changes
	protected void OnResolutionScaleChanged(SCR_SliderComponent comp, float value)
	{
		value = Math.Round(value * 100);

		comp.ShowCustomValue(value.ToString());

		OnRenderResolutionChanged();

		OnCustomMenuItemChanged("ResolutionScale");
	}

	//------------------------------------------------------------------------------------------------
	//! method called when resolution or resolution scale changes
	protected void OnRenderResolutionChanged()
	{
		if (!m_ResolutionScale || !m_sResolutionScaleLocalisation)
			return;

		TextWidget labelWidget = m_ResolutionScale.GetLabel();
		if (!labelWidget)
			return;

		float uiResWidth, uiResHeight;
		int renderResWidth, renderResHeight;
		GetGame().GetWorkspace().GetScreenSize(uiResWidth, uiResHeight);
		float scale = m_ResolutionScale.GetValue();
		renderResWidth = Math.Round(uiResWidth * scale);
		renderResHeight = Math.Round(uiResHeight * scale);

		labelWidget.SetTextFormat(m_sResolutionScaleLocalisation, renderResWidth, renderResHeight);
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when shadow quality spinbox changes
	protected void OnShadowDetailChanged(SCR_SpinBoxComponent comp, int i)
	{
		if (!m_Pipeline)
			return;

		m_Pipeline.Set("ShadowQuality", i + 1);
		OnCustomMenuItemChanged("ShadowQuality");
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when shadow quality slider changes
	protected void OnViewDistanceChanged(SCR_SliderComponent comp, float value)
	{
		GetGame().SetViewDistance(value);
		// Get desired type of DOF
		BaseContainer m_VideoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (m_VideoSettings)
		{
			m_VideoSettings.Set("m_iViewDistance", value);
		}
		OnCustomMenuItemChanged("DrawDistance");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGrassDistanceChanged(SCR_SliderComponent comp, float value)
	{
		GetGame().SetGrassDistance(value);
		OnCustomMenuItemChanged("GrassDistance");
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when window mode combo box changes
	protected void OnWindowModeChanged(SCR_SpinBoxComponent comp, int index)
	{
		if (!m_Video || !m_ResolutionCombo || !m_ResolutionScale)
			return;

		bool borderlessFullscreen = (index == 0);

		m_ResolutionCombo.GetRootWidget().SetVisible(!borderlessFullscreen);
		m_ResolutionScale.GetRootWidget().SetVisible(borderlessFullscreen);

		int nativeWidth, nativeHeight;
		float scale;

		if (borderlessFullscreen)
		{
			// Set to borderless fulscreen
			m_Video.Set("WindowMode", EWindowMode.BORDERLESS);

			SCR_ComboBoxComponent combo = SCR_ComboBoxComponent.GetComboBoxComponent("Resolution", m_wRoot);
			if (!combo)
				return;

			System.GetNativeResolution(nativeWidth, nativeHeight);
			int i = combo.GetCurrentIndex();
			scale = m_aWidths[i] / nativeWidth;
			m_Video.Set("ResolutionScale", scale);

			m_ResolutionScale.SetValue(scale);
		}
		else
		{
			// Set to windowed mode
			m_Video.Set("WindowMode", EWindowMode.WINDOWED);

			m_Video.Get("ResolutionScale", scale);
			System.GetNativeResolution(nativeWidth, nativeHeight);

			int width = nativeWidth * scale;
			int bestDiff = int.MAX;
			int bestIdx = 0;

			for (int i, cnt = m_aWidths.Count(); i < cnt; i++)
			{
				int diff = Math.AbsInt(m_aWidths[i] - width);

				if (diff < bestDiff)
				{
					bestDiff = diff;
					bestIdx = i;

					if (diff == 0)
						break;
				}
			}

			m_Video.Set("ScreenWidth", m_aWidths[bestIdx]);
			m_Video.Set("ScreenHeight", m_aHeights[bestIdx]);
			m_Video.Set("ResolutionScale", 1.0);

			SetResolutionCombo(m_ResolutionCombo);
		}

		OnCustomMenuItemChanged("WindowMode");
	}

	//------------------------------------------------------------------------------------------------
	protected void SetResolutionCombo(SCR_ComboBoxComponent combo)
	{
		if (!m_Video)
			return;

		int scale, nativeWidth, nativeHeight, width, height;
		m_Video.Get("ResolutionScale", scale);
		m_Video.Get("ScreenWidth", width);
		m_Video.Get("ScreenHeight", height);
		System.GetNativeResolution(nativeWidth, nativeHeight);

		width = width * scale;
		height = height * scale;
		int bestDiff = int.MAX;
		int bestIdx = 0;

		for (int i, cnt = m_aWidths.Count(); i < cnt; i++)
		{
			int diff = Math.AbsInt(m_aWidths[i] - width);

			if (diff < bestDiff)
			{
				bestDiff = diff;
				bestIdx = i;
				if (diff == 0)
					break;
			}
		}

		int bestDiffH = int.MAX;
		int bestIdxH = 0;

		for (int i, cnt = m_aHeights.Count(); i < cnt; i++)
		{
			int diff = Math.AbsInt(m_aHeights[i] - height);

			if (diff < bestDiffH)
			{
				bestDiffH = diff;
				bestIdxH = i;
				if (diff == 0)
					break;
			}
		}

		string resolutionStr = string.Format(RESOLUTION_FORMAT, m_aWidths[bestIdx], m_aHeights[bestIdxH]);

		foreach (int i, string elementName : combo.m_aElementNames)
		{
			if (elementName != resolutionStr)
				continue;

			combo.SetCurrentItem(i);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupViewDistance()
	{
		float viewDistance;
		SCR_SliderComponent distance = SCR_SliderComponent.GetSliderComponent("DrawDistance", m_wRoot);
		if (!distance)
			return;

		float min = GetGame().GetMinimumViewDistance();
		float max = GetGame().GetMaximumViewDistance();
		
		// Get desired type of DOF
		BaseContainer m_VideoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (m_VideoSettings)
		{
			m_VideoSettings.Get("m_iViewDistance", viewDistance);
			GetGame().SetViewDistance(viewDistance);
		}
		
		float current = GetGame().GetViewDistance();
		distance.SetMin(min);
		distance.SetMax(max);
		distance.SetStep(50);

		distance.SetValue(current);
		distance.GetOnChangedFinal().Insert(OnViewDistanceChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupGrassDistance()
	{
		SCR_SliderComponent distance = SCR_SliderComponent.GetSliderComponent("GrassDistance", m_wRoot);
		if (!distance)
			return;

		int min = GetGame().GetMinimumGrassDistance();
		int max = GetGame().GetMaximumGrassDistance();
		int current = GetGame().GetGrassDistance();
		distance.SetMin(min);
		distance.SetMax(max);
		distance.SetStep(1);

		distance.SetValue(current);
		distance.GetOnChangedFinal().Insert(OnGrassDistanceChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupShadowQuality(bool isInit = true)
	{
		if (!m_Pipeline)
			return;
		// Shadow detail quality is little special, because we want to skip the lowest setting
		SCR_SpinBoxComponent shadowDetail = SCR_SpinBoxComponent.GetSpinBoxComponent("ShadowQuality", m_wRoot);
		if (!shadowDetail)
			return;

		int shadowQuality;
		m_Pipeline.Get("ShadowQuality", shadowQuality);
		shadowDetail.SetCurrentItem(Math.ClampInt(shadowQuality - 1, 0, shadowDetail.GetNumItems() - 1));

		if (isInit)
			shadowDetail.m_OnChanged.Insert(OnShadowDetailChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupResolution()
	{
		if (!m_ResolutionCombo || !m_ResolutionScale || !m_WindowMode || !m_Video)
			return;

		int mode;
		m_Video.Get("WindowMode", mode);

		bool fullScreen, showWindowMode;
		
		// Hide resolutions as we have EPlatform.WINDOWS_HANDHELD			
		if (System.GetPlatform() == EPlatform.WINDOWS_HANDHELD)
		{
			fullScreen = true;
			showWindowMode = false;			
		}	
		else
		{
			// dead code if EPlatform.WINDOWS hides this submenu
			fullScreen = (mode != EWindowMode.WINDOWED);
			showWindowMode = true;
		}

#ifdef WORKBENCH
		fullScreen = true;
		showWindowMode = false;
#endif

		if (showWindowMode)
		{
			m_WindowMode.SetCurrentItem(!fullScreen);
			m_WindowMode.m_OnChanged.Insert(OnWindowModeChanged);
		}

		m_WindowMode.GetRootWidget().SetVisible(showWindowMode);

		if (m_ResolutionScale)
			m_ResolutionScale.GetRootWidget().SetVisible(fullScreen);
		if (m_ResolutionCombo)
			m_ResolutionCombo.GetRootWidget().SetVisible(!fullScreen);

		System.GetSupportedResolutions(m_aWidths, m_aHeights);
		for (int i = 0, cnt = m_aWidths.Count(); i < cnt; i++)
		{
			m_ResolutionCombo.AddItem(string.Format(RESOLUTION_FORMAT, m_aWidths[i], m_aHeights[i]));
		}

		SetResolutionCombo(m_ResolutionCombo);
		m_ResolutionCombo.m_OnChanged.Insert(UpdateResolution);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupResolutionScale()
	{
		if (!m_ResolutionScale)
			return;

		float scale = m_ResolutionScale.GetValue() * 100;
		scale = Math.Round(scale);
		m_ResolutionScale.ShowCustomValue(scale.ToString());

		m_ResolutionScale.m_OnChanged.Insert(OnResolutionScaleChanged);
		OnRenderResolutionChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes Quality Presets focus - as it is a powerful setting (and is the first one)
	//! and sets it to the first button found under SettingsEntries
	protected  void RemoveQualityPresetFocus()
	{
		ButtonWidget qualityPreset = ButtonWidget.Cast(m_wRoot.FindAnyWidget("QualityPreset"));
		if (!qualityPreset)
			return;

		VerticalLayoutWidget settingsEntries = VerticalLayoutWidget.Cast(m_wScroll.FindAnyWidget("SettingsEntries"));
		if (!settingsEntries)
			return;

		Widget child = settingsEntries.GetChildren();
		while (child)
		{
			if (ButtonWidget.Cast(child))
			{
				GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, child, false);
				return;
			}
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when resolution combo box changes
	protected void UpdateResolution(SCR_ComboBoxComponent resolution, int current)
	{
		if (!resolution || !m_Video)
			return;

		m_Video.Set("ScreenWidth", m_aWidths[current]);
		m_Video.Set("ScreenHeight", m_aHeights[current]);

		OnRenderResolutionChanged();

		OnCustomMenuItemChanged("Resolution");
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupMaxFPS()
	{
		if (!m_Video || !m_MaxFPS)
			return;

		int maxFPS;
		m_Video.Get("MaxFps", maxFPS);

		if (maxFPS < 1)
		{
			m_MaxFPS.SetCurrentItem(0);
		}
		else
		{
			for (int i = 1, cnt = m_MaxFPS.GetNumItems(); i < cnt; i++)
			{
				if (m_MaxFPS.GetItemName(i).ToInt() == maxFPS)
				{
					m_MaxFPS.SetCurrentItem(i);
					break;
				}
			}
		}

		m_MaxFPS.m_OnChanged.Insert(OnMaxFPSChanged);
	}
	
	protected void HandleDescription()
	{
		ButtonWidget drawDistance = ButtonWidget.Cast(m_wRoot.FindAnyWidget("DrawDistance"));
		if (!drawDistance)
			return;
		
		SCR_ModularButtonComponent buttonComp = SCR_ModularButtonComponent.Cast(drawDistance.FindHandler(SCR_ModularButtonComponent));
		if (!buttonComp)
			return;
		
		buttonComp.m_OnFocus.Insert(DescriptionOnFocus);
		buttonComp.m_OnFocusLost.Insert(DescriptionOnFocusLost);
	}
	
	protected void DescriptionOnFocus()
	{
		RichTextWidget title = RichTextWidget.Cast(m_wRoot.FindAnyWidget("DescriptionHeader"));
		RichTextWidget body = RichTextWidget.Cast(m_wRoot.FindAnyWidget("DescriptionBody"));
		
		if (!title || !body)
			return;
		
		title.SetText("#AR-Settings_DrawDistance");
		body.SetText("#AR-Settings_Description_DrawDistance");
	}
	
	protected void DescriptionOnFocusLost()
	{
		RichTextWidget title = RichTextWidget.Cast(m_wRoot.FindAnyWidget("DescriptionHeader"));
		RichTextWidget body = RichTextWidget.Cast(m_wRoot.FindAnyWidget("DescriptionBody"));
		
		if (!title || !body)
			return;
		
		title.SetText(string.Empty);
		body.SetText(string.Empty);
	}
}