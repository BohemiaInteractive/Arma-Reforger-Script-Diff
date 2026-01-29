[WorkbenchPluginAttribute("CSV Object Import", "Import CSV data and create entities", wbModules: { "WorldEditor" }, awesomeFontCode: 0xf0d0)]
class SCR_ObjectImportPlugin : WorldEditorPlugin
{
	[Attribute("", UIWidgets.ResourceNamePicker, "csv where space is the separator", "csv")]
	ResourceName m_sDataPath;

	[Attribute("true", UIWidgets.CheckBox, "Makes Y coordinate relative to terrain vs absolute in world")]
	bool m_bRelativeY;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (Workbench.ScriptDialog("CSV Object Import", "Import objects from a CSV file\n", this) == 0)
			return;

		if (!m_sDataPath) // .IsEmpty()
		{
			Workbench.Dialog("", "No Data Path was provided - please provide a (valid) CSV file.");
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

		ParseHandle parser = FileIO.BeginParse(m_sDataPath.GetPath());
		if (!parser)
		{
			Print("Cannot create parser from file: " + m_sDataPath.GetPath(), LogLevel.ERROR);
			return;
		}

		const int wantedTokensPerLine = 9;
		const int quatStart = 4;
		const int posStart = 1;
		const int tagStart = 0;
		const int scaleStart = 8;
		array<string> tokens = {};
		if (!worldEditorAPI.BeginEntityAction())
		{
			Print("Cannot begin action", LogLevel.ERROR);
			return;
		}

		const int layerID = worldEditorAPI.GetCurrentEntityLayerId();

		for (int i; true; ++i)
		{
			int numTokens = parser.ParseLine(i, tokens);
			if (numTokens < 1)
				break;

			if (numTokens != wantedTokensPerLine)
			{
				PrintFormat("Line %1: Invalid data format - expected %3 tokens, got %2", i, numTokens, wantedTokensPerLine, level: LogLevel.ERROR);
				for (int j; j < numTokens; ++j)
				{
					PrintFormat("#%1: %2", j, tokens[j], level: LogLevel.ERROR);
				}

				continue;
			}

			float quat[4];
			for (int j = quatStart; j < quatStart + 4; ++j)
			{
				quat[j - quatStart] = tokens[j].ToFloat();
			}

			quat[2] = -quat[2]; // flip z
			quat[3] = -quat[3]; // flip rotation because of handiness
			Math3D.QuatNormalize(quat);

			vector pos;
			for (int j = posStart; j < posStart + 3; ++j)
			{
				pos[j - posStart] = tokens[j].ToFloat();
			}

			pos[2] = -pos[2]; // flip z

			if (m_bRelativeY)
				pos[1] = pos[1] + worldEditorAPI.GetTerrainSurfaceY(pos[0], pos[2]);

			string resourceHash = tokens[tagStart];
			float scale = tokens[scaleStart].ToFloat();

			vector angles = Math3D.QuatToAngles(quat);
			// Convert angles from (yaw, pitch, roll) to (xRotate, yRotate, zRotate) as expected by `CreateEntity`
			float tmp = angles[0];
			angles[0] = angles[1];
			angles[1] = tmp;

			ResourceName entityResource = resourceHash;
			if (!entityResource.GetPath()) // .IsEmpty()
			{
				PrintFormat("Line %1: Hash %2 does not correspond to any resource", i, resourceHash, level: LogLevel.ERROR);
				continue;
			}

			//Print("Creating " + entityResource.GetPath() + " on: " + pos + " angles: " + angles + " scale: " + scale, LogLevel.VERBOSE);
			IEntitySource entitySource = worldEditorAPI.CreateEntity(resourceHash, "", layerID, null, pos, angles);
			if (!entitySource)
			{
				PrintFormat("Line %1: Entity cannot be created (%2)", i, resourceHash, level: LogLevel.ERROR);
				continue;
			}

			worldEditorAPI.SetVariableValue(entitySource, null, "scale", scale.ToString());
		}

		worldEditorAPI.EndEntityAction();

		parser.EndParse();
		Print("Import done", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Import", true)]
	protected int BtnImport()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int BtnCancel()
	{
		return 0;
	}
}
