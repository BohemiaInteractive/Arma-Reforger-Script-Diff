enum EMapLightMode
{
	NONE,
	LIGHTER,
	FLASH_DEFAULT,
	FLASH_RED
}

[BaseContainerProps(configRoot: true)]
class SCR_MapLightConfig
{	
	[Attribute("", UIWidgets.Object, "2D map light modes")]
	ref array<ref SCR_MapLightMode> m_aMapLightModes;
}

[BaseContainerProps()]
class SCR_MapLightMode
{	
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMapLightMode), desc: "Select light mode to configure")]
	EMapLightMode m_eMode;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Light overlay color")]
	ref Color m_vColor; 
	
	[Attribute("1", UIWidgets.Slider, desc: "Light overlay opacity", "0 1 0.05")]
	float m_fOverlayOpacity;
	
	[Attribute("1", UIWidgets.Slider, desc: "Light cone opacity", "0 1 0.05")]
	float m_fLightConeOpacity;
	
	[Attribute("{FFCDEB9793720A1C}UI/Textures/Sights/binocular_vignette2_ca.edds", UIWidgets.ResourceNamePicker, desc: "Zones config", params: "edds")]
	ResourceName m_sConeTexture;
}

//! Map light effects
class SCR_MapLightUI : SCR_MapUIBaseComponent
{
	protected bool m_bActive = false;
	protected bool m_bIsDark = false;
	protected float m_fSunriseTime;
	protected float m_fSunsetTime;
	protected EMapLightMode m_eLightMode = EMapLightMode.NONE;
	
	protected Widget m_wLightOverlay;
	protected Widget m_wLightCone;
	protected Widget m_wFillTop;
	protected Widget m_wFillLeft;
	protected Widget m_wConeImg;
	
	protected TimeAndWeatherManagerEntity m_TimeMgr;
	protected SCR_MapCursorModule m_CursorModule;
	
	// TEMP 
	protected ref SCR_MapLightConfig m_LightCfg;
		
	//------------------------------------------------------------------------------------------------
	//! Toggle map light 
	void ToggleActive()
	{
		if (!m_bActive)
			ActivateLight();
		else 
			DeactivateLight();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate map light 
	protected void ActivateLight()
	{
		UpdateTime();
		
		// Not dark enough
		if (!m_bIsDark)
			return;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(SCR_PlayerController.GetLocalControlledEntity());
		if (!gadgetManager)
			return;
		
		IEntity flashlight = gadgetManager.GetGadgetByType(EGadgetType.FLASHLIGHT);
		if (flashlight)
		{
			m_bActive = true;
			m_wLightCone.SetVisible(true);
			m_eLightMode = EMapLightMode.FLASH_DEFAULT;
			// SetMode based on flashlight lense
		}
		//else 
		//m_eLightMode = EMapLightMode.LIGHTER;
				
		UpdateLightMode();
	}
	 
	//------------------------------------------------------------------------------------------------
	//! Deactivate map light 
	protected void DeactivateLight()
	{
		m_bActive = false;
		m_wLightCone.SetVisible(false);
		m_eLightMode = EMapLightMode.NONE;
		UpdateLightMode();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use config to configure light when switching modes
	protected void UpdateLightMode()
	{
		if (m_LightCfg)
		{
			foreach ( SCR_MapLightMode mode : m_LightCfg.m_aMapLightModes )
			{
				if (mode.m_eMode == m_eLightMode)
				{
					m_wLightOverlay.SetColor(mode.m_vColor);
					m_wLightOverlay.SetOpacity(mode.m_fOverlayOpacity);
					
					// Update light cone texture
					if (m_eLightMode != EMapLightMode.NONE)
					{
						ImageWidget.Cast(m_wConeImg).LoadImageTexture(0, mode.m_sConeTexture);
						m_wLightCone.SetOpacity(mode.m_fLightConeOpacity);
					}

					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update light modes based on time of day
	protected void UpdateTime()
	{
		if (!m_wLightOverlay || !m_wLightCone)
			return;
		
		if (!m_TimeMgr)
			return;
		
		float time = m_TimeMgr.GetTimeOfTheDay();
				
		// day/night
		m_bIsDark = time < m_fSunriseTime || time > m_fSunsetTime;
		m_wLightOverlay.SetVisible(m_bIsDark);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update light position
	protected void UpdatePosition()
	{
		float sizeX, sizeY, cursorX, cursorY;
					
		SCR_MapCursorInfo cursorInfo = m_CursorModule.GetCursorInfo();
		cursorX = cursorInfo.x;
		cursorY = cursorInfo.y;

		m_wConeImg.GetScreenSize(sizeX, sizeY);
				
		// calculate and set new size of top & left fill imageWidgets
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		cursorX = cursorX - workspace.DPIUnscale(sizeX)/2;
		cursorY = cursorY - workspace.DPIUnscale(sizeY)/2;

		ImageWidget.Cast(m_wFillLeft).SetSize(cursorX, 0);
		ImageWidget.Cast(m_wFillTop).SetSize(0, cursorY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialisation
	//! \param[in] widget is the root widget
	protected void Init(Widget widget)
	{
		m_wLightOverlay = widget.FindAnyWidget("LightOverlay");
		m_wLightCone = widget.FindAnyWidget("LightCone");
		m_wFillTop = widget.FindAnyWidget("FillTop");
		m_wFillLeft = widget.FindAnyWidget("FillLeft");
		m_wConeImg = widget.FindAnyWidget("Cone");
		
		if (m_wLightCone)
			m_wLightCone.SetVisible(false);
		
		if (m_wLightOverlay)
			m_wLightOverlay.SetVisible(false);

		ChimeraWorld world = GetGame().GetWorld();
		m_TimeMgr = world.GetTimeAndWeatherManager();
		if (m_TimeMgr)
		{
			m_TimeMgr.GetSunriseHour(m_fSunriseTime);
			m_TimeMgr.GetSunsetHour(m_fSunsetTime);
		}
		
		// TODO hardcoded
		if (!m_LightCfg)
		{
			Resource container = BaseContainerTools.LoadContainer("{D442DD4867C97496}Configs/Map/MapLightModes.conf");
			if (container)					
				m_LightCfg = SCR_MapLightConfig.Cast( BaseContainerTools.CreateInstanceFromContainer( container.GetResource().ToBaseContainer() ) );
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		Init(SCR_MapEntity.GetMapInstance().GetMapConfig().RootWidgetRef);
		m_CursorModule = SCR_MapCursorModule.Cast(SCR_MapEntity.GetMapInstance().GetMapModule(SCR_MapCursorModule));
		
		ActivateLight();
		
		UpdateLightMode();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		DeactivateLight();
		
		m_wLightOverlay.SetVisible(false);
		m_bIsDark = false;
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (!m_TimeMgr)
			return;
		
		UpdateTime();
		
		if (m_eLightMode != EMapLightMode.NONE && m_wLightCone)
			UpdatePosition();
	}	
}
