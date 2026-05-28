[WorkbenchPluginAttribute(
	name: PLUGIN_NAME,
	description: PLUGIN_DESCRIPTION,
	wbModules: { "WorldEditor" },
	category: SCR_PluginCategory.WORLDEDITOR_IMPORT_EXPORT,
	awesomeFontCode: 0xF0D0)]
class SCR_ObjectImportPlugin : WorldEditorPlugin
{
	[Attribute(desc: "CSV file containing objects information.\nExpected line format: Prefab, x, y, z, quatX, quatY, quatZ, quatW, scale\nBoth space and comma are accepted value separators", params: "csv")]
	ResourceName m_sDataPath;

	[Attribute("1", UIWidgets.CheckBox, "Makes Y coordinate relative to terrain vs absolute in world")]
	bool m_bRelativeY;

	protected static const int TOKENS_EXPECTED_COUNT = 9;
	protected static const int RESOURCEHASH_INDEX = 0;
	protected static const int POS_INDEX = 1;	// 1, 2, 3
	protected static const int QUAT_INDEX = 4;	// 4, 5, 6, 7
	protected static const int SCALE_INDEX = 8;	// final index... for now!

	protected static const string PLUGIN_NAME = "CSV Object Import";
	protected static const string PLUGIN_DESCRIPTION = "Import entities from CSV file data";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (Workbench.ScriptDialog(PLUGIN_NAME, PLUGIN_DESCRIPTION, this) == 0)
			return;

		if (!m_sDataPath) // .IsEmpty()
		{
			SCR_WorkbenchHelper.PrintDialog("No Data Path was provided - please provide a (valid) CSV file.", PLUGIN_NAME, LogLevel.WARNING);
			Run();
			return;
		}

		ImportData();
	}

	//------------------------------------------------------------------------------------------------
	protected void ImportData()
	{
		Print("Importing entities", LogLevel.NORMAL);

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.ERROR);
			return;
		}

		string dataPath = m_sDataPath.GetPath();
		ParseHandle parser = FileIO.BeginParse(dataPath);
		if (!parser)
		{
			SCR_WorkbenchHelper.PrintDialog("Cannot create parser from file: " + dataPath, PLUGIN_NAME, LogLevel.ERROR);
			return;
		}

		array<string> lines;
		int linesCount;

		array<string> tokens = {};
		array<string> tokensComma = {};
		if (!worldEditorAPI.BeginEntityAction())
		{
			Print("Cannot begin action", LogLevel.ERROR);
			return;
		}

		const int layerID = worldEditorAPI.GetCurrentEntityLayerId();

		int i, totalCreated;

		for (; true; ++i)
		{
			int numTokens = parser.ParseLine(i, tokens);
			if (numTokens < 1)
				break;

			if (numTokens < TOKENS_EXPECTED_COUNT) // let's try and use commas - it is CSV after all
			{
				if (!lines)
				{
					lines = SCR_FileIOHelper.ReadFileContent(dataPath, false);
					if (!lines)
					{
						worldEditorAPI.EndEntityAction();
						parser.EndParse();

						SCR_WorkbenchHelper.PrintDialog("Cannot open/read " + dataPath, PLUGIN_NAME, LogLevel.ERROR);
						return;
					}

					linesCount = lines.Count();
				}

				if (i >= linesCount)
					break;

				lines[i].Split(",", tokensComma, false); // read CSV line

				if (numTokens < TOKENS_EXPECTED_COUNT) // additional fields are ignored, allowing comments
				{
					if (tokens.Count() < tokensComma.Count()) // let's take the biggest split
						tokens = tokensComma;

					PrintFormat("Line %1: Invalid data format - expected at least %3 tokens, got %2:", i + 1, tokens.Count(), TOKENS_EXPECTED_COUNT, level: LogLevel.WARNING);
					foreach (int tokenIndex, string token : tokens)
					{
						PrintFormat("#%1: %2", tokenIndex + 1, token, level: LogLevel.WARNING);
					}
	
					continue;
				}

				tokens = tokensComma;
			}

			string resourceHash = tokens[RESOURCEHASH_INDEX];
			ResourceName resourceName = resourceHash;
			if (!resourceName.GetPath()) // .IsEmpty()
			{
				PrintFormat("Line %1: Hash %2 does not correspond to any resource", i + 1, resourceHash, level: LogLevel.ERROR);
				continue;
			}

			float scale = tokens[SCALE_INDEX].ToFloat();
			if (scale <= 0)
			{
				PrintFormat("Line %1: Wrong scale provided (parsed \"%2\" as %3)", i + 1, tokens[SCALE_INDEX], scale, level: LogLevel.WARNING);
				continue;
			}

			vector pos;
			for (int j; j < 3; ++j)
			{
				pos[j] = tokens[POS_INDEX + j].ToFloat();
			}

			pos[2] = -pos[2]; // flip z

			if (m_bRelativeY)
				pos[1] = pos[1] + worldEditorAPI.GetTerrainSurfaceY(pos[0], pos[2]);

			float quat[4];
			for (int j; j < 4; ++j)
			{
				quat[j] = tokens[QUAT_INDEX + j].ToFloat();
			}

			quat[2] = -quat[2]; // flip z
			quat[3] = -quat[3]; // flip rotation because of handiness
			Math3D.QuatNormalize(quat);

			vector angles = Math3D.QuatToAngles(quat);
			// Convert angles from (yaw, pitch, roll) to (xRotate, yRotate, zRotate) as expected by `CreateEntity`
			float tmp = angles[0];
			angles[0] = angles[1];
			angles[1] = tmp;

			// all data collected, let's proceed

			// PrintFormat("Creating %1 on pos %2 angles %3 scale %4", resourceName.GetPath(), pos, angles, scale, level: LogLevel.VERBOSE);
			IEntitySource entitySource = worldEditorAPI.CreateEntity(resourceHash, "", layerID, null, pos, angles);
			if (!entitySource)
			{
				PrintFormat("Line %1: Entity cannot be created (%2)", i + 1, resourceHash, level: LogLevel.ERROR);
				continue;
			}

			if (!worldEditorAPI.SetVariableValue(entitySource, null, "scale", scale.ToString()))
				PrintFormat("Line %1: Cannot set scale on created entity (scale %2)", i + 1, scale, level: LogLevel.WARNING);

			++totalCreated;
		}

		worldEditorAPI.EndEntityAction();

		parser.EndParse();

		SCR_WorkbenchHelper.PrintDialog(string.Format("CSV import done: %1 creations over %2 lines", totalCreated, i + 1), PLUGIN_NAME, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Import", true)]
	protected int ButtonImport()
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
