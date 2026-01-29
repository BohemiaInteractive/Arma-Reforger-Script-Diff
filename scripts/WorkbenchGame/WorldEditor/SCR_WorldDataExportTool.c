#ifdef WORKBENCH
enum SCR_EMapDataExportType
{
	GEOMETRY_2D,
	SOUNDMAP,
	RASTERIZATION
}

/*!
Exports data of the current map based on selected type.
*/

//------------------------------------------------------------------------------------------------
[WorkbenchToolAttribute("Export Map data", "Export data of various types from the current world", "", awesomeFontCode: 0xF0AC)]
class SCR_WorldMapExportTool : WorldEditorTool
{
	/*
		Category: Export File
	*/

	[Attribute(desc: "Folder where the file will be saved", params: "unregFolders", category: "Export File")]
	protected ResourceName m_sDestinationPath;

	[Attribute("0", UIWidgets.ComboBox, "Export type", enumType: SCR_EMapDataExportType, category: "Export File")]
	protected SCR_EMapDataExportType m_eType;

	/*
		Category: Geometry 2D
	*/

	[Attribute("1", UIWidgets.CheckBox, "Generation of Roads' data", category: "Geometry 2D")]
	protected bool m_bGenerateRoads;

	[Attribute("1", UIWidgets.CheckBox, "Save Roads' data to exported file", category: "Geometry 2D")]
	protected bool m_bSaveRoads;

	[Attribute("1", UIWidgets.CheckBox, "Generation of Powerlines' data", category: "Geometry 2D")]
	protected bool m_bGeneratePowerLines;

	[Attribute("1", UIWidgets.CheckBox, "Save Powerlines' data to exported file", category: "Geometry 2D")]
	protected bool m_bSavePowerLines;

	[Attribute("1", UIWidgets.CheckBox, "Generation of Buildings' data", category: "Geometry 2D")]
	protected bool m_bGenerateBuildings;

	[Attribute("1", UIWidgets.CheckBox, "Save Buildings' data to exported file", category: "Geometry 2D")]
	protected bool m_bSaveBuildings;

	[Attribute("1", UIWidgets.CheckBox, "Generation of Areas' data", category: "Geometry 2D")]
	protected bool m_bGenerateAreas;

	[Attribute("1", UIWidgets.CheckBox, "Ignore Generator entities during generation", category: "Geometry 2D")]
	protected bool m_bIgnoreGeneratorAreas;

	[Attribute("1", UIWidgets.CheckBox, "Save Areas' data to exported file", category: "Geometry 2D")]
	protected bool m_bSaveAreas;

	[Attribute("1", UIWidgets.CheckBox, "Generation of Water Bodies' data", category: "Geometry 2D")]
	protected bool m_bGenerateWaterBodies;

	[Attribute("1", UIWidgets.CheckBox, "Save Water Bodies' data to exported file", category: "Geometry 2D")]
	protected bool m_bSaveWaterBodies;

	[Attribute("1", UIWidgets.CheckBox, "Generation of Hills' data", category: "Geometry 2D")]
	protected bool m_bGenerateHills;

	[Attribute("50.0", UIWidgets.Slider, "Minimum hill height", "30 155 5", category: "Geometry 2D")]
	protected float m_fHillMinimumHeight;

	[Attribute("1", UIWidgets.CheckBox, "Save Hills' data to exported file", category: "Geometry 2D")]
	protected bool m_bSaveHills;

	/*
		Category: Rasterization
	*/

	[Attribute("1.0 1.0 1.0 1.0", UIWidgets.ColorPicker, "Land color Bright", category: "Rasterization")]
	protected ref Color m_LandColorBright;

	[Attribute("0.5 0.5 0.5 1.0", UIWidgets.ColorPicker, "Land color Dark", category: "Rasterization")]
	protected ref Color m_LandColorDark;

	[Attribute("0.863 0.980 1.0 1.0", UIWidgets.ColorPicker, "Ocean color Bright", category: "Rasterization")]
	protected ref Color m_OceanColorBright;

	[Attribute("0.757 0.91 0.929 1.0", UIWidgets.ColorPicker, "Ocean color Dark", category: "Rasterization")]
	protected ref Color m_OceanColorDark;

	[Attribute("2.5", UIWidgets.Slider, "Land scale factor", "0 10 0.1", category: "Rasterization")]
	protected float m_fLandScale;

	[Attribute("1.2", UIWidgets.Slider, "Ocean scale factor", "0 10 0.1", category: "Rasterization")]
	protected float m_fOceanScale;

	[Attribute("500.0", UIWidgets.Slider, "Height scale factor", "-1000 1000 1.0", category: "Rasterization")]
	protected float m_fHeightScale;

	[Attribute("60.0", UIWidgets.Slider, "Depth scale factor", "-1000 1000 1.0", category: "Rasterization")]
	protected float m_fDepthScale;

	[Attribute("-50.0", UIWidgets.Slider, "Depth Lerp meters", "-1000 1000 1.0", category: "Rasterization")]
	protected float m_fDepthLerpMeters;

	[Attribute("0.5", UIWidgets.Slider, "Shade intensity factor", "0.5 15 0.01", category: "Rasterization")]
	protected float m_fShadeIntensity;

	[Attribute("1.8", UIWidgets.Slider, "Height intensity factor", "0.5 15 0.01", category: "Rasterization")]
	protected float m_fHeightIntensity;

	[Attribute("1", UIWidgets.CheckBox, "Include Areas defined by Generators", category: "Rasterization")]
	protected bool m_bIncludeGeneratorAreas;

	[Attribute("0.608 0.784 0.529 1.0", UIWidgets.ColorPicker, "Forest area color", category: "Rasterization")]
	protected ref Color m_ForestAreaColor;

	[Attribute("1.25", UIWidgets.Slider, "Forest area intensity factor", "0.5 15 0.01", category: "Rasterization")]
	protected float forestAreaIntensity;

	[Attribute("0.745 0.745 0.745 1.0", UIWidgets.ColorPicker, "Other area color", category: "Rasterization")]
	protected ref Color m_OtherAreaColor;

	[Attribute("1.0", UIWidgets.Slider, "Other area intensity factor", "0.5 15 0.01", category: "Rasterization")]
	protected float m_fOtherAreaIntensity;

	protected ref MapDataExporter m_DataExporter = new MapDataExporter();

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Export")]
	protected void Execute()
	{
		Export();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] errorCode
	//! \return errorCode's error message
	static string GetReportMessage(DataExportErrorType errorCode)
	{
		switch (errorCode)
		{
			case DataExportErrorType.DataExportErrorNoGame: return "Game not found";
			case DataExportErrorType.DataExportErrorNoDataType: return "Data type not found";
			case DataExportErrorType.DataExportErrorNoMapEntity: return "Map Entity not found";
			case DataExportErrorType.DataExportErrorNoWorld: return "World not found";
			case DataExportErrorType.DataExportErrorNoWorldPath: return "World path empty";
			case DataExportErrorType.DataExportErrorNoOutputFile: return "Could not open output file";
			case DataExportErrorType.DataExportErrorNoCompatibleWorld: return "World is not compatible";
			case DataExportErrorType.DataExportErrorNoSoundWorld: return "Sound World not found";
			case DataExportErrorType.DataExportErrorNotSavingToFile: return "Not saving to file, Default Map created";
			case DataExportErrorType.DataExportErrorInvalidSoundmap: return "Invalid Soundmap";
			case DataExportErrorType.DataExportErrorNoLandOrOcean: return "No Land or Ocean color found";
			case DataExportErrorType.DataExportErrorNoTerrain: return "Terrain not found";
		}

		return "Unknown, code=" + errorCode;
	}

	//------------------------------------------------------------------------------------------------
	// Export
	protected bool Export()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);

		WBProgressDialog progress = new WBProgressDialog("Processing", worldEditor);

		string exportPath = m_sDestinationPath.GetPath();

		if (m_eType == SCR_EMapDataExportType.GEOMETRY_2D)
		{
			Print("Exporting map 2D map data...", LogLevel.NORMAL);
			ExcludeGenerateFlags generateFlags = ExcludeGenerateFlags.ExcludeGenerateNone;
			if (!m_bGenerateRoads)
				generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateRoads;

			if (!m_bGeneratePowerLines)
				generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGeneratePowerlines;

			if (!m_bGenerateBuildings)
				generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateBuildings;

			if (!m_bGenerateAreas)
				generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateAreas;

			if (!m_bGenerateWaterBodies)
				generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateWaterBodies;

			if (!m_bGenerateHills)
				generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateHills;

			ExcludeSaveFlags saveFlags = ExcludeSaveFlags.ExcludeSaveNone;
			if (!m_bSaveRoads)
				saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveRoads;

			if (!m_bSavePowerLines)
				saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSavePowerlines;

			if (!m_bSaveBuildings)
				saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveBuildings;

			if (!m_bSaveAreas)
				saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveAreas;

			if (!m_bSaveWaterBodies)
				saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveWaterBodies;

			if (!m_bSaveHills)
				saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveHills;

			DataExportErrorType result = m_DataExporter.ExportData(EMapDataType.Geometry2D, exportPath, worldPath, m_fHillMinimumHeight, m_bIgnoreGeneratorAreas, generateFlags, saveFlags);
			if (result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = GetReportMessage(result);
				Print(string.Format("Could not export 2D map data. (%1)", reason), LogLevel.ERROR);
				return false;
			}
		}
		else if (m_eType == SCR_EMapDataExportType.SOUNDMAP)
		{
			Print("Exporting Soundmap...", LogLevel.NORMAL);
			DataExportErrorType result = m_DataExporter.ExportData(EMapDataType.Soundmap, exportPath, worldPath);
			if (result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = GetReportMessage(result);
				Print(string.Format("Could not export Soundmap data. (%1)", reason), LogLevel.ERROR);
				return false;
			}
		}
		else if (m_eType == SCR_EMapDataExportType.RASTERIZATION)
		{
			Print("Exporting map rasterization...", LogLevel.NORMAL);
			DataExportErrorType result = m_DataExporter.SetupColors(m_LandColorBright, m_LandColorDark, m_OceanColorBright, m_OceanColorDark, m_ForestAreaColor, m_OtherAreaColor);
			if (result == DataExportErrorType.DataExportErrorNone)
				result = m_DataExporter.ExportRasterization(exportPath, worldPath, m_fLandScale, m_fOceanScale, m_fHeightScale, m_fDepthScale, m_fDepthLerpMeters, m_fShadeIntensity, m_fHeightIntensity, m_bIncludeGeneratorAreas, forestAreaIntensity, m_fOtherAreaIntensity);

			if (result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = GetReportMessage(result);
				Print(string.Format("Could not export map rasterization. (%1)", reason), LogLevel.ERROR);
				return false;
			}
		}

		return true;
	}
}

[WorkbenchPluginAttribute("World Data Export Plugin", "Open world and export defined data type")]
class WorldDataExport : WorkbenchPlugin // TODO: SCR_WorldDataExportPlugin
{
	//------------------------------------------------------------------------------------------------
	// CLI arguments
	override void RunCommandline()
	{
		ResourceManager rb = Workbench.GetModule(ResourceManager);
		string path;
		rb.GetCmdLine("-world", path);
		string type;
		rb.GetCmdLine("-type", type);
		string dir;
		rb.GetCmdLine("-saveDir", dir);
		
		ExcludeGenerateFlags generateflags = ExcludeGenerateFlags.ExcludeGenerateNone;
		ExcludeSaveFlags saveFlags = ExcludeSaveFlags.ExcludeSaveNone;
		string tmp;
		if (rb.GetCmdLine("-excludehills", tmp))
		{
			generateflags = generateflags | ExcludeGenerateFlags.ExcludeGenerateHills;
			saveFlags = saveFlags  | ExcludeSaveFlags.ExcludeSaveHills;
		}

		if (ExportWorldData(path, type, dir, generateflags, saveFlags))
			Workbench.Exit(0);
		else
			Workbench.Exit(-1);
	}

	//------------------------------------------------------------------------------------------------
	// Export
	protected bool ExportWorldData(string path, string type, string dir, ExcludeGenerateFlags excludeFlags, ExcludeSaveFlags saveFlags)
	{
		Print("WorldDataExport: opening world " + path, LogLevel.VERBOSE);

		Workbench.OpenModule(WorldEditor);
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor.SetOpenedResource(path))
			return false;

		Sleep(300);

		bool success = true;
		bool exportMap = true;
		bool exportSound = true;
		bool exportRasterization = true;
		if (!type.IsEmpty())
		{
			if (type.StartsWith("2DMap"))
			{
				exportSound = false;
				exportRasterization = false;
			}
			else if (type.StartsWith("SoundMap"))
			{
				exportMap = false;
				exportRasterization = false;
			}
			else if (type.StartsWith("2DRasterization"))
			{
				exportSound = false;
				exportMap = false;
			}
		}

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);

		MapDataExporter mapDataExporter = new MapDataExporter();

		WBProgressDialog progress = new WBProgressDialog("Processing", worldEditor);
		if (exportMap)
		{
			Print("Exporting map 2D map data...", LogLevel.NORMAL);
			DataExportErrorType result = mapDataExporter.ExportData(EMapDataType.Geometry2D, dir, worldPath, 50, true, excludeFlags, saveFlags);
			if (result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = SCR_WorldMapExportTool.GetReportMessage(result);
				Print(string.Format("Could not export 2D map data. (%1)", reason), LogLevel.ERROR);
				success = false;
			}
		}

		if (exportSound)
		{
			Print("Exporting Soundmap...", LogLevel.NORMAL);
			DataExportErrorType result = mapDataExporter.ExportData(EMapDataType.Soundmap, dir, worldPath);
			if (result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = SCR_WorldMapExportTool.GetReportMessage(result);
				Print(string.Format("Could not export Soundmap data. (%1)", reason), LogLevel.ERROR);
				success = false;
			}
		}

		if (exportRasterization)
		{
			Print("Exporting map rasterization...", LogLevel.NORMAL);
			//TODO: change to user defined values (along with WorldMapExportTool)
			const Color landColorBright = new Color(1.0, 1.0, 1.0, 1.0);
			const Color landColorDark = new Color(0.5, 0.5, 0.5, 1.0);
			const Color oceanColorBright = new Color(0.863, 0.980, 1.0, 1.0);
			const Color oceanColorDark = new Color(0.757, 0.91, 0.929, 1.0);
			const float landScale = 2.5;
			const float oceanScale = 1.2;
			const float heightScale = 500.0;
			const float depthScale = 60.0;
			const float depthLerpMeters = -50.0;
			const float shadeIntensity = 0.5;
			const float heightIntensity = 1.8;
			const bool includeGeneratorAreas = true;
			const Color forestAreaColor = new Color(0.608, 0.784, 0.529, 1.0);
			const float forestAreaIntensity = 1.25;
			const Color otherAreaColor = new Color(0.745, 0.745, 0.745, 1.0);
			const float otherAreaIntensity = 1.0;

			DataExportErrorType result = mapDataExporter.SetupColors(landColorBright, landColorDark, oceanColorBright, oceanColorDark, forestAreaColor, otherAreaColor);
			if (result == DataExportErrorType.DataExportErrorNone)
				result = mapDataExporter.ExportRasterization(dir, worldPath, landScale, oceanScale, heightScale, depthScale, depthLerpMeters, shadeIntensity, heightIntensity, includeGeneratorAreas, forestAreaIntensity, otherAreaIntensity);

			if (result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = SCR_WorldMapExportTool.GetReportMessage(result);
				Print(string.Format("Could not export map rasterization. (%1)", reason), LogLevel.ERROR);
				success = false;
			}
		}

		if (success)
			Print("WorldDataExport: export successful!", LogLevel.VERBOSE);

		Sleep(100);
		worldEditor.Close();

		return success;
	}
}
#endif // WORKBENCH
