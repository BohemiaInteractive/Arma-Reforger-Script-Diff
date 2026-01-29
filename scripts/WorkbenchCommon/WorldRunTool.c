//------------------------------------------------------------------------------------------------
enum WorldRunConfiguration
{
	PC = 0,
	XB1 = 1,
	PS4 = 2
}


[WorkbenchPluginAttribute("Run game with current world", "Runs currently opened world with specified parameters and game profile", "", "", {"WorldEditor"},"",0xf144)]
class WorldRunPlugin: WorkbenchPlugin
{
	[Attribute("game.exe", UIWidgets.EditBox)]
	string Executable;
	[Attribute("0", UIWidgets.ComboBox, "Configuration", "", enumType: WorldRunConfiguration)]
	int Configuration;
	
	[Attribute("-1", UIWidgets.EditBox, "Horizontal coordinate on the screen if running in window mode. Default if -1.")]
	int	X;

	[Attribute("-1", UIWidgets.EditBox, "Vertical coordinate on the screen if running in window mode. Default if -1.")]
	int	Y;

	[Attribute("-1", UIWidgets.EditBox, "Width of the app window if running in window mode. Default if -1.")]
	int	Width;

	[Attribute("-1", UIWidgets.EditBox, "Height of the app window if running in window mode. Default if -1.")]
	int	Height;

	[Attribute("true", UIWidgets.CheckBox, "If true the app will run in window mode.")]
	bool Windowed;

	[Attribute("true", UIWidgets.CheckBox, "If true the app will run at full speed even when its window doesn't have focus.")]
	bool ForceUpdate;

	[Attribute("true", UIWidgets.CheckBox, "If true the app won't steal focus from the current active window.")]
	bool NoFocus;

	[Attribute("true", UIWidgets.CheckBox, "If true, no network-related timeout will result in disconnect. Necessary for debugging. Don't use for anything else.")]
	bool RplDisableTimeout;

	[Attribute("-1", UIWidgets.EditBox, "Maximum allowed FPS. No limit if -1.")]
	int MaxFps;

	[Attribute("WorldRunPlugin", UIWidgets.EditBox, "Profile name used for the app.")]
	string Profile;
	
	[Attribute("", UIWidgets.EditBox)]
	string Params;
	
	override void Run()
	{
		string command = Executable + " ";
		
		if (Configuration == WorldRunConfiguration.XB1)
			command += "-configuration xbox ";
		else if (Configuration == WorldRunConfiguration.PS4)
			command += "-configuration ps4 ";
		
		WorldEditor mod = Workbench.GetModule(WorldEditor);
		if (!mod)
		{
			Print("World editor module is not available.", LogLevel.ERROR);
			return;
		}
		
		WorldEditorAPI api = mod.GetApi();

		string gproj = Workbench.GetCurrentGameProjectFile();

		if (gproj && command.IndexOf("-gproj") == -1)
			command += "-gproj " + gproj + " ";
		
		string world;
		api.GetWorldPath(world); // It would be better to have API to get relative path to the world
		if (world)
		{
			int worldIndex = world.LastIndexOf("worlds/");
			if (worldIndex != -1)
			{
				world = world.Substring(worldIndex, world.Length() - worldIndex);
				command += "-world " + world + " ";
			}
		}
		else
		{
			Print("World needs to be opened in the World Editor.", LogLevel.ERROR);
			return;
		}

		if(Windowed)
		{
			command += "-window ";
		}

		if(X != -1)
		{
			command += "-posX " + X + " ";
		}

		if(Y != -1)
		{
			command += "-posY " + Y + " ";
		}

		if(Width != -1)
		{
			command += "-screenWidth " + Width + " ";
		}

		if(Height != -1)
		{
			command += "-screenHeight " + Height + " ";
		}

		if(MaxFps > 0)
		{
			command += "-maxFPS " + MaxFps + " ";
		}

		if(ForceUpdate)
		{
			command += "-forceupdate ";
		}

		if(NoFocus)
		{
			command += "-nofocus ";
		}

		if(RplDisableTimeout)
		{
			command += "-rpl-timeout-disable ";
		}
		
		if (Profile)
		{
			command += "-profile " + Profile + " ";
		}
		
		if (Params)
		{
			command += Params + " ";
		}

		
		Print("Running command: " + command);
		Workbench.RunCmd(command);		
	}
	
	override void Configure()
	{
		Workbench.ScriptDialog("Configure World run tool", "You can configure execution name, profile and parameters", this);
	}
	
	[ButtonAttribute("OK")]
	void OkButton()	{}
}

