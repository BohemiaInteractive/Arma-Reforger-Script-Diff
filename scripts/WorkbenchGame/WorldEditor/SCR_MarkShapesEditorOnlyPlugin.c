#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Mark shapes editor only", wbModules: { "WorldEditor"}, awesomeFontCode: 0xF023)]
class SCR_MarkShapesEditorOnlyPlugin : WorkbenchPlugin
{
	[Attribute("false", UIWidgets.CheckBox)]
	protected bool ActiveLayerOnly;

	protected ref array<string> m_aEditorOnlyGenerators = {
		"ForestGeneratorEntity",
		"PrefabGeneratorEntity",
		"RoadGeneratorEntity",
		"SCR_PowerlineGeneratorEntity",
		"WallGeneratorEntity"
	};

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected bool OkButton()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
			return;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return;

		if (!Workbench.ScriptDialog("Configure editor only marking", "", this))
			return;

		if (!worldEditorAPI.BeginEntityAction())
			return;

		Print("--- Start marking shapes editor only", LogLevel.NORMAL);

		// Progress dialog
		int approximateCount = worldEditorAPI.GetEditorEntityCount();
		WBProgressDialog progress = new WBProgressDialog("Marking", worldEditor);

		worldEditorAPI.ToggleGeneratorEvents(false);

		EditorEntityIterator iter = new EditorEntityIterator(worldEditorAPI);
		IEntitySource src = iter.GetNext();
		float prevProgress, currProgress;
		while (src)
		{
			ProcessEntity(worldEditorAPI, src);
			src = iter.GetNext();

			currProgress = iter.GetCurrentIdx() / approximateCount;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}

		worldEditorAPI.ToggleGeneratorEvents(true);
		worldEditorAPI.EndEntityAction();

		Print("--- Done marking shapes editor only", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEditorOnly(WorldEditorAPI worldEditorAPI, IEntitySource src)
	{
		int flags;
		src.Get("Flags", flags);
		if ((flags & EntityFlags.EDITOR_ONLY) == 0)
		{
			flags |= EntityFlags.EDITOR_ONLY;
			worldEditorAPI.SetVariableValue(src, null, "Flags", flags.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessEntity(WorldEditorAPI worldEditorAPI, IEntitySource src)
	{
		if (ActiveLayerOnly)
		{
			int activeLayer = worldEditorAPI.GetCurrentEntityLayerId();
			if (src.GetLayerID() != activeLayer)
				return;
		}

		int childCount = src.GetNumChildren();
		string classname = src.GetClassName();

		if (classname == "PolylineShapeEntity" || classname == "SplineShapeEntity")
		{
			// All shape entites are considered Editor Only by default
			bool shapeEdOnly = true;

			// All children must "support" editor only for the shape to be eligible
			IEntitySource childSrc;
			for (int i = 0; i < childCount; ++i)
			{
				// Check if given child falls into the "eligable" category
				bool isEdOnlyGenerator = false;
				childSrc = src.GetChild(i);
				foreach (string editorOnlyGenerator : m_aEditorOnlyGenerators)
				{
					if (childSrc.GetClassName() == editorOnlyGenerator)
					{
						isEdOnlyGenerator = true;
						// Even if the whole shape may not be editor only, the generator itself can
						SetEditorOnly(worldEditorAPI, childSrc);
						break;
					}
				}

				// We have some entity which may need the shape in game, let's be conservative and don't mark it ed only
				if (!isEdOnlyGenerator)
				{
					shapeEdOnly = false;
					break;
				}
			}

			if (shapeEdOnly)
				SetEditorOnly(worldEditorAPI, src);
		}
		else
		{
			for (int i = 0; i < childCount; ++i)
			{
				ProcessEntity(worldEditorAPI, src.GetChild(i));
			}
		}
	}
}
#endif // WORKBENCH
