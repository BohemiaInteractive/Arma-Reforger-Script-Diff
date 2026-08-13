#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: PLUGIN_NAME,
	description: "See terrain slopes.",
	shortcut: "",
	wbModules: { "WorldEditor" },
	category: "Terrain",
	awesomeFontCode: 0xF1B3)]
class SCR_TerrainSlopeViewerPlugin : WorldEditorPlugin
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "GetHeightmap mode - under, over water, or both", enums: SCR_ParamEnumArray.FromString("Everything;Above water only;Underwater only"))]
	protected int m_iMode; //!< 0 = everything, 1 = above water, 2 = underwater

	[Attribute(defvalue: "0", desc: "Force reobtaining terrain's elevation - useful after terrain edits")]
	protected bool m_bForceHeightmapRefresh;

	[Attribute(defvalue: "1", desc: "Step modulo (for big terrains, 1 every X cell)", params: "1 inf")]
	protected int m_iStepModulo;

	protected ref array<float> m_aHeightmap;
	protected string m_sLastLoadedWorld;

//	protected static const int MODE_ALL_TERRAIN = 0;
	protected static const int MODE_ABOVE_WATER = 1;
	protected static const int MODE_UNDER_WATER = 2;

	protected static const float DEBUG_SHAPE_HEIGHT = 20;

	protected static const string PLUGIN_NAME = "Terrain Slope Viewer";

	//------------------------------------------------------------------------------------------------
	override protected void Run()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);

		if (!worldPath)
		{
			SCR_WorkbenchHelper.PrintDialog("A world must be loaded first.", PLUGIN_NAME, LogLevel.WARNING);
			return;
		}

		if (!SCR_WorldEditorToolHelper.HasTerrainMesh())
		{
			SCR_WorkbenchHelper.PrintDialog("This world does not have a terrain or its heightmap is not generated.", PLUGIN_NAME, LogLevel.WARNING);
			return;
		}

		if (Workbench.ScriptDialog(PLUGIN_NAME, "", this) == 0)
			return;

		if (m_bForceHeightmapRefresh || !m_aHeightmap || m_sLastLoadedWorld != worldPath)
		{
			WBProgressDialog progress = new WBProgressDialog("Gathering terrain heightmap, please wait...", Workbench.GetModule(WorldEditor));
			progress.SetProgress(0.42);
//			m_aHeightmap = SCR_WorldEditorToolHelper.GetTerrainHeightmap();
			m_aHeightmap = null;
			if (!m_aHeightmap)
			{
				SCR_WorkbenchHelper.PrintDialog("Cannot load terrain's heightmap.", PLUGIN_NAME, LogLevel.ERROR);
				return;
			}

			worldEditorAPI.GetWorldPath(m_sLastLoadedWorld);
		}

		if (Workbench.ScriptDialog(PLUGIN_NAME, "You are about to process " + m_aHeightmap.Count() + " vertices; continue?", new WorkbenchDialog_OKCancel()) == 0)
			return;

		array<float> resultAngles = GetSlope090AngleRatios();
		if (!resultAngles)
			return;

		array<int> resultAngles0255 = {};
		resultAngles0255.Resize(resultAngles.Count());

		float maxAngle;
		foreach (int i, float angle : resultAngles)
		{
			if (maxAngle < angle)
				maxAngle = angle;

			resultAngles0255[i] = Math.Round(angle * 255);
		}

		Print("max angle = " + maxAngle);

		// TODO: debug
//		if (SCR_HeatmapHelper.CreateHeatmapImageFromData(
//			"SLOPE_VIEWER_median.dds",
//			resultAngles0255,
//			colourMode: SCR_HeatmapHelper.COLOUR_MODE_THERMAL,
//			maxValueMode: SCR_HeatmapHelper.MAX_MODE_2MEDIAN))
		if (SCR_HeatmapHelper.CreateHeatmapImageFromData("SLOPE_VIEWER.dds", resultAngles0255))
			Print("success");
		else
			Print("failure");
	}

	//------------------------------------------------------------------------------------------------
	//! in 0..1 between flat and wall cell
	protected array<float> GetSlope090AngleRatios()
	{
		array<float> result = {};
		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		BaseWorld world = worldEditorAPI.GetWorld();

		int mode;
		bool hasOcean = world.IsOcean();
		if (!hasOcean && m_iMode == MODE_UNDER_WATER)
		{
			SCR_WorkbenchHelper.PrintDialog("Temporarily changing mode from underwater to all terrain, provided this terrain has no ocean.", PLUGIN_NAME, LogLevel.NORMAL);
			mode = 0;
		}
		else
		{
			mode = m_iMode;
		}

		float oceanLevel;
		if (hasOcean)
			oceanLevel = world.GetOceanBaseHeight();

		int verticesXMinus1 = worldEditorAPI.GetTerrainResolutionX() - 1;
		int verticesZMinus1 = worldEditorAPI.GetTerrainResolutionY() - 1;
		int verticesXMinus2 = verticesXMinus1 - 1;
		int verticesZMinus2 = verticesZMinus1 - 1;
		float step = worldEditorAPI.GetTerrainUnitScale();

		int totalPoints = verticesXMinus1 * verticesZMinus1;
		result.Resize(totalPoints);

		WBProgressDialog progress = new WBProgressDialog("Gathering angles...", worldEditor);

		int progressStep, progressStepLimit = totalPoints * 0.01;
		Debug.BeginTimeMeasure();

		TraceParam traceParam = new TraceParam();
		traceParam.Flags = TraceFlags.WORLD;
		for (int z = 1; z < verticesZMinus2; ++z)
		{
			if ((z - 1) % m_iStepModulo != 0)
			{
				progressStep += verticesZMinus1;
				continue;
			}

			for (int x; x < verticesXMinus2; ++x)
			{
				++progressStep;
				if (progressStep >= progressStepLimit) // min 1%
				{
					progress.SetProgress((verticesXMinus1 * z + x) / totalPoints); // expensive
					progressStep = 0;
				}

				if (x % m_iStepModulo != 0)
					continue;

				int indexBL = verticesZMinus1 * z + x;
				int indexBR = indexBL + 1;
				int indexTL = verticesZMinus1 * (z - 1) + x;
				int indexTR = indexTL + 1;

				float yBL = m_aHeightmap[indexBL];
				float yBR = m_aHeightmap[indexBR];
				float yTL = m_aHeightmap[indexTL];
				float yTR = m_aHeightmap[indexTR];

				float yAvg = (yBL + yBR + yTL + yTR) * 0.25;
				if (mode == MODE_ABOVE_WATER)
				{
//					float yAvg = (yBL + yBR + yTL + yTR) * 0.25;
					if (yAvg < oceanLevel)
						continue;
				}
				else
				if (mode == MODE_UNDER_WATER)
				{
//					float yAvg = (yBL + yBR + yTL + yTR) * 0.25;
					if (yAvg >= oceanLevel)
						continue;
				}

				// get normal
				vector normal;

//				vector vBLTL = { 0, yTL - yBL, step };
//				vector vBLBR = { step, yBR - yBL, 0 };
//				normal = vBLTL * vBLBR;
//				if (normal[1] < 0)
//					normal = -normal;

				traceParam.Start = { (x + 0.5) * step, yAvg + 1, (z + 0.5) * step };
				traceParam.End = traceParam.Start;
				traceParam.End[1] = yAvg - 1;

				world.TraceMove(traceParam);

				normal = traceParam.TraceNorm;

				// get angle value
				float angleRad = Math.Acos(vector.Dot(normal, vector.Up));

				// set angle value
				float value01 = Math.InverseLerp(0, Math.PI_HALF, angleRad);
				if (value01 == 0)
					continue;

				if (value01 < 0)
				{
					Print("value01 < 0 " + value01, LogLevel.NORMAL);
					return null;
				}

				if (value01 > 1)
				{
					Print("value01 > 1 " + value01, LogLevel.NORMAL);
					return null;
				}

				int imgIndex = verticesZMinus1 * (verticesZMinus1 - z) + x;
				result[imgIndex] = value01;
			}
		}

		Debug.EndTimeMeasure("Comparing angles");

		return result;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Process", true)]
	protected int ButtonProcess()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}
#endif
