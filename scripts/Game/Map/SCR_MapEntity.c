class SCR_MapEntityClass: MapEntityClass
{
};

//------------------------------------------------------------------------------------------------
// invoker typedefs
void MapConfigurationInvoker(MapConfiguration config);
typedef func MapConfigurationInvoker;

void MapItemInvoker(MapItem mapItem);
typedef func MapItemInvoker;

void ScriptInvokerFloat2Bool(float f1, float f2, bool b1);
typedef func ScriptInvokerFloat2Bool;


//------------------------------------------------------------------------------------------------
//! Map entity
[EntityEditorProps(category: "GameScripted/Map", description: "Map entity, handles displaying of map etc", sizeMin: "-5 -5 -5", sizeMax: "5 5 5", color: "255 255 200 0", dynamicBox: true)]
class SCR_MapEntity: MapEntity
{				
	const int FRAME_DELAY = 1;	
	protected int m_iDelayCounter = FRAME_DELAY;			// used to delay the map logic by set amount of frames to give the map widget time to properly init
	
	// generic
	protected bool m_bIsOpen;								// is open flag
	protected bool m_bDoReload;								// mark whether map config changed in order to reload modules/components
	protected bool m_bDoUpdate;								// mark whether user setting changed, update zoom & position
	protected bool m_bIsDebugMode;							// variable debug mode
	protected int m_iMapSizeX;								// map size X in meters/units
	protected int m_iMapSizeY;								// map size Y in meters/units 
	protected int m_iMapOffsetX;							// map offset X in meters/units
	protected int m_iMapOffsetY;							// map offset Y in meters/units 
	protected vector m_vVisibleFrameMin;					// cache visible frame min point for use elsewhere
	protected vector m_vVisibleFrameMax;					// cache visible frame max point for use elsewhere
	protected EMapEntityMode m_eLastMapMode;				// cached mode of last map for reload check
	protected Widget m_wMapRoot;							// map menu root widget
	protected CanvasWidget m_MapWidget;						// map widget
	protected WorkspaceWidget m_Workspace;
	protected ref MapConfiguration m_ActiveMapCfg; 			// map config
	protected static SCR_MapEntity s_MapInstance;			// map entity instance
	
	protected MapItem m_HoveredMapItem;						// currently hovered map item
		
	// zoom
	protected bool m_bIsZoomInterp;		// is currently zoom animating
	protected float m_fZoomPPU = 1;		// current zoom PixelPerUnit value
	protected float m_fStartPPU;		// zoom start PixelPerUnit
	protected float m_fTargetPPU = 1;	// zoom target PixelPerUnit
	protected float m_fZoomTimeModif;	// zoom anim speed modifier
	protected float m_fZoomSlice;		// zoom anim timeslce
	protected float m_fMinZoom = 1;		// minimal zoom PixelPerUnit
	protected float m_fMaxZoom;			// maximal zoom PixelPerUnit
	
	// pan
	protected bool m_bIsPanInterp;		// is currently pan animating
	protected int m_iPanX = 0;			// current horizontal pan offset - UNSCALED value in px
	protected int m_iPanY = 0;			// current vertical pan offset - UNSCALED value in px
	protected int m_aStartPan[2];		// pan start coords - UNSCALED value in px
	protected int m_aTargetPan[2];		// pan target coords - UNSCALED value in px
	protected float m_fPanTimeModif;	// pan anim speed modifier
	protected float m_fPanSlice;		// pan anim timeslce
	
	// modules & components
	protected ref array<ref SCR_MapModuleBase> m_aActiveModules = {};
	protected ref array<ref SCR_MapModuleBase> m_aLoadedModules = {};
	protected ref array<ref SCR_MapUIBaseComponent> m_aActiveComponents = {};
	protected ref array<ref SCR_MapUIBaseComponent> m_aLoadedComponents = {};
	
	// invokers
	protected static ref ScriptInvokerBase<MapConfigurationInvoker> s_OnMapInit = new ScriptInvokerBase<MapConfigurationInvoker>();	// map init, called straight after opening the map
	protected static ref ScriptInvokerBase<MapConfigurationInvoker> s_OnMapOpen = new ScriptInvokerBase<MapConfigurationInvoker>();	// map open, called after map is properly initialized
	protected static ref ScriptInvokerBase<MapConfigurationInvoker> s_OnMapClose = new ScriptInvokerBase<MapConfigurationInvoker>();// map close
	protected static ref ScriptInvokerBase<ScriptInvokerFloat2Bool> s_OnMapPan 	= new ScriptInvokerBase<ScriptInvokerFloat2Bool>;	// map pan, passes UNSCALED x & y
	protected static ref ScriptInvokerFloat2 s_OnMapPanEnd 						= new ScriptInvokerFloat2();						// map pan interpolated end
	protected static ref ScriptInvokerFloat s_OnMapZoom							= new ScriptInvokerFloat();							// map zoom
	protected static ref ScriptInvokerFloat s_OnMapZoomEnd 						= new ScriptInvokerFloat();							// map zoom interpolated end
	protected static ref ScriptInvokerVector s_OnSelection 						= new ScriptInvokerVector();						// any click/selection on map
	protected static ref ScriptInvokerInt s_OnLayerChanged 						= new ScriptInvokerInt();							// map layer changed
	protected static ref ScriptInvokerBase<MapItemInvoker> s_OnSelectionChanged = new ScriptInvokerBase<MapItemInvoker>();			// map items de/selected
	protected static ref ScriptInvokerBase<MapItemInvoker> s_OnHoverItem 		= new ScriptInvokerBase<MapItemInvoker>();			// map item hovered
	protected static ref ScriptInvokerBase<MapItemInvoker> s_OnHoverEnd 		= new ScriptInvokerBase<MapItemInvoker>();			// map item hover end
		
	//------------------------------------------------------------------------------------------------
	// GETTERS / SETTERS
	//------------------------------------------------------------------------------------------------
	//! Get on map init invoker, caution: called during the first frame of opening the map when widget related stuff is not initialized yet
	static ScriptInvokerBase<MapConfigurationInvoker> GetOnMapInit() { return s_OnMapInit; }
	//! Get on map open invoker
	static ScriptInvokerBase<MapConfigurationInvoker> GetOnMapOpen() { return s_OnMapOpen; }
	//! Get on map close invoker
	static ScriptInvokerBase<MapConfigurationInvoker> GetOnMapClose() { return s_OnMapClose; }
	//! Get on map pan invoker
	static ScriptInvokerBase<ScriptInvokerFloat2Bool> GetOnMapPan() { return s_OnMapPan; }
	//! Get on map pan interpolated end invoker
	static ScriptInvokerFloat2 GetOnMapPanEnd() { return s_OnMapPanEnd; }
	//! Get on map zoom invoker
	static ScriptInvokerFloat GetOnMapZoom() { return s_OnMapZoom; }
	//! Get on map zoom interpolated end invoker
	static ScriptInvokerFloat GetOnMapZoomEnd() { return s_OnMapZoomEnd; }
	//! Get on selection changed invoker
	static ScriptInvokerBase<MapItemInvoker> GetOnSelectionChanged() { return s_OnSelectionChanged; }
	//! Get on selection invoker
	static ScriptInvokerVector GetOnSelection() { return s_OnSelection; }
	//! Get on hover item invoker
	static ScriptInvokerBase<MapItemInvoker> GetOnHoverItem() { return s_OnHoverItem; }
	//! Get on hover end invoker
	static ScriptInvokerBase<MapItemInvoker> GetOnHoverEnd() { return s_OnHoverEnd; }
	//! Get on layer changed invoker
	static ScriptInvokerInt GetOnLayerChanged() { return s_OnLayerChanged; }
	//! Get map entity instance
	static SCR_MapEntity GetMapInstance() { return s_MapInstance; }
	
	//------------------------------------------------------------------------------------------------
	//! Get map config
	MapConfiguration GetMapConfig() 
	{ 
		return m_ActiveMapCfg; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the map is opened
	bool IsOpen() 
	{ 
		return m_bIsOpen; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get map sizeX in meters
	int GetMapSizeX() 
	{ 
		return m_iMapSizeX; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get map sizeY in meters
	int GetMapSizeY() 
	{ 
		return m_iMapSizeY; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get maximal zoom
	float GetMaxZoom() 
	{ 
		return m_fMaxZoom; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get minimal zoom
	float GetMinZoom() 
	{ 
		return m_fMinZoom; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get target zoom in the form of PixelPerUnit value, which is different from current zoom if interpolation is ongoing
	float GetTargetZoomPPU() 
	{
		return m_fTargetPPU; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get whether zoom interpolation is ongoing
	bool IsZooming() 
	{
		return m_bIsZoomInterp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get current DPIScaled pan offsets 
	vector GetCurrentPan() 
	{
		return { m_Workspace.DPIScale(m_iPanX), m_Workspace.DPIScale(m_iPanY), 0 }; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get map widget
	CanvasWidget GetMapWidget() 
	{
		return m_MapWidget; 
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get map menu root widget
	Widget GetMapMenuRoot() 
	{ 
		return m_wMapRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set map widget
	void SetMapWidget(Widget mapW) 
	{ 
		m_MapWidget = CanvasWidget.Cast(mapW);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get hovered item
	MapItem GetHoveredItem() 
	{ 
		return m_HoveredMapItem; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return visible frame in the form of min and max point, used to ignore update of f.e. icons when they are not visible
	void GetMapVisibleFrame(out vector min, out vector max)
	{
		min = m_vVisibleFrameMin;
		max = m_vVisibleFrameMax;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get how much pixels per unit(meter) are currently visible on screen. If this value is 1 and resolution is 1920x1080, then 1920 units(meters) of map will be visible
	//! \return Current pixel per unit value
	float GetCurrentZoom()
	{		
		return m_fZoomPPU;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get map cursor world position
	//! \param worldX is x coord
	//! \param worldY is y coord
	void GetMapCursorWorldPosition(out float worldX, out float worldY)
	{
		ScreenToWorld(m_Workspace.DPIScale(SCR_MapCursorInfo.x), m_Workspace.DPIScale(SCR_MapCursorInfo.y), worldX, worldY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get map center of screen world position
	//! \param worldX is x coord
	//! \param worldY is y coord
	void GetMapCenterWorldPosition(out float worldX, out float worldY)
	{
		float screenX, screenY;
		m_MapWidget.GetScreenSize(screenX, screenY);
		
		ScreenToWorld(screenX/2, screenY/2, worldX, worldY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get a specific map module
	//! \param moduleType is a typename of the wanted module
	//! \return Returns requested module or null if not found
	SCR_MapModuleBase GetMapModule(typename moduleType)
	{
		foreach ( SCR_MapModuleBase module : m_aActiveModules )
		{
			if ( module.IsInherited(moduleType) )
				return module;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get a specific map UI component
	//! \param moduleType is a typename of the wanted UI component
	//! \return Returns requested UI component or null if not found
	SCR_MapUIBaseComponent GetMapUIComponent(typename componentType)
	{
		foreach ( SCR_MapUIBaseComponent comp : m_aActiveComponents )
		{
			if ( comp.IsInherited(componentType) )
				return comp;
		}
		
		return null;
	}
		
	//------------------------------------------------------------------------------------------------
	// OPEN / CLOSE
	//------------------------------------------------------------------------------------------------
	//! Open the map
	//! \param config is the configuration object
	void OpenMap(MapConfiguration config)
	{
		if (!config)
			return;
		
		if (m_bIsOpen)
		{
			Print("SCR_MapEntity: Attempted opening a map while it is already open", LogLevel.WARNING);
			CloseMap();
		}
		
		if (config.MapEntityMode != m_eLastMapMode)
			m_bDoReload = true;
		
		m_eLastMapMode = config.MapEntityMode;
		m_ActiveMapCfg = config;
		m_Workspace = GetGame().GetWorkspace();
		m_wMapRoot = config.RootWidgetRef;
		
		SetMapWidget(config.RootWidgetRef.FindAnyWidget(SCR_MapConstants.MAP_WIDGET_NAME));		
		
		if (config.MapEntityMode == EMapEntityMode.FULLSCREEN)
		{			
			ChimeraCharacter char = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
			if (char)
				SCR_CharacterControllerComponent.Cast(char.GetCharacterController()).m_OnLifeStateChanged.Insert(OnLifeStateChanged);

			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
		}
		
		InitLayers(config);
				
		SetFrame(Vector(0, 0, 0), Vector(0, 0, 0)); // Gamecode starts rendering stuff like descriptors straight away instead of waiting a frame - this is a hack to display nothing, avoiding the "blink" of icons
		
		m_bIsOpen = true;
		
		s_OnMapInit.Invoke(config);

		PlayerController plc = GetGame().GetPlayerController();
		if (plc && GetGame().GetCameraManager().CurrentCamera() == plc.GetPlayerCamera())
			plc.SetCharacterCameraRenderActive(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Close the map
	void CloseMap()
	{
		if (!m_bIsOpen)
			return;
		
		OnMapClose();
		
		m_bIsOpen = false;
		m_iDelayCounter = FRAME_DELAY;
		auto plc = GetGame().GetPlayerController();
		if (plc)
			plc.SetCharacterCameraRenderActive(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Map open event
	//! \param config is the configuration object
	protected void OnMapOpen(MapConfiguration config)
	{						
		// init zoom & layers
		m_MapWidget.SetSizeInUnits(Vector(m_iMapSizeX, m_iMapSizeY, 0));	// unit size to meters
		UpdateZoomBounds();
		AssignViewLayer(true);
		
		if (m_bDoUpdate)	// when resolution changes, zoom to the same PPU to update zoom and pos
		{
			ZoomSmooth(m_fZoomPPU, reinitZoom: true);
			m_bDoUpdate = false;
		}
		
		// activate modules & components
		ActivateModules(config.Modules);
		ActivateComponents(config.Components);
		ActivateOtherComponents(config.OtherComponents);
		
		m_bDoReload = false;
		
		if (s_OnMapOpen)
			s_OnMapOpen.Invoke(config);

		if (config.MapEntityMode == EMapEntityMode.FULLSCREEN)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_OPEN);
		
		EnableVisualisation(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Map close event
	protected void OnMapClose()
	{
		if (s_OnMapClose)
			s_OnMapClose.Invoke(m_ActiveMapCfg);
		
		if (m_ActiveMapCfg.MapEntityMode == EMapEntityMode.FULLSCREEN)
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_CLOSE);
			
			PlayerController controller = GetGame().GetPlayerController();
			if (controller)
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(controller.GetControlledEntity());
				if (char)
					SCR_CharacterControllerComponent.Cast(char.GetCharacterController()).m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			}
			
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerDeleted().Remove(OnPlayerDeleted);
		}
		
		if ( m_ActiveMapCfg.OtherComponents & EMapOtherComponents.LEGEND_SCALE)
			EnableLegend(false);
		
		EnableVisualisation(false);
		
		Cleanup();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CharacterControllerComponent event
	//! Called only in case of a fullscreen map
	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.ALIVE)
			return;
		
		SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(GetGame().GetPlayerController().GetControlledEntity());
		if (!gadgetMgr)
			return;
		
		IEntity mapGadget = gadgetMgr.GetGadgetByType(EGadgetType.MAP);
		if (!mapGadget)
			return;
		
		SCR_MapGadgetComponent mapComp = SCR_MapGadgetComponent.Cast(mapGadget.FindComponent(SCR_MapGadgetComponent));
		mapComp.SetMapMode(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	//! Called only in case of a fullscreen map
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{
		if (playerId != GetGame().GetPlayerController().GetPlayerId())
			return;
		
		MenuManager menuManager = g_Game.GetMenuManager();
		menuManager.CloseMenuByPreset(ChimeraMenuPreset.MapMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Game event
	protected void OnUserSettingsChanged()
	{
		UpdateTexts();
		
		if (m_bIsOpen)
		{
			UpdateZoomBounds();
			ZoomSmooth(m_fZoomPPU, reinitZoom: true);
		}
		else
			m_bDoUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Game event
	protected void OnWindowResized(int width, int heigth, bool windowed)
	{
		OnUserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	// MAP SETUP
	//------------------------------------------------------------------------------------------------
	//! Prepare MapConfiguration object from provided map config
	//! \param mapMode is the last map mode opened, if its the same we do not need to setup new config
	//! \param configPath is the path to the map config resource
	//! \param rootWidget is the root widget of the layout used to display map
	MapConfiguration SetupMapConfig(EMapEntityMode mapMode, ResourceName configPath, Widget rootWidget)
	{		
		if (mapMode == m_eLastMapMode)
		{
			m_ActiveMapCfg.RootWidgetRef = rootWidget;
			return m_ActiveMapCfg;
		}
		
		// clear loaded compas and modules
		m_aLoadedComponents.Clear();
		m_aLoadedModules.Clear();
		
		// Load config
		Resource container = BaseContainerTools.LoadContainer(configPath);
		if (!container)	
			return null;	
					
		SCR_MapConfig mapConfig = SCR_MapConfig.Cast( BaseContainerTools.CreateInstanceFromContainer( container.GetResource().ToBaseContainer() ) );
		MapConfiguration configObject = new MapConfiguration();
		
		// basic
		configObject.RootWidgetRef = rootWidget;
		configObject.MapEntityMode = mapConfig.m_iMapMode;
				
		// modules & componentss
		configObject.Modules = mapConfig.m_aModules;
		configObject.Components = mapConfig.m_aUIComponents;
		
		if (mapConfig.m_bEnableLegendScale == true)
			configObject.OtherComponents |= EMapOtherComponents.LEGEND_SCALE;
		
		if (mapConfig.m_bEnableGrid == true)
			configObject.OtherComponents |= EMapOtherComponents.GRID;
			
		SetupLayersAndProps(configObject, mapConfig);
		SetupDescriptorTypes(mapConfig.m_DescriptorDefaultsConfig);
					
		return configObject;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Layer and map properties config setup, if missing creates a default one
	//! \param configObject is a config object created for map init
	//! \param mapCfg is the provided map config
	protected void SetupLayersAndProps(inout MapConfiguration configObject, SCR_MapConfig mapCfg)
	{
		SCR_MapLayersBase layersCfg = mapCfg.m_LayersConfig;	// layers
		if (!layersCfg)	// use default if above fails
		{
			Resource containerDefs = BaseContainerTools.LoadContainer(SCR_MapConstants.CFG_LAYERS_DEFAULT);
			layersCfg = SCR_MapLayersBase.Cast( BaseContainerTools.CreateInstanceFromContainer( containerDefs.GetResource().ToBaseContainer() ) );	
		}
				
		configObject.LayerConfig = layersCfg;
		
		if (layersCfg.m_aLayers.IsEmpty())
			configObject.LayerCount = 0;
		else 
			configObject.LayerCount = layersCfg.m_aLayers.Count();
		
		SCR_MapPropsBase propsCfg = mapCfg.m_MapPropsConfig;	// map properties
		if (!propsCfg)	// use default if above fails
		{
			Resource containerDefs = BaseContainerTools.LoadContainer(SCR_MapConstants.CFG_PROPS_DEFAULT);
			propsCfg = SCR_MapPropsBase.Cast( BaseContainerTools.CreateInstanceFromContainer( containerDefs.GetResource().ToBaseContainer() ) );	
		}
		
		configObject.MapPropsConfig = propsCfg;
		
		SCR_MapDescriptorVisibilityBase descriptorViewCfg = mapCfg.m_DescriptorVisibilityConfig; // descriptors visibility within layers
		if (!descriptorViewCfg)	// use default if above fails
		{
			Resource containerDefs = BaseContainerTools.LoadContainer(SCR_MapConstants.CFG_DESCVIEW_DEFAULT);
			descriptorViewCfg = SCR_MapDescriptorVisibilityBase.Cast( BaseContainerTools.CreateInstanceFromContainer( containerDefs.GetResource().ToBaseContainer() ) );	
		}
		
		configObject.DescriptorVisibilityConfig = descriptorViewCfg;
		
		
		SCR_MapDescriptorDefaults descriptorDefaults = mapCfg.m_DescriptorDefaultsConfig;
		if (!descriptorDefaults)
		{
			Resource containerDefs = BaseContainerTools.LoadContainer(SCR_MapConstants.CFG_DESCTYPES_DEFAULT);
			descriptorDefaults = SCR_MapDescriptorDefaults.Cast( BaseContainerTools.CreateInstanceFromContainer( containerDefs.GetResource().ToBaseContainer() ) );	
		}
		
		configObject.DescriptorDefsConfig = descriptorDefaults;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Map properties config setup, matches imageset indices defined in map layout (MapWidget) to descriptor types
	//! \param configObject is a config object created for map init
	//! \param mapCfg is the provided map config
	protected void SetupDescriptorTypes(SCR_MapDescriptorDefaults descriptorDefaultsConfig)
	{
		array<int> imagesetIndices = {};
		
		if (!descriptorDefaultsConfig.m_aDescriptorDefaults.IsEmpty())
		{
			for (int i = 0; i < EMapDescriptorType.MDT_COUNT; ++i)
			{
				imagesetIndices.Insert(-1);
			}
			
			foreach (SCR_DescriptorDefaultsBase descriptorDefs : descriptorDefaultsConfig.m_aDescriptorDefaults)
			{
				imagesetIndices[descriptorDefs.m_iDescriptorType] = descriptorDefs.m_iImageSetIndex;
			}
			
			SetImagesetMapping(imagesetIndices);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// MAP CONTROL METHODS
	//------------------------------------------------------------------------------------------------
	//! Center the map
	void CenterMap()
	{
		if (m_iDelayCounter > 0)
		{
			Print("SCR_MapEntity: Attempt to call CenterMap before map init is completed", LogLevel.WARNING);
			return;
		}		
		
		//! get current size of map(includes zoom level) in screen space
		int x, y;
		WorldToScreen(m_iMapOffsetX + m_iMapSizeX / 2, m_iMapOffsetY + m_iMapSizeY / 2, x, y);

		SetPan(x, y); 
	}
		
	//------------------------------------------------------------------------------------------------
	//! Set minimal zoom and center the map
	void ZoomOut()
	{
		SetZoom(m_fMinZoom, true);
		CenterMap();
	}
			
	//------------------------------------------------------------------------------------------------
	//! Show/hide debug info table
	void ShowScriptDebug()
	{
		m_bIsDebugMode = !m_bIsDebugMode;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Set zoom value
	//! \param targetPPU is wanted pixel per unit (zoom) value
	//! \param instant determines whether it is a one time zoom or an animation
	void SetZoom(float targetPPU, bool instant = false)
	{
		if (m_iDelayCounter > 0)
		{
			Print("SCR_MapEntity: Attempt to call SetZoom before map init is completed", LogLevel.NORMAL);
			return;
		}
		
		targetPPU = Math.Clamp(targetPPU, m_fMinZoom, m_fMaxZoom); 
		
		m_fZoomPPU = targetPPU;
		AssignViewLayer(false);
		ZoomChange(targetPPU / m_MapWidget.PixelPerUnit());	// contours
		
		if (instant)
			m_fTargetPPU = m_fZoomPPU;
		
		s_OnMapZoom.Invoke(targetPPU);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Interpolated zoom
	//! \param targetPixPerUnit is the target pixel per unit value
	//! \param zoomTime is interpolation duration
	//! \param zoomToCenter determines whether zoom target is screen center (true) or relative position of mouse within window (false)
	//! \param reinitZoom determines whether the condition blocking zooming to the same PPU value is ignored (f.e. when changing resolution)
	void ZoomSmooth(float targetPixPerUnit, float zoomTime = 0.25, bool zoomToCenter = true, bool reinitZoom = false)
	{
		if (m_iDelayCounter > 0)
		{
			Print("SCR_MapEntity: Attempt to call ZoomSmooth before map init is completed", LogLevel.NORMAL);
			return;
		}
		
		if (zoomTime <= 0)
			zoomTime = 0.1;

		targetPixPerUnit = Math.Clamp(targetPixPerUnit, m_fMinZoom, m_fMaxZoom);
		if (!reinitZoom && targetPixPerUnit == m_fZoomPPU)
			return;
		
		m_fStartPPU = m_fZoomPPU;
		m_fTargetPPU = targetPixPerUnit;
		m_fZoomTimeModif = 1/zoomTime;
		m_fZoomSlice = 1.0;
		m_bIsZoomInterp = true;
		
		float screenX, screenY, worldX, worldY, targetScreenX, targetScreenY;
		m_MapWidget.GetScreenSize(screenX, screenY);
		
		if (zoomToCenter)
		{
			// zoom according to the current screen center
			GetMapCenterWorldPosition(worldX, worldY);
			WorldToScreenCustom( worldX, worldY, targetScreenX, targetScreenY, targetPixPerUnit, false );
			PanSmooth( targetScreenX, targetScreenY, zoomTime ); 
		}
		else
		{
			// Calculate target pan position in a way which makes cursor stay on the same world pos while zooming
			float diffX = screenX/2 - m_Workspace.DPIScale(SCR_MapCursorInfo.x); // difference in pixels between screen center and cursor
			float diffY = screenY/2 - m_Workspace.DPIScale(SCR_MapCursorInfo.y);

			GetMapCursorWorldPosition(worldX, worldY); // current cursor world pos, relative anchor of zoom
			WorldToScreenCustom( worldX, worldY, targetScreenX, targetScreenY, targetPixPerUnit, false ); // target screen pos of cursor with zoom applied
			PanSmooth( targetScreenX + diffX, targetScreenY + diffY, zoomTime );  // offset the target position by the pix diference from screen center
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Interpolated zoom with custom pan target for simultaneous use of zoom+pan
	//! \param targetPixPerUnit is the target pixel per unit value
	//! \param worldX is world pos X panning target
	//! \param worldY is world pos Y panning target
	//! \param zoomTime is interpolation duration
	void ZoomPanSmooth(float targetPixPerUnit, float worldX, float worldY, float zoomTime = 0.25)
	{
		if (m_iDelayCounter > 0)
		{
			Print("SCR_MapEntity: Attempt to call ZoomPanSmooth before map init is completed", LogLevel.NORMAL);
			return;
		}
		
		if (zoomTime <= 0)
			zoomTime = 0.1;

		if (targetPixPerUnit > m_fMaxZoom)
			targetPixPerUnit = m_fMaxZoom;
		else if (targetPixPerUnit < m_fMinZoom)
			targetPixPerUnit = m_fMinZoom;

		m_fStartPPU = m_fZoomPPU;
		m_fTargetPPU = targetPixPerUnit;
		m_fZoomTimeModif = 1/zoomTime;
		m_fZoomSlice = 1.0;
		m_bIsZoomInterp = true;
		
		float screenX, screenY, targetScreenX, targetScreenY;
		m_MapWidget.GetScreenSize(screenX, screenY);
		
		WorldToScreenCustom( worldX, worldY, targetScreenX, targetScreenY, targetPixPerUnit, false ); // target pos with zoom applied
		PanSmooth( targetScreenX, targetScreenY, zoomTime );
	}
		
	//------------------------------------------------------------------------------------------------
	//! Pan the map to target position, all of scripted panning is called through this
	//! \param x is horizontal screen UNSCALED coordinate
	//! \param y is vertical screen UNSCALED coordinate
	//! \param center determines whether the map should center to the supplied coordinates
	//! \param IsPanEnd determines whether this is also the end of panning operation, resetting the start pos for drag pannning
	void SetPan(float x, float y, bool isPanEnd = true, bool center = true)
	{	
		if (m_iDelayCounter > 0)
		{
			Print("SCR_MapEntity: Attempt to call SetPan before map init is completed", LogLevel.NORMAL);
			return;
		}
			
		bool adjustedPan = false;
		
		// test bounds
		if (!FitPanBounds(x, y, center))
			adjustedPan = true;
		
		// save current pan
		m_iPanX = x;
		m_iPanY = y;
		
		PosChange(m_Workspace.DPIScale(m_iPanX), m_Workspace.DPIScale(m_iPanY));
		
		if (isPanEnd)
			SCR_MapCursorInfo.startPos = {0, 0};
		
		s_OnMapPan.Invoke(m_iPanX, m_iPanY, adjustedPan);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Pan the map by px amount
	//! \param panMode is mode of panning
	//! \param panValue is amount of pixels to pan
	void Pan(EMapPanMode panMode, float panValue = 0)
	{
		if (m_iDelayCounter > 0)
		{
			Print("SCR_MapEntity: Attempt to call Pan before map init is completed", LogLevel.NORMAL);
			return;
		}
		
		float panX, panY;
		
		// panning mode				
		if (panMode == EMapPanMode.DRAG)
		{			
			// begin drag
			if (SCR_MapCursorInfo.startPos[0] == 0 && SCR_MapCursorInfo.startPos[1] == 0)
				SCR_MapCursorInfo.startPos = {SCR_MapCursorInfo.x, SCR_MapCursorInfo.y};
			
			// mouse position difference
			int diffX = SCR_MapCursorInfo.x - SCR_MapCursorInfo.startPos[0]; 
			int diffY = SCR_MapCursorInfo.y - SCR_MapCursorInfo.startPos[1];
			
			panX = m_iPanX + diffX;
			panY = m_iPanY + diffY;
			
			SCR_MapCursorInfo.startPos[0] = SCR_MapCursorInfo.x;
			SCR_MapCursorInfo.startPos[1] = SCR_MapCursorInfo.y;
		}
		else if (panMode == EMapPanMode.HORIZONTAL)
		{
			panX = m_iPanX + panValue;
			panY = m_iPanY;
		}
		else if (panMode == EMapPanMode.VERTICAL)
		{
			panX = m_iPanX;
			panY = m_iPanY + panValue;
		}
		
		// Pan
		SetPan(panX, panY, false, false);
		s_OnMapPanEnd.Invoke(panX, panY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Interpolated pan
	//! \param panX is x target screen coordinate in DPIScaled px
	//! \param panY is y target screen coordinate in DPIScaled px
	//! \param panTime is interpolation duration
	void PanSmooth(float panX, float panY, float panTime = 0.25)
	{		
		if (m_iDelayCounter > 0)
		{
			Print("SCR_MapEntity: Attempt to call PanSmooth before map init is completed", LogLevel.NORMAL);
			return;
		}
		
		float screenWidth, screenHeight;
		m_MapWidget.GetScreenSize(screenWidth, screenHeight);
		
		if (panTime <= 0)
			panTime = 0.1;
		
		// un-center to get direct pan pos
		m_aStartPan = { m_Workspace.DPIUnscale(screenWidth/2) - m_iPanX, m_Workspace.DPIUnscale(screenHeight/2) - m_iPanY };
		m_aTargetPan = { m_Workspace.DPIUnscale(panX), m_Workspace.DPIUnscale(panY)};
		m_fPanTimeModif = 1/panTime;
		m_fPanSlice = 1.0;
		m_bIsPanInterp = true;
	}
				
	//------------------------------------------------------------------------------------------------
	//! Triggers OnSelection event
	//! \param selectionPos the selection (click) position in scaled screen coordinates
	void InvokeOnSelect(vector selectionPos)
	{
		s_OnSelection.Invoke(selectionPos);
	}
	
	//! Select target MapItem
	//! \param MapItem is the target 
	void SelectItem(MapItem item)
	{
		item.Select(true);
		item.SetHighlighted(true);
		

		s_OnSelectionChanged.Invoke(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set hover mode to target MapItem
	//! \param MapItem is the target 
	void HoverItem(MapItem item)
	{
		item.SetHovering(true);
		m_HoveredMapItem = item;
		
		s_OnHoverItem.Invoke(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear selected items
	void ClearSelection()
	{
		ResetSelected();
		ResetHighlighted();
		
		s_OnSelectionChanged.Invoke(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear hover state
	void ClearHover()
	{
		ResetHovering();

		s_OnHoverEnd.Invoke(m_HoveredMapItem);
		m_HoveredMapItem = null;
	}
	
	//------------------------------------------------------------------------------------------------
	// CONVERSION METHODS
	//------------------------------------------------------------------------------------------------
	//! Use canvas world coords to get DPIscaled screen coords, flips the y-axis
	// \param worldX is world x
	// \param worldY is world y
	// \param screenPosX is screen x
	// \param screenPosY is screen y
	// \param withPan determines whether current pan is added to the result
	void WorldToScreen(float worldX, float worldY, out int screenPosX, out int screenPosY, bool withPan = false)
	{
		worldY = m_iMapSizeY - worldY; // fix Y axis which is reversed between screen and world
	
		if (withPan)
		{
			screenPosX = ((worldX - m_iMapOffsetX) * m_fZoomPPU) + m_Workspace.DPIScale(m_iPanX);
			screenPosY = ((worldY + m_iMapOffsetY) * m_fZoomPPU) + m_Workspace.DPIScale(m_iPanY);
		}
		else
		{
			screenPosX = (worldX - m_iMapOffsetX) * m_fZoomPPU;
			screenPosY = (worldY + m_iMapOffsetY) * m_fZoomPPU;		
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Use canvas world coords and defined pixel per unit to get DPIscaled screen coords, flips the y-axis
	// \param worldX is world x
	// \param worldY is world y
	// \param screenPosX is screen x
	// \param screenPosY is screen y
	// \param withPan determines whether current pan is added to the result
	// \param targetPPU sets PixelPerUnit used for the calculation (useful for precalculating)
	void WorldToScreenCustom(float worldX, float worldY, out int screenPosX, out int screenPosY, float targetPPU, bool withPan = false)
	{
		worldY = m_iMapSizeY - worldY;
		
		if (withPan)
		{
			screenPosX = ((worldX - m_iMapOffsetX) * targetPPU) + m_Workspace.DPIScale(m_iPanX);
			screenPosY = ((worldY + m_iMapOffsetY) * targetPPU) + m_Workspace.DPIScale(m_iPanY);
		}
		else
		{
			screenPosX = (worldX - m_iMapOffsetX) * targetPPU;
			screenPosY = (worldY + m_iMapOffsetY) * targetPPU;		
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Use scaled screen coords to get canvas world coords, flips the y-axis
	// \param screenPosX is screen x
	// \param screenPosY is screen y
	// \param worldX is world x
	// \param worldY is world y
	void ScreenToWorld(int screenPosX, int screenPosY, out float worldX, out float worldY)
	{				
		worldX = (screenPosX - m_Workspace.DPIScale(m_iPanX)) / m_fZoomPPU + m_iMapOffsetX;
		worldY = (screenPosY - m_Workspace.DPIScale(m_iPanY)) / m_fZoomPPU - m_iMapOffsetY;
		worldY =  m_iMapSizeY - worldY;	// fix Y axis which is reversed between screen and world
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use scaled screen coords to get canvas world coords without flipping the y-axis
	// \param screenPosX is screen x
	// \param screenPosY is screen y
	// \param worldX is world x
	// \param worldY is world y
	void ScreenToWorldNoFlip(int screenPosX, int screenPosY, out float worldX, out float worldY)
	{		
		worldX = (screenPosX - m_Workspace.DPIScale(m_iPanX)) / m_fZoomPPU + m_iMapOffsetX;
		worldY = (screenPosY - m_Workspace.DPIScale(m_iPanY)) / m_fZoomPPU + m_iMapOffsetY;
	}
		
	/*!
	Get grid coordinates for given position.
	\param pos World position
	\param resMin Minimum grid resolution as 10^x, e.g., 2 = 100, 3 = 1000, etc.
	\param resMax Maximum grid resolution as 10^x
	\param delimiter String added between horizontal and vertical coordinate
	\return Grid coordinates
	*/
	//--- ToDo: Use grid sizes configured in layers in case someone will define non-metric grid
	static string GetGridLabel(vector pos, int resMin = 2, int resMax = 4, string delimiter = " ")
	{
		int gridX, gridZ;
		GetGridPos(pos, gridX, gridZ, resMin, resMax);
		return gridX.ToString() + delimiter + gridZ.ToString();
	}
	
	/*!
	Get grid coordinates for given position.
	\param pos World position
	\param resMin Minimum grid resolution as 10^x, e.g., 2 = 100, 3 = 1000, etc.
	\param resMax Maximum grid resolution as 10^x
	\return gridX, gridZ coordinates
	*/
	static void GetGridPos(vector pos, out int gridX, out int gridZ, int resMin = 2, int resMax = 4)
	{
		//--- Convert to int so we can use native mod operator %
		int posX = pos[0];
		int posZ = pos[2];
		
		for (int i = resMax; i >= resMin; i--)
		{
			int mod = Math.Pow(10, i);
			int modX = posX - posX % mod;
			int modZ = posZ - posZ % mod;
			gridX = gridX * 10 + (modX / mod);
			gridZ = gridZ * 10 + (modZ / mod);
			posX -= modX;
			posZ -= modZ;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// SUPPORT METHODS
	//------------------------------------------------------------------------------------------------
	//! Adjust provided pan values by pan bounds conditions
	//! \param panX is UNSCALED pan offset x in px
	//! \param panY is UNSCALED pan offset y in px
	//! \return false if pan had to be adjusted to fit the pan rules
	protected bool FitPanBounds(inout float panX, inout float panY, bool center)
	{
		float windowWidth, windowHeight;
		m_MapWidget.GetScreenSize(windowWidth, windowHeight);
		
		windowWidth = m_Workspace.DPIUnscale(windowWidth);
		windowHeight = m_Workspace.DPIUnscale(windowHeight);
		
		// center to coords
		if (center)
		{			
			panX = windowWidth/2 - panX;
			panY = windowHeight/2 - panY;
		}
		
		int width 	= m_iMapSizeX * m_fZoomPPU;
		int height 	= m_iMapSizeY * m_fZoomPPU;
		
		// center of the screen can travel everywhere within map
		int minCoordX = windowWidth/2 - m_Workspace.DPIUnscale(width);
		int minCoordY = windowHeight/2 - m_Workspace.DPIUnscale(height);
		int maxCoordX = windowWidth/2;
		int maxCoordY = windowHeight/2;
		
		// cannot pan outside of map bounds 
		/*int minCoordX = windowWidth - width;
		int minCoordY = windowHeight - height;
		int maxCoordX = 0;
		int maxCoordY = 0;*/
		
		bool adjusted = false;
		
		// stop when over min/max
		if (panX < minCoordX)
		{
			panX = minCoordX;
			adjusted = true;
		}
		
		if (panX > maxCoordX)
		{ 
			panX = maxCoordX;
			adjusted = true;
		}
		
		if (panY < minCoordY) 
		{ 
			panY = minCoordY;
			adjusted = true;
		}
		
		if (panY > maxCoordY)
		{
			panY = maxCoordY;
			adjusted = true;
		}
		
		if (adjusted)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculate zoom min/max 
	//! \return true if successful
	bool UpdateZoomBounds()
	{
		float screenWidth, screenHeight;
		m_MapWidget.GetScreenSize(screenWidth, screenHeight);

		float maxUnitsPerScreen = screenHeight / m_MapWidget.PixelPerUnit();	
		m_fMinZoom = (maxUnitsPerScreen / m_iMapSizeY) * m_MapWidget.PixelPerUnit();
		m_fMaxZoom = SCR_MapConstants.MAX_PIX_PER_METER;
		
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Checks whether layer should change based on current zoom
	protected void AssignViewLayer(bool isInit)
	{
		int count = LayerCount();
		if (count == 0) // No layers to change to
			return;
		
		for ( int layerID = 0; layerID < count; layerID++ )
		{
			if ( GetCurrentZoom() >= GetLayer(layerID).GetCeiling() )
			{
				if (isInit || layerID != GetLayerIndex())
				{
					SetLayer(layerID);
					s_OnLayerChanged.Invoke(layerID);
				}
				
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// INIT / CLEANUP METHODS
	//------------------------------------------------------------------------------------------------
	//! Initialize layers from config
	protected void InitLayers(MapConfiguration mapConfig)
	{		
		SCR_MapLayersBase layerConfig = mapConfig.LayerConfig;
		
		int layerCount = mapConfig.LayerCount;
		InitializeLayers(layerCount);
		
		if (layerCount < 1)
			return;
		
		for (int i = 0; i < layerCount; i++)	// per layer configuration
		{
			MapLayer layer = GetLayer(i);
			if (layer)
			{
				// setr ceiling vals
				layerConfig.m_aLayers[i].SetLayerProps(layer);
									
				// props configs
				foreach ( SCR_MapPropsConfig propsCfg : mapConfig.MapPropsConfig.m_aMapPropConfigs)
				{
					propsCfg.SetDefaults(layer);
				}
				
				// descriptor configuration init
				InitDescriptors(i, layer, layerConfig.m_aLayers[i], mapConfig);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init visibility and default config values for descriptors based on type
	//! \param layerID is id of the layer
	//! \param layer is the map layer
	//! \param layerConfig is the config for the layer
	protected void InitDescriptors(int layerID, MapLayer layer, SCR_LayerConfiguration layerConfig, MapConfiguration mapConfig)
	{
		// create descriptor visiiblity map
		map<int, int> descriptorVisibility = new map<int, int>;
		
		// descriptor visibility configuration
		foreach (SCR_DescriptorViewLayer descConfig : mapConfig.DescriptorVisibilityConfig.m_aDescriptorViewLayers)
		{
			if (descConfig.m_iViewLayer >= layerID + 1)
				descriptorVisibility.Set(descConfig.m_iDescriptorType, layerID); // match descriptor type with visibility
		}
		
		foreach (SCR_DescriptorDefaultsBase descriptorType : mapConfig.DescriptorDefsConfig.m_aDescriptorDefaults)
		{
			// visible in the current layer
			if ( descriptorVisibility.Get(descriptorType.m_iDescriptorType) >= layerID )
			{	
				MapDescriptorProps props;
				
				// is using faction based configuration
				if (descriptorType.m_bUseFactionColors)
				{
					foreach (SCR_FactionColorDefaults factionDefaults : mapConfig.DescriptorDefsConfig.m_aFactionColors)
					{
						props = layer.GetPropsFor(factionDefaults.m_iFaction, descriptorType.m_iDescriptorType);
						if (props)
						{
							descriptorType.SetDefaults(props);
							factionDefaults.SetColors(props);
						}
					}
				}
				else
				{
					props = layer.GetPropsFor(EFactionMapID.UNKNOWN, descriptorType.m_iDescriptorType);
					if (props)
					{
						descriptorType.SetDefaults(props);
						descriptorType.SetColors(props);
					}
				}
			}
			// not visible in the current layer
			else 
			{
				MapDescriptorProps props;
				
				// is using faction based configuration
				if (descriptorType.m_bUseFactionColors)
				{
					foreach (SCR_FactionColorDefaults factionDefaults : mapConfig.DescriptorDefsConfig.m_aFactionColors)
					{
						props = layer.GetPropsFor(factionDefaults.m_iFaction, descriptorType.m_iDescriptorType);
						if (props)
							props.SetVisible(false);
					}
				}
				else
				{
					props = layer.GetPropsFor(EFactionMapID.UNKNOWN, descriptorType.m_iDescriptorType);
					if (props)
						props.SetVisible(false);
				}
			}
		}
	}
			
	//------------------------------------------------------------------------------------------------
	//! Activate modules 
	//! \param modules is an array of modules to activate
	//------------------------------------------------------------------------------------------------
	protected void ActivateModules( array<ref SCR_MapModuleBase> modules )
	{
		if (modules.IsEmpty())
			return;
				
		array<SCR_MapModuleBase> modulesToInit = new array<SCR_MapModuleBase>();
		
		foreach ( SCR_MapModuleBase module : modules ) 
		{
			// load new module
			if (m_bDoReload)
			{
				if (module.IsConfigDisabled())
					continue;
				
				m_aLoadedModules.Insert(module);
				modulesToInit.Insert(module);
				m_aActiveModules.Insert(module);
				module.SetActive(true);
			}
			// activate modules
			else 
			{
				int count = m_aLoadedModules.Count();
				for (int i = 0; i < count; i++)
				{
					if (!m_aLoadedModules[i].IsInherited(module.Type()))
						continue;
					
					// is module active
					if (m_aActiveModules.Find(m_aLoadedModules[i]) == -1)
					{
						m_aActiveModules.Insert(m_aLoadedModules[i]);
						m_aLoadedModules[i].SetActive(true);
						break;
					}	
				}
			}
		}
		
		foreach ( SCR_MapModuleBase module : modulesToInit )
		{
			module.Init();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate UI components 
	//! \param modules is an array of module typenames
	protected void ActivateComponents( array<ref SCR_MapUIBaseComponent> components )
	{		
		if (components.IsEmpty())
			return;
				
		array<SCR_MapUIBaseComponent> componentsToInit = new array<SCR_MapUIBaseComponent>();
		
		foreach ( SCR_MapUIBaseComponent component : components ) 
		{
			// load new component
			if (m_bDoReload)
			{
				if (component.IsConfigDisabled())
					continue;
				
				m_aLoadedComponents.Insert(component);
				componentsToInit.Insert(component);
				m_aActiveComponents.Insert(component);
				component.SetActive(true);
			}
			// activate components
			else
			{
				int count = m_aLoadedComponents.Count();
				for (int i = 0; i < count; i++)
				{
					if (!m_aLoadedComponents[i].IsInherited(component.Type()))
						continue;
									
					// is component active
					if (m_aActiveComponents.Find(m_aLoadedComponents[i]) == -1)
					{
						m_aActiveComponents.Insert(m_aLoadedComponents[i]);
						m_aLoadedComponents[i].SetActive(true);
						break;
					}	
				}
			}
		}
		
		foreach ( SCR_MapUIBaseComponent component : componentsToInit )
		{
			component.Init();
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate other components
	//! \param modules is an array of component names
	protected void ActivateOtherComponents(EMapOtherComponents componentFlags)
	{
		if (componentFlags & EMapOtherComponents.LEGEND_SCALE)
			EnableLegend(true);
		else 
			EnableLegend(false);
		
		if (componentFlags & EMapOtherComponents.GRID)
			EnableGrid(true);
		else 
			EnableGrid(false);

	}
	
	//------------------------------------------------------------------------------------------------
	//! Deactivate module, removing it from loadable list until config is reloaded
	void DeactivateModule(SCR_MapModuleBase module)
	{
		m_aActiveModules.RemoveItem(module);
		m_aLoadedModules.RemoveItem(module);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deactivate UI component, removing it from loadable list until config is reloaded
	void DeactivateComponent(SCR_MapUIBaseComponent component)
	{
		m_aActiveComponents.RemoveItem(component);
		m_aLoadedComponents.RemoveItem(component);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Map close cleanup
	protected void Cleanup()
	{
		// deactivate components & modules
		foreach (SCR_MapUIBaseComponent component : m_aActiveComponents )
		{
			component.SetActive(false, true);
		}
		m_aActiveComponents.Clear();
		
		foreach (SCR_MapModuleBase module : m_aActiveModules )
		{
			module.SetActive(false, true);
		}
		m_aActiveModules.Clear();
				
		m_bIsDebugMode = false;
	}
	
	//------------------------------------------------------------------------------------------------
	// UPDATE METHODS
	//------------------------------------------------------------------------------------------------
	//! Updates view port
	void UpdateViewPort()
	{
		float screenWidth, screenHeight;
		m_MapWidget.GetScreenSize(screenWidth, screenHeight);
			
		float minCoordX, minCoordY, maxCoordX, maxCoordY;
			
		ScreenToWorld(screenWidth, 0, maxCoordX, maxCoordY);
		ScreenToWorld(0, screenHeight, minCoordX, minCoordY);
			
		m_vVisibleFrameMin = Vector(minCoordX, 0, minCoordY);
		m_vVisibleFrameMax = Vector(maxCoordX, 0, maxCoordY);
		SetFrame(m_vVisibleFrameMin, m_vVisibleFrameMax);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Interpolated pan update
	//! \param timeSlice is frame timeSlice
	protected void PanUpdate(float timeSlice)
	{
		m_fPanSlice -= timeSlice * m_fPanTimeModif;
		
		// End interpolation
		if (m_fPanSlice <= 0)
		{
			m_bIsPanInterp = false;
			SetPan(m_aTargetPan[0], m_aTargetPan[1]);
			s_OnMapPanEnd.Invoke(m_aTargetPan[0],  m_aTargetPan[1]);
		}
		else
		{
			int panX = Math.Lerp(m_aStartPan[0], m_aTargetPan[0], 1 - m_fPanSlice);
			int panY = Math.Lerp(m_aStartPan[1], m_aTargetPan[1], 1 - m_fPanSlice);
			SetPan(panX, panY, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Interpolated zoom update
	//! \param timeSlice is frame timeSlice
	protected void ZoomUpdate(float timeSlice)
	{
		m_fZoomSlice -= timeSlice * m_fZoomTimeModif;
		
		// End interpolation
		if (m_fZoomSlice <= 0)
		{
			SetZoom(m_fTargetPPU);
			s_OnMapZoomEnd.Invoke(m_fTargetPPU);
			m_bIsZoomInterp = false;
		}
		else
		{
			float zoom = Math.Lerp(m_fStartPPU, m_fTargetPPU, 1 - m_fZoomSlice);
			SetZoom(zoom);
		}
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update map debug table
	protected void UpdateDebug()
	{		
		float wX, wY;
		float x = m_Workspace.DPIScale(SCR_MapCursorInfo.x);
		float y = m_Workspace.DPIScale(SCR_MapCursorInfo.y);
		ScreenToWorld(x, y, wX, wY);
		
		vector pan = GetCurrentPan();
		array<MapItem> outItems = {};
		GetSelected(outItems);
		
		DbgUI.Begin("Map debug");
		const string dbg1 = "CURSOR SCREEN POS: x: %1 y: %2";
		DbgUI.Text( string.Format( dbg1, x, y ) );
		const string dbg2 = "CURSOR WORLD POS: x: %1 y: %2";
		DbgUI.Text( string.Format( dbg2, wX, wY ) );
		const string dbg3 = "PAN OFFSET: x: %1 y: %2 ";
		DbgUI.Text( string.Format( dbg3, pan[0], pan[1] ) );
		const string dbg4 = "ZOOM: min: %1 max: %2 | pixPerUnit: %3";
		DbgUI.Text( string.Format( dbg4, GetMinZoom(), GetMaxZoom(), GetCurrentZoom() ) );
		const string dbg5 = "LAYER: current: %1 | pixPerUnit ceiling: %2";
		DbgUI.Text( string.Format( dbg5, GetLayerIndex(), GetLayer(GetLayerIndex()).GetCeiling() ) );
		const string dbg6 = "MODULES: loaded: %1 | active: %2 | list: %3 ";
		DbgUI.Text( string.Format( dbg6, m_aLoadedModules.Count(), m_aActiveModules.Count(), m_aActiveModules ) );
		const string dbg7 = "COMPONENTS: loaded: %1 | active: %2 | list: %3 ";
		DbgUI.Text( string.Format( dbg7, m_aLoadedComponents.Count(), m_aActiveComponents.Count(), m_aActiveComponents ) );
		const string dbg8 = "MAPITEMS: selected: %1 | hovered: %2 ";
		DbgUI.Text( string.Format( dbg8, outItems, m_HoveredMapItem ) );
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle update of modules, components and pan/zoom interpolation
	//! \param timeSlice is frame timeSlice 
	protected void UpdateMap(float timeSlice)
	{						
		// update modules
		foreach ( SCR_MapModuleBase module : m_aActiveModules)
		{
			module.Update(timeSlice);
		}
		
		//update components
		foreach ( SCR_MapUIBaseComponent component : m_aActiveComponents)
		{
			component.Update(timeSlice);
		}
		
		// interpolation update
		if (m_bIsZoomInterp)
			ZoomUpdate(timeSlice);
		
		if (m_bIsPanInterp)
			PanUpdate(timeSlice);
	}
	
#ifndef DISABLE_GADGETS		
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (IsOpen())
		{
			// delayed init -> This is here so the MapWidget has time to initiate its size & PixelPerUnit calculation
			if (m_iDelayCounter >= 0)
			{
				if (m_iDelayCounter == 0)
				{
					m_iDelayCounter--;
					OnMapOpen(m_ActiveMapCfg);
				}
				else 
				{
					m_iDelayCounter--;
					return;
				}
			}
						
			UpdateMap(timeSlice);
			
			if (m_bIsDebugMode)
				UpdateDebug();
						
			UpdateViewPort();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{

		vector size = Size();
		m_iMapSizeX = size[0];
		m_iMapSizeY = size[2];
		vector offset = Offset();
		m_iMapOffsetX = offset[0];
		m_iMapOffsetY = offset[2];
		
		if (m_iMapSizeX == 0 || m_iMapSizeY == 0)
		{
			Print("SCR_MapEntity: Cannot get the size from terrain. Using default.", LogLevel.WARNING);
			m_iMapSizeX = 1024;
			m_iMapSizeY = 1024;
		}
		
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.RegisterEntityToBeUpdatedWhileGameIsPaused(this);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_MapEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);

		s_MapInstance = this;
				
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_UI_MAP_DEBUG_OPTIONS, "", "Enable map debug menu", "UI");
		
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
		GetGame().OnWindowResizeInvoker().Insert(OnWindowResized);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapEntity()
	{
		if (m_bIsOpen)
			CloseMap();
		
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.UnregisterEntityToBeUpdatedWhileGameIsPaused(this);
		
		s_OnMapInit.Clear();
		s_OnMapOpen.Clear();
		s_OnMapClose.Clear();
		s_OnMapPan.Clear();
		s_OnMapPanEnd.Clear();
		s_OnMapZoom.Clear();
		s_OnMapZoomEnd.Clear();
		s_OnSelectionChanged.Clear();
		s_OnSelection.Clear();
		s_OnHoverItem.Clear();
		s_OnHoverEnd.Clear();
		s_OnLayerChanged.Clear();
		
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_UI_MAP_DEBUG_OPTIONS);
		
		s_MapInstance = null;
	}

#endif	
};