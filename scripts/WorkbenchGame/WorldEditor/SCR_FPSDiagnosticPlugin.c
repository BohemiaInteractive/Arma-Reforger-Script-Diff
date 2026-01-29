#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "FPS Diagnostic",
	description: "Collect FPS all over the terrain and create a heatmap of it",
	shortcut: "Ctrl+Alt+Shift+F",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF625)]
class SCR_FPSDiagnosticPlugin : WorldEditorPlugin
{
	//
	//	Camera
	//

	[Attribute(category: "Camera", defvalue: "0 " + CAMERA_DEFAULT_ALTITUDE + " 0", desc: "Camera offset from terrain/ocean position (default " + CAMERA_DEFAULT_ALTITUDE + "m above terrain/ocean)")]
	protected vector m_vPositionOffset;

	[Attribute(category: "Camera", defvalue: "0", desc: "If surface is ocean, skip the check")]
	protected bool m_bAnalyseOnOcean;

	[Attribute(category: "Camera", defvalue: "0",
		desc: "If checked, positions are randomised, otherwise they are done in order (so the area is usually preloaded)"
			+ "\nDoes not work if the total amount of positions is greater than " + SCR_Math.MAX_RANDOM)]
	protected bool m_bRandomisePositions;

	[Attribute(category: "Camera", uiwidget: UIWidgets.Slider, desc: "Camera angles in degrees from which to get FPS\nX: 0 = horizon, 90 = look straight up, -30 = look down 30°, etc\nY: 0 = North, -90/270 = West, etc\nZ: banking angle"
		+ "\n\nIf left empty, 4 cardinal direction cameras with " + CAMERA_DEFAULT_PITCH + " degrees pitch will be used", params: "-180 360")]
	protected ref array<vector> m_aOrientations;

	[Attribute(category: "Camera", defvalue: "2", desc: "Delay in seconds before capture starts (to allow proper loading and setting fullscreen)", params: "0 30 0.5", precision: 1)]
	protected float m_fStartDelay;

	[Attribute(category: "Camera", defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "wait time per scene after the camera is moved", params: "0.1 30 0.1", precision: 2)]
	protected float m_fScenePause;

	//
	//	Heatmap
	//

//	[Attribute(category: "Heatmap", defvalue: "1", desc: "Generate heatmap")]
//	protected bool m_bGenerateHeatmap;

	[Attribute(category: "Heatmap", defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "- Greyscale: from black to white\n- Thermal: from blue to green to red\n- Alpha: from transparent to white", enums: SCR_ParamEnumArray.FromString("Greyscale,From black to white;Thermal,From blue to green to red;Alpha,From transparent to white"))]
	protected int m_iHeatmapColourMode;

	[Attribute(category: "Heatmap", defvalue: "0", desc: "Invert pixel value (except for max value), e.g black = most dense, white = least dense instead of the opposite")]
	protected bool m_bHeatmapValueInversion;

	[Attribute(category: "Heatmap", defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "This setting can prevent a density peak from \"darkening\" the image everywhere else - it also offers to highlight pixels above the 0..2×median range", enums: SCR_ParamEnumArray.FromString("Raw value;2 × average;2 × median"))]
	protected int m_iHeatmapMaxValueMode;

	[Attribute(category: "Heatmap", defvalue: "1", desc: "Highlight values above max (when Heat Map Max Value Mode is not Raw)\n- Greyscale: red pixels\n- Thermal: white pixels")]
	protected bool m_bHeatmapHighlightValuesAboveMax;

	[Attribute(category: "Heatmap", defvalue: DEFINITION_DEFAULT.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Number of detection squares in terrain height/width", enums: SCR_ParamEnumArray.FromString("4,4×4,;8,8×8,;16,16×16,;32,32×32,;64,64×64,;128,128×128,;256,256×256,;512,512×512,;1024,1024×1024,;2048,2048×2048,;4096,4096×4096,;1080,1080×1080 (wallpaper),;1440,1440×1440 (wallpaper),;2160,2160×2160 (wallpaper),"))]
	protected int m_iHeatmapDefinition;

	[Attribute(category: "Heatmap", defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Definition multiplier to obtain a bigger image - e.g ×2 = 1 data point takes 2×2 (4) pixels\nMaximum resolution allowed: " + SCR_HeatmapHelper.MAX_RESOLUTION, enums: SCR_ParamEnumArray.FromString(SCR_HeatmapHelper.GetResolutionFactorEnum()))]
	protected int m_iHeatmapResolutionFactor;

	//
	//	Mode
	//

	[Attribute(category: "Mode", defvalue: "1", desc: "Use Game mode instead of Workbench mode (Workbench has performance overhead due to edit mode)\nIgnored when Fake Data is used")]
	protected bool m_bUseGameMode;

	[Attribute(category: "Mode", defvalue: "1", desc: "Use fullscreen (if Use Game Mode above is ticked)")]
	protected bool m_bUseFullScreen;

	//
	//	Misc
	//

	[Attribute(category: "Misc", defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Open the heatmap/directory once generated", enums: SCR_ParamEnumArray.FromString("Open heatmap;Open heatmap directory;Open heatmap and directory"))]
	protected int m_iOpenHeatmap;

	[Attribute(category: "Misc", defvalue: "30", uiwidget: UIWidgets.Slider, desc: "Force progress bar estimate refresh and print a time estimate in the log console every X seconds - 0 = disabled (if used, progress bar still updates its estimate every percent)", params: "0 3600 30")]
	protected int m_iTimeEstimateFrequency;

	//
	//	Debug
	//

	[Attribute(category: "Debug", defvalue: "0", desc: "Use fake (randomised) data - useful to debug water detection\nIgnores Use Game Mode (Workbench itself generates the fake data)")]
	protected bool m_bUseFakeData;

	protected bool m_bInternalUseGameMode;
	protected int m_iLastEstimateTick;

	protected static const float CAMERA_DEFAULT_ALTITUDE = 7.5;
	protected static const float CAMERA_DEFAULT_PITCH = -10;
	protected static const int CAMERA_DEFAULT_ANGLE_COUNT = 4; // N, E, S, W

	protected static const int DEFINITION_DEFAULT = 64;

	protected static const float FPS_UNFOCUSED = 3.0;			//!< if FPS are equal to this value, the capture ends
	protected static const float FPS_UNFOCUSED_DELTA = 0.01;	//!< delta used in float.AlmostEqual
	protected static const float FPS_UNFOCUSED_DURATION_MS = 0;	//!< how much time must FPS stay at FPS_UNFOCUSED to trigger cancellation
	protected static const int MAX_FPS = 500;					//!< if FPS are above this threshold, FPS is reobtained (Workbench bug)
	protected static const float FPS_HIGH_DURATION_MS = 2000;	//!< if FPS are above the threshold for more than this delay (s), cancel
	protected static const int FPS_CHECK_FREQUENCY_MS = 100;	//!< waiting time between FPS measurements when the threshold is crossed

	protected static const int FPS_RANDOM_MIN = 0;
	protected static const int FPS_RANDOM_MID = 45;
	protected static const int FPS_RANDOM_MAX = 60;

	protected static const float ESTIMATE_FACTOR = 1.10;	//!< by how much the theoretical duration must be multiplied

	//! %1 = world name, %2 = colour mode (BW, RGB), %3 = image's definition
	protected static const string OUTPUT_HEATMAP_NAME = "Heatmap_FPS_%1_%2_%3x%3.dds";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
		{
			SCR_WorkbenchHelper.PrintDialog("World Editor is not available", level: LogLevel.ERROR);
			return;
		}

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
		{
			SCR_WorkbenchHelper.PrintDialog("World Editor API is not available", level: LogLevel.ERROR);
			return;
		}

		if (!SCR_WorldEditorToolHelper.IsWorldLoaded())
		{
			SCR_WorkbenchHelper.PrintDialog("Be sure to load a world (or have it saved) before using the FPS Diagnostic plugin.");
			return;
		}

		if (worldEditor.IsPrefabEditMode())
		{
			SCR_WorkbenchHelper.PrintDialog("Be sure to load a world (or have it saved) before using the FPS Diagnostic plugin.");
			return;
		}

		IEntitySource terrainEntity = SCR_WorldEditorToolHelper.GetTerrainEntitySource();
		if (!terrainEntity)
		{
			SCR_WorkbenchHelper.PrintDialog("Be sure to have a terrain entity (of type GenericTerrainEntity) before using the FPS Diagnostic plugin.");
			return;
		}

		if (Workbench.ScriptDialog("FPS Diagnostic", "", this) == 0)
			return;

		vector terrainOrigin;
		if (!terrainEntity.Get("coords", terrainOrigin))
			return;

		vector terrainDimensions = SCR_WorldEditorToolHelper.GetTerrainDimensions();

		if (m_iHeatmapDefinition < 1)
			m_iHeatmapDefinition = DEFINITION_DEFAULT;

		float pixelWidth = terrainDimensions[0] / m_iHeatmapDefinition;
		float pixelHeight = terrainDimensions[2] / m_iHeatmapDefinition;

		if (pixelWidth < 0.1 || pixelHeight < 0.1)
		{
			SCR_WorkbenchHelper.PrintFormatDialog("Invalid pixel size (%1×%2) - try setting a lower definition.", pixelWidth.ToString(), pixelHeight.ToString(), level: LogLevel.WARNING);
			return;
		}

		BaseWorld world = worldEditorAPI.GetWorld();

		bool isOceanEnabled = world.IsOcean();
		float oceanBaseHeight;
		if (isOceanEnabled)
			oceanBaseHeight = world.GetOceanBaseHeight();

		array<int> oceanIndices;
		if (!m_bAnalyseOnOcean && isOceanEnabled)
			oceanIndices = {};

		map<int, vector> positionsMap = new map<int, vector>();
		int pixelIndex;
		for (int z = m_iHeatmapDefinition - 1; z >= 0; --z)
		{
			float zPos = terrainOrigin[2] + (z + 0.5) * pixelHeight;
			for (int x; x < m_iHeatmapDefinition; ++x)
			{
				float xPos = terrainOrigin[0] + (x + 0.5) * pixelWidth;

				float yPos = worldEditorAPI.GetTerrainSurfaceY(xPos, zPos);
				if (!m_bAnalyseOnOcean && isOceanEnabled && yPos < oceanBaseHeight) // allow ocean's base height
					oceanIndices.Insert(pixelIndex);

				positionsMap.Insert(pixelIndex, { xPos, yPos, zPos });
				++pixelIndex;
			}
		}

		int positionsCount = positionsMap.Count();
		if (positionsCount < 1)
		{
			SCR_WorkbenchHelper.PrintDialog("No relevant positions were found - try setting a higher definition.");
			return;
		}

		int oceanIndicesCount;
		if (oceanIndices)
			oceanIndicesCount = oceanIndices.Count();

		int relevantPositionsCount = positionsCount - oceanIndicesCount;
		if (relevantPositionsCount < 1)
		{
			SCR_WorkbenchHelper.PrintDialog("No non-water positions were found - try setting a higher definition.");
			return;
		}

		int orientationsCount = m_aOrientations.Count();
		if (orientationsCount < 1)
		{
			if (CAMERA_DEFAULT_ANGLE_COUNT < 1)
			{
				SCR_WorkbenchHelper.PrintDialog("No orientations provided - add at least one camera orientation.");
				return;
			}

			for (int i; i < CAMERA_DEFAULT_ANGLE_COUNT; ++i)
			{
				m_aOrientations.Insert({ CAMERA_DEFAULT_PITCH, i * 360 / CAMERA_DEFAULT_ANGLE_COUNT, 0 });
				++orientationsCount;
			}
		}

		int relevantScenesCount = relevantPositionsCount * orientationsCount;
		if (relevantScenesCount < 1)
		{
			SCR_WorkbenchHelper.PrintDialog("No scenes created - no proper positions found.");
			return; // safety
		}

		string captionPrefix;
		if (m_bUseFakeData)
			captionPrefix = "[DEBUG] ";

		if (Workbench.ScriptDialog(
			captionPrefix + "FPS Diagnostic",
			string.Format(
				"The plugin will process %1 positions in %2 angle(s) for a total of %3 scenes (every %4m)",
				relevantPositionsCount,
				orientationsCount,
				relevantScenesCount,
				pixelWidth.ToString(-1, 1))
			+ string.Format(
				"\nHeatmap definition: %1×%1 stretched ×%2 to %3×%3",
				m_iHeatmapDefinition,
				m_iHeatmapResolutionFactor,
				m_iHeatmapDefinition * m_iHeatmapResolutionFactor)
			+ string.Format(
				"\n\n%1 estimated duration with %2s waiting time per scene (theoretical duration ×%3)"
				+ "\n\nDo NOT touch or close the Workbench during that time"
				+ "\nDo NOT unfocus the Workbench (unless -forceUpdate is used or you want to abort the benchmark)"
				+ "\n\nA waiting time of %4s will happen before the benchmark begins.",
				SCR_FormatHelper.FormatTime(m_fStartDelay + relevantScenesCount * m_fScenePause * ESTIMATE_FACTOR),
				m_fScenePause,
				ESTIMATE_FACTOR,
				m_fStartDelay),
				new SCR_OKCancelWorkbenchDialog()) == 0)
			return;

		float lowestFPS = float.MAX;
		float highestFPS;
		float addedFPS;
		array<float> fpsArray = {};
		fpsArray.Resize(positionsCount * orientationsCount);

		int width, height;
		array<int> resolutionList = {};
		resolutionList.Reserve(relevantScenesCount * 2); // not resize

		int validPositionsDone;

		// randomisation
		array<int> keys = SCR_MapHelperT<int, vector>.GetKeys(positionsMap);
		if (m_bRandomisePositions)
		{
			 if (positionsCount <= SCR_Math.MAX_RANDOM)
				SCR_ArrayHelperT<int>.Shuffle(keys);
			else
				PrintFormat("Too many (%1 > %2) positions to randomise", positionsCount, SCR_Math.MAX_RANDOM, level: LogLevel.WARNING);
		}

		// progress and estimate
		string estimatedDuration = SCR_FormatHelper.FormatTime(relevantScenesCount * m_fScenePause * ESTIMATE_FACTOR);
		Print("Estimated duration: " + estimatedDuration, LogLevel.NORMAL);

		float prevProgress, currProgress;
		string progressText;
		if (!m_bInternalUseGameMode)
		{
			if (m_bUseFakeData)
				progressText = "[DEBUG] Generating scenes's performance...\nEstimated time left: %1";
			else
				progressText = "Capturing scenes's performance...\nEstimated time left: %1";
		}

		WBProgressDialog progress;

		m_bInternalUseGameMode = m_bUseGameMode && !m_bUseFakeData;		

		vector cameraPos, traceEnd, cameraDir;
		int screenWidth = worldEditorAPI.GetScreenWidth();
		int screenHeight = worldEditorAPI.GetScreenHeight();;
		if (!m_bInternalUseGameMode) // WorkbenchCamera
			worldEditorAPI.TraceWorldPos(screenWidth * 0.5, screenHeight * 0.5, TraceFlags.WORLD, cameraPos, traceEnd, cameraDir);

		CameraBase camera;

		// enter game mode
		if (m_bInternalUseGameMode)
		{
			worldEditor.SwitchToGameMode(fullScreen: m_bUseFullScreen);

			while (!worldEditorAPI.IsGameMode())
			{
				if (float.AlmostEqual(System.GetFPS(), FPS_UNFOCUSED))
				{
					Print("Switching to Game mode got unfocused - capture cancelled", LogLevel.ERROR);
					worldEditor.SwitchToEditMode();
					return;
				}

				Sleep(100); // needed
			}

			SCR_CameraManager cameraManager = SCR_CameraManager.Cast(GetGame().GetCameraManager());
			if (!cameraManager)
			{
				SCR_WorkbenchHelper.PrintDialog("Camera Manager cannot be found", level: LogLevel.ERROR);
				worldEditor.SwitchToEditMode();
				return;
			}

			camera = CameraBase.Cast(GetGame().SpawnEntity(CameraBase));
			if (!camera)
			{
				SCR_WorkbenchHelper.PrintDialog("Camera cannot be created", level: LogLevel.ERROR);
				worldEditor.SwitchToEditMode();
				return;
			}

			cameraManager.SetCamera(camera);
		}
		else
		{
			progress = new WBProgressDialog(string.Format(progressText, estimatedDuration), worldEditor);
		}

		if (!m_bUseFakeData && m_fStartDelay > 0)
		{
			PlaceCamera(worldEditorAPI, camera, terrainOrigin + m_vPositionOffset, -vector.Up);

			Print("Starting in " + m_fStartDelay + "s, unfocus the Workbench to cancel", LogLevel.NORMAL);
			if (!WaitFocused(m_fStartDelay))
			{
				PlaceCamera(worldEditorAPI, camera, cameraPos, cameraDir);
				Sleep(1);
				SCR_WorkbenchHelper.PrintDialog("Workbench was unfocused - capture cancelled");
				if (m_bInternalUseGameMode)
					worldEditor.SwitchToEditMode();

				return;
			}

			Print("Starting", LogLevel.NORMAL);
		}

		if (float.AlmostEqual(System.GetFPS(), FPS_UNFOCUSED, FPS_UNFOCUSED_DELTA))
		{
			SCR_WorkbenchHelper.PrintDialog("Workbench is unfocused on start - capture cancelled");
			if (m_bInternalUseGameMode)
				worldEditor.SwitchToEditMode();

			return;
		}

		const int startTime = System.GetTickCount();
		m_iLastEstimateTick = startTime;

		Debug.BeginTimeMeasure();

		// foreach (int index, vector position : positionsMap)
		foreach (int index : keys)
		{
			vector position = positionsMap.Get(index);

			if (m_iTimeEstimateFrequency > 0 && System.GetTickCount(m_iLastEstimateTick) >= m_iTimeEstimateFrequency * 1000)
			{
				int now = System.GetTickCount();
				int spentTime = now - startTime;

				if (currProgress > 0)
				{
					float timePerPercentage = (spentTime - m_fStartDelay) / currProgress;
					float estimatedRemaining = timePerPercentage * (1 - currProgress);

					if (progress) // refresh text - currently the only way is to regreate the progress dialog
					{
						progress = new WBProgressDialog(string.Format(progressText, SCR_FormatHelper.FormatTime(estimatedRemaining * 0.001 * ESTIMATE_FACTOR)), worldEditor);
						progress.SetProgress(currProgress);
					}

					PrintFormat(
						"Progress: %1%%; Spent time: %2 Estimated remaining time: %3",
						(currProgress * 100).ToString(-1, 2),
						SCR_FormatHelper.FormatTime(spentTime * 0.001),
						SCR_FormatHelper.FormatTime(estimatedRemaining * 0.001 * ESTIMATE_FACTOR),
						level: LogLevel.NORMAL);

					m_iLastEstimateTick = now;
				}
			}

			// avoid ocean if needed
			if (oceanIndices && oceanIndices.Contains(index))
			{
				for (int i; i < orientationsCount; ++i)
				{
					fpsArray[index * orientationsCount + i] = -1;
				}

				continue;
			}

			++validPositionsDone;
			currProgress = validPositionsDone / relevantPositionsCount;
			if (currProgress - prevProgress >= 0.01)		// min 1%
			{
				if (progress)
				{
					if (currProgress > 0)
					{
						int now = System.GetTickCount();
						int spentTime = now - startTime;
						float timePerPercentage = (spentTime - m_fStartDelay) / currProgress;
						float estimatedRemaining = timePerPercentage * (1 - currProgress);
						progress = new WBProgressDialog(string.Format(progressText, SCR_FormatHelper.FormatTime(estimatedRemaining * 0.001 * ESTIMATE_FACTOR)), worldEditor);
					}

					progress.SetProgress(currProgress);		// expensive
				}

				prevProgress = currProgress;
			}

			foreach (int orientationIndex, vector orientation : m_aOrientations)
			{
				float sceneFPS;
				if (m_bUseFakeData)
				{
					if (float.AlmostEqual(System.GetFPS(), FPS_UNFOCUSED))
					{
						SCR_WorkbenchHelper.PrintDialog("Workbench was unfocused - capture cancelled");
						worldEditor.SwitchToEditMode();
						return;
					}

					sceneFPS = SCR_Math.RandomGaussFloat(FPS_RANDOM_MIN, FPS_RANDOM_MID, FPS_RANDOM_MAX);
				}
				else
				{
					if (m_bInternalUseGameMode && !worldEditorAPI.IsGameMode())
					{
						SCR_WorkbenchHelper.PrintDialog("Workbench Game Mode was exited - capture cancelled");
						return;
					}

					// Yaw Pitch Roll to Pitch Yaw Roll
					float tmp = orientation[0];
					orientation[0] = orientation[1];
					orientation[1] = tmp;

					PlaceCamera(worldEditorAPI, camera, position + m_vPositionOffset, orientation.AnglesToVector());

					if (!WaitFocused(m_fScenePause))
					{
						PlaceCamera(worldEditorAPI, camera, position + m_vPositionOffset, orientation.AnglesToVector());
						Sleep(1);
						progress = null;
						SCR_WorkbenchHelper.PrintDialog("Workbench was unfocused - capture cancelled");
						if (m_bInternalUseGameMode)
							worldEditor.SwitchToEditMode();

						return;
					}

					PlaceCamera(worldEditorAPI, camera, position + m_vPositionOffset, orientation.AnglesToVector());
					sceneFPS = System.GetFPS();
					int waitTime = FPS_HIGH_DURATION_MS;
					while (sceneFPS > MAX_FPS)
					{
						Sleep(FPS_CHECK_FREQUENCY_MS);
						PlaceCamera(worldEditorAPI, camera, position + m_vPositionOffset, orientation.AnglesToVector());
						sceneFPS = System.GetFPS();
						waitTime -= FPS_CHECK_FREQUENCY_MS;
						if (waitTime < 1)
						{
							worldEditorAPI.SetCamera(cameraPos, cameraDir);
							Sleep(1);
							SCR_WorkbenchHelper.PrintFormatDialog("FPS > %1 for at least %2! Use VSync if needed", MAX_FPS.ToString(), (FPS_HIGH_DURATION_MS * 0.001).ToString(), level: LogLevel.ERROR);
							if (m_bInternalUseGameMode)
								worldEditor.SwitchToEditMode();

							return;
						}
					}

					waitTime = FPS_UNFOCUSED_DURATION_MS;
					while (float.AlmostEqual(sceneFPS, FPS_UNFOCUSED, FPS_UNFOCUSED_DELTA))
					{
						Sleep(FPS_CHECK_FREQUENCY_MS);
						PlaceCamera(worldEditorAPI, camera, position + m_vPositionOffset, orientation.AnglesToVector());
						sceneFPS = System.GetFPS();
						waitTime -= FPS_CHECK_FREQUENCY_MS;
						if (waitTime < 1)
						{
							PlaceCamera(worldEditorAPI, camera, cameraPos, cameraDir);
							Sleep(1);
							progress = null;
							SCR_WorkbenchHelper.PrintDialog("Workbench was unfocused - capture cancelled");							
							if (m_bInternalUseGameMode)
								worldEditor.SwitchToEditMode();

							return;
						}
					}
				}

				fpsArray[index * orientationsCount + orientationIndex] = sceneFPS;
				addedFPS += sceneFPS;

				if (sceneFPS < lowestFPS)
					lowestFPS = sceneFPS;
				else
				if (sceneFPS > highestFPS)
					highestFPS = sceneFPS;

				resolutionList.Insert(worldEditorAPI.GetScreenWidth());
				resolutionList.Insert(worldEditorAPI.GetScreenHeight());
			}
		}

		Debug.EndTimeMeasure("Measuring " + relevantScenesCount + " scenes");

		if (m_bInternalUseGameMode)
			worldEditor.SwitchToEditMode();
		else
			PlaceCamera(worldEditorAPI, camera, cameraPos, cameraDir);

		array<float> medianArray = {};
		medianArray.Copy(fpsArray);
		medianArray.RemoveItem(-1);
		medianArray.Sort();

		float medianFPS;
		if (!medianArray.IsEmpty())
			medianFPS = medianArray[medianArray.Count() * 0.5];

		Print("Average FPS: " + addedFPS / relevantScenesCount, LogLevel.NORMAL);
		Print("Median  FPS: " + medianFPS, LogLevel.NORMAL);
		Print("Highest FPS: " + highestFPS, LogLevel.NORMAL);
		Print("Lowest  FPS: " + lowestFPS, LogLevel.NORMAL);

		int resolutionListCount = resolutionList.Count();

		bool differentResolutions;
		for (int i; i < resolutionListCount; i += 2) // step 2
		{
			if (i == 0)
			{
				width = resolutionList[0];
				height = resolutionList[1];
			}
			else // compare
			{
				if (width != resolutionList[i] || height != resolutionList[i + 1])
				{
					differentResolutions = true;
					break;
				}
			}
		}

		if (differentResolutions)
		{
			int avgWidth, avgHeight;
			for (int i; i < resolutionListCount; i += 2) // step 2
			{
				avgWidth += resolutionList[i];
				avgHeight += resolutionList[i + 1];
			}

			PrintFormat("Resolution was changed during the FPS test! Started with %1×%2. Average resolution: %3×%4", screenWidth, screenHeight, avgWidth / relevantScenesCount, avgHeight / relevantScenesCount, level: LogLevel.WARNING);
		}
		else
		{
			PrintFormat("Resolution: %1×%2", width, height, level: LogLevel.NORMAL);
		}

		string colourMode;
		if (m_iHeatmapColourMode == SCR_HeatmapHelper.COLOUR_MODE_ALPHA)
			colourMode = "Alpha";
		else
		if (m_iHeatmapColourMode == SCR_HeatmapHelper.COLOUR_MODE_THERMAL)
			colourMode = "RGB";
		else
//		if (m_iHeatmapColourMode == SCR_HeatmapHelper.COLOUR_MODE_GREYSCALE) // default
			colourMode = "BW";

		if (m_bHeatmapValueInversion)
			colourMode += "inv";

		string worldName = SCR_WorldEditorToolHelper.GetWorldName();
		string fileName = string.Format(OUTPUT_HEATMAP_NAME, worldName, colourMode, m_iHeatmapDefinition);
		string absoluteFileName;
		if (!Workbench.GetAbsolutePath(fileName, absoluteFileName, false) || !CreateImage(absoluteFileName, m_iHeatmapDefinition, fpsArray))
		{
			Print("Heatmap cannot be created at " + absoluteFileName, LogLevel.ERROR);
			return;
		}

		Print("Heatmap successfully created at " + absoluteFileName, LogLevel.NORMAL);
		absoluteFileName.Replace("/", "\\");
		if (m_iOpenHeatmap == 1 || m_iOpenHeatmap == 2)
			Workbench.RunCmd("explorer \"" + FilePath.StripFileName(absoluteFileName) + "\"");

		if (m_iOpenHeatmap == 0 || m_iOpenHeatmap == 2)
			Workbench.RunCmd("explorer \"file:/" + SCR_StringHelper.DOUBLE_SLASH + absoluteFileName + "\"");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] secondsToWait
	//! \return true on successful wait, false on unfocus
	protected bool WaitFocused(float secondsToWait)
	{
		int waitTime = secondsToWait * 1000;
		int unfocusDuration;
		while (waitTime > 0)
		{
			Sleep(FPS_CHECK_FREQUENCY_MS);
			waitTime -= FPS_CHECK_FREQUENCY_MS;

			if (!float.AlmostEqual(System.GetFPS(), FPS_UNFOCUSED, FPS_UNFOCUSED_DELTA))
			{
				unfocusDuration = 0;
				continue;
			}

			unfocusDuration += FPS_CHECK_FREQUENCY_MS;
			if (unfocusDuration >= FPS_UNFOCUSED_DURATION_MS)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaceCamera(notnull WorldEditorAPI worldEditorAPI, CameraBase camera, vector position, vector direction)
	{
		if (m_bInternalUseGameMode)
		{
			World world = GetGame().GetWorld();
			if (!world)
				return; // leaving game mode

			direction.VectorToAngles();

			// Pitch Yaw Roll to Yaw Pitch Roll
			float tmp = direction[0];
			direction[0] = direction[1];
			direction[1] = tmp;

			camera.SetOrigin(position + m_vPositionOffset);
			camera.SetAngles(direction);
		}
		else
		{
			worldEditorAPI.SetCamera(position + m_vPositionOffset, direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] imagePath
	//! \param[in] definition
	//! \param[in] fpsArray
	//! \return true on success, false otherwise
	protected bool CreateImage(string imagePath, int definition, notnull array<float> fpsArray)
	{
		int definitionSq = definition * definition;
		array<int> imageData = {}; // temporarily used to store entity count first
		imageData.Resize(definitionSq);

		int scenePerPosition = fpsArray.Count() / definitionSq;
		if (scenePerPosition < 1)
		{
			PrintFormat("[SCR_FPSDiagnosticPlugin.CreateImage] scenePerPosition %1/%2 = 0!", fpsArray.Count(), definitionSq, level: LogLevel.ERROR);
			return false;
		}

		Debug.BeginTimeMeasure();

		int fpsArrayCount = fpsArray.Count();

		for (int i; i < definitionSq; ++i)
		{
			float avgOrientationsFPS;
			int measuresCount;
			for (int fpsI; fpsI < scenePerPosition; ++fpsI)
			{
				float fps = fpsArray[i * scenePerPosition + fpsI];
				if (fps < 0)
					break;

				avgOrientationsFPS += fps;

				++measuresCount;
			}

			if (measuresCount != scenePerPosition)
				continue;

			if (avgOrientationsFPS >= 0 && measuresCount > 0)
				avgOrientationsFPS /= measuresCount;

			imageData[i] = Math.Round(avgOrientationsFPS);
		}

		Debug.EndTimeMeasure(
			string.Format(
				"Processing %1 scene FPS for image creation (%2×%2 = %3 pixels)",
				definition * scenePerPosition,
				definition,
				definitionSq));

		if (m_bUseFakeData && definition < 16)
		{
			int maxValue = imageData[0];
			for (int i; i < fpsArrayCount; i += definition)
			{
				string toPrint;
				for (int j; j < definition; ++j)
				{
					int datum = imageData[i + j];
					if (datum > maxValue)
						maxValue = datum;
	
					if (j == 0)
						toPrint += datum.ToString(2);
					else
						toPrint += "," + datum.ToString(2);
	
					
				}
	
				Print("" + toPrint);
			}
	
			Print("Image made of " + fpsArrayCount + " pixels");
			Print("Min = 0, Max = " + maxValue);
		}

		return SCR_HeatmapHelper.CreateHeatmapImageFromData(
			imagePath,
			imageData,
			m_iHeatmapColourMode,
			m_bHeatmapValueInversion,
			m_iHeatmapMaxValueMode,
			m_bHeatmapHighlightValuesAboveMax,
			m_iHeatmapResolutionFactor);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Diagnose", true)]
	protected int ButtonDiagnose()
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
#endif // WORKBENCH
