#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "World Entities Statistics",
	description: "Get Prefabs usage and Entities statistics in the currently opened world",
	wbModules: { "WorldEditor" },
//	shortcut: "Ctrl+Shift+A",
	awesomeFontCode: 0xF1FE)] // terrain-looking chart!
class SCR_WorldEntitiesStatisticsPlugin : WorkbenchPlugin
{
	/*
		Category: General
	*/

	[Attribute(defvalue: "0", desc: "Output result in storage (txt and edds), otherwise Log Console", category: "General")]
	protected bool m_bOutputToFile;

	/*
		Category: Text Output
	*/

	[Attribute(defvalue: "0", desc: "Only analyse Entities/Prefabs from the active layer", category: "Text Output")]
	protected bool m_bActiveLayerOnly;

	[Attribute(defvalue: "0", desc: "Top X values to display\n0 = display everything", params: "0 inf", category: "Text Output")]
	protected int m_iMaxDisplayedEntries;

	[Attribute(defvalue: "0", desc: "Value under which entity counts are not listed", params: "0 inf", category: "Text Output")]
	protected int m_iDisplayThreshold;

	/*
		Category: Image Output
	*/

	[Attribute(defvalue: "1", desc: "Output terrain entities's density map (requires Output To File)", category: "Heatmap")]
	protected bool m_bOutputHeatmap;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Heat Map Type:\n- density = entity count per pixel\n- variety = different model (not entities/Prefabs) count per pixel", enums: SCR_ParamEnumArray.FromString("Entity density,Number of entities per pixel;Model variety,Number of different models per pixel"), category: "Heatmap")]
	protected int m_iHeatmapType;

	// not working, IDK why yet
//	[Attribute(defvalue: "0", desc: "Generate a density map for each layer (requires Output To File)", category: "Heatmap")]
	protected bool m_bOutputHeatmapForAllLayers;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "- Greyscale: from black to white\n- Thermal: from blue to green to red\n- Alpha: from transparent to white", enums: SCR_ParamEnumArray.FromString("Greyscale,From black to white;Thermal,From blue to green to red;Alpha,From transparent to white"), category: "Heatmap")]
	protected int m_iHeatmapColourMode;

	[Attribute(defvalue: "0", desc: "Invert pixel value (except for max value), e.g black = most dense, white = least dense instead of the opposite", category: "Heatmap")]
	protected bool m_bHeatmapValueInversion;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "This setting can prevent a density peak from \"darkening\" the image everywhere else - it also offers to highlight pixels above the 0..2×median range", enums: SCR_ParamEnumArray.FromString("Raw value;2 × average;2 × median"), category: "Heatmap")]
	protected int m_iHeatmapMaxValueMode;

	[Attribute(defvalue: "1", desc: "Highlight values above max (when Heat Map Max Value Mode is not Raw)\n- Greyscale: red pixels\n- Thermal: white pixels", category: "Heatmap")]
	protected bool m_bHeatmapHighlightValuesAboveMax;

	[Attribute(defvalue: "512", uiwidget: UIWidgets.ComboBox, desc: "Number of detection squares in terrain height/width", enums: SCR_ParamEnumArray.FromString("16,16×16,;32,32×32,;64,64×64,;128,128×128,;256,256×256,;512,512×512,;1024,1024×1024,;2048,2048×2048,;4096,4096×4096,;1080,1080×1080 (wallpaper),;1440,1440×1440 (wallpaper),;2160,2160×2160 (wallpaper),"), category: "Heatmap")]
	protected int m_iHeatmapDefinition;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Definition multiplier to obtain a bigger image - e.g ×2 = 1 data point takes 2×2 (4) pixels\nMaximum resolution allowed: " + SCR_HeatmapHelper.MAX_RESOLUTION, enums: SCR_ParamEnumArray.FromString(SCR_HeatmapHelper.GetResolutionFactorEnum()), category: "Heatmap")]
	protected int m_iHeatmapResolutionFactor;

//	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Determine density maps's file format", enums: SCR_ParamEnumArray.FromString(".dds;.png") category: "Heatmap")]
//	protected int m_iHeatmapFileFormat;

	/*
		Debug
	*/

	[Attribute(defvalue: "0", desc: "Visual shapes debug - display e.g land surface vs water surface", category: "Debug")]
	protected bool m_bDebug;

	protected ref SCR_DebugShapeManager m_DebugShapeManager;

	protected static const string WORLD_DIRECTORY_FORMAT = "WorldStatistics_%1";
	protected static const string LAYERS_SUBDIR = "Layers";
	protected static const string OUTPUT_FILE_NAME = "PrefabStatistics.txt";
	protected static const string LAYER_OUTPUT_FILE_NAME = "%1_%2_PrefabStatistics.txt"; //!< %1 = subScene ID, %2 = layer name
	protected static const string OUTPUT_MAP_NAME = "Heatmap_%1_%2.dds"; //!< %1 = data type (density, variety), %2 = colour mode (BW, RGB)
	protected static const string LAYER_OUTPUT_MAP_NAME = "%1_%2_Heatmap_%3_%4.dds"; //!< %1 = subScene ID, %2 = layer name, %3 = data type (density, variety), %4 = colour mode (BW, RGB)

	protected static const string NO_MAP_CONTENT_FORMAT = "No %1 found"; //!< %1 = resultMap provided description
	protected static const int MIN_TERRAIN_DEBUG_RESOLUTION = 50; // min 50m
	protected static const float SQM_TO_SQKM = 0.000001; // 1km = 1000×1000 m² = 1M m²

	protected static const int TYPE_DENSITY = 0;
	protected static const int TYPE_VARIETY = 1;

	protected static const int MIN_ENTITIES_RGB = 8; //!< minimum entities per pixel in RGB mode; if less than that, prints a warning

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		if (!Workbench.ScriptDialog("", "Get world entities statistics (or close the window)", this))
			return;

		Debug.BeginTimeMeasure();
		int scannedEntitiesCount = ScanWorld();
		if (scannedEntitiesCount < 0)
			Debug.EndTimeMeasure("World entity scan error");
		else
			Debug.EndTimeMeasure(string.Format("World scan (%1 entities)", scannedEntitiesCount));
	}

	//------------------------------------------------------------------------------------------------
	//! \return number of scanned entities, -1 on error
	protected int ScanWorld()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.ERROR);
			return -1;
		}

		string worldPath = SCR_WorldEditorToolHelper.GetWorldPath();
		if (!worldPath) // .IsEmpty()
		{
			Workbench.Dialog("", "Be sure to load a world (or have it saved) before using the World Entities Statistics plugin.");
			return -1;
		}

		IEntitySource terrainEntity = SCR_WorldEditorToolHelper.GetTerrainEntitySource();
		if (!terrainEntity)
		{
			Workbench.Dialog("", "Be sure to have a terrain entity (of type GenericTerrainEntity) before using the World Entities Statistics plugin.");
			return -1;
		}

		vector terrainOrigin;
		if (!terrainEntity.Get("coords", terrainOrigin))
			return -1;

		float terrainUnitScale = worldEditorAPI.GetTerrainUnitScale();
		float terrainX = terrainUnitScale * worldEditorAPI.GetTerrainResolutionX();
		float terrainZ = terrainX;
		vector terrainSize = { terrainX, 0, terrainZ };

		float landRatio = GetLandAboveWaterRatio(worldEditorAPI, terrainOrigin, terrainSize, terrainUnitScale);

		PrintFormat(
			"%1km² (%2%% land, %3%% water)",
			(terrainX * terrainZ * SQM_TO_SQKM).ToString(lenDec: 2),
			(landRatio * 100).ToString(lenDec: 2),
			((1 - landRatio) * 100).ToString(lenDec: 2));

		IEntitySource entitySource;
		IEntity entity;
		IEntitySource ancestor;
		int noSourceFound;
		int sourcesWithoutEntity;

		int currentSceneId;
		int currentLayerId;
		int subScenesCount = worldEditorAPI.GetNumSubScenes();
		map<int, ref map<int, ref SCR_WorldEntitiesStatisticsPlugin_Report>> subSceneLayerReports = new map<int, ref map<int, ref SCR_WorldEntitiesStatisticsPlugin_Report>>();
		map<int, ref SCR_WorldEntitiesStatisticsPlugin_Report> layerReports;
		SCR_WorldEntitiesStatisticsPlugin_Report currentLayerReport;
		SCR_WorldEntitiesStatisticsPlugin_Report mainReport;

		SCR_WorldEntitiesStatisticsPlugin_Report_World worldInfo = new SCR_WorldEntitiesStatisticsPlugin_Report_World();
		worldInfo.m_fTerrainSurface = terrainX * terrainZ;
		worldInfo.m_fTerrainSurfaceASLRatio = landRatio;
		worldInfo.m_vTerrainMin = terrainOrigin;
		worldInfo.m_vTerrainMax = terrainSize;

		if (m_bActiveLayerOnly)
		{
			currentSceneId = worldEditorAPI.GetCurrentSubScene();
			currentLayerId = worldEditorAPI.GetCurrentEntityLayerId();
		}
		else
		{
			mainReport = new SCR_WorldEntitiesStatisticsPlugin_Report();
			mainReport.m_WorldInfo = worldInfo;
		}

		Debug.BeginTimeMeasure();

		int editorEntitiesCount = worldEditorAPI.GetEditorEntityCount();
		for (int i = editorEntitiesCount - 1; i >= 0; --i)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (!entitySource) // wat
			{
				++noSourceFound;
				continue;
			}

			int entitySceneId = entitySource.GetSubScene();
			int entityLayerId = entitySource.GetLayerID();
			if (m_bActiveLayerOnly && (entityLayerId != currentLayerId || entitySceneId != currentSceneId))
				continue;

			if (!subSceneLayerReports.Find(entitySceneId, layerReports))
			{
				layerReports = new map<int, ref SCR_WorldEntitiesStatisticsPlugin_Report>();
				subSceneLayerReports.Set(entitySceneId, layerReports);
			}

			if (!layerReports.Find(entityLayerId, currentLayerReport))
			{
				currentLayerReport = new SCR_WorldEntitiesStatisticsPlugin_Report();
				currentLayerReport.m_WorldInfo = worldInfo;

				currentLayerReport.m_iSubSceneId = entitySceneId;
				currentLayerReport.m_sSubSceneName = string.Format("SubScene #%1/%2", entitySceneId + 1, subScenesCount);
				currentLayerReport.m_iLayerId = entityLayerId;
				currentLayerReport.m_sLayerName = worldEditorAPI.GetSubsceneLayerPath(entitySceneId, entityLayerId);
				layerReports.Insert(entityLayerId, currentLayerReport);
			}

			// add to all
			entity = worldEditorAPI.SourceToEntity(entitySource);
			vector worldPos;
			if (entity)
			{
				worldPos = entity.GetOrigin();
				currentLayerReport.m_aAllEntityPositions.Insert(worldPos);
				if (mainReport)
					mainReport.m_aAllEntityPositions.Insert(worldPos);
			}
			else
			{
				++sourcesWithoutEntity;
			}

			ancestor = entitySource.GetAncestor();
			if (ancestor)														// a Prefab
			{
				ResourceName resourceName = ancestor.GetResourceName();
				if (resourceName) // !.IsEmpty()
				{
					currentLayerReport.m_mPrefabResult.Set(resourceName, currentLayerReport.m_mPrefabResult.Get(resourceName) + 1);

					IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, "MeshObject");
					ResourceName model;
					if (componentSource)
					{
						if (componentSource.Get("Object", model) && model) // !.IsEmpty()
						{
							currentLayerReport.m_aModels.Insert(resourceName);
							currentLayerReport.m_aModelPositions.Insert(worldPos);
						}
					}

					if (mainReport)
					{
						mainReport.m_mPrefabResult.Set(resourceName, mainReport.m_mPrefabResult.Get(resourceName) + 1);
						if (model) // !.IsEmpty()
						{
							mainReport.m_aModels.Insert(resourceName);
							mainReport.m_aModelPositions.Insert(worldPos);
						}
					}
				}
				else															// a Prefab without a name...
				{
					resourceName = entitySource.GetResourceName();
					if (resourceName) // !.IsEmpty()							// a Prefab with Prefab set on entity itself?
					{
						currentLayerReport.m_mPrefabNoAncestorResult.Set(resourceName, currentLayerReport.m_mPrefabNoAncestorResult.Get(resourceName) + 1);
						if (mainReport)
							mainReport.m_mPrefabNoAncestorResult.Set(resourceName, mainReport.m_mPrefabNoAncestorResult.Get(resourceName) + 1);
					}
					else														// a Prefab without a Prefab path
					{
						string className = ancestor.GetClassName();
						currentLayerReport.m_mEmptyResult.Set(className, currentLayerReport.m_mEmptyResult.Get(className) + 1);
						if (mainReport)
							mainReport.m_mEmptyResult.Set(className, mainReport.m_mEmptyResult.Get(className) + 1);
					}
				}
			}
			else																// a raw entity (spline, GenericEntity, etc)
			{
				string className = entitySource.GetClassName();
				currentLayerReport.m_mGenericResult.Set(className, currentLayerReport.m_mGenericResult.Get(className) + 1);
				if (mainReport)
					mainReport.m_mGenericResult.Set(className, mainReport.m_mGenericResult.Get(className) + 1);
			}
		}

		Debug.EndTimeMeasure("Sorting " + editorEntitiesCount + " entities");

		if (noSourceFound > 0)
			PrintFormat("%1 entities do NOT have an Entity Source!", noSourceFound, level: LogLevel.WARNING);

		if (sourcesWithoutEntity > 0)
			PrintFormat("%1 entity sources do NOT have an Entity!", sourcesWithoutEntity, level: LogLevel.WARNING);

		// ######
		// output
		// ######

		string worldName = SCR_WorldEditorToolHelper.GetWorldName();
		string worldDirectory = string.Format(WORLD_DIRECTORY_FORMAT, worldName);
		string layersDirectory = FilePath.Concat(worldDirectory, LAYERS_SUBDIR);
//		layersDirectory = worldDirectory;

		string filePath;
		foreach (int subSceneId, map<int, ref SCR_WorldEntitiesStatisticsPlugin_Report> layersReports : subSceneLayerReports)
		{
			foreach (int layerId, SCR_WorldEntitiesStatisticsPlugin_Report layerReport : layersReports)
			{
				if (m_bOutputToFile)
				{
					filePath = FilePath.Concat(layersDirectory, string.Format(LAYER_OUTPUT_FILE_NAME, layerReport.m_iSubSceneId, layerReport.m_sLayerName));
					string fileDir = FilePath.StripFileName(filePath);
					if (!FileIO.MakeDirectory(fileDir))
					{
						Print("Cannot create " + fileDir, LogLevel.WARNING);
						continue;
					}

//					if (m_bOutputHeatmapForAllLayers)
//					{
////						filePath = FilePath.Concat(layersDirectory, string.Format(LAYER_OUTPUT_MAP_NAME, layerReport.m_iSubSceneId, layerReport.m_sLayerName));
//						filePath = FilePath.Concat(worldDirectory, string.Format(LAYER_OUTPUT_MAP_NAME, layerReport.m_iSubSceneId, layerReport.m_sLayerName));
//						if (!CreateDensityImage(
//							filePath,
//							m_iHeatmapDefinition,
//							layerReport.m_WorldInfo.m_vTerrainMin,
//							layerReport.m_WorldInfo.m_vTerrainMax,
//							layerReport.m_aAllEntityPositions))
//						{
//							Print("Cannot create image " + filePath, LogLevel.WARNING);
//						}
//					}
				}

				OutputLines(GetReportLines(layerReport), filePath);

				if (m_bActiveLayerOnly && filePath) // !.IsEmpty()
				{
					string absPath;
					if (Workbench.GetAbsolutePath(filePath, absPath, true))
						Workbench.RunCmd(string.Format("notepad \"%1\"", absPath));
				}
			}
		}

		if (mainReport)
		{
			filePath = FilePath.Concat(worldDirectory, OUTPUT_FILE_NAME);
			OutputLines(GetReportLines(mainReport), filePath);
			if (filePath) // !.IsEmpty()
			{
				string absPath;
				if (Workbench.GetAbsolutePath(filePath, absPath, true))
				{
					string cmd = string.Format("explorer \"%1\"", FilePath.StripFileName(absPath));
					cmd.Replace(SCR_StringHelper.SLASH, SCR_StringHelper.ANTISLASH); // explorer does not support slashes, notepad does -_-
					Workbench.RunCmd(cmd);
				}
			}

			if (m_bOutputToFile && m_bOutputHeatmap)
			{
				string type;
				if (m_iHeatmapType == TYPE_DENSITY)
					type = "density";
				else
//				if (m_iHeatmapType == TYPE_VARIETY)
					type = "variety";

				string colourMode;
				if (m_iHeatmapColourMode == SCR_HeatmapHelper.COLOUR_MODE_GREYSCALE)
					colourMode = "BW";
				else
				if (m_iHeatmapColourMode == SCR_HeatmapHelper.COLOUR_MODE_THERMAL)
					colourMode = "RGB";
				else
//				if (m_iHeatmapColourMode == SCR_HeatmapHelper.COLOUR_MODE_ALPHA)
					colourMode = "Alpha";

				if (m_bHeatmapValueInversion)
					colourMode += "inv";

				filePath = FilePath.Concat(worldDirectory, string.Format(OUTPUT_MAP_NAME, type, colourMode));
				if (m_iHeatmapType == TYPE_DENSITY)
				{
					if (!CreateDensityImage(
						filePath,
						m_iHeatmapDefinition,
						mainReport.m_WorldInfo.m_vTerrainMin,
						mainReport.m_WorldInfo.m_vTerrainMax,
						mainReport.m_aAllEntityPositions))
					{
						Print("Cannot create image " + filePath, LogLevel.WARNING);
					}
				}
				else
//				if (m_iHeatmapType == TYPE_VARIETY)
				{
					if (!CreateVarietyImage(
						filePath,
						m_iHeatmapDefinition,
						mainReport.m_WorldInfo.m_vTerrainMin,
						mainReport.m_WorldInfo.m_vTerrainMax,
						mainReport.m_aModels,
						mainReport.m_aModelPositions))
					{
						Print("Cannot create image " + filePath, LogLevel.WARNING);
					}
				}
			}
		}

		return editorEntitiesCount;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] worldEditorAPI
	//! \param[in] terrainOrigin
	//! \param[in] terrainSize
	//! \param[in] terrainUnitScale
	//! \return land ratio 0..1 land coverage ratio (dry terrain / total terrain)
	protected float GetLandAboveWaterRatio(notnull WorldEditorAPI worldEditorAPI, vector terrainOrigin, vector terrainSize, float terrainUnitScale)
	{
		if (m_bDebug)
		{
			if (m_DebugShapeManager)
				m_DebugShapeManager.Clear();
			else
				m_DebugShapeManager = new SCR_DebugShapeManager();
		}
		else
		{
			m_DebugShapeManager = null;
		}

		float oceanLevel = worldEditorAPI.GetWorld().GetOceanBaseHeight();

		float stepX;
		float stepZ;
		if (m_bDebug && terrainUnitScale < MIN_TERRAIN_DEBUG_RESOLUTION)
		{
			stepX = MIN_TERRAIN_DEBUG_RESOLUTION;
			stepZ = MIN_TERRAIN_DEBUG_RESOLUTION;
			PrintFormat("Debug is enabled - terrain surface tracing is every %1m instead of %2m, losing precision", MIN_TERRAIN_DEBUG_RESOLUTION, terrainUnitScale, level: LogLevel.WARNING);
		}
		else
		{
			stepX = terrainUnitScale;
			stepZ = terrainUnitScale;
		}

		int aslMeasures;
		int totalMeasures;
		for (float x = terrainOrigin[0]; x < terrainSize[0]; x += stepX)
		{
			for (float z = terrainOrigin[2]; z < terrainSize[2]; z += stepZ)
			{
				float y = worldEditorAPI.GetTerrainSurfaceY(x, z);
				bool isAboveOrEqualWaterLevel = y >= oceanLevel;
				if (isAboveOrEqualWaterLevel)
					++aslMeasures;

				++totalMeasures;

				if (m_DebugShapeManager)
				{
					vector pos2D = { x, 0, z };
					vector pos3D = { x, y, z };

//					if (isAboveOrEqualWaterLevel)
//						m_DebugShapeManager.AddRectangleXZ(pos, 0, stepX, stepZ, Color.GREEN);
//					else
//						m_DebugShapeManager.AddRectangleXZ(pos, 0, stepX, stepZ, Color.BLUE);

					if (isAboveOrEqualWaterLevel)
						m_DebugShapeManager.AddLine(pos2D, pos3D, Color.GREEN);
					else
						m_DebugShapeManager.AddLine(pos2D, pos3D, Color.BLUE);
				}
			}
		}

		if (totalMeasures < 1 || aslMeasures == totalMeasures)
			return 1;	// 100%

		if (aslMeasures < 1)
			return 0;	// 0%

		return aslMeasures / totalMeasures;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] report
	//! \return
	protected array<string> GetReportLines(notnull SCR_WorldEntitiesStatisticsPlugin_Report report)
	{
		array<string> lines = {};

		lines.InsertAll(GetFormattedMapLines("Prefab entities", report.m_mPrefabResult, report));
		lines.InsertAll(GetFormattedMapLines("entities with Prefab path on EntitySource", report.m_mPrefabNoAncestorResult, report));
		lines.InsertAll(GetFormattedMapLines("entities with empty Prefab path", report.m_mEmptyResult, report));
		lines.InsertAll(GetFormattedMapLines("generic entities", report.m_mGenericResult, report));

		return lines;
	}

	//------------------------------------------------------------------------------------------------
	//! Print statistics results in log console
	//! \param[in] lines
	//! \param[in] filePath
	protected void OutputLines(notnull array<string> lines, string filePath)
	{
		if (lines.IsEmpty())
			return;

		if (m_bOutputToFile && filePath)	// write to txt
		{
			if ((!FileIO.FileExists(filePath) || FileIO.DeleteFile(filePath)) && SCR_FileIOHelper.WriteFileContent(filePath, lines))
			{
				Print("Successfully wrote lines to file", LogLevel.DEBUG);
				return;
			}

			Print("Export failed: cannot write to file - printing instead", LogLevel.WARNING);
		}
											// print to log console
		foreach (string line : lines)
		{
			Print("" + line, LogLevel.NORMAL);
		}

		Print("\n", LogLevel.NORMAL); // or Print("" + SCR_StringHelper.LINE_RETURN, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Print statistics results in log console
	//! \param[in] description
	//! \param[in] resultMap
	protected array<string> GetFormattedMapLines(string description, notnull map<ResourceName, int> resultMap, notnull SCR_WorldEntitiesStatisticsPlugin_Report report)
	{
		int count = resultMap.Count();
		if (count < 1)
			return { string.Format(NO_MAP_CONTENT_FORMAT, description) };

		array<string> resultLines = {};

		array<int> values = {};
		values.Reserve(count);
		int entitiesCount;

		// standard entities

		foreach (string key, int value : resultMap)
		{
			values.Insert(value);
			entitiesCount += value;
		}

		float entityDensity;
		if (report.m_WorldInfo.m_fTerrainSurface > 0)
			entityDensity = entitiesCount / (report.m_WorldInfo.m_fTerrainSurface * SQM_TO_SQKM);

		resultLines.Insert(
			string.Format(
				"Found and processed %1 %2 (density: %3/km² on land, %4/km² in water, %5/km² total)", // no ':' colon as the following lines can be hidden by filters
				entitiesCount,
				description,
				(entityDensity * report.m_WorldInfo.m_fTerrainSurfaceASLRatio).ToString(lenDec: 2),
				(entityDensity * (1 - report.m_WorldInfo.m_fTerrainSurfaceASLRatio)).ToString(lenDec: 2),
				(entityDensity).ToString(lenDec: 2)
			));

		values.Sort(true); // DESC
		if (m_iMaxDisplayedEntries > 0 && values.Count() > m_iMaxDisplayedEntries)
			values.Resize(m_iMaxDisplayedEntries);

		foreach (int value : values)
		{
			if (value < m_iDisplayThreshold)
				break;

			ResourceName key;
			foreach (ResourceName mapKey, int mapValue : resultMap)
			{
				if (mapValue == value)
				{
					key = mapKey;
					resultMap.Remove(key); // fix duplicates
					resultLines.Insert(string.Format("%1× %2", value, key));
					break;
				}
			}

			if (!key) // !.IsEmpty()
				Print("empty key found", LogLevel.WARNING);
		}

		return resultLines;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] imagePath
	//! \param[in] resolution
	//! \param[in] terrainMin
	//! \param[in] terrainMax
	//! \param[in] entityPositions
	//! \return true on success, false otherwise
	protected bool CreateDensityImage(string imagePath, int resolution, vector terrainMin, vector terrainMax, notnull array<vector> entityPositions)
	{
		if (resolution < 1)
		{
			PrintFormat("Invalid resolution (%1×%1)", resolution, level: LogLevel.WARNING);
			return false;
		}

		if (terrainMin[0] >= terrainMax[0]
			// || terrainMin[1] >= terrainMax[1]
			|| terrainMin[2] >= terrainMax[2])
		{
			PrintFormat("Invalid terrain min/max (%1 & %2)", terrainMin, terrainMax, level: LogLevel.WARNING);
			return false;
		}

		float pixelWidth = (terrainMax[0] - terrainMin[0]) / resolution;
		float pixelHeight = (terrainMax[2] - terrainMin[2]) / resolution;

		if (pixelWidth < 0.1 || pixelHeight < 0.1)
		{
			PrintFormat("Invalid pixel size (%1×%2)", pixelWidth, pixelHeight, level: LogLevel.WARNING);
			return false;
		}

		array<int> imageData = {}; // temporarily used to store entity count first
		imageData.Resize(resolution * resolution);

		Debug.BeginTimeMeasure();

		foreach (vector entityPos : entityPositions)
		{
			// do not round - pixel 0,0 is for 0..0.99 entities

			entityPos -= terrainMin;

			int x = entityPos[0] / pixelWidth;
			if (x < 0 || x >= resolution)
				continue;

			int z = resolution - entityPos[2] / pixelHeight;
			if (z < 0 || z >= resolution)
				continue;

			imageData[x + resolution * z] = imageData[x + resolution * z] + 1;
		}

		Debug.EndTimeMeasure(
			string.Format(
				"Processing all %1 entity positions for image creation (%2×%2 = %3 pixels)",
				entityPositions.Count(),
				resolution,
				resolution * resolution));

		return SCR_HeatmapHelper.CreateHeatmapImageFromData(imagePath, imageData, m_iHeatmapColourMode, m_bHeatmapValueInversion, m_iHeatmapMaxValueMode, m_bHeatmapHighlightValuesAboveMax, m_iHeatmapResolutionFactor);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] imagePath
	//! \param[in] resolution
	//! \param[in] terrainMin
	//! \param[in] terrainMax
	//! \param[in] models
	//! \param[in] prefabPositions
	//! \return true on success, false otherwise
	protected bool CreateVarietyImage(string imagePath, int resolution, vector terrainMin, vector terrainMax, notnull array<ResourceName> models, notnull array<vector> prefabPositions)
	{
		if (resolution < 1)
		{
			PrintFormat("Invalid resolution (%1×%1)", resolution, level: LogLevel.WARNING);
			return false;
		}

		if (terrainMin[0] >= terrainMax[0]
			// || terrainMin[1] >= terrainMax[1]
			|| terrainMin[2] >= terrainMax[2])
		{
			PrintFormat("Invalid terrain min/max (%1 & %2)", terrainMin, terrainMax, level: LogLevel.WARNING);
			return false;
		}

		float pixelWidth = (terrainMax[0] - terrainMin[0]) / resolution;
		float pixelHeight = (terrainMax[2] - terrainMin[2]) / resolution;

		if (pixelWidth < 0.1 || pixelHeight < 0.1)
		{
			PrintFormat("Invalid pixel size (%1×%2)", pixelWidth, pixelHeight, level: LogLevel.WARNING);
			return false;
		}

		array<int> imageData = {}; // temporarily used to store entity count first
		array<ref set<ResourceName>> pixelPrefabs = {};
		imageData.Resize(resolution * resolution);
		pixelPrefabs.Resize(resolution * resolution);

		Debug.BeginTimeMeasure();

		set<ResourceName> modelsSet;
		foreach (int i, vector prefabPosition : prefabPositions)
		{
			// do not round - pixel 0,0 is for 0..0.99 entities

			prefabPosition -= terrainMin;

			int x = prefabPosition[0] / pixelWidth;
			if (x < 0 || x >= resolution)
				continue;

			int z = resolution - prefabPosition[2] / pixelHeight;
			if (z < 0 || z >= resolution)
				continue;

			modelsSet = pixelPrefabs[x + resolution * z];
			if (!modelsSet)
			{
				modelsSet = new set<ResourceName>();
				pixelPrefabs[x + resolution * z] = modelsSet;
			}

			modelsSet.Insert(models[i]);

			int value = modelsSet.Count();
			imageData[x + resolution * z] = value;
		}

		Debug.EndTimeMeasure(
			string.Format(
				"Processing all %1 entity positions for image creation (%2×%2 = %3 pixels)",
				prefabPositions.Count(),
				resolution,
				resolution * resolution));

		return SCR_HeatmapHelper.CreateHeatmapImageFromData(imagePath, imageData);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Analyse world", true)]
	protected int ButtonAnalyse()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected int ButtonClose()
	{
		return 0;
	}
}

class SCR_WorldEntitiesStatisticsPlugin_Report_World
{
	float m_fTerrainSurface;
	float m_fTerrainSurfaceASLRatio;
	vector m_vTerrainMin;
	vector m_vTerrainMax;
}

class SCR_WorldEntitiesStatisticsPlugin_Report
{
	ref SCR_WorldEntitiesStatisticsPlugin_Report_World m_WorldInfo;

	int m_iSubSceneId;
	string m_sSubSceneName;

	int m_iLayerId;
	string m_sLayerName;	//!< empty = global

	// density
	ref array<vector> m_aAllEntityPositions = {};

	// variety
	ref array<ResourceName> m_aModels = {};
	ref array<vector> m_aModelPositions = {};

	// statis
	ref map<ResourceName, int> m_mPrefabResult = new map<ResourceName, int>();
	ref map<ResourceName, int> m_mPrefabNoAncestorResult = new map<ResourceName, int>();
	ref map<ResourceName, int> m_mEmptyResult = new map<ResourceName, int>();
	ref map<ResourceName, int> m_mGenericResult = new map<ResourceName, int>(); // should ideally be map<string, int>

	// TODO?
//	ref map<ResourceName, ref array<ref array<int>>> m_mPrefabsPresenceMap = new map<ResourceName, ref array<ref array<int>>>();
}

class SCR_WorldEntitiesStatisticsPlugin_Item
{
	string m_sClassName;
	ResourceName m_sResourceName;

	int m_iSubScene;
	int m_iLayer;

	vector m_vPosition;
}
#endif // WORKBENCH
