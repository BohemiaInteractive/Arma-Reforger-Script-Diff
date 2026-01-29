[WorkbenchPluginAttribute("Build shore map", "")]
class BuildShoreMap: WorkbenchPlugin
{	
	override void RunCommandline()
	{				
		string path;
		WorldEditor we = Workbench.GetModule(WorldEditor);
		bool exitAfter;
		if (we.GetApi())
		{
			// plugged into WorldEditor module
			we.GetCmdLine("-world", path);
			string dummy;
			exitAfter = we.GetCmdLine("-exitAfter", dummy);
		}
		else
		{
			// plugged into RM module
			ResourceManager rm = Workbench.GetModule(ResourceManager);
			rm.GetCmdLine("-world", path);
			Workbench.OpenModule(WorldEditor);
			exitAfter = true;
		}
		
		bool hasLoadedWorld = false;
		if (path.IsEmpty())
		{
			if (we.GetApi())
			{
				we.GetApi().GetWorldPath(path);
				hasLoadedWorld = !path.IsEmpty();
			}
		}
		else
		{
			hasLoadedWorld = we.SetOpenedResource(path);
		}
		
		if (!hasLoadedWorld)
		{
			Print("BuildShoreMap Plugin: Unable to load a world or no world file specified. Terminating.", LogLevel.ERROR);
			Workbench.Exit(-1);
		}
		else if (exitAfter)
		{
			we.GetApi().BuildShoreMap();
			Print("BuildShoreMap Plugin: Done. Exiting Workbench.");
			Workbench.Exit(0);
		}
		else
		{
			we.GetApi().BuildShoreMap();
			Print("BuildShoreMap Plugin: Done.");
		}
	}
}

[WorkbenchPluginAttribute("Build shore map", "Generate and save/overwrite shore map", "", "", {"WorldEditor"},"",0xf7a2)]
class BuildShoreMapWEPlugin: WorkbenchPlugin
{
	// WE entry points
	override void Run()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = we.GetApi();
		api.BuildShoreMap();
	}
}
