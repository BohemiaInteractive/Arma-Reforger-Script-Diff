#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Dedicated Server Tool", shortcut: "Ctrl+Shift+D", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF233)]
class SCR_DedicatedServerPlugin : WorldEditorPlugin
{
	/*
		Category: Server
	*/

	[Attribute(desc: "Server configuration. Dedicated server cannot be launched without this!", category: "Server")]
	protected ref DedicatedServerPluginCLI m_Config;

	[Attribute("ArmaReforgerServer.exe", UIWidgets.FileNamePicker, desc: "Executable file used to start the server.", params: "exe FileNameFormat=absolute", category: "Server")]
	protected string m_sExecutable;

	[Attribute("", desc: "Profile used on the server.", category: "Server")]
	protected string m_sProfile;

	[Attribute("0", desc: "Suppresses all sorts of errors (like asserts), so the game continues without stalling and disconnecting clients.", category: "Server")]
	protected bool m_bNoThrow;

	[Attribute("", uiwidget: UIWidgets.ComboBox, desc: "Enable Backend logs\n- Basic: Turn on basic logs\n- Http: Turn on logging of Http Response & Requests\n- File: Turn on saving of related (mostly JSON) files send and/or recieved from backend into logs folder and subfolder .backend\n- Trace: Turn on logging of inner Http comm - valid only for libcurl (windows ATM)", category: "Server", enums: { ParamEnum("None", "0"), ParamEnum("Basic", "1"), ParamEnum("Http", "2"), ParamEnum("File", "3"), ParamEnum("Trace", "4")})]
	protected int m_iLogBackend;

	[Attribute("", uiwidget: UIWidgets.ComboBox, desc: "llows manual rerouting to one of our Backend Environments (only in internal version, retail is baked to production system).\n- Dev: Internal development environment\n- Preprod: Preproduction\n- Submission: Submission (preproduction RC candidate environment for testing by QA)\n- Production: Production, this mean players\n- Local: For working without deployed version on your machine", category: "Server", enums: { ParamEnum("None", "0"), ParamEnum("Dev", "1"), ParamEnum("Preprod", "2"), ParamEnum("Submission", "3"), ParamEnum("Production", "4"), ParamEnum("Local", "5")})]
	protected int m_iBackendEnv;

	[Attribute("30", desc: "Maximum FPS on the server. When 0, no limit will be set.", category: "Server")]
	protected int m_iMaxFPS;

	[Attribute(LogLevel.NORMAL.ToString(), desc: "Set the highest shown log level.\nAll others below in the list will be included as well\n(e.g., VERBOSE also enables DEBUG, NORMAL, WARNING, etc, but not SPAM).", uiwidget: UIWidgets.ComboBox, category: "Server", enumType: LogLevel)]
	protected int m_iLogLevel;

	[Attribute(desc: "Additional command line params.", category: "Server")]
	protected string m_sParams;

	/*
		Category: Peers
	*/

	[Attribute(desc: "Run peers defined by the PeerTool.", category: "Peers")]
	protected bool m_bRunPeers;

	[Attribute("ArmaReforger.exe", UIWidgets.FileNamePicker, desc: "Peer exe path", params: "exe FileNameFormat=absolute", category: "Peers")]
	protected string m_sPeerExecutable;

	[Attribute(desc: "Peers configuration.", category: "Peers")]
	protected ref array<ref PeerConfig> m_aPeersConfig;

	protected ProcessHandle m_ServerHandle;

	//------------------------------------------------------------------------------------------------
	protected string GetAddonsDirCLI()
	{
		string addonsDir;
		System.GetCLIParam("addonsDir", addonsDir);
		return addonsDir;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetAddonsCLI()
	{
		string addonIDs;

		array<string> addonsGUIDs = {};
		GameProject.GetLoadedAddons(addonsGUIDs);

		foreach (string GUID : addonsGUIDs)
		{
			if (GameProject.IsVanillaAddon(GUID))
				continue;

			if (!addonIDs.IsEmpty())
				addonIDs += ",";

			addonIDs += GameProject.GetAddonID(GUID);
		}

		return addonIDs;
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Start Dedicated Server", "Configure dedicated server before running it.                                             ", this))
			return;

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();

		//--- Generate run command
		string gproj = Workbench.GetCurrentGameProjectFile();
		string process = string.Format("%1 -gproj=\"%2\"", m_sExecutable, gproj);

		string configParam;
		if (m_Config && m_Config.GetCLI(configParam, api))
		{
			process += " " + configParam;
		}
		else
		{
			Print("Cannot start dedicated server, 'Config' setting is incomplete!", LogLevel.WARNING);
			return;
		}

		string addonsDir = GetAddonsDirCLI();
		if (!addonsDir.IsEmpty())
			process += " -addonsDir " + addonsDir;

		string addonIDs = GetAddonsCLI();
		if (!addonIDs.IsEmpty())
			process += " -addons " + addonIDs;

		if (m_sProfile)
			process += string.Format(" -profile %1", m_sProfile);

		if (m_bNoThrow)
			process += " -nothrow";

		switch (m_iLogBackend)
		{
			case 1: process += " -backendLog"; break;
			case 2: process += " -backendLog Http"; break;
			case 3: process += " -backendLog File"; break;
			case 4: process += " -backendLog Trace"; break;
		}

		switch (m_iBackendEnv)
		{
			case 1: process += " -backendEnv Dev"; break;
			case 2: process += " -backendEnv Preprod"; break;
			case 3: process += " -backendEnv Submission"; break;
			case 4: process += " -backendEnv Production"; break;
			case 5: process += " -backendEnv Local"; break;
		}

		if (m_iMaxFPS > 0)
			process += string.Format(" -maxFPS %1", m_iMaxFPS);

		if (m_iLogLevel != LogLevel.NORMAL)
			process += string.Format(" -logLevel %1", typename.EnumToString(LogLevel, m_iLogLevel));

		if (m_sParams)
			process += " " + m_sParams;

		//--- Start dedicated server
		m_ServerHandle = Workbench.RunProcess(process);
		Print(string.Format("Starting dedicated server: %1", process), LogLevel.NORMAL);

		//--- Connect to the server
		if (!m_bRunPeers)
			return;

		int profileIndex;

		foreach (PeerConfig conf : m_aPeersConfig)
		{
			if (!conf.Enabled)
				continue;

			profileIndex++; // start indexing peers from 1
			string sCmd = m_sPeerExecutable + " -gproj \"" + gproj + "\" ";

			if (!addonsDir.IsEmpty())
				sCmd += "-addonsDir " + addonsDir + " ";

			if (!addonIDs.IsEmpty())
				sCmd += "-addons " + addonIDs + " ";

			if (conf.Windowed)
				sCmd += "-window ";

			if (conf.X != -1)
				sCmd += "-posX " + conf.X + " ";

			if (conf.Y != -1)
				sCmd += "-posY " + conf.Y + " ";

			if (conf.Width != -1)
				sCmd += "-screenWidth " + conf.Width + " ";

			if (conf.Height != -1)
				sCmd += "-screenHeight " + conf.Height + " ";

			if (conf.MaxFPS > 0)
				sCmd += "-maxFPS " + conf.MaxFPS + " ";

			if (conf.ForceUpdate)
				sCmd += "-forceupdate ";

			if (conf.NoFocus)
				sCmd += "-nofocus ";

			if (conf.RplAutoJoin)
				sCmd += "-client ";

			if (conf.RplAutoReconnect)
				sCmd += "-rpl-reconnect ";

			if (conf.RplDisableTimeout)
				sCmd += "-rpl-timeout-disable ";

			if (conf.Profile)
				sCmd += "-profile " + conf.Profile + profileIndex.ToString() + " ";

			if (conf.Params)
				sCmd += conf.Params + " ";

			Print("Running command: " + sCmd, LogLevel.NORMAL);
			conf.Handle = Workbench.RunProcess(sCmd);
			if (!conf.Handle)
				Print("Peer #" + profileIndex + " couldn't run. Check if your PeerExecutable or other settings are correct", LogLevel.ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnded()
	{
		// Close any connected peers
		foreach (PeerConfig conf : m_aPeersConfig)
		{
			if (!conf.Handle) // what if the game mode wasn't the PeerTool?
				continue;

			Workbench.KillProcess(conf.Handle);
			conf.Handle = null;
		}

		// Close the server when returning from play mode
		if (m_ServerHandle)
			Workbench.KillProcess(m_ServerHandle);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	protected bool ButtonRun()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return false;
	}
}

// TODO: SCR_
[BaseContainerProps()]
class DedicatedServerPluginCLI
{
	//------------------------------------------------------------------------------------------------
	//! \param[out] outParam
	//! \param[in] api
	//! \return
	bool GetCLI(out string outParam, WorldEditorAPI api);
}

// TODO: SCR_
[BaseContainerProps()]
class DedicatedServerPluginCLI_Server : DedicatedServerPluginCLI
{
	[Attribute(desc: "World which the server will start.\nWhen empty, currently opened world will be used.", uiwidget: UIWidgets.ResourceNamePicker, params: "ent")]
	protected ResourceName m_World;

	[Attribute(desc: "Mission configuration, can enable settings like metabolism / vehicles, set loading screen image, etc.", uiwidget: UIWidgets.ResourceNamePicker, params: "conf class=SCR_MissionHeader")]
	protected ResourceName m_MissionHeader;

	[Attribute(desc: "Systems configuration to be run if not using the .gproj default one.", uiwidget: UIWidgets.ResourceNamePicker, params: "conf class=SystemSettings")]
	protected ResourceName m_SystemsConfig;

	[Attribute(desc: "Optional password for logging-in as administrator (in game, type '#login <password>' in chat)")]
	protected string m_sAdminPassword;

	//------------------------------------------------------------------------------------------------
	override bool GetCLI(out string outParam, WorldEditorAPI api)
	{
		ResourceName worldPath = m_World;
		if (!worldPath)
			api.GetWorldPath(worldPath);

		outParam = string.Format("-server \"%1\"", worldPath.GetPath());

		if (m_MissionHeader)
			outParam += string.Format(" -MissionHeader \"%1\"", m_MissionHeader.GetPath());

		if (m_SystemsConfig)
			outParam += string.Format(" -worldSystemsConfig \"%1\"", m_SystemsConfig.GetPath());

		if (m_sAdminPassword)
			outParam += string.Format(" -adminPassword %1", m_sAdminPassword);

		return !worldPath.IsEmpty();
	}
}

// TODO: SCR_
[BaseContainerProps()]
class DedicatedServerPluginCLI_Config : DedicatedServerPluginCLI
{
	[Attribute(desc: "JSON server configuration file. Does not work when the game is running with local (i.e., non-workshop) addons!", uiwidget: UIWidgets.ResourceNamePicker, params: "json")]
	protected ResourceName m_Config;

	//------------------------------------------------------------------------------------------------
	override bool GetCLI(out string outParam, WorldEditorAPI api)
	{
		//--- Convert to absolute path, relative ResourceName is in the wrong format
		string absPath;
		Workbench.GetAbsolutePath(m_Config.GetPath(), absPath);

		outParam = string.Format("-config \"%1\"", absPath);
		return !m_Config.IsEmpty();
	}
}
#endif // WORKBENCH
