[WorkbenchPluginAttribute("Peer Tool", "Runs peer clients that connect to the WB instance.", "", "", {"WorldEditor"})]
class PeerPlugin : WorldEditorPlugin
{
	[Attribute("game.exe", UIWidgets.FileNamePicker, desc: "Peer exe path", params: "exe FileNameFormat=absolute")]
	string Executable;

	[Attribute("Peers configuration")]
	ref array<ref PeerConfig> PeersWindows;

	ProcessHandle Handle = null;
	
	private string GetAddonsDirCLI()
	{
		string addonsDir;
		array<string> addonsGUIDs = {};
		GameProject.GetLoadedAddons(addonsGUIDs);
		
		foreach (string GUID: addonsGUIDs)
		{
			if (!GameProject.IsVanillaAddon(GUID))
			{
				string addonPath = "$" + GameProject.GetAddonID(GUID) + ":";
				string absPath;
				if (Workbench.GetAbsolutePath(addonPath, absPath))
				{
					if (!addonsDir.IsEmpty())
						addonsDir += ",";
					
					addonsDir += absPath;
				}
			}
		}
		return "\"" + addonsDir + "\"";
	}
	
	private string GetAddonsCLI()
	{
		string addonIDs;
		
		array<string> addonsGUIDs = {};
		GameProject.GetLoadedAddons(addonsGUIDs);
		
		foreach (string GUID: addonsGUIDs)
		{
			if (!GameProject.IsVanillaAddon(GUID))
			{
				if (!addonIDs.IsEmpty())
					addonIDs += ",";
				
				addonIDs += GameProject.GetAddonID(GUID);
			}
		}
		
		return addonIDs;
	}

	void Start()
	{
		int profileIndex = 0;
		
		string addonsDir = GetAddonsDirCLI();
		string addonIDs = GetAddonsCLI();
		
		foreach(PeerConfig conf : PeersWindows)
		{
			if (!conf.Enabled)
				continue;
			
			profileIndex++; // start indexing peers from 1
			string gproj = Workbench.GetCurrentGameProjectFile();
			string sCmd = Executable + " -gproj \"" + gproj + "\" ";
						
			if (!addonsDir.IsEmpty())
				sCmd += "-addonsDir " + addonsDir + " ";				
			if (!addonIDs.IsEmpty())
				sCmd += "-addons " + addonIDs + " ";
			if(conf.Windowed)
				sCmd += "-window ";
			if(conf.X != -1)
				sCmd += "-posX " + conf.X + " ";
			if(conf.Y != -1)
				sCmd += "-posY " + conf.Y + " ";
			if(conf.Width != -1)
				sCmd += "-screenWidth " + conf.Width + " ";
			if(conf.Height != -1)
				sCmd += "-screenHeight " + conf.Height + " ";
			if(conf.MaxFPS > 0)
				sCmd += "-maxFPS " + conf.MaxFPS + " ";
			if(conf.ForceUpdate)
				sCmd += "-forceupdate ";
			if(conf.NoFocus)
				sCmd += "-nofocus ";
			if(conf.RplAutoJoin)
				sCmd += "-client ";
			if(conf.RplAutoReconnect)
				sCmd += "-rpl-reconnect ";
			if(conf.RplDisableTimeout)
				sCmd += "-rpl-timeout-disable ";
			if(conf.Profile)
				sCmd += "-profile " + conf.Profile + profileIndex.ToString() + " ";
			if (conf.Params)
				sCmd += conf.Params + " ";

			Print("Running command: " + sCmd);
			conf.Handle = Workbench.RunProcess(sCmd);
			if (!conf.Handle)
				Print("Peer #" + profileIndex + " couldn't run. Check if your Executable or other settings are correct", LogLevel.ERROR);
		}
	}

	void End()
	{
		foreach(PeerConfig conf : PeersWindows)
		{
			if (!conf.Handle) // what if the game mode wasn't the PeerTool?
				continue;
			
			Workbench.KillProcess(conf.Handle);
			conf.Handle = null;
		}
	}

	override void OnGameModeStarted(string worldName,
									string gameMode,
									bool	 playFromCameraPos,
									vector cameraPosition,
									vector cameraAngles)
	{
		if(gameMode == "Server localhost + PeerTool") // would consider different (if possible) solution for this. if this is changed on the C++ side, it will stop working
		{
			Start();
		}
	}

	override void OnGameModeEnded()
	{
		End();
	}

	override void Configure()
	{
		Workbench.ScriptDialog("Configure Peer tool", "Settings:", this);
	}

	[ButtonAttribute("OK")]
	void OkButton()	{}
}