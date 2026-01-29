#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	"World Window DataDrop Plugin",
	"Allows to handle dropped data into a world edit window and completely reimplement default editor functionality",
	wbModules: { "WorldEditor" })]
class SCR_WorldWindowDataDropPlugin : WorldEditorPlugin
{
	protected static const string DATA_TYPE_ENTITY_CLASSNAME = "WorldEditor/EntityType";	// entity class name type
	protected static const string DATA_TYPE_RESOURCE_FILES = "Workbench/ResourceFiles";		// registered resource files type

	//------------------------------------------------------------------------------------------------
	override bool OnWorldEditWindowDataDropped(int windowType, int posX, int posY, string dataType, array<string> data)
	{
		if (dataType == DATA_TYPE_ENTITY_CLASSNAME)
		{
			string className = data[0];

			//check whether the particular dropped class is interesting for us
			if (className == "SomeNotExistingClassAsExample")
			{
				//if so then we can spawn it from here with any additional customizations
				SpawnCustomizedEntityAsExample(windowType, posX, posY, className);
				return true;	//event is processed. Editor's default implementation won't be done
			}
		}
		else if (dataType == DATA_TYPE_RESOURCE_FILES)
		{
			//iterate over dropped resource files
			for (int n = 0; n < data.Count(); n++)
			{
				string resName = data[n];

				string ext;
				FilePath.StripExtension(resName, ext);
				ext.ToLower();

				//check whether the particular dropped resoure file is interesting for us
				if (ext == "ptc")
				{
					//if so then we can spawn anything custom that uses the dropped data
					SpawnParticleEffect(windowType, posX, posY, resName);
					return true;	//means event is processed. Editor's default implementation won't be done
				}
			}
		}

		return false;	//event not processed. Let work editor's default implementation
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnParticleEffect(int windowType, int posX, int posY, ResourceName resName)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (worldEditorAPI.BeginEntityAction("Creating entity"))
		{
			worldEditorAPI.ClearEntitySelection();

			//use the same creating funtion as editor does because it solves correct placing etc.
			IEntitySource ent = worldEditorAPI.CreateEntityInWindowEx(windowType, posX, posY, "ParticleEffectEntity", "", worldEditorAPI.GetCurrentEntityLayerId());
			if (ent)
			{
				worldEditorAPI.SetVariableValue(ent, null, "EffectPath", resName);
				worldEditorAPI.AddToEntitySelection(ent);
			}

			worldEditorAPI.EndEntityAction();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnCustomizedEntityAsExample(int windowType, int posX, int posY, string className)
	{
		//this is an example how entity must be created.
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (worldEditorAPI.BeginEntityAction("Creating entity"))
		{
			worldEditorAPI.ClearEntitySelection();

			//use the same creating funtion as editor does because it solves correct placing etc.
			IEntitySource ent = worldEditorAPI.CreateEntityInWindowEx(windowType, posX, posY, className, "", worldEditorAPI.GetCurrentEntityLayerId());
			if (ent)
				worldEditorAPI.AddToEntitySelection(ent);

			worldEditorAPI.EndEntityAction();
		}
	}
}
#endif // WORKBENCH
